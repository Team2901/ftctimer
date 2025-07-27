// ota_header.h
#pragma once

typedef struct {
    char version[16];
} ota_header_t;

extern const ota_header_t ota_header;
