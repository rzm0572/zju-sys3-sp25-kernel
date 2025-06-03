#include "reg.h"
#include <stddef.h>
#include <signal.h>
#include <printk.h>
#include <proc.h>
#include <vm.h>
#include <mm.h>

extern struct task_struct *task[NR_TASKS];
extern struct wait_task wait_queue[NR_TASKS];
extern struct task_struct *current;
extern uint64_t num_tasks;

static int sig_handle_seq[] = {
    SIGSTOP,
    SIGINT,
    SIGCHLD
};

static char *sig_name[NSIG] = {
    [SIGSTOP] = "SIGSTOP",
    [SIGINT] = "SIGINT",
    [SIGCHLD] = "SIGCHLD"
};

void sigint_handler(int signum, struct pt_regs *regs) {
    (void)regs;
    printk(MSG("signal", "Caught signal %d: %s\n"), signum, sig_name[signum]);
    if (signum == SIGINT) {
        current->signal->pending.signal_pending &= ~(1 << signum);
        do_exit(0);
    }
}

void sigstop_handler(int signum, struct pt_regs *regs) {
    (void)regs;
    printk(MSG("signal", "Caught signal %d: %s\n"), signum, sig_name[signum]);
    if (signum == SIGSTOP) {
        current->signal->pending.signal_pending &= ~(1 << signum);
        current->state = TASK_STOPPED;
        schedule();
    }
}

void sigchld_handler(int signum, struct pt_regs *regs) {
    printk(MSG("signal", "Caught signal %d: %s\n"), signum, sig_name[signum]);
    if (signum == SIGCHLD) {
        int stop_waiting = 0;
        current->signal->pending.signal_pending &= ~(1 << signum);
        for (int i = 0; current->cpid[i] != 0; i++) {
            struct task_struct *child = task[current->cpid[i]];
            if (child != NULL && child->state == TASK_ZOMBIE) {
                delete_mapping(child->pgd);
                free_pages(child->pgd);
                regs->x[RISCV_REG_A0] = child->pid;
                regs->x[RISCV_REG_A1] = child->exit_code;
                free_pages(child);
                task[current->cpid[i]] = NULL;
                stop_waiting = 1;
                break;
            }
        }

        if (stop_waiting) {
            schedule();
        }
    }
}

struct k_sigaction dft_actions[NSIG] = {
    [SIGINT] = {.handle_type = SIG_DFL, .k_handler = sigint_handler, .u_handler = NULL},
    [SIGCHLD] = {.handle_type = SIG_DFL, .k_handler = sigchld_handler, .u_handler = NULL}
};

void signal_init(int pid) {
    if (task[pid]->signal == NULL) {
        task[pid]->signal = (struct signal_struct*)alloc_page();
    }

    struct signal_struct *signal = task[pid]->signal;
    signal->pending.signal_pending = 0;
    for (int i = 0; i < NSIG; i++) {
        signal->action[i].handle_type = SIG_DFL;
        signal->action[i].k_handler = NULL;
        signal->action[i].u_handler = NULL;
        signal->action[i].u_ext_handler = NULL;
    }
    
    signal->action[SIGINT].k_handler = sigint_handler;
    signal->action[SIGCHLD].k_handler = sigchld_handler;
    signal->action[SIGSTOP].k_handler = sigstop_handler;

    wait_queue[pid].valid = 0;
    wait_queue[pid].mask = 0ULL;

    printk(MSG("signal", "signal_init for task %d done!\n"), pid);
}

int do_kill(int pid, int sig) {
    if (pid < 0 || pid > (int)num_tasks) {
        return -1;
    }

    if (task[pid] == NULL) {
        return -1;
    }

    task[pid]->signal->pending.signal_pending |= (1 << sig);
    return 0;
}

void signal_handler(struct pt_regs *regs) {
    int sig_list_len = sizeof(sig_handle_seq) / sizeof(int);
    for (int i = 0; i < sig_list_len; i++) {
        int sig = sig_handle_seq[i];
        if (!(current->signal->pending.signal_pending & (1 << sig))) {
            continue;
        }
        struct k_sigaction *action = &current->signal->action[sig];
        switch (action->handle_type) {
            case SIG_DFL:
                if (action->k_handler != NULL) {
                    action->k_handler(sig, regs);
                }
                break;
            case SIG_IGN:
                break;
            case SIG_CST:
                if (action->u_handler != NULL) {
                    action->u_handler(sig);
                }
                break;
            case SIG_CST_EXT:
                if (action->u_ext_handler != NULL) {
                    action->u_ext_handler(sig, regs);
                }
                break;
            default:
                break;
        }
    }
}
