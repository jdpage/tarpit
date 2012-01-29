#pragma once

// error return codes

#define OK 0
#define COMPILE_ERROR 1

// error messages

#define PRINT_ERROR(...) fprintf(stderr, __VA_ARGS__)

#define MSG_OUT_OF_MEMORY "ran out of memory\n"
#define MSG_WRITE_OUT_OF_BOUNDS "tried to write off end of array. Accessed %u, limit %u\n"
#define MSG_READ_OUT_OF_BOUNDS "tried to read beyond end of array. Accessed %u, limit %u\n"
#define MSG_WRITE_ERROR "writing file failed\n"
#define MSG_POPPED_EMPTY_STACK "tried to pop empty stack\n"
#define MSG_UNMATCHED_CLOSE_BRACKET "compile error: unmatched ] token!\n"
#define MSG_UNMATCHED_OPEN_BRACKETS "compile error: found %d unmatched [ tokens!\n"

// constants

#define MEMORY_ORG 0x08048000
#define PAGE_SIZE 0x1000
