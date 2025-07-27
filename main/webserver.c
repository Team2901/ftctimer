#include "app.h"
#include "version.h"
// Captive Portal Example
// https://github.com/CDFER/Captive-Portal-ESP32/blob/main/src/main.cpp
// https://github.com/cornelis-61/esp32_Captdns/blob/master/main/captdns.c

static const char *TAG = "wifi_ap";

static int connected = 0;
static bool g_bApMode = false;
static httpd_handle_t http_server = NULL;
static esp_event_handler_instance_t wifi_handler = NULL;
static esp_netif_t *default_sta_netif = NULL;
static ota_request_t otaRequest;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        int was_connected = connected;
        switch (event_id)
        {

        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "Disconnected from WiFi, retrying...");
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "Connected to the AP");
            check_dns();
            break;

        case WIFI_EVENT_AP_STACONNECTED:
        {
            connected++;
            wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
            ESP_LOGI(TAG, "Client connected: MAC=" MACSTR, MAC2STR(event->mac));
            break;
        }

        case WIFI_EVENT_AP_STADISCONNECTED:
        {
            connected--;
            wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
            ESP_LOGI(TAG, "Client disconnected: MAC=" MACSTR, MAC2STR(event->mac));
            break;
        }

        default:
            break;
        }
        if (connected != was_connected)
        {
            // if (connected == 0)
            // {
            //     TREE_LoadNamedPattern("APMode", 0);
            // }
            // else
            // {
            //     TREE_LoadNamedPattern("APConnected", 0);
            // }
        }
    }
}

static esp_err_t log_request_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Received request: URI=%s Method=%d", req->uri, req->method);

    // Log common headers
    size_t buf_len;
    char *buf;

    // Log the Host header
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "  Host: %s", buf);
        }
        free(buf);
    }

    // Log the User-Agent header
    buf_len = httpd_req_get_hdr_value_len(req, "User-Agent") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "User-Agent", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "  User-Agent: %s", buf);
        }
        free(buf);
    }
    return ESP_OK;
}

void wifi_init_sta(const char *ssid, const char *password)
{
    default_sta_netif = esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
        },
    };
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &wifi_handler));

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished ssid=%s password=%s.", ssid, password);
}

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename)
{
    ESP_LOGI(TAG, "Checking against %s", filename);
    if (ends_with(filename, ".pdf"))
    {
        ESP_LOGI(TAG, "application/pdf");
        return httpd_resp_set_type(req, "application/pdf");
    }
    else if (ends_with(filename, ".css"))
    {
        ESP_LOGI(TAG, "text/css");
        return httpd_resp_set_type(req, "text/css");
    }
    else if (ends_with(filename, ".js"))
    {
        ESP_LOGI(TAG, "text/javascript");
        return httpd_resp_set_type(req, "text/javascript");
    }
    else if (ends_with(filename, ".html"))
    {
        ESP_LOGI(TAG, "text/html");
        return httpd_resp_set_type(req, "text/html");
    }
    else if (ends_with(filename, ".htm"))
    {
        ESP_LOGI(TAG, "text/html");
        return httpd_resp_set_type(req, "text/html");
    }
    else if (ends_with(filename, ".jpeg"))
    {
        ESP_LOGI(TAG, "image/jpeg");
        return httpd_resp_set_type(req, "image/jpeg");
    }
    else if (ends_with(filename, ".ico"))
    {
        ESP_LOGI(TAG, "image/x-icon");
        return httpd_resp_set_type(req, "image/x-icon");
    }
    /* This is a limited set only */
    /* For any other type always set as plain text */
    ESP_LOGI(TAG, "text/plain");
    return httpd_resp_set_type(req, "text/plain");
}

