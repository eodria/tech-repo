/*
 * tsh - A tiny shell program with job control
 *
 * <Estevon Odrie andrew id: eodria>
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "csapp.h"

/* Misc manifest constants */
#define MAXLINE_TSH    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* Job states */
#define UNDEF         0   /* undefined */
#define FG            1   /* running in foreground */
#define BG            2   /* running in background */
#define ST            3   /* stopped */

/*
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Parsing states */
#define ST_NORMAL   0x0   /* next token is an argument */
#define ST_INFILE   0x1   /* next token is the input file */
#define ST_OUTFILE  0x2   /* next token is the output file */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE_TSH];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE_TSH];  /* command line */
};
struct job_t job_list[MAXJOBS]; /* The job list */

struct cmdline_tokens {
    int argc;               /* Number of arguments */
    char *argv[MAXARGS];    /* The arguments list */
    char *infile;           /* The input file */
    char *outfile;          /* The output file */
    enum builtins_t {       /* Indicates if argv[0] is a builtin command */
        BUILTIN_NONE,
        BUILTIN_QUIT,
        BUILTIN_JOBS,
        BUILTIN_BG,
        BUILTIN_FG} builtins;
};

/* End global variables */

/* Function prototypes */
void eval(char *cmdline);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, struct cmdline_tokens *tok);
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *job_list);
int maxjid(struct job_t *job_list);
int addjob(struct job_t *job_list, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *job_list, pid_t pid);
pid_t fgpid(struct job_t *job_list);
struct job_t *getjobpid(struct job_t *job_list, pid_t pid);
struct job_t *getjobjid(struct job_t *job_list, int jid);
int pid2jid(pid_t pid);
void listjobs(struct job_t *job_list, int output_fd);

void usage(void);


/*
 * main - The shell's main routine
 */
int
main(int argc, char **argv)
{
    char c;
    char cmdline[MAXLINE_TSH];    /* cmdline for fgets */
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
            break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
            break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
            break;
        default:
            usage();
        }
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */
    Signal(SIGTTIN, SIG_IGN);
    Signal(SIGTTOU, SIG_IGN);

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler);

    /* Initialize the job list */
    initjobs(job_list);

    /* Execute the shell's read/eval loop */
    while (1) {

        if (emit_prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }
        if ((fgets(cmdline, MAXLINE_TSH, stdin) == NULL) && ferror(stdin))
            app_error("fgets error");
        if (feof(stdin)) {
            /* End of file (ctrl-d) */
            printf ("\n");
            fflush(stdout);
            fflush(stderr);
            exit(0);
        }

        /* Remove the trailing newline */
        cmdline[strlen(cmdline)-1] = '\0';

        /* Evaluate the command line */

        eval(cmdline);

        fflush(stdout);
        fflush(stdout);
    }

    exit(0); /* control never reaches here */
}

/* checks for builtin commands */
int builtin_cmd(char *argv[MAXARGS]) {
  unsigned int id;
  struct job_t *j;

  if (strcmp(argv[0], "quit") == 0) {
    exit(0); // terminate shell
  }
  if (strcmp(argv[0], "jobs") == 0) {
    listjobs(job_list, 1); // call helper function
    return 1;
  }
  /* recall for background jobs, we can have as many */
  /* as we want up to limit */
  if (strcmp(argv[0], "bg") == 0) {
    char *str = argv[1]; // either PID or a JID
    if (str[0] == '%') { // compare chars, not strings
      str = str+1;
      id = atoi(str);
      j = getjobjid(job_list, id);
    }
    else {
      id = atoi(argv[1]);
      j = getjobpid(job_list, id);
    }
    Kill(-(j->pid), SIGCONT); // send signal
    j->state = 2; // switch state to background, as many bg jobs we want
    printf("[%d] (%d) %s\n", j->jid, j->pid, j->cmdline);
    return 1;
  }
  /* recall for foreground jobs, at most ONLY ONE */
  if ((strcmp(argv[0], "fg") == 0)) {
    char *str = argv[1]; // either PID or a JID
    if (str[0] == '%') {
      str = str+1;
      id = atoi(str);
      j = getjobjid(job_list, id);
    }
    else {
      id = atoi(argv[1]);
      j = getjobpid(job_list, id);
    }

    kill(j->pid, SIGCONT);
    j->state = 1; // state switch
    return 1;
  }
  return 0;
}

