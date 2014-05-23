#ifndef SD_H_INCLUDED
#define SD_H_INCLUDED

#include "types.h"

result_t sd_init(void);
result_t sd_readBlock(void* buffer, size_t size, u32 address);

#endif // SD_H_INCLUDED
