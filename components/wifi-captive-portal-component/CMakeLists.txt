# wifi-captive-portal-component
#
# Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>
#
# This code is released under the license terms contained in the
# file named LICENSE, which is found in the top-level folder in
# this project. You must agree to follow those license terms,
# otherwise you aren't allowed to copy, distribute, or use any
# part of this project in any way.
idf_component_register(
    SRCS
    "wifi-captive-portal.c"
    "wifi-captive-portal-dns.c"
    "wifi-captive-portal-httpd.c"
    "wifi-captive-portal-wifi.c"
    REQUIRES
    bootloader_support
    esp_http_server
    fatfs
    lwip
    esp_event
    esp_timer
    esp_netif
    esp_partition
    vfs
    http_parser
    esp_wifi

    INCLUDE_DIRS
    "include"
)
