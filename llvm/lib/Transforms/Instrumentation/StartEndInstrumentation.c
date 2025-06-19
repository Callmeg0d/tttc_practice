#include <stdio.h>

void instrument_start(const char* func_name) {
    fprintf(stderr, "[START] Function: %s\n", func_name);
}

void instrument_end(const char* func_name) {
    fprintf(stderr, "[END] Function: %s\n", func_name);
}