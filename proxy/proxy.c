#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "csapp.h"

/* Name: Estevon Odria */
/* andrewid: eodria */

/* ****************************************************/
/* Concurrent web proxy implementation with threading */
/* The proxy acts like the server to the client and   */
/* begins parsing the request headers and writing     */
/* to the server. It extracts the relevant hostname   */
/* and port number then attemps to make a connection. */
/* The server then writes back info to proxy which    */
/* in turn reads back to the client thus serving it   */
/* the resources.                                     */
/*****************************************************/


/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
//#define CACHE_ARR_LEN 1000;

typedef struct buffdata cache_t;

void serve_client(int fd);
void fetch_connect(char uri[MAXLINE],
        struct addrinfo *host_info, struct addrinfo hints);
int proxy_to_server(char *hostname, char *port);
int read_requesthdrs(rio_t *rp);
char *parse_buf(char *header);
char *parse_gets(char *header, size_t host_len, size_t port_len,
                 bool parse_port);
void **parse_host(char buf[MAXLINE]);
void write_to_server(int fd, char **buffers);
void read_to_client(int fd1, int fd2);
void *thread(void *vargp);
uint64_t hash_webobject(char *buffer);
cache_t **cache_new(size_t len);
void add_to_cache(cache_t **cache, char *buffer);

/* data structure used to implement web caching */
/* store the text in buffer */
/* check valid to see if we can use it */
struct buffdata {
    cache_t *next;
    uint64_t tag;
    char *buffer;
};

/* simple web proxy that handles HTTP/1.0 GET requests */
/* if not such a request, ignore */

int main(int argc, char **argv)
{
    int listenfd;
    int *connfdp;
    char hostname[MAXLINE]; // client's host name
    char port[MAXLINE];     // client's port #
    struct sockaddr_storage clientaddr;
    socklen_t clientlen;
    pthread_t tid; // for implementing concurrent proxy
    cache_t **cache;

    cache = cache_new(1000); // initialize the web cache
    (void)cache;

    /* client must supply server host and port */
    /* proxy also must be supplied a port number */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]); // port which proxy listens on
    while (1) {
        /* we will set up the connected descriptors */
        /* and fetch the client's pertinent info */
        clientlen = sizeof(clientaddr);
        connfdp = Malloc(sizeof(int));
        if (connfdp == NULL) {
            fprintf(stderr, "Not enough available memory resources.\n");
            exit(1);
        }
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE,
                    port, MAXLINE, 0);
        Pthread_create(&tid, NULL, thread, connfdp);
    }
    return 0;
}

void serve_client(int fd)
{

    rio_t rio;
    int clientfd;

    /* Start to read the request line and headers */

    Rio_readinitb(&rio, fd);
    /* read requests headers and establish connection */
    clientfd = read_requesthdrs(&rio);
    read_to_client(clientfd, fd);

    return;
}

/* helper routine for web proxy */
/* parses the first request header */
/* to extract host name and port # we will return an array of void pointers */
/* allocate mem on heap for all 3 elements */
void **parse_host(char buf[MAXLINE])
{
    uint64_t host_start;
    size_t host_len;
    size_t port_len;
    uint64_t idx;
    void **res = malloc(3*sizeof(void*));
    if (res == NULL) {
        fprintf(stderr, "Not enough available memory resources.\n");
        exit(1);
    }
    unsigned int offset;

    bool lim;
    bool *parse_port;

    host_start = 6; // host name begins after first space in header
    idx = host_start;
    host_len = 0;
    port_len = 0;
    /* lim is very important in our algorithm */
    /* if no port number, set to false to stop parsing */
    /* default to port number 80 */
    lim = true;
    parse_port = malloc(sizeof(bool));
    if (parse_port == NULL) {
        fprintf(stderr, "Not enough available memory resouces. Abort.\n");
        exit(1);
    }
    *parse_port = true;

    /* start fetching host name */
    /* recall that port starts right after colon */
    while (buf[idx] != ':' && lim) {
        if (buf[idx] == '\r') {
            *parse_port = false;
            lim = false; // stops loop
        }
        else {
            host_len = host_len+1;
            idx = idx+1;
        }
    }

    idx = idx+1;
    /* start fetching port number */
    while (buf[idx] != '\r') {
        port_len = port_len+1;
        idx = idx+1;
    }

    char *host = malloc(host_len+1);
    if (host == NULL) {
        fprintf(stderr, "Not enough available memory resources. Abort\n");
        exit(1);
    }

    char *port;
    /* now we must deal with port parsing */
    /* if no port specified, default to port 80 */
    if (port_len == 0) {
        *parse_port = false;
        port = malloc(3); // '8', '0', '\0'
    }
    else port = malloc(port_len+1);
    if (port == NULL) {
        fprintf(stderr, "Not enough available memory resources. Abort\n");
        exit(1);
    }

    idx = host_start; // reset idx
    /* start building up host string */
    /* use a logical AND -> we must stop parsing host */
    /* when we encounter a colon (port specified */
    /* or when carriage return is seen (default 80) */
    while (buf[idx] != ':' && buf[idx] != '\r') {
        /* safety assert */
        assert(0 <= idx-host_start && idx-host_start < host_len);
        host[idx-host_start] = buf[idx];
        idx = idx+1;
    }

    idx = idx+1; // skip the colon
    offset = host_start+host_len+1;
    /* now build up the port string if parse_port set*/

    while (buf[idx] != '\r' && *parse_port) {
        assert(0 <= idx-offset && idx-offset < port_len); // safety assert
        port[idx-offset] = buf[idx]; // correct???
        idx = idx+1;
    }

    host[host_len] = '\0'; // null terminate
    if (*parse_port) {
        port[port_len] = '\0'; // null terminate
    }
    else {
        port[0] = '8';
        port[1] = '0';
        port[2] = '\0';
    }
    /* cast all pertinent info to void pointers and add to result arr */
    res[0] = (void*)host;
    res[1] = (void*)port;
    res[2] = (void*)parse_port;
    return res;
}