/* helper function for echo */
/* helps parse command line */
/* also takes into consideration */
/* escape sequences */
unsigned int scan1(char *cmdline) {
    int counter;
    int i;
    char curr;

    counter = 0;
    i = 0;
    while (cmdline[i] != '\0') { // check for NULL termination
        curr = cmdline[i];
        if (curr == '\\') {
            counter = counter+3;
        }
        i++;
    }

    return counter;
}

/* helper function for calls to */
/* /bin/kill --> SIGTERM */
void term(char *cmdline, char *arg) {
    int adjust1;
    unsigned int i;
    unsigned int len;
    pid_t pd;
    struct job_t *curr;
    char *buf = malloc(MAXLINE_TSH);
    if (buf == NULL) {
        exit(0);
    }
    adjust1 = 10;
    cmdline = cmdline+adjust1;
    len = strlen(arg);

    /* search all jobs by job ids up to nextjid in for loop */
    for (i = 1; i < nextjid; i++) {
        curr = getjobjid(job_list, i);
        pd = curr->pid; // fetch job pid
        strcpy(buf, curr->cmdline);
        buf = buf+2;
        buf[len] = '\0'; // NULL terminate for easy comparisom
        if (strcmp(cmdline, buf) == 0) {
            Kill(-pd, SIGTERM);
        }
    }

    buf = buf-2;
    free(buf);
    return;
}

/* helper function */
/* accesses string at char \ */
/* either parse as >, \,  &, or ' */
char escape(char *cmdline) {
    unsigned int i;
    int found;
    char res;

    i = 0;
    found = 0;

    while (cmdline[i] != '\0' && !found) {
        if (cmdline[i] == '7' && cmdline[i+1] == '6') {
            res = '>';
            found = 1;
        }
        if (cmdline[i] == '7' && cmdline[i+1] == '4') {
            res = '<';
            found = 1;
        }
        if (cmdline[i] == '4' && cmdline[i+1] == '6') {
            res = '&';
            found = 1;
        }
        if (cmdline[i] == '4' && cmdline[i+1] == '7') {
            res = '\'';
            found = 1;
        }
        i++;
    }

    return res;
}

/* helper funtion */
/* note \076 is > and \046 is & */
/* pass in command line */
char *echo(char *cmdline) {
    /* cmdline starts with /bin/echo -e */
    int adj1;
    int adj2;
    unsigned int len;
    unsigned int idx1;
    unsigned int idx2;
    char curr;
    char *res; // the result string

    adj1 = 13; // subtract for /bin/echo -e [space]
    adj2 = scan1(cmdline+adj1);
    cmdline = cmdline+adj1; // pointer arithmetic
    len = strlen(cmdline); // grab len
    res = malloc(len-adj2);

    if (res == NULL) {
        exit(0);
    }
    idx2 = 0;

    //printf("len: %d\n", len);
    for (idx1 = 0; idx1 < len; idx1++) {
        assert(idx1 <= len);
        curr = cmdline[idx1];
        if (curr == '\\') {
            curr = escape(cmdline+idx1); // access cmdline at ith index
            idx1 = idx1+3;
        }
        res[idx2] = curr; // put into result string
        idx2++;
    }

    printf("%s\n", res);
    return res;
}

/* writer helper function */
/* writes from a file to standard output */
void write1(int fd) {

    int n;
    rio_t rio;
    char buf[MAXLINE];

    Rio_readinitb(&rio, fd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        Rio_writen(STDOUT_FILENO, buf, n);
    }

    return;
}

