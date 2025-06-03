#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include <signum.h>
#include <stdint.h>
#include <reg.h>

enum sig_handle_type {
    SIG_DFL,
    SIG_IGN,
    SIG_CST,
    SIG_CST_EXT
};

typedef uint32_t sigset_t;

struct sigpending {
    sigset_t signal_pending;
};

struct k_sigaction {
    enum sig_handle_type handle_type;
    void (*k_handler)(int, struct pt_regs *);
    void (*u_handler)(int);
    void (*u_ext_handler)(int, void *);
};

struct signal_struct {
    struct sigpending pending;
    struct k_sigaction action[NSIG];
};

int do_kill(int pid, int sig);

void signal_init(int pid);

#endif /* _SIGNAL_H_ */
