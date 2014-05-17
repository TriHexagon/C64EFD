#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#ifdef DEBUG

#include "types.h"

void debug_init(void);
void debug_putc(unsigned char c);
void debug_putData(const void* data, size_t size);
void debug_puts(const char* str);

#else

#define debug_init()
#define debug_putc(a)
#define debug_putData(a, b)
#define debug_puts(a)

#endif // DEBUG

#endif // DEBUG_H_INCLUDED