void write2(int fd) {
    int n;
    rio_t rio;
    char buf[MAXLINE];

    Rio_readinitb(&rio, STDOUT_FILENO);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        Rio_writen(fd, buf, n);
    }

    return;
}

void write3(int fd1, int fd2) {
    int n;
    rio_t rio;
    char buf[MAXLINE];

    Rio_readinitb(&rio, fd1);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        Rio_writen(fd2, buf, n);
    }

    return;

}

/* I/O redirection detector helper function */
int ioscan(char *cmdline, struct cmdline_tokens *tok) {

    int out; // display contents of file to stdout?
    int redirect; // redirect output of file to another?
    unsigned int i;
    unsigned int len;
    int fd1;
    int fd2;
    int det;

    out = 0;
    redirect = 0;
    len = strlen(cmdline);
    det = 0;
    for (i = 0; i < len; i++) {
        if (cmdline[i] == '<') {
            out = 1;
            det = 1;
        }
        if (cmdline[i] == '>') {
            redirect = 1;
            det = 1;
        }
    }

    /* first case -- if only <, simply write to standard */
    /* output */
    if (out && !redirect) {
        fd1 = Open(tok->infile, O_RDWR, 0);
        write1(fd1); // simply write to standard output
    }

    if (!out && redirect) {
        printf(tok->outfile);
        fd1 = Open(tok->outfile, O_RDWR, 0);
        dup2(fd1, 1);
    }

    /* do not write to standard output directly */
    /* write contents to redirect file */
    if (out && redirect) {
      fd1 = Open(tok->infile, O_RDWR, 0);
      fd2 = Open(tok->outfile, O_RDWR, 0);
      write3(fd1, fd2);
    }


    return det;
}

/*
 * eval - Evaluate the command line that the user has just typed in
 *
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.
 */
void
eval(char *cmdline)
{
    int bg;              /* should the job run in bg or fg? */
    struct cmdline_tokens tok;
    int builtin;         /* is command already built-in? */
    pid_t pid;
    int io;              /* will there be I/O redirection? */

    sigset_t mask_all;
    sigset_t mask;
    sigset_t prev_mask;
    sigset_t empty_mask;

    Sigfillset(&mask_all);
    Sigemptyset(&mask);
    Sigemptyset(&empty_mask);
    Sigaddset(&mask, SIGCHLD);
    Sigaddset(&mask, SIGINT);
    Sigaddset(&mask, SIGTSTP);


   /* Parse command line */
    bg = parseline(cmdline, &tok);

    if (bg == -1) /* parsing error */
        return;
    if (tok.argv[0] == NULL) /* ignore empty lines */
        return;
    if (strcmp(tok.argv[0], "/bin/echo") == 0) {
        cmdline = echo(cmdline); // pass in command line
        return;
    }
    if (strcmp(tok.argv[0], "/bin/kill") == 0) {
        term(cmdline, tok.argv[1]);
        return;
    }

    io = ioscan(cmdline, &tok);

    if ((builtin = builtin_cmd(tok.argv))) {
        return;
    }

    if (io) return;

    if (!builtin) {
        /* parent must use sigprocmask before forking child */
        Sigprocmask(SIG_BLOCK, &mask, &prev_mask);

        /* fork child process and run job in child's context */
        if ((pid = fork()) == 0) {

            //Sigprocmask(SIG_SETMASK, &prev_mask, NULL);

            /* unblock signals for child process */
            //Sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            const char *filename = tok.argv[0];
            setpgid(0, 0);

            /* reinstall signal handlers */

            Signal(SIGCHLD, SIG_DFL);
            Signal(SIGINT, SIG_DFL);
            Signal(SIGTSTP, SIG_DFL);

            Sigdelset(&prev_mask, SIGCHLD);
            Sigdelset(&prev_mask, SIGINT);
            Sigdelset(&prev_mask, SIGTSTP);

            Sigprocmask(SIG_SETMASK, &prev_mask, NULL);

            if (execve(filename, tok.argv, environ) < 0) {
              printf("%s: Command not found.\n", tok.argv[0]);
              exit(0);
            }
        }

        Sigprocmask(SIG_BLOCK, &mask_all, NULL);
        addjob(job_list, pid, bg+1, cmdline);
        Sigprocmask(SIG_SETMASK, &prev_mask, NULL);

        /* block all signals here to prevent race conditions */
        Sigfillset(&prev_mask);
        Sigprocmask(SIG_BLOCK, &prev_mask, NULL);

        if (!bg) {

            while (fgpid(job_list)) {
                Sigemptyset(&prev_mask);
                Sigsuspend(&prev_mask);
            }

        }

        if (bg) { /* for background jobs, print and do stuff */
            nextjid = nextjid-1;
            printf("[%d] (%d) %s\n", nextjid, pid, cmdline);
            nextjid = nextjid+1;
        }

    }

    return;
}