static esp_err_t serve_spiffs_file(httpd_req_t *req, const char *filename)
{
    char filepath[ESP_VFS_PATH_MAX + 128];
    FILE *file = NULL;
    struct stat file_stat;

    // snprintf(filepath, sizeof(filepath), "/spiffs%s", req->uri);
    strcpy(filepath, "/spiffs");
    strncat(filepath, filename, sizeof(filepath) - 9);

    ESP_LOGI(TAG, "Looking to open '%s'\n", filepath);
    if (stat(filepath, &file_stat) == -1)
    {
        // We didn't find it, so see if we can find it with index.html appended
        strncat(filepath, "index.html", sizeof(filepath) - strlen(filepath) - 1);
        if (stat(filepath, &file_stat) == -1)
        {
            ESP_LOGI(TAG, "Sending 404\n");
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
    }

    file = fopen(filepath, "r");
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to read file: %s", filepath);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    set_content_type_from_file(req, filepath);
    char *chunk = malloc(1024);
    size_t chunksize;
    do
    {
        chunksize = fread(chunk, 1, 1024, file);
        if (chunksize > 0)
        {
            if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK)
            {
                fclose(file);
                free(chunk);
                httpd_resp_sendstr_chunk(req, NULL);
                httpd_resp_send_500(req);
                return ESP_FAIL;
            }
        }
    } while (chunksize != 0);

    fclose(file);
    free(chunk);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

typedef struct
{
    const char *name;
    rgb_t value;
} NamedColor;
static const NamedColor named_colors[] = {
    {"white", 0xFFFFFF},
    {"red", 0xFF0000},
    {"lime", 0x00FF00},
    {"blue", 0x0000FF},
    {"yellow", 0xFFFF00},
    {"aqua", 0x00FFFF},
    {"fuchsia", 0xFF00FF},
    {"silver", 0xC0C0C0},
    {"gray", 0x808080},
    {"maroon", 0x800000},
    {"olive", 0x808000},
    {"green", 0x008000},
    {"purple", 0x800080},
    {"teal", 0x008080},
    {"navy", 0x000080},
    {"orange", 0xFFA500},
    {"pink", 0xFFC0CB},
    {"brown", 0xA52A2A},
    {"gold", 0xFFD700},
    {"indigo", 0x4B0082},
    {"violet", 0xEE82EE},
    {"turquoise", 0x40E0D0},
    {"coral", 0xFF7F50},
    {"salmon", 0xFA8072},
    {"tomato", 0xFF6347},
    {"orchid", 0xDA70D6},
    {"khaki", 0xF0E68C},
    {"chocolate", 0xD2691E},
    {"crimson", 0xDC143C},
    {"darkorange", 0xFF8C00},
    {"lightgreen", 0x90EE90},
    {"skyblue", 0x87CEEB},
    {"black", 0x010101},
    {"darkgray", 0xA9A9A9},
    {"darkred", 0x8B0000},
    {"darkgreen", 0x006400},
    {"darkblue", 0x00008B},
    {"darkcyan", 0x008B8B},
    {"darkmagenta", 0x8B008B},
    {"darkgoldenrod", 0xB8860B},
    {"darkslategray", 0x2F4F4F},
    {"lightcoral", 0xF08080},
    {"lightsalmon", 0xFFA07A},
    {"lightseagreen", 0x20B2AA},
    {"lightskyblue", 0x87CEFA},
    {"lightsteelblue", 0xB0C4DE},
    {"mediumaquamarine", 0x66CDAA},
    {"mediumblue", 0x0000CD},
    {"mediumorchid", 0xBA55D3},
    {"mediumpurple", 0x9370DB},
    {"mediumseagreen", 0x3CB371},
    {"mediumslateblue", 0x7B68EE},
    {"mediumspringgreen", 0x00FA9A},
    {"mediumturquoise", 0x48D1CC},
    {"mediumvioletred", 0xC71585},
    {"darkred", 0x8B0000},
    {"darkgreen", 0x006400},
    {"darkblue", 0x00008B},
    {"darkgray", 0xA9A9A9},
    {"lightgray", 0xD3D3D3},
    {"lightblue", 0xADD8E6},
    {"lightpink", 0xFFB6C1},
    {"lightyellow", 0xFFFFE0},
    {"lightcyan", 0xE0FFFF},
    {"lightcoral", 0xF08080},
    {"lightsalmon", 0xFFA07A},
    {"lightseagreen", 0x20B2AA},
    {"lightskyblue", 0x87CEFA},
    {"lightsteelblue", 0xB0C4DE},
    {"mediumaquamarine", 0x66CDAA},
    {"mediumblue", 0x0000CD},
    {"mediumorchid", 0xBA55D3},
    {"mediumpurple", 0x9370DB},
    {"mediumseagreen", 0x3CB371},
    {"mediumslateblue", 0x7B68EE},
    {"mediumspringgreen", 0x00FA9A},
    {"mediumturquoise", 0x48D1CC},
    {"mediumvioletred", 0xC71585},
    {"midnightblue", 0x191970},
};

static void to_lower_str(char *dst, const char *src, size_t max_len)
{
    size_t i;
    for (i = 0; i < max_len - 1 && src[i]; i++)
    {
        dst[i] = (char)tolower((unsigned char)src[i]);
    }
    dst[i] = '\0';
}

rgb_t parse_rgb_hex(const char *input)
{
    if (!input)
        return RGB_WHITE;

    char lowered[64];
    to_lower_str(lowered, input, sizeof(lowered));
    const char *hex = lowered[0] == '#' ? &lowered[1] : lowered;

    // 1. Check for named color
    for (size_t i = 0; i < sizeof(named_colors) / sizeof(named_colors[0]); i++)
    {
        if (strcmp(hex, named_colors[i].name) == 0)
        {
            return named_colors[i].value;
        }
    }

    // 2. Check for rgb(R,G,B)
    if (strncmp(lowered, "rgb(", 4) == 0)
    {
        int r, g, b;
        if (sscanf(lowered, "rgb(%d,%d,%d)", &r, &g, &b) != 3)
        {
            // Try with whitespace
            if (sscanf(lowered, "rgb( %d , %d , %d )", &r, &g, &b) != 3)
                return RGB_WHITE;
        }

        if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
        {
            return RGB_WHITE;
        }
        return (r << 16) | (g << 8) | b;
    }

    // 3. 6-digit hex
    if (strlen(hex) == 6)
    {
        for (int i = 0; i < 6; i++)
        {
            if (!isxdigit((unsigned char)hex[i]))
                return RGB_WHITE;
        }
        char r_str[3] = {hex[0], hex[1], '\0'};
        char g_str[3] = {hex[2], hex[3], '\0'};
        char b_str[3] = {hex[4], hex[5], '\0'};

        uint32_t r = strtoul(r_str, NULL, 16);
        uint32_t g = strtoul(g_str, NULL, 16);
        uint32_t b = strtoul(b_str, NULL, 16);

        return (r << 16) | (g << 8) | b;
    }

    // 4. 3-digit shorthand hex
    if (strlen(hex) == 3)
    {
        char r_str[3] = {hex[0], hex[0], '\0'};
        char g_str[3] = {hex[1], hex[1], '\0'};
        char b_str[3] = {hex[2], hex[2], '\0'};

        if (!isxdigit((unsigned char)r_str[0]) || !isxdigit((unsigned char)g_str[0]) || !isxdigit((unsigned char)b_str[0]))
            return RGB_WHITE;

        uint32_t r = strtoul(r_str, NULL, 16);
        uint32_t g = strtoul(g_str, NULL, 16);
        uint32_t b = strtoul(b_str, NULL, 16);

        return (r << 16) | (g << 8) | b;
    }

    return RGB_WHITE;
}
static void url_decode(char *dst, const char *src, size_t max_len)
{
    char a, b;
    size_t i = 0;
    while (*src && i + 1 < max_len)
    {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            isxdigit(a) && isxdigit(b))
        {
            if (i + 1 >= max_len)
                break;
            *dst++ = (char)((isdigit(a) ? a - '0' : tolower(a) - 'a' + 10) << 4 |
                            (isdigit(b) ? b - '0' : tolower(b) - 'a' + 10));
            src += 3;
        }
        else
        {
            *dst++ = *src++;
        }
        i++;
    }
    *dst = '\0';
}
static esp_err_t display_handler(httpd_req_t *req)
{
    char text[32], color[32];
    size_t ssid_len = httpd_req_get_url_query_len(req) + 1;
    char *buf = malloc(ssid_len);
    if (httpd_req_get_url_query_str(req, buf, ssid_len) == ESP_OK)
    {
        char text_raw[40], color_raw[32];
        httpd_query_key_value(buf, "text", text_raw, sizeof(text_raw));
        httpd_query_key_value(buf, "color", color_raw, sizeof(color_raw));
        url_decode(text, text_raw, sizeof(text));
        url_decode(color, color_raw, sizeof(color));

        if (strlen(color) > 31)
        {
            ESP_LOGE(TAG, "Invalid color input!");
            strcpy(color, "white"); // fallback
        }

        ESP_LOGI(TAG, "TEXT: %s, Color: %s", text, color);

        // Interpret the text input
        if (text[1] == ':' && (strlen(text) <= 4 || (strlen(text) == 5 && text[3] == '.')))
        {
            strncpy(text + strlen(text), "  ", 3);
            text[5] = '\0';
            strncpy(appData.scrollMessage, text, MAX_SCROLL_MESSAGE);
            appData.scrollMessage[MAX_SCROLL_MESSAGE] = '\0';
            appData.bScrollable = false;
        }
        else if ((text[0] == '.' && strlen(text) <= 2) ||
                 (text[1] == '.' && strlen(text) <= 3) ||
                 (text[2] == '.' && strlen(text) <= 4))
        {
            int nOffset = 0;
            if (text[0] == '.')
                nOffset = 2;
            else if (text[1] == '.')
                nOffset = 1;
            else if (text[2] == '.')
                nOffset = 2;

            strncpy(appData.scrollMessage, "  . ", MAX_SCROLL_MESSAGE);
            memcpy(appData.scrollMessage + nOffset, text, strlen(text));
            appData.scrollMessage[MAX_SCROLL_MESSAGE] = '\0';
            appData.bScrollable = false;
        }
        else if (strlen(text) <= 3)
        {
            int nOffset = 3 - strlen(text);
            strncpy(appData.scrollMessage, "   ", MAX_SCROLL_MESSAGE);
            memcpy(appData.scrollMessage + nOffset, text, strlen(text));
            appData.scrollMessage[MAX_SCROLL_MESSAGE] = '\0';
            appData.bScrollable = false;
        }
        else
        {
            strncpy(appData.scrollMessage, text, MAX_SCROLL_MESSAGE);
            appData.scrollMessage[MAX_SCROLL_MESSAGE] = '\0';
            appData.bScrollable = true;
        }

        // Parse color
        appData.scrollColor = parse_rgb_hex(color);

        // Build response string
        char response[128];
        snprintf(response, sizeof(response),
                 "Message: \"%s\"\nColor: #%02X%02X%02X\nScrolling: %s\n",
                 appData.scrollMessage,
                 (unsigned int)((appData.scrollColor >> 16) & 0xFF),
                 (unsigned int)((appData.scrollColor >> 8) & 0xFF),
                 (unsigned int)(appData.scrollColor & 0xFF),
                 appData.bScrollable ? "true" : "false");

        httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    }
    else
    {
        httpd_resp_send_500(req);
    }

    free(buf);
    return ESP_OK;
}

