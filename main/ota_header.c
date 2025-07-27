// ota_header.c
#include "ota_header.h"
#include "version.h"

// Ensure this variable is placed at the beginning of the firmware image
__attribute__((section(".ota_header"))) const ota_header_t ota_header = {
    .version = FIRMWARE_VERSION,
};