/*
 * parseline - Parse the command line and build the argv array.
 *
 * Parameters:
 *   cmdline:  The command line, in the form:
 *
 *                command [arguments...] [< infile] [> oufile] [&]
 *
 *   tok:      Pointer to a cmdline_tokens structure. The elements of this
 *             structure will be populated with the parsed tokens. Characters
 *             enclosed in single or double quotes are treated as a single
 *             argument.
 * Returns:
 *   1:        if the user has requested a BG job
 *   0:        if the user has requested a FG job
 *  -1:        if cmdline is incorrectly formatted
 *
 * Note:       The string elements of tok (e.g., argv[], infile, outfile)
 *             are statically allocated inside parseline() and will be
 *             overwritten the next time this function is invoked.
 */
int
parseline(const char *cmdline, struct cmdline_tokens *tok)
{

    static char array[MAXLINE_TSH];          /* holds local copy of command line */
    const char delims[10] = " \t\r\n";   /* argument delimiters (white-space) */
    char *buf = array;                   /* ptr that traverses command line */
    char *next;                          /* ptr to the end of the current arg */
    char *endbuf;                        /* ptr to end of cmdline string */
    int is_bg;                           /* background job? */

    int parsing_state;                   /* indicates if the next token is the
                                            input or output file */

    if (cmdline == NULL) {
        (void) fprintf(stderr, "Error: command line is NULL\n");
        return -1;
    }

    (void) strncpy(buf, cmdline, MAXLINE_TSH);
    endbuf = buf + strlen(buf);

    tok->infile = NULL;
    tok->outfile = NULL;

    /* Build the argv list */
    parsing_state = ST_NORMAL;
    tok->argc = 0;

    while (buf < endbuf) {
        /* Skip the white-spaces */
        buf += strspn (buf, delims);
        if (buf >= endbuf) break;

        /* Check for I/O redirection specifiers */
        if (*buf == '<') {
            if (tok->infile) {
                (void) fprintf(stderr, "Error: Ambiguous I/O redirection\n");
                return -1;
            }
            parsing_state |= ST_INFILE;
            buf++;
            continue;
        }
        if (*buf == '>') {
            if (tok->outfile) {
                (void) fprintf(stderr, "Error: Ambiguous I/O redirection\n");
                return -1;
            }
            parsing_state |= ST_OUTFILE;
            buf ++;
            continue;
        }

        if (*buf == '\'' || *buf == '\"') {
            /* Detect quoted tokens */
            buf++;
            next = strchr (buf, *(buf-1));
        } else {
            /* Find next delimiter */
            next = buf + strcspn (buf, delims);
        }

        if (next == NULL) {
            /* Returned by strchr(); this means that the closing
               quote was not found. */
            (void) fprintf (stderr, "Error: unmatched %c.\n", *(buf-1));
            return -1;
        }

        /* Terminate the token */
        *next = '\0';

        /* Record the token as either the next argument or the i/o file */
        switch (parsing_state) {
        case ST_NORMAL:
            tok->argv[tok->argc++] = buf;
            break;
        case ST_INFILE:
            tok->infile = buf;
            break;
        case ST_OUTFILE:
            tok->outfile = buf;
            break;
        default:
            (void) fprintf(stderr, "Error: Ambiguous I/O redirection\n");
            return -1;
        }
        parsing_state = ST_NORMAL;

        /* Check if argv is full */
        if (tok->argc >= MAXARGS-1) break;

        buf = next + 1;
    }

    if (parsing_state != ST_NORMAL) {
        (void) fprintf(stderr,
                       "Error: must provide file name for redirection\n");
        return -1;
    }

    /* The argument list must end with a NULL pointer */
    tok->argv[tok->argc] = NULL;

    if (tok->argc == 0)  /* ignore blank line */
        return 1;

    if (!strcmp(tok->argv[0], "quit")) {                 /* quit command */
        tok->builtins = BUILTIN_QUIT;
    } else if (!strcmp(tok->argv[0], "jobs")) {          /* jobs command */
        tok->builtins = BUILTIN_JOBS;
    } else if (!strcmp(tok->argv[0], "bg")) {            /* bg command */
        tok->builtins = BUILTIN_BG;
    } else if (!strcmp(tok->argv[0], "fg")) {            /* fg command */
        tok->builtins = BUILTIN_FG;
    } else {
        tok->builtins = BUILTIN_NONE;
    }

    /* Should the job run in the background? */
    if ((is_bg = (*tok->argv[tok->argc-1] == '&')) != 0)
        tok->argv[--tok->argc] = NULL;

    return is_bg;
}