static esp_err_t config_set_handler(httpd_req_t *req)
{
    char ssid[32], password[32];
    size_t ssid_len = httpd_req_get_url_query_len(req) + 1;
    char *buf = malloc(ssid_len);
    if (httpd_req_get_url_query_str(req, buf, ssid_len) == ESP_OK)
    {
        httpd_query_key_value(buf, "ssid", ssid, sizeof(ssid));
        httpd_query_key_value(buf, "password", password, sizeof(password));

        nvs_handle_t nvs_handle;
        esp_err_t ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
        if (ret == ESP_OK)
        {
            nvs_set_str(nvs_handle, "ssid", ssid);
            nvs_set_str(nvs_handle, "password", password);
            nvs_commit(nvs_handle);
            nvs_close(nvs_handle);
        }
        ESP_LOGI(TAG, "SSID: %s, Password: %s", ssid, password);
    }
    free(buf);
    httpd_resp_send(req, "Settings saved! Rebooting...", HTTPD_RESP_USE_STRLEN);
    esp_restart();
    return ESP_OK;
}

static esp_err_t status_handler(httpd_req_t *req)
{
    // We are returning {"apmode": true}
    char *res = "{\"apmode\":false}";
    if (g_bApMode)
    {
        res = "{\"apmode\":true}";
    }
    httpd_resp_send(req, res, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t ssids_handler(httpd_req_t *req)
{
    int found = wifi_captive_portal_get_scan_count();
    char *out = NULL;
    // We are returning:
    //  [["ssid",rssi,0/1],["ssid",rssi,0/1]]
    // This works out to:
    //    Overhead of [] 3 bytes with the final null
    //    each record uses
    //       3  bytes for [], (except the last one)
    //       32 bytes for the ssid
    //       3 bytes for ssid quote and comma
    //       4 bytes for rssi (positive only) and comma
    //       1 byte for open/closed indicator
    char *res = malloc(3 +
                       (found *
                        (3    //  bytes for [], (except the last one)
                         + 32 // bytes for the ssid
                         + 3  // bytes for ssid quote and comma
                         + 4  // bytes for rssi (positive only) and comma
                         + 1  // byte for open/closed indicator
                         )));
    // If we can't get the memory then we have to bail
    if (res == NULL)
    {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    out = res;
    *out++ = '[';
    // Now we need to go through and fill in the result
    for (int i = 0; i < found; i++)
    {
        wifi_ap_info_t rec;
        if (wifi_captive_portal_get_scan_result(i, &rec))
        {
            if (i > 0)
            {
                *out++ = ',';
            }
            out += sprintf(out, "[\"%s\',%d,%d]", rec.ssid, abs(rec.rssi), rec.authmode == WIFI_AUTH_OPEN ? 0 : 1);
        }
    }
    // Give it the final ]
    strcpy(out, "]");
    httpd_resp_send(req, res, HTTPD_RESP_USE_STRLEN);
    free(res);
    return ESP_OK;
}

/* Copies the full path into destination buffer and returns
 * pointer to path (skipping the preceding base path) */
static const char *get_path_from_uri(char *dest, const char *uri, size_t destsize)
{
    size_t pathlen = strlen(uri);

    const char *quest = strchr(uri, '?');
    if (quest)
    {
        pathlen = MIN(pathlen, quest - uri);
    }
    const char *hash = strchr(uri, '#');
    if (hash)
    {
        pathlen = MIN(pathlen, hash - uri);
    }

    if (pathlen + 1 > destsize)
    {
        /* Full path string won't fit into destination buffer */
        return NULL;
    }

    /* Construct full path (base + path) */
    strlcpy(dest, uri, pathlen + 1);

    /* Return pointer to path, skipping the base */
    return dest;
}

static esp_err_t locateFile(httpd_req_t *req, const char *uriBase, char *filepath, int nFilePathLen)
{
    struct stat file_stat;
    char filetemp[FILE_PATH_MAX];

    /* Skip leading portion from URI to get filename */
    const char *filename = get_path_from_uri(filetemp, req->uri + strlen(uriBase), sizeof(filetemp));
    ESP_LOGI(TAG, "File '%s'", filename);

    if (!filename)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
    }

    /* Filename cannot have a trailing '/' */
    if (filename[strlen(filename) - 1] == '/')
    {
        ESP_LOGE(TAG, "Invalid filename : %s", filename);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Invalid filename");
        return ESP_FAIL;
    }

    if (!makeSpiffsFileName(filename, filepath, nFilePathLen))
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Unable to make filename");
        return ESP_FAIL;
    }
    if (stat(filepath, &file_stat) == 0)
    {
        ESP_LOGE(TAG, "File already exists : %s", filepath);
        // We assume that the caller will handle this case for exampe
        // httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File already exists");
        return ESP_OK;
    }
    return ESP_ERR_NOT_FOUND;
}
/* Handler to delete a pattern from the server */
static esp_err_t delete_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];

    esp_err_t res = locateFile(req, "/delete/", filepath, sizeof(filepath));

    if (res == ESP_ERR_NOT_FOUND)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File does not exist");
        return ESP_OK;
    }
    // Ok we can delete the file!
    if (unlink(filepath) != 0)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Unable to delete file");
        return ESP_FAIL;
    }
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* Handler to upload a file onto the server */
static esp_err_t upload_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];
    FILE *fd = NULL;
    ESP_LOGI(TAG, "Request '%s'", req->uri);

    esp_err_t res = locateFile(req, "/upload/", filepath, sizeof(filepath));

    if (res == ESP_FAIL)
    {
        return res;
    }
    if (res == ESP_OK)
    {
        ESP_LOGE(TAG, "File already exists : %s", filepath);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File already exists");
        return ESP_FAIL;
    }

    /* File cannot be larger than a limit */
    if (req->content_len > MAX_FILE_SIZE)
    {
        ESP_LOGE(TAG, "File too large : %d bytes", req->content_len);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                            "File size must be less than " MAX_FILE_SIZE_STR "!");
        /* Return failure to close underlying connection else the
         * incoming file content will keep the socket busy */
        return ESP_FAIL;
    }

    fd = fopen(filepath, "w");
    if (!fd)
    {
        ESP_LOGE(TAG, "Failed to create file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Receiving file : %s...", filepath);

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *buf = malloc(SCRATCH_BUFSIZE);
    int received;

    /* Content length of the request gives
     * the size of the file being uploaded */
    int remaining = req->content_len;

    while (remaining > 0)
    {

        ESP_LOGI(TAG, "Remaining size : %d", remaining);
        /* Receive the file part by part into a buffer */
        if ((received = httpd_req_recv(req, buf, MIN(remaining, SCRATCH_BUFSIZE))) <= 0)
        {
            if (received == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* Retry if timeout occurred */
                continue;
            }

            /* In case of unrecoverable error,
             * close and delete the unfinished file*/
            fclose(fd);
            unlink(filepath);

            ESP_LOGE(TAG, "File reception failed!");
            free(buf);
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
            return ESP_FAIL;
        }

        /* Write buffer content to file on storage */
        if (received && (received != fwrite(buf, 1, received, fd)))
        {
            /* Couldn't write everything to file!
             * Storage may be full? */
            fclose(fd);
            unlink(filepath);

            ESP_LOGE(TAG, "File write failed!");
            free(buf);
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to write file to storage");
            return ESP_FAIL;
        }

        /* Keep track of remaining size of
         * the file left to be uploaded */
        remaining -= received;
    }

    /* Close file upon upload completion */
    fclose(fd);
    ESP_LOGI(TAG, "File reception complete");
    free(buf);

    // list_spiffs_files();

    /* Redirect onto root to see the updated file list */
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
    httpd_resp_set_hdr(req, "Connection", "close");
#endif
    httpd_resp_sendstr(req, "File uploaded successfully");
    return ESP_OK;
}

