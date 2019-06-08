/* Segregated explicit free lists implementation for       */
/* memory allocation. Minimum block size for allocated     */
/* blocks is 32 bytes, with the payload and header rounded */
/* to the nearest multiple of 16 for heap alignment        */

/* Allocated blocks: consist of the header which           */
/* contains the sign and allocation bit set to 1, as well  */
/* as the payload for the memory request. Footers have     */
/* been removed to increase peak memory utilization.       */


/* Free blocks: consist of the header, the footer which is */
/* a replica of the header, and previous and next links    */
/* for the explicit segregated free lists implementation   */
/* An extra bit after the allocation bit is reserved to    */
/* determine if the left adjacent block is free or not     */
/* since we can no longer rely on footers of the left      */
/* to coalesce                                             */

/* Do not change the following! */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "mm.h"
#include "memlib.h"

#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#define memset mem_memset
#define memcpy mem_memcpy
#endif /* def DRIVER */

/* You can change anything from here onward */

/*
 * If DEBUG is defined, enable printing on dbg_printf and contracts.
 * Debugging macros, with names beginning "dbg_" are allowed.
 * You may not define any other macros having arguments.
 */
//#define DEBUG

#ifdef DEBUG
/* When debugging is enabled, these form aliases to useful functions */
#define dbg_printf(...) printf(__VA_ARGS__)
#define dbg_requires(...) assert(__VA_ARGS__)
#define dbg_assert(...) assert(__VA_ARGS__)
#define dbg_ensures(...) assert(__VA_ARGS__)
#else
/* When debugging is disnabled, no code gets generated for these */
#define dbg_printf(...)
#define dbg_requires(...)
#define dbg_assert(...)
#define dbg_ensures(...)
#endif

/* What is the correct alignment? */
#define ALIGNMENT 16

/*
 * If NEXT_FIT is defined, use next fit search; otherwise use first-fit search
 */
// #define NEXT_FIT // uncomment this line to use next-fit search

/* Basic constants */
typedef uint64_t word_t;
static const size_t wsize = sizeof(word_t);   // word, header, footer size (bytes)
static const size_t dsize = 2*wsize;          // double word size (bytes)
static const size_t min_block_size = 2*dsize; // Minimum block size
static const size_t chunksize = (1 << 12);    // requires (chunksize % 16 == 0)

typedef struct block block_t;
typedef struct link link_t;

struct link
{
    block_t *prev;
    block_t *next;
};

union free_union
{
    /* l contains pointers for explicit free list */
    link_t l;
    /* declare char array of size 0 to allow computation */
    /* of starting address using pointer notation */
    char payload[0];

};

struct block
{
    /* Header contains size + allocation flag */
    word_t header;

    /* stores the prev, next links */
    /* also payload array */
    union free_union u;
    /*
     * We can't declare the footer as part of the struct, since its starting
     * position is unknown
     */
};


