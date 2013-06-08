#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>


void pezPrintStackTrace() {
  void *array[512];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 512);

  // print out all the frames to stderr
  //fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, 2);
}