/*****************
 * Signal handlers
 *****************/

/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP, SIGTSTP, SIGTTIN or SIGTTOU signal. The
 *     handler reaps all available zombie children, but doesn't wait
 *     for any other currently running children to terminate.
 */
void
sigchld_handler(int sig)
{
    int olderrno = errno;
    sigset_t mask;
    sigset_t prev_mask;
    pid_t pid;
    struct job_t *j;
    int jid;
    int status;

    Sigfillset(&mask);
    Sigprocmask(SIG_BLOCK, &mask, &prev_mask);

    /* waitpid returns PID of child if OK*/

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {

            j = getjobpid(job_list, pid);
            jid = j->jid;

            if (WIFEXITED(status)) {       // normal exit or return
                deletejob(job_list, pid);
            }

            if (WTERMSIG(status) == SIGINT) {
                Sio_puts("Job [");
                Sio_putl(jid);
                Sio_puts("] ");
                Sio_puts("(");
                Sio_putl(pid);
                Sio_puts(") terminated by signal 2\n");
                deletejob(job_list, pid); // delete terminated jobs
            }

            else if (WTERMSIG(status) == SIGTERM) {
                deletejob(job_list, pid);
            }

            else if (WSTOPSIG(status) == SIGTSTP) {
                Sio_puts("Job [");
                Sio_putl(jid);
                Sio_puts("] ");
                Sio_puts("(");
                Sio_putl(pid);
                Sio_puts(") stopped by signal 20\n");
                j->state = ST;
            }

    }

    Sigprocmask(SIG_SETMASK, &prev_mask, NULL);
    errno = olderrno;

    return;
}

/*
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.
 */
