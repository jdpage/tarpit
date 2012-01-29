#include <execinfo.h>
#include "barf.h"

#define STDERR 2

void barf() {
	void *buffer[100];
	int size;
	size = backtrace(buffer, 100);
	backtrace_symbols_fd(buffer, size, STDERR);
}