static esp_err_t ota_update_handler(httpd_req_t *req, OtaCommand eOtaCommand)
{
    otaRequest.eOtaCommand = eOtaCommand;
    // Implement OTA logic here
    xTaskCreate(&ota_update_task, "ota_update_task", 8192, &otaRequest, 5, NULL);

    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t ota_query_handler(httpd_req_t *req)
{
    return ota_update_handler(req, OTA_QUERY);
}
static esp_err_t ota_execute_handler(httpd_req_t *req)
{
    return ota_update_handler(req, OTA_EXECUTE);
}

static esp_err_t otastatus_handler(httpd_req_t *req)
{
    char res[256];
    snprintf(res, sizeof(res), "{"
                               "\"current\":\"%s\","
                               "\"new\":\"%s\","
                               "\"date\":\"%s\","
                               "\"time\":\"%s\","
                               "\"read\":\"%d\","
                               "\"size\":\"%d\""
                               "}",

             "v" FIRMWARE_VERSION,
             appData.aucOtaNewVersion,
             appData.aucOtaImageDate,
             appData.aucOtaImageTime,
             appData.nOtaBytesRead,
             appData.nOtaImageSize);
    return httpd_resp_send(req, res, HTTPD_RESP_USE_STRLEN);
}
static esp_err_t wifi_scan_handler(httpd_req_t *req)
{
    wifi_captive_portal_wifi_scan(true);
    return serve_spiffs_file(req, "/wifi.html");
}

// Define the function pointer type for the handler functions
typedef esp_err_t (*httpd_handler_t)(httpd_req_t *req);

// Define the HandlerLookup structure
typedef struct
{
    const char *uri_pattern;
    httpd_handler_t handler;
} HandlerLookup;

const HandlerLookup HandlerMap[] = {
    {"/set*", config_set_handler},
    {"/status", status_handler},
    {"/otaquery", ota_query_handler},
    {"/ota", ota_execute_handler},
    {"/ssids", ssids_handler},
    {"/upload*", upload_handler},
    {"/otastatus", otastatus_handler},
    {"/delete/*", delete_handler},
    {"/scan", wifi_scan_handler},
    {"/display*", display_handler},
};
/**
 * @brief Routing handler for http requests
 *
 * @param req
 * @return esp_err_t
 */
static esp_err_t common_handler(httpd_req_t *req)
{
    log_request_handler(req);
    const size_t urilen = strlen(req->uri);
    // Look up the handlers in our own table.  Note that we can't use the system registration mechanism
    // because we don't get a good hook with the captive portal component
    for (size_t i = 0; i < sizeof(HandlerMap) / sizeof(HandlerMap[0]); i++)
    {
        if ((strcmp(req->uri, HandlerMap[i].uri_pattern) == 0) ||
            httpd_uri_match_wildcard(HandlerMap[i].uri_pattern, req->uri, urilen))
        {
            return HandlerMap[i].handler(req);
        }
    }

    // We didn't recognize it as one of the special ones, so try to serve a file for it.
    return serve_spiffs_file(req, req->uri);
}

void register_common_handler(httpd_handle_t server)
{
    httpd_uri_t uri_handler = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = common_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(server, &uri_handler);

    uri_handler.method = HTTP_POST;
    httpd_register_uri_handler(server, &uri_handler);

    // Add other HTTP methods if needed
}

void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.lru_purge_enable = true;

    if (httpd_start(&http_server, &config) == ESP_OK)
    {
        ESP_LOGI(TAG, "Web Server Started");
        register_common_handler(http_server);
    }
    else
    {
        http_server = NULL;
    }
}

