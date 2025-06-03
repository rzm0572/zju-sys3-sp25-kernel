#ifndef _SIGNUM_H_
#define _SIGNUM_H_

#define NSIG 32

/* ISO C99 signals.  */
#define	SIGINT		2	/* Interactive attention signal.  */
#define	SIGILL		4	/* Illegal instruction.  */
#define	SIGABRT		6	/* Abnormal termination.  */
#define	SIGFPE		8	/* Erroneous arithmetic operation.  */
#define	SIGSEGV		11	/* Invalid access to storage.  */
#define	SIGTERM		15	/* Termination request.  */

/* Historical signals specified by POSIX. */
#define	SIGHUP		1	/* Hangup.  */
#define	SIGQUIT		3	/* Quit.  */
#define	SIGTRAP		5	/* Trace/breakpoint trap.  */
#define	SIGKILL		9	/* Killed.  */
#define	SIGPIPE		13	/* Broken pipe.  */
#define	SIGALRM		14	/* Alarm clock.  */

/* Adjustments and additions to the signal number constants for
   most Linux systems.  */
#define SIGSTKFLT	16	/* Stack fault (obsolete).  */
#define SIGPWR		30	/* Power failure imminent.  */

/* Historical signals specified by POSIX. */
#define SIGBUS		 7	/* Bus error.  */
#define SIGSYS		31	/* Bad system call.  */

/* New(er) POSIX signals (1003.1-2008, 1003.1-2013).  */
#define SIGURG		23	/* Urgent data is available at a socket.  */
#define SIGSTOP		19	/* Stop, unblockable.  */
#define SIGTSTP		20	/* Keyboard stop.  */
#define SIGCONT		18	/* Continue.  */
#define SIGCHLD		17	/* Child terminated or stopped.  */
#define SIGTTIN		21	/* Background read from control terminal.  */
#define SIGTTOU		22	/* Background write to control terminal.  */
#define SIGPOLL		29	/* Pollable event occurred (System V).  */
#define SIGXFSZ		25	/* File size limit exceeded.  */
#define SIGXCPU		24	/* CPU time limit exceeded.  */
#define SIGVTALRM	26	/* Virtual timer expired.  */
#define SIGPROF		27	/* Profiling timer expired.  */
#define SIGUSR1		10	/* User-defined signal 1.  */
#define SIGUSR2		12	/* User-defined signal 2.  */

/* Nonstandard signals found in all modern POSIX systems
   (including both BSD and Linux).  */
#define SIGWINCH	28	/* Window size change (4.3 BSD, Sun).  */

#endif /* _SIGNUM_H_ */