void
sigint_handler(int sig)
{
    int olderrno = errno; // save errno
    sigset_t mask;
    sigset_t prev_mask;
    pid_t pd;

    /* block all signals because job_list is global */
    /* and is being accessed!!! */
    Sigfillset(&mask);
    Sigprocmask(SIG_BLOCK, &mask, &prev_mask); // block signals

    pd = fgpid(job_list); // fetch PID of current fg job

    Kill(-pd, SIGINT);

    Sigprocmask(SIG_SETMASK, &prev_mask, NULL);
    errno = olderrno;
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.
 */
void
sigtstp_handler(int sig)
{
    int olderrno = errno; // save
    sigset_t mask;        // initialize masks
    sigset_t prev_mask;
    pid_t pd;

    Sigfillset(&mask);
    Sigprocmask(SIG_BLOCK, &mask, &prev_mask);

    pd = fgpid(job_list);

    Kill(-pd, SIGTSTP);

    Sigprocmask(SIG_SETMASK, &prev_mask, NULL);
    errno = olderrno;
    return;
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void
sigquit_handler(int sig)
{
    sio_error("Terminating after receipt of SIGQUIT signal\n");
}



/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void
clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void
initjobs(struct job_t *job_list) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
        clearjob(&job_list[i]);
}

/* maxjid - Returns largest allocated job ID */
int
maxjid(struct job_t *job_list)
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
        if (job_list[i].jid > max)
            max = job_list[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int
addjob(struct job_t *job_list, pid_t pid, int state, char *cmdline)
{
    int i;

    if (pid < 1)
        return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (job_list[i].pid == 0) {
            job_list[i].pid = pid;
            job_list[i].state = state;
            job_list[i].jid = nextjid++;
            if (nextjid > MAXJOBS)
                nextjid = 1;
            strcpy(job_list[i].cmdline, cmdline);
            if(verbose){
                printf("Added job [%d] %d %s\n",
                       job_list[i].jid,
                       job_list[i].pid,
                       job_list[i].cmdline);
            }
            return 1;
        }
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int
deletejob(struct job_t *job_list, pid_t pid)
{
    int i;

    if (pid < 1)
        return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (job_list[i].pid == pid) {
            clearjob(&job_list[i]);
            nextjid = maxjid(job_list)+1;
            return 1;
        }
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t
fgpid(struct job_t *job_list) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
        if (job_list[i].state == FG)
            return job_list[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t
*getjobpid(struct job_t *job_list, pid_t pid) {
    int i;

    if (pid < 1)
        return NULL;
    for (i = 0; i < MAXJOBS; i++)
        if (job_list[i].pid == pid)
            return &job_list[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *job_list, int jid)
{
    int i;

    if (jid < 1)
        return NULL;
    for (i = 0; i < MAXJOBS; i++)
        if (job_list[i].jid == jid)
            return &job_list[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int
pid2jid(pid_t pid)
{
    int i;

    if (pid < 1)
        return 0;
    for (i = 0; i < MAXJOBS; i++)
        if (job_list[i].pid == pid) {
            return job_list[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void
listjobs(struct job_t *job_list, int output_fd)
{
    int i;
    char buf[MAXLINE_TSH];

    for (i = 0; i < MAXJOBS; i++) {
        memset(buf, '\0', MAXLINE_TSH);
        if (job_list[i].pid != 0) {
            sprintf(buf, "[%d] (%d) ", job_list[i].jid, job_list[i].pid);
            if(write(output_fd, buf, strlen(buf)) < 0) {
                fprintf(stderr, "Error writing to output file\n");
                exit(1);
            }
            memset(buf, '\0', MAXLINE_TSH);
            switch (job_list[i].state) {
            case BG:
                sprintf(buf, "Running    ");
                break;
            case FG:
                sprintf(buf, "Foreground ");
                break;
            case ST:
                sprintf(buf, "Stopped    ");
                break;
            default:
                sprintf(buf, "listjobs: Internal error: job[%d].state=%d ",
                        i, job_list[i].state);
            }
            if(write(output_fd, buf, strlen(buf)) < 0) {
                fprintf(stderr, "Error writing to output file\n");
                exit(1);
            }
            memset(buf, '\0', MAXLINE_TSH);
            sprintf(buf, "%s\n", job_list[i].cmdline);
            if(write(output_fd, buf, strlen(buf)) < 0) {
                fprintf(stderr, "Error writing to output file\n");
                exit(1);
            }
        }
    }
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void
usage(void)
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}