void stop_webserver(void)
{
    if (http_server != NULL)
    {
        // Unregister all the handlers
        httpd_unregister_uri(http_server, "/*");
        // And stop the server
        ESP_ERROR_CHECK(
            httpd_stop(http_server));
        http_server = NULL;
    }
    // Stop the wifi if it is already running
    esp_err_t wifi_res;
    wifi_res = esp_wifi_stop(); // Stop the Wi-Fi driver
    if (wifi_res != ESP_ERR_WIFI_NOT_INIT && wifi_res != ESP_OK)
    {
        printf("esp_wifi_stop error: %s\n", __STRINGIFY(wifi_res));
    }

    if (default_sta_netif != NULL)
    {
        esp_netif_destroy_default_wifi(default_sta_netif);
        default_sta_netif = NULL;
    }
}

void stop_mdns_service(void)
{
    mdns_free();
}

void start_mdns_service(void)
{
    mdns_init();
    mdns_hostname_set("FTCTIMER");
    mdns_instance_name_set("FTC Timer");

    mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);

    ESP_LOGI(TAG, "mDNS service started");
}

/**
 * @brief Start up the Captive Portal Access Point
 * This is called when you hold the button long enough to get into configuration mode.
 * It shuts down any existing web server and then starts the Access Point Captive portal code
 */
