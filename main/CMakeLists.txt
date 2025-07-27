idf_component_register(
    SRCS
    "filesystem.c"
    "webserver.c"
    "main.c"
    "app.c"
    "ota.c"
    "ota_header.c"
    "dnscheck.c"

    REQUIRES
    esp_wifi
    esp_http_server
    mdns
    nvs_flash
    http_parser
    touch_element
    spiffs
    fatfs
    esp_http_client
    esp_https_ota
    app_update
    esp_netif
    lwip
    mbedtls
    esp_adc

    wifi-captive-portal-component

    INCLUDE_DIRS
    "."

    EMBED_TXTFILES ../myothermind.pem
)

# Set components directory with proper escaping
set(COMPONENT_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/../components"
)

# set(CMAKE_C_FLAGS
# "${CMAKE_C_FLAGS} -DLOG_LOCAL_LEVEL=ESP_LOG_ERROR"
# )
