#include "app.h"
#include "ota_header.h"
#include "esp_https_ota.h"
#include "version.h"

static const char *TAG = "OTA";

extern const char server_cert_pem_start[] asm("_binary_myothermind_pem_start");
extern const char server_cert_pem_end[] asm("_binary_myothermind_pem_end");

esp_err_t validate_image_header(esp_app_desc_t *new_app_info)
{
    if (new_app_info == NULL)
    {
        ESP_LOGE(TAG, "Failed to get new app description");
        return ESP_FAIL;
    }

    if (new_app_info->magic_word != 0xE9)
    {
        ESP_LOGE(TAG, "Invalid magic byte in image header (expected 0xE9, saw 0x%02lx)", new_app_info->magic_word);
        return ESP_FAIL;
    }

    return ESP_OK;
}

void dump_bytes(void *data, int len)
{
    uint8_t *byte_data = (uint8_t *)data;
    const int bytes_per_line = 16;
    char hex_str[3 * bytes_per_line + 1]; // 2 chars per byte + 1 space, plus null terminator
    char ascii_str[bytes_per_line + 1];   // 1 char per byte, plus null terminator

    for (int i = 0; i < len; i += bytes_per_line)
    {
        int hex_index = 0;
        int ascii_index = 0;

        // Build the hex and ASCII strings
        for (int j = 0; j < bytes_per_line; ++j)
        {
            if (i + j < len)
            {
                hex_index += sprintf(&hex_str[hex_index], "%02x ", byte_data[i + j]);
                char c = byte_data[i + j];
                ascii_str[ascii_index++] = (c >= 32 && c <= 126) ? c : '.';
            }
            else
            {
                hex_index += sprintf(&hex_str[hex_index], "   ");
                ascii_str[ascii_index++] = ' ';
            }
        }
        hex_str[hex_index] = '\0';
        ascii_str[ascii_index] = '\0';

        // Log the formatted line
        ESP_LOGI("DUMP", "%04x: %s  %s", i, hex_str, ascii_str);
    }
}

esp_err_t ota_http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        // dump_bytes(evt->data, evt->data_len);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        // dump_bytes(evt->data, evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGI(TAG, "HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}

void ota_update_task(void *pvParameter)
{
    ota_request_t *otaRequest = (ota_request_t *)pvParameter;

    esp_http_client_config_t config = {
        .url = CONFIG_APPLICATION_OTA_URL,
        .cert_pem = server_cert_pem_start, // Include the server certificate here
        .event_handler = ota_http_event_handler,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    esp_https_ota_handle_t ota_handle = NULL;
    esp_err_t ret = esp_https_ota_begin(&ota_config, &ota_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_https_ota_begin failed, error=%d", ret);
        vTaskDelete(NULL);
    }

    esp_app_desc_t new_app_info;
    ret = esp_https_ota_get_img_desc(ota_handle, &new_app_info);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_https_ota_get_img_desc failed, error=%d", ret);
        esp_https_ota_abort(ota_handle);
        vTaskDelete(NULL);
    }

    // Save our status so that it can be reported to the web page
    strncpy(appData.aucOtaNewVersion, new_app_info.version, sizeof(appData.aucOtaNewVersion));
    strncpy(appData.aucOtaImageDate, new_app_info.date, sizeof(appData.aucOtaImageDate));
    strncpy(appData.aucOtaImageTime, new_app_info.time, sizeof(appData.aucOtaImageTime));
    appData.nOtaBytesRead = 0;
    appData.nOtaImageSize = esp_https_ota_get_image_size(ota_handle);

    if ((otaRequest == NULL) || (otaRequest->eOtaCommand == OTA_QUERY))
    {
        esp_https_ota_abort(ota_handle);
        vTaskDelete(NULL);
    }

    while (1)
    {
        ret = esp_https_ota_perform(ota_handle);

        appData.nOtaBytesRead = esp_https_ota_get_image_len_read(ota_handle);

        if (ret != ESP_ERR_HTTPS_OTA_IN_PROGRESS)
        {
            break;
        }
        ESP_LOGI(TAG, "OTA in progress...");
    }

    if (esp_https_ota_is_complete_data_received(ota_handle))
    {
        ESP_LOGI(TAG, "OTA update successful, restarting...");
        esp_https_ota_finish(ota_handle);
        esp_restart();
    }
    else
    {
        ESP_LOGE(TAG, "OTA update failed");
        esp_https_ota_abort(ota_handle);
    }

    vTaskDelete(NULL);
}