/* Global variables */
static block_t *heap_listp = NULL;
static word_t *prologue = NULL;
static block_t *epilogue = NULL;
static block_t *seglists[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
/* end nodes are used to traverse the seglists from the back */
/* helps with memory utilization */
static block_t *end_nodes[6] = {NULL, NULL, NULL, NULL, NULL, NULL};

/* Function prototypes for internal helper routines */
static block_t *extend_heap(size_t size);

static bool fetch_prev_alloc(block_t *block);
static void update_block(block_t *block, bool det);

static void place(block_t *block, size_t asize);
static block_t *find_fit(size_t asize);
static block_t *coalesce(block_t *block);

static size_t max(size_t x, size_t y);
static size_t min(size_t x, size_t y);
static size_t round_up(size_t size, size_t n);
static word_t pack(size_t size, bool alloc);

static size_t extract_size(word_t header);
static size_t get_size(block_t *block);
static size_t get_payload_size(block_t *block);

static bool extract_alloc(word_t header);
static bool get_alloc(block_t *block);

static void write_header(block_t *block, size_t size, bool alloc);
static void write_footer(block_t *block, size_t size, bool alloc);

static block_t *payload_to_header(void *bp);
static void *header_to_payload(block_t *block);

static block_t *find_next(block_t *block);
static word_t *find_prev_footer(block_t *block);
static block_t *find_prev(block_t *block);
/* newly implemented functions */
/* for explicit free lists */

static unsigned fetch_class(size_t asize);
static block_t *find_free(size_t asize);
static block_t *find_prev_free(block_t *block);
static block_t *find_next_free(block_t *block);

static void add_free_block(block_t *block);
static void remove_free_block(block_t *block);
static void destroy_free_block(block_t *block, unsigned i);

static size_t align(size_t x);
static bool in_heap(const void *p);
static bool aligned(const void *p);
static bool heap_check();
static bool head_eq_foot();
static bool coalesce_check();
static bool in_seglist();

static bool in_list(block_t *block, unsigned i);
static bool circularity();
static bool consistent_pointers();
static bool free_count();
bool mm_checkheap(int lineno);

/* rounds up to the nearest multiple of ALIGNMENT */
static size_t align(size_t x)
{
    return ALIGNMENT * ((x+ALIGNMENT-1)/ALIGNMENT);
}

static bool in_heap(const void *p)
{
    return p <= mem_heap_hi() && p >= mem_heap_lo();
}

static bool aligned(const void *p)
{
    size_t ip = (size_t)p;
    return align(ip) == ip;
}

/*
 * mm_init: initializes the heap; it is run once when heap_start == NULL.
 *          prior to any extend_heap operation, this is the heap:
 *              start            start+8           start+16
 *          INIT: | PROLOGUE_FOOTER | EPILOGUE_HEADER |
 * heap_listp ends up pointing to the epilogue header.
 */
bool mm_init(void)
{
    // Create the initial empty heap
    word_t *start = (word_t *)(mem_sbrk(2*wsize));
    block_t *res;

    if (start == (void *)-1)
    {
        return false;
    }

    start[0] = pack(0, true); // Prologue footer
    start[1] = pack(0, true); // Epilogue header
    // Heap starts with first block header (epilogue)
    prologue = &(start[0]);
    heap_listp = (block_t *) &(start[1]);

    // Extend the empty the heap with a free block of chunksize bytes

    res = extend_heap(chunksize);
    if (res == NULL)
    {
        return false; // memory failure
    }

    return true;
}

/*
 * malloc: allocates a block with size at least 32 bytes, rounds up to
 *         the nearest 16 bytes for all payloads, with a minimum of 2*dsize.
 *         Seeks a sufficiently-large unallocated block on the heap to be
 *         allocated. If no such block is found, extends heap by the maximum
 *         between chunksize and (size + wsize) rounded up to the nearest 16 bytes,
 *         and then attempts to allocate all, or a part of, that memory.
 *         Returns NULL on failure, otherwise returns a pointer to such block.
 *         The allocated block will not be used for further allocations until
 *         freed. All allocated blocks will lack footers in our optimization.
 */
void *malloc(size_t size)
{
    size_t asize;      // Adjusted block size
    size_t extendsize; // Amount to extend heap if no fit is found
    block_t *block;
    void *bp = NULL;

    if (heap_listp == NULL) // Initialize heap if it isn't initialized
    {
        mm_init();
    }

    if (size == 0) // Ignore spurious request
    {
	dbg_printf("Malloc(%zd) --> %p\n", size, bp);
        return bp;
    }

    // Adjust block size to include overhead and to meet alignment requirements
    /* For footer optimization in segregated free lists, remove footer for
    allocated blocks */
    if (size <= 8)
    {
        asize = 32; // minimum block size
    }
    else
    {
        // round up payload and header to nearest multiple of 16
        asize = round_up(size+wsize, dsize);
    }

    block = find_free(asize);

    // If no fit is found, request more memory, and then place the block
    if (block == NULL)
    {
        extendsize = max(asize, chunksize);
        block = extend_heap(extendsize);

        if (block == NULL) // extend_heap returns an error
        {
	    dbg_printf("Malloc(%zd) --> %p\n", size, bp);
            return bp;
        }

    }

    remove_free_block(block); // remove block to be allocated from seglist
    place(block, asize);
    bp = header_to_payload(block);
    dbg_printf("Malloc(%zd) --> %p\n", size, bp);
    return bp;
}

/*
 * free: Frees the block such that it is no longer allocated while still
 *       maintaining its size. Block will be available for use on malloc.
 */
void free(void *bp)
{
    if (bp == NULL)
    {
        return;
    }
    /* recall free is freeing address allocated for use */
    block_t *block = payload_to_header(bp);
    coalesce(block);
    dbg_printf("Completed free(%p)\n", bp);
}

/*
 * realloc: returns a pointer to an allocated region of at least size bytes:
 *          if ptrv is NULL, then call malloc(size);
 *          if size == 0, then call free(ptr) and returns NULL;
 *          else allocates new region of memory, copies old data to new memory,
 *          and then free old block. Returns old block if realloc fails or
 *          returns new pointer on success.
 */
void *realloc(void *ptr, size_t size)
{
    block_t *block = payload_to_header(ptr);
    size_t copysize;
    void *newptr;

    // If size == 0, then free block and return NULL
    if (size == 0)
    {
        free(ptr);
        return NULL;
    }

    // If ptr is NULL, then equivalent to malloc
    if (ptr == NULL)
    {
        return malloc(size);
    }

    // Otherwise, proceed with reallocation
    newptr = malloc(size);
    // If malloc fails, the original block is left untouched
    if (!newptr)
    {
        return NULL;
    }

    // Copy the old data
    copysize = get_payload_size(block); // gets size of old payload
    if(size < copysize)
    {
        copysize = size;
    }
    memcpy(newptr, ptr, copysize);

    // Free the old block
    free(ptr);

    return newptr;
}

/*
 * calloc: Allocates a block with size at least (elements * size + dsize)
 *         through malloc, then initializes all bits in allocated memory to 0.
 *         Returns NULL on failure.
 */
void *calloc(size_t nmemb, size_t size)
{
    void *bp;
    size_t asize = nmemb * size;

    if (asize/nmemb != size)
	// Multiplication overflowed
	return NULL;

    bp = malloc(asize);
    if (bp == NULL)
    {
        return NULL;
    }
    // Initialize all bits to 0
    memset(bp, 0, asize);

    return bp;
}

/******** The remaining content below are helper and debug routines ********/

/*
 * extend_heap: Extends the heap with the requested number of bytes, and
 *              recreates epilogue header. Returns a pointer to the result of
 *              coalescing the newly-created block with previous free block, if
 *              applicable, or NULL in failure.
 */
static block_t *extend_heap(size_t size)
{
    void *bp;

    // Allocate an even number of words to maintain alignment
    size = round_up(size, dsize);

    if ((bp = mem_sbrk(size)) == (void *)-1)
    {
        return NULL;
    }

    // Initialize free block header/footer
    block_t *block = payload_to_header(bp);
    bool det = fetch_prev_alloc(block);

    write_header(block, size, false);
    write_footer(block, size, false);
    update_block(block, det);

    // Create new epilogue header
    block_t *block_next = find_next(block);
    write_header(block_next, 0, true);
    epilogue = block_next; // reassign epilogue

    /* in the case we have a free block from a very large */
    /* malloc request, we will end up having a small block */
    /* placed accidentally in our largest class size */
    /* call destroy_free_block to fix this error */

    if (block == seglists[5])
    {
        destroy_free_block(block, 5);
    }

    return coalesce(block);
}

/* Coalesce: Coalesces current block with previous and next blocks if
 *           either or both are unallocated; otherwise the block is not
 *           modified. Then, insert coalesced block into the segregated list.
 *           Returns pointer to the coalesced block. After coalescing, the
 *           immediate contiguous previous and next blocks must be allocated.
 */
static block_t *coalesce(block_t * block)
{
    block_t *block_next;
    block_t *block_prev;
    bool prev_alloc;
    bool next_alloc;

    block_next = find_next(block);
    block_prev = find_prev(block);

    prev_alloc = fetch_prev_alloc(block);
    next_alloc = get_alloc(block_next);

    size_t size = get_size(block);

    write_header(block, size, false); // simply change allocation bit
    write_footer(block, size, false);

    /* for each coalesce case, we must remove from the seglists */
    /* the rightmost blocks merging with the leftmost blocks */
    /* the add the resulting joined block */
    if (prev_alloc && next_alloc)              // no coalescing required
    {
        add_free_block(block);
        update_block(block_next, false);
        return block; // case OK
    }

    else if (prev_alloc && !next_alloc)        // coalesce w/ right block
    {
        remove_free_block(block_next);
        size += get_size(block_next);
        write_header(block, size, false);
        write_footer(block, size, false);
        add_free_block(block);
        // block to the left must be allocated!
        update_block(block_next, false);
    }

    else if (!prev_alloc && next_alloc)        // coalesce w/ left block
    {
        remove_free_block(block_prev);
        size += get_size(block_prev);
        write_header(block_prev, size, false);
        write_footer(block, size, false);
        add_free_block(block_prev);
        block = block_prev;
        update_block(block_next, false);
    }

    else                                        // coalesce all 3 blocks
    {
        remove_free_block(block_prev);
        remove_free_block(block_next);
        size += get_size(block_next) + get_size(block_prev);
        write_header(block_prev, size, false);
        write_footer(block_next, size, false);
        add_free_block(block_prev);
        block = block_prev;
        update_block(find_next(block), false);
    }
    return block;
}

/* helper function for segregated free lists and footer optimization */
/* recall that allocated blocks will no longer need a footer */
/* but we must check for allocated status of previous block */
/* if free, store in excess lower bits of allocated block's header*/
static void update_block(block_t *block, bool alloc)
{
    uint64_t mask = 0x2;

    if (alloc)
    {
        block->header = block->header | mask; // mark lower bit in next block
    }
    else
    {
        // preserve all bits but mask out bit at index 1
        block->header = block->header & (~mask);
    }
    return;
}

/*
 * place: Places block with size of asize at the start of bp. If the remaining
 *        size is at least the minimum block size, then split the block to the
 *        the allocated block and the remaining block as free, which is then
 *        inserted into the segregated list. Requires that the block is
 *        initially unallocated.
 */
static void place(block_t *block, size_t asize)
{

    size_t csize = get_size(block);
    if ((csize - asize) >= min_block_size)
    {
	    block_t *block_next;
        write_header(block, asize, true);

        block_next = find_next(block);
        write_header(block_next, csize-asize, false);
        write_footer(block_next, csize-asize, false); // keep footer

        add_free_block(block_next); // split block, add to appropriate seglist
        // pdate one more block over in the case of heap extension!
    }

    else
    {
        write_header(block, csize, true);
        update_block(find_next(block), true); // always true
    }
    update_block(block, true);
    return;
}

/*
 * max: returns x if x > y, and y otherwise.
 */
static size_t max(size_t x, size_t y)
{
    return (x > y) ? x : y;
}

static size_t min(size_t x, size_t y)
{
    return (x < y) ? x : y;
}


/*
 * round_up: Rounds size up to next multiple of n
 */
static size_t round_up(size_t size, size_t n)
{
    return (n * ((size + (n-1)) / n));
}

/*
 * pack: returns a header reflecting a specified size and its alloc status.
 *       If the block is allocated, the lowest bit is set to 1, and 0 otherwise.
 */
static word_t pack(size_t size, bool alloc)
{
    return alloc ? (size | 1) : size;
}


/*
 * extract_size: returns the size of a given header value based on the header
 *               specification above.
 */
static size_t extract_size(word_t word)
{
    return (word & ~(word_t) 0xF);
}

/*
 * get_size: returns the size of a given block by clearing the lowest 4 bits
 *           (as the heap is 16-byte aligned).
 */
static size_t get_size(block_t *block)
{
    return extract_size(block->header);
}

/*
 * get_payload_size: returns the payload size of a given block, equal to
 *                   the entire block size minus the header.
 */
static word_t get_payload_size(block_t *block)
{
    size_t asize = get_size(block);
    return asize - wsize;
}

/*
 * extract_alloc: returns the allocation status of a given header value based
 *                on the header specification above.
 */
static bool extract_alloc(word_t word)
{
    return (bool)(word & 0x1);
}

/*
 * get_alloc: returns true when the block is allocated based on the
 *            block header's lowest bit, and false otherwise.
 */
static bool get_alloc(block_t *block)
{
    return extract_alloc(block->header);
}

/*
 * write_header: given a block and its size and allocation status,
 *               writes an appropriate value to the block header.
 */
static void write_header(block_t *block, size_t size, bool alloc)
{
    block->header = pack(size, alloc);
}


/*
 * write_footer: given a block and its size and allocation status,
 *               writes an appropriate value to the block footer by first
 *               computing the position of the footer.
 */
static void write_footer(block_t *block, size_t size, bool alloc)
{
    word_t *footerp = (word_t *)((block->u.payload) + get_size(block) - dsize);
    *footerp = pack(size, alloc);
}


/*
 * find_next: returns the next consecutive block on the heap by adding the
 *            size of the block.
 */
static block_t *find_next(block_t *block)
{
    return (block_t *)(((char *)block) + get_size(block));
}

/* helper function for segregated free lists */
/* with footer optimizations */
/* simply return value of bit at 1st index */
static bool fetch_prev_alloc(block_t *block)
{
    word_t *prev_foot = find_prev_footer(block);
    if (prev_foot == prologue) // edge case
    {
        return true;
    }
    uint64_t m1 = 1;
    uint64_t m2 = 0x1;
    return (bool)((block->header >> m1) & m2);
}

/*
 * find_prev_footer: returns the footer of the previous block.
 */
static word_t *find_prev_footer(block_t *block)
{
    // Compute previous footer position as one word before the header
    return (&(block->header)) - 1;
}

/*
 * find_prev: returns the previous block position by checking the previous
 *            block's footer and calculating the start of the previous block
 *            based on its size.
 */
static block_t *find_prev(block_t *block)
{
    word_t *footerp = find_prev_footer(block);
    size_t size = extract_size(*footerp);
    return (block_t *)((char *)block - size);
}

/*
 * payload_to_header: given a payload pointer, returns a pointer to the
 *                    corresponding block.
 */
static block_t *payload_to_header(void *bp)
{
    return (block_t *)(((char *)bp) - offsetof(block_t, u.payload));
}

/*
 * header_to_payload: given a block pointer, returns a pointer to the
 *                    corresponding payload.
 */
static void *header_to_payload(block_t *block)
{
    return (void *)(block->u.payload);
}

/* helper function for segregated free lists */
/* grab respective free list based on block's size */
/* choose powers of 2 or 16 for good distributions*/
static unsigned fetch_class(size_t asize)
{
    if (asize <= 64)
    {
        return 0;
    }
    else if (asize <= 128)
    {
        return 1;
    }
    else if (asize <= 256)
    {
        return 2;
    }
    else if (asize <= 512)
    {
        return 3;
    }
    else if (asize <= 1024)
    {
        return 4;
    }
    else return 5;
}


/* helper function for segregated free lists */
/* simply traverse free list until free block */
/* of sufficient size is found */
static block_t *find_free(size_t asize)
{
    block_t *curr;
    unsigned i;

    i = fetch_class(asize);
    size_t block_size;

    /* sufficient to use for memory allocation in list */
    /* traverse doubly-linked list */
    /* note that we start from the end of each seglist */
    /* to help conserve memory over the course of our */
    /* memory requests */
    for (curr = end_nodes[i]; i < 6; curr = end_nodes[i])
    {
        while (curr != NULL)
        {
            block_size = get_size(curr);

            if (asize <= block_size)
            {
                return curr;
            }
            curr = curr->u.l.prev;
        }
        i++;
    }
    return NULL;
}

/* helper function for segregated free lists */
/* called whenever we free a block */
/* prepend to seg list */
/* Last-in-first-out policy */
static void add_free_block(block_t *block)
{
    block_t *temp;
    unsigned i;

    i = fetch_class(get_size(block));
    temp = seglists[i]; // make copy of current root
    if (temp == block) return; // circularity guard
    block->u.l.prev = NULL; // prepending so NULL terminate
    block->u.l.next = temp; // ok if prev root = NULL
    seglists[i] = block;
    if (temp != NULL)
    {
        temp->u.l.prev = block;
    }
    if (temp == NULL) end_nodes[i] = block;
    return;
}

/* helper function for explicit free lists */
/* adjusts free list since it free block is to be allocated */
static void remove_free_block(block_t *block)
{
    /* recall block's payload is written to */
    /* we cannot reference the link struct!!! */
    //block_t *next;
    block_t *prev_node;
    block_t *next_node;
    unsigned i;

    prev_node = block->u.l.prev;
    next_node = block->u.l.next;
    i = fetch_class(get_size(block));

    /* case 1: block is the root (beginning of seglist) */
    if (prev_node == NULL)
    {
        seglists[i] = next_node; // make root next block in list
        if (next_node != NULL)
        {
            next_node->u.l.prev = NULL;
        }
        if (seglists[i] == NULL) end_nodes[i] = NULL;
    }

    /* case 2: block is in middle of free list */
    else if (prev_node != NULL && next_node != NULL)
    {
        prev_node->u.l.next = next_node;
        next_node->u.l.prev = prev_node;
    }

    /* case 3: block being removed at end of free list */
    else if (prev_node != NULL && next_node == NULL)
    {
        prev_node->u.l.next = NULL; // NULL terminate
        end_nodes[i] = prev_node;
    }

    block->u.l.prev = NULL;
    block->u.l.next = NULL;
    return;
}

/* helper function for segregated free lists */
/* only called when we have a free block placed in wrong */
/* size class due to heap extension */
static void destroy_free_block(block_t *block, unsigned i)
{
    block_t *next_node = block->u.l.next;
    seglists[i] = next_node;
    return;
}

/* data invariant checked for in heap checker */
/* there should never be two adjacent free blocks */
/* input will ALWAYS be  free block */
static bool coalesce_check(block_t *block)
{
    block_t *next;
    next = find_next(block);
    if (!get_alloc(block) && !get_alloc(next))
    {
        return false;
    }
    return true;
}

/* data invariant checked for in heap checker */
/* only free blocks will be checked for agreeging */
/* boundary tags */
static bool head_eq_foot(block_t *block)
{
    word_t *header;
    word_t *footer;

    header = &(block->header);
    footer = (word_t*)((block->u.payload) + get_size(block) - dsize);
    return (*header == *footer);
}

/* helper function for free_list_check() */
/* go from left to right checking for pointer consistency */
static bool consistent_pointers()
{
    block_t *curr;
    block_t *next;
    unsigned i;

    i = 0;
    for (curr = seglists[0]; i < 6; curr = seglists[i])
    {
        while (curr != NULL)
        {
            next = curr->u.l.next;
            /* if A's next pointer is to B, B's prev pointer should be to A */
            /* do NULL check for safety */
            if (next != NULL && next->u.l.prev != curr)
            {
                 return false;
            }
            curr = curr->u.l.next; // travel to next node
        }
        i++;
    }
    return true;
}

/* data invariant for segregated free lists */
/* check to see if each block in each seglist */
/* actually belongs in that size class */
static bool in_seglist()
{
    block_t *curr;
    unsigned i;
    bool det;

    i = 0;
    det = true;

    for (curr = seglists[i]; i < 6; curr = seglists[i])
    {
        while (curr != NULL)
        {
            if (fetch_class(get_size(curr)) != i)
            {
                printf("size: %zu\n i: %u\n", get_size(curr), i);
                det = false;
            }
            curr = curr->u.l.next;
        }
        i++;
    }
    return det;
}

/* helper function for heap checker */
/* iterate through every free block and traverse free list */
static bool free_count()
{
    block_t *curr;
    block_t *block;
    unsigned int counter1;
    unsigned int counter2;
    unsigned int i;

    i = 0;
    counter1 = 0; // explicit free list counter
    counter2 = 0; // heap list counter

    for (curr = seglists[0]; i < 6; curr = seglists[i])
    {
        while (curr != NULL)
        {
            curr = curr->u.l.next; // go to next node
            counter1++; // increment
        }
        i++; // move onto next seglist
    }

    for (block = heap_listp; block != epilogue; block = find_next(block))
    {
        if (!(get_alloc(block))) // check to see if block is free
        {
            counter2++;
        }
    }

    if (counter1 != counter2)
    {
        printf("Counter 1: %d\n", counter1);
        printf("Counter 2: %d\n", counter2);
    }
    return counter1 == counter2;
}

static bool heap_check()
{
    bool det;

    det = true;
    block_t *block;
    for (block = heap_listp; block != epilogue; block = find_next(block))
    {

        if (!(in_heap(block)))
        {
            printf("Block not in heap.\n");
            det = false;
        }
        if (!aligned((void*)get_size(block)))
        {
            printf("Block not aligned.\n");
            det = false;

        }
        if (!coalesce_check(block))
        {
            printf("Adjacent free blocks!\n");
            det = false;
        }
        if (!get_alloc(block) && !head_eq_foot(block))
        {
            printf("Not equal tags.\n");
            det = false;
        }
    }
    return det;
}

/* data invariant checkef for in heap checker */
/* checks for consistent pointers --> no loops */
/* all free list pointers between mem_heap_lo() and mem_heap_high() */
/* check to see if free blocks in heap match number of blocks in list */
static bool free_list_check()
{
    bool det;
    det = true;

    if (!heap_check())
    {
        printf("Free list pointer not in bounds.\n");
        det = false;
    }
    if (!in_seglist())
    {
        det = false;
    }
    if (!free_count())
    {
        printf("Mismatching free counts.\n");
        det = false;
    }
    if (!consistent_pointers())
    {
        printf("Inconsistent pointers.\n");
        det = false;
    }
    return det;
}

/* mm_checkheap: checks the heap for correctness; returns true if
 *               the heap is correct, and false otherwise
 */
bool mm_checkheap(int lineno)
{
    bool res;
    res = heap_check() && free_list_check();
    if (!res)
    {
        printf("Heap invariants failed!\n");
        return false;
    }
    return true;
}