/* helper routine for our web proxy */
/* takes host and port to establishes connection to server */
int proxy_to_server(char *hostname, char *port)
{
    /* use wrapper function to let proxy */
    /* establish connection with server */
    return open_clientfd(hostname, port);
}

/* helper routine */
/* reads and ignores request headers */
/* we will need to extract full URL */
int read_requesthdrs(rio_t *rp)
{
    char method[MAXLINE];
    char uri[MAXLINE];
    char version[MAXLINE];

    char buf[MAXLINE];
    void **infoarr;
    char *host;
    char *port;
    bool parse_port;
    char **buffers;
    char *connection;
    char *prox;
    uint32_t counter;
    int clientfd;

    /* buffers holds our arrays of buffers */
    /* with which we write to the server */

    buffers = malloc(7*sizeof(char*));
    if (buffers == NULL) {
        fprintf(stderr, "Not enough available memory resources. Abort.\n");
        exit(1);
    }
    counter = 0;

    /* we must ALWAYS send these to our server */
    connection = "Connection: close\n";
    prox = "Proxy-Connection: close\n";

    Rio_readlineb(rp, buf, MAXLINE);
    buffers[counter] = parse_buf(buf);
    counter++;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) {
        return -1; // ignore non-GET requests
    }

    Rio_readlineb(rp, buf, MAXLINE);
    buffers[counter] = parse_buf(buf);
    counter++;
    printf("%s", buf);
    while (strcmp(buf, "\r\n")) {

        if (counter == 4) {
            /* we won't have to read from buf */
            /* since we have to add it ourselves */
            buffers[counter] = connection;
            printf(connection);
            counter++;
        }
        else if (counter == 5) {
            /* we must read this line regardless */
            /* to keep correct file position */
            Rio_readlineb(rp, buf, MAXLINE);
            buffers[counter] = prox;
            printf(prox);
            counter++;
        }
        else {
            /* start reading lines and inserting */
            /* into the buffer array */
            Rio_readlineb(rp, buf, MAXLINE);
            buffers[counter] = parse_buf(buf);
            printf("%s", buf);
            counter++;
        }
    }

    /* extract pertinent host and port to make */
    /* successful connection to web server */
    infoarr = parse_host(buffers[2]);

    if (strcmp((char*)infoarr[0], "gent") == 0) {
        infoarr = parse_host(buffers[1]);
    }

    host = (char*)infoarr[0];
    port = (char*)infoarr[1];
    parse_port = *((bool*)infoarr[2]);
    buffers[0] = parse_gets(buffers[0], strlen(host), strlen(port),
                            parse_port);
    clientfd = proxy_to_server(host, port); // make connection
    /* host, port, and parse port allocated on heap */
    free(infoarr[0]);
    free(infoarr[1]);
    free(infoarr[2]);
    /* now write to the server */
    write_to_server(clientfd, buffers);
    return clientfd;
}

/* helper routine for web proxy */
/* proxy begins to write and communicate */
/* with server by using buffers in input array */
void write_to_server(int fd, char **buffers)
{
    unsigned int bufflen = 7; // was 6...
    unsigned int i;

    rio_t rio;
    Rio_readinitb(&rio, fd); // initialize before writing
    for (i = 0; i < bufflen; i++) {
        Rio_writen(fd, buffers[i], strlen(buffers[i]));
    }

    return;
}

/* helper routine for web proxy */
/* once server writes back to proxy */
/* we must read all pertinent information */
/* to the client */
void read_to_client(int fd1, int fd2)
{
    char buf[MAXLINE];
    rio_t rio;
    int n;

    Rio_readinitb(&rio, fd1);
    while ((n = Rio_readnb(&rio, buf, MAXLINE)) != 0) {
        Rio_writen(fd2, buf, n);
    }
    Close(fd1);
    return;
}

