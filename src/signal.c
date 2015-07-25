#include "signal.h"
#include "log.h"

#include <signal.h>
#include <fenv.h>

extern int feenableexcept(int flags);

void tds_signal_segv(int p) {
	tds_logf(TDS_LOG_CRITICAL, "Received SIGSEGV signal, terminating immediately.\n");
}

void tds_signal_kill(int p) {
	tds_logf(TDS_LOG_CRITICAL, "Received SIGKILL or SIGSTOP signal, terminating immediately.\n");
}

void tds_signal_fpe(int p) {
	tds_logf(TDS_LOG_CRITICAL, "Received SIGFPE signal, terminating immediately.\n");
}

void tds_signal_init(void) {
	feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW);

	signal(SIGSEGV, tds_signal_segv);
	signal(SIGKILL, tds_signal_kill);
	signal(SIGSTOP, tds_signal_kill);
	signal(SIGFPE, tds_signal_fpe);
}