void AP_Start()
{
    stop_webserver();
    stop_mdns_service();

    if (wifi_handler != NULL)
    {
        ESP_ERROR_CHECK(
            esp_event_handler_instance_unregister(WIFI_EVENT,
                                                  ESP_EVENT_ANY_ID,
                                                  wifi_handler));
        wifi_handler = NULL;
    }

    // Register the Wi-Fi event handler
    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(WIFI_EVENT,
                                            ESP_EVENT_ANY_ID,
                                            &wifi_event_handler,
                                            NULL,
                                            NULL));

    g_bApMode = true;

    wifi_captive_portal(common_handler);
    start_mdns_service();
}

/**
 * @brief Initialize the Wifi environment
 * This is called when the tree starts initially.  If there are wifi credentials stored
 * in the NVS, then use those to attempt to connect to that network.  Otherwise
 * We just initialize the basic Wifi but don't actually connect to anything.
 */
void Wifi_Init(void)
{
    esp_log_level_set("mdns", ESP_LOG_VERBOSE);
    g_bApMode = false;
    // esp_log_level_set("httpd_txrx", ESP_LOG_VERBOSE);
    // esp_log_level_set("httpd_uri", ESP_LOG_VERBOSE);
    // esp_log_level_set("*", ESP_LOG_VERBOSE);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(
            nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(
        esp_netif_init());
    ESP_ERROR_CHECK(
        esp_event_loop_create_default());

    start_mdns_service();

    nvs_handle_t nvs_handle;
    ret = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error opening NVS handle!");
    }
    else
    {
        size_t required_size;
        char ssid[32] = "toebes";
        char password[64] = "toebes42";

        // Check if SSID and password are stored
        ret = nvs_get_str(nvs_handle, "ssid", NULL, &required_size);
        if (ret == ESP_OK)
        {
            ret = nvs_get_str(nvs_handle, "ssid", ssid, &required_size);
        }
        ret = nvs_get_str(nvs_handle, "password", NULL, &required_size);
        if (ret == ESP_OK)
        {
            ret = nvs_get_str(nvs_handle, "password", password, &required_size);
        }
        nvs_close(nvs_handle);

        if (ret == ESP_OK || strlen(ssid) > 0)
        {
            // Attempt to connect to the saved Wi-Fi network
            wifi_init_sta(ssid, password);
            start_webserver();
        }
        else
        {
            // Initialize Wi-Fi driver
            wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
            ESP_ERROR_CHECK(
                esp_wifi_init(&cfg));
            ESP_LOGI(TAG, "No stored Wi-Fi credentials found.");
        }
    }
}