/* helper routine for web proxy */
/* as we parse request headers, write to */
/* result string and pass into buffer arr */
char *parse_buf(char *header)
{
    unsigned int i;
    char *res;
    size_t len;

    i = 0;
    len = 0;
    while (header[i] != '\0') {
        len++;
        i++;
    }

    res = malloc(len+1);
    i = 0;
    while (header[i] != '\0') {
        res[i] = header[i];
        i++;
    }
    assert(i == len);
    res[i] = '\0';
    return res;
}

/* helper routine for web proxy */
/* as we parse request headers, write to */
/* userbuf as needed */
char *parse_gets(char *header, size_t host_len, size_t port_len,
                 bool parse_port)
{
    unsigned int idx;
    unsigned int start;
    unsigned int adj;
    unsigned int gets;
    bool det = false;

    size_t len;
    char *resbuf;
    size_t connlen;

    start = 11; // this is where host starts
    /* we can skip host and port by adding start and connlen */
    /* where connlen = length of host and port */
    /* HOWEVER, if parse_port = false then idx = length(hostname) */
    if (!parse_port) connlen = host_len;
    /* if we do parse port, add 1 for colon char! */
    else connlen = host_len+port_len+1;

    idx = start+connlen;

    adj = idx;
    len = 0;
    gets = 4;

    /* end of string reached when NULL terminating char reached */

    /* find length and then build up res buffer */
    //printf("idx: %u\n", idx);
    if (header[idx] == ' ') det = true;

    if (header[idx] == '/' || header[idx] == ' ') {
        while (header[idx] != '\0') {
            if (header[idx] == ' ') {
                idx++;
            }
            else {
                len++;
                idx++;
            }
        }
    }
    if (det) {
        resbuf = "GET / HTTP/1.0\r\n";
        return resbuf;
    }
    else resbuf = malloc(len+gets+1);
    resbuf[0] = 'G';
    resbuf[1] = 'E';
    resbuf[2] = 'T';
    resbuf[3] = ' ';

    for (idx = start+connlen; idx-adj < len; idx++)
    {
        if (header[idx] == '\r') {
            // convert HTTP 1.1 to HTTP 1.0
            resbuf[idx-adj-1+gets] = '0';
        }

        resbuf[idx-adj+gets] = header[idx];
    }
    resbuf[idx-adj+gets] = '\0';
    return resbuf;
}

/* thread routine for implementation of concurrent web proxy */
void *thread(void *vargp)
{
    int connfd = *((int*)vargp);
    /* detach so each thread will self reap */
    Pthread_detach(pthread_self());
    Free(vargp);
    serve_client(connfd); // call to serve client
    Close(connfd);
    return NULL;
}

/* helper routine for web proxy */
/* helps implement caching web objects */
/* use powers of 31 to help create equal distribtion */
/* in cache array */
uint64_t hash_object(char *buffer)
{
    unsigned int i;
    uint64_t counter;
    size_t len;
    char c;
    int val;
    int mul;

    if (strlen(buffer) == 0) return 0; // empty string case
    counter = 0;
    len = strlen(buffer);
    mul = 31;
    for (i = 0; i < len-1; i++) {
        c = buffer[i]; // index into string to grab current char
        val = (int)c;
        counter = (counter + val) * mul;
    }
    counter = counter + (int)(buffer[len-1]);
    return counter;
}

/* helper routine for web proxy */
/* helps implement cachine */
/* allocates an array of arrays */
/* each inner array is of length 4 */
cache_t **cache_new(size_t len)
{
    unsigned int i;
    cache_t **res;

    /* allocate an array of pointers to webobjects */
    res = malloc(len*sizeof(struct buffdata*));
    for (i = 0; i < len; i++) {
        res[i] = NULL; // initialize all buckets
    }

    return res;
}

/* helper routinate for web proxy */
/* helps implement caching */
/* we take the hash index and enter the array */
/* if tags match, then we have a hit */
bool in_cache(cache_t **cache, char *webobject)
{
    unsigned int i;
    uint64_t curr_tag;
    uint64_t target_tag;
    size_t len;
    cache_t *curr;

    len = 1000;
    i = hash_object(webobject) % len;
    curr = cache[i];
    target_tag = hash_object(webobject);

    while (curr != NULL) {
        curr_tag = hash_object(curr->buffer);
        if (curr_tag == target_tag) {
            return true;
        }
        curr = curr->next; // go to next node
    }

    return false;
}

/* helper routine for web proxy */
/* helps implement caching */
/* calculates index into cache array */
/* and checks to see if already added */
void add_to_cache(cache_t **cache, char *buffer)
{
    assert(!in_cache(cache, buffer));
    /* calculate index with hash function */
    unsigned int idx;
    size_t len;
    cache_t *start;
    cache_t *res;

    len = 1000;
    idx = hash_object(buffer) % len;
    start = cache[idx];

    /* allocate memory for new entry in our cache */
    /* initialize all fields and prepend to current node */
    res = malloc(sizeof(struct buffdata));
    res->buffer = buffer;
    res->next = start;

    /* for tag, do not take the modulus */
    /* otherwise all nodes in bucket will have same tag */
    /* which renders our hashing cache useless */
    res->tag = hash_object(buffer);
    assert(in_cache(cache, buffer));
    return;
}
