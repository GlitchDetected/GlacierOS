#pragma once

#include <fat.h>
#include <stdint.h>

typedef struct {
    uint8_t scratch_sector[SECTOR_SIZE];
    FILEINFO file_info;
} FILE;

FILE* fopen(const char* filename, const char* mode);
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);
int fclose(FILE* stream);
uint32_t fsize(FILE* stream);
