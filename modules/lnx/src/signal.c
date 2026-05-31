#include "lnx/def.h"
#include "lnx/signal.h"

#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

struct lnx_signal_guard {
    struct sigaction saved[NSIG];
    size_t           n;
};

LNX_API lnx_signal_guard_t *
lnx_signal_guard_init(void)
{
    lnx_signal_guard_t *g = calloc(1, sizeof(*g));
    return g;
}

LNX_API void
lnx_signal_guard_free(lnx_signal_guard_t *g)
{
    free(g);
}

LNX_API int
lnx_signal_guard_capture(lnx_signal_guard_t *g, int signum)
{
    if (!g || signum < 1 || signum >= NSIG)
        return -1;
    if (sigaction(signum, NULL, &g->saved[signum]) != 0)
        return -1;
    if (g->n <= (size_t)signum)
        g->n = (size_t)signum + 1;
    return 0;
}

LNX_API int
lnx_signal_guard_restore(lnx_signal_guard_t *g, int signum)
{
    if (!g || signum < 1 || signum >= NSIG)
        return -1;
    return sigaction(signum, &g->saved[signum], NULL);
}

LNX_API int
lnx_signal_guard_restore_all(lnx_signal_guard_t *g)
{
    if (!g)
        return -1;
    int ret = 0;
    for (size_t i = 1; i < g->n; i++) {
        if (g->saved[i].sa_handler != SIG_DFL) {
            if (sigaction((int)i, &g->saved[i], NULL) != 0)
                ret = -1;
        }
    }
    return ret;
}

LNX_API int
lnx_signal_block(int signum)
{
    sigset_t ss;
    sigemptyset(&ss);
    sigaddset(&ss, signum);
    return pthread_sigmask(SIG_BLOCK, &ss, NULL);
}

LNX_API int
lnx_signal_unblock(int signum)
{
    sigset_t ss;
    sigemptyset(&ss);
    sigaddset(&ss, signum);
    return pthread_sigmask(SIG_UNBLOCK, &ss, NULL);
}

LNX_API int
lnx_signal_ignore(int signum)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    return sigaction(signum, &sa, NULL);
}

LNX_API int
lnx_signal_default(int signum)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    return sigaction(signum, &sa, NULL);
}

LNX_API int
lnx_signal_name(int signum, char *buf, size_t size)
{
    const char *name = NULL;
    switch (signum) {
        case SIGHUP:    name = "SIGHUP";    break;
        case SIGINT:    name = "SIGINT";    break;
        case SIGQUIT:   name = "SIGQUIT";   break;
        case SIGILL:    name = "SIGILL";    break;
        case SIGABRT:   name = "SIGABRT";   break;
        case SIGFPE:    name = "SIGFPE";    break;
        case SIGKILL:   name = "SIGKILL";   break;
        case SIGSEGV:   name = "SIGSEGV";   break;
        case SIGPIPE:   name = "SIGPIPE";   break;
        case SIGALRM:   name = "SIGALRM";   break;
        case SIGTERM:   name = "SIGTERM";   break;
        case SIGUSR1:   name = "SIGUSR1";   break;
        case SIGUSR2:   name = "SIGUSR2";   break;
        case SIGCHLD:   name = "SIGCHLD";   break;
        case SIGCONT:   name = "SIGCONT";   break;
        case SIGSTOP:   name = "SIGSTOP";   break;
        case SIGTSTP:   name = "SIGTSTP";   break;
        case SIGTTIN:   name = "SIGTTIN";   break;
        case SIGTTOU:   name = "SIGTTOU";   break;
        case SIGSYS:    name = "SIGSYS";    break;
        default:
            if (signum >= SIGRTMIN && signum <= SIGRTMAX) {
                snprintf(buf, size, "SIGRTMIN+%d", signum - SIGRTMIN);
                return 0;
            }
            snprintf(buf, size, "SIGUNKNOWN(%d)", signum);
            return 0;
    }
    snprintf(buf, size, "%s", name);
    return 0;
}
