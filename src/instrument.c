#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define RED "\x1b[1;31m"
#define GREEN "\x1b[1;32m"
#define BLUE "\x1b[1;35m"
#define RESET "\x1b[0m"

static void pretty_print_trace_entry(char* start)
{
    // entry format
    // "<function signature> at /...<path to src>.../<src basename> ..."
    char const* func_name = start;

    char* loc = strstr(start, " at ");
    loc[0] = '\0';

    // skip library functions
    char* path = &loc[4];
    if (strncmp(path, "/usr", 4) == 0)
        return;

    // get basename
    while (*path != '/')
        ++path;
    char const* after_last_slash;
    while (*path != '\0') {
        ++path;
        after_last_slash = path;
        while (*path != '/' && *path != '\0')
            ++path;
    }

    fprintf(stderr, GREEN "%s" RESET "\n\tat " BLUE "%s\n" RESET, func_name, after_last_slash);
}

static void sighandler(int sig)
{
#define TRACE_SZ 64
    void* trace[TRACE_SZ];
    char** messages = NULL;

    int trace_size = backtrace(trace, TRACE_SZ);
    messages = backtrace_symbols(trace, trace_size);

    char const* type;
    size_t cutoff, end_cutoff;

    switch (sig) {
    case SIGSEGV:
        type = "SEGFAULT";
        cutoff = 2;
        break;
    case SIGABRT:
        type = "ABORT";
        cutoff = 7;
        break;
    default:
        type = "UNKNOWN SIGNAL";
        cutoff = 0;
        break;
    }
    end_cutoff = 3;

    fprintf(stderr, RED "[%s] Backtrace:\n" RESET, type);

    // prepare argv as
    // /bin/addr2line -Cfpe ./sclp <addr1> <addr2> ...
    // (atmost TRACE_SZ addresses
    //      hence size 3+TRACE_SZ+1)
    char const* argv[3+TRACE_SZ+1] = { "/bin/addr2line", "-Cfpe", "./sclp" };
    for (size_t i = cutoff, a = 3; i < trace_size - end_cutoff; ++i, ++a) {
        size_t p = 0;
        while (messages[i][p] != '(')
            ++p;
        ++p;
        argv[a] = &messages[i][p];
        while (messages[i][p] != ')')
            ++p;
        messages[i][p] = '\0';
    }
    argv[trace_size - end_cutoff - cutoff + 3] = NULL;

    int f[2];
    pipe(f);
    int c = fork();
    if (c == 0) {
        close(f[0]);
        dup2(f[1], STDOUT_FILENO);
        close(f[1]);

        char const* envp[] = { NULL };

        execve(argv[0], (void const*)argv, (void const*)envp);
    }
    close(f[1]);
    wait(NULL);

    // extract each line separately and print
#define SZ 512
    char buf[SZ];
    size_t last_read;
    size_t read_to = 0;
    while ((last_read = read(f[0], &buf[read_to], SZ - read_to)) > 0) {
        size_t j = 0;
        size_t valid_buf_end = last_read + read_to;
        while (j < valid_buf_end) {
            if (buf[j] == '\n') {
                buf[j] = '\0';
                pretty_print_trace_entry(&buf[0]);
                memmove(&buf[0], &buf[j+1], valid_buf_end - 1 - j);
                valid_buf_end -= (j + 1);
                j = 0;
            } else
                j++;
        }
        read_to = valid_buf_end;
    }
    close(f[0]);

    exit(-1);
}

void init_instrument()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sighandler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
}
