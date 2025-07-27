/* ota.ld - Custom linker script for OTA header */
SECTIONS
{
    .ota_header :
    {
        KEEP(*(.ota_header))
    } > ota_header

    /* Other sections */
    .flash.text :
    {
        *(.text)
        *(.literal)
    } > iram0_0_seg

    /* ... */
}

/* Define the region for the OTA header */
MEMORY
{
    ota_header (RX) : ORIGIN = 0x10000, LENGTH = 0x100
    /* Other memory regions */
    iram0_0_seg (RX) : ORIGIN = 0x40080000, LENGTH = 0x20000
    /* ... */
}
