/**	wifi-captive-portal-component

  Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

  This code is released under the license terms contained in the
  file named LICENSE, which is found in the top-level folder in
  this project. You must agree to follow those license terms,
  otherwise you aren't allowed to copy, distribute, or use any
  part of this project in any way.

  Contains some modified example code from here:
  https://github.com/espressif/esp-idf/blob/release/v4.2/examples/system/ota/advanced_https_ota/main/advanced_https_ota_example.c

  Original Example Code Header:
  This code is in the Public Domain (or CC0 licensed, at your option.)

  Unless required by applicable law or agreed to in writing, this
  software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
  CONDITIONS OF ANY KIND, either express or implied.
*/
#include "wifi-captive-portal.h"

#define DEFAULT_SCAN_LIST_SIZE 10 // Define the number of APs to store

static const char *TAG = "wifi-captive-portal-wifi";

esp_event_loop_handle_t wifi_captive_portal_wifi_event_loop_handle;

ESP_EVENT_DEFINE_BASE(WIFI_CAPTIVE_PORTAL_WIFI_EVENT);

static wifi_ap_info_t scan_result[CONFIG_CAPTIVE_PORTAL_MAX_SCAN_SSIDS];
static EventGroupHandle_t wifi_event_group;

void store_ap_info(const wifi_ap_record_t *ap_info)
{
    int min_rssi_index = -1;
    int min_rssi_value = 127; // RSSI is typically between -100 and 0

    // Find the position to insert the new entry based on RSSI value
    for (int i = 0; i < CONFIG_CAPTIVE_PORTAL_MAX_SCAN_SSIDS; i++)
    {
        if (scan_result[i].ssid[0] == 0) // Empty slot
        {
            min_rssi_index = i;
            break;
        }
        // Have we already seen this SSID?
        if (!strncmp((const char *)(scan_result[i].ssid), (const char *)(ap_info->ssid), sizeof(scan_result[min_rssi_index].ssid) - 1))
        {
            if (ap_info->rssi > scan_result[i].rssi)
            {
                min_rssi_index = i;
            }
            else
            {
                min_rssi_index = -1;
            }
            break;
        }
        if (scan_result[i].rssi < min_rssi_value)
        {
            min_rssi_value = scan_result[i].rssi;
            min_rssi_index = i;
        }
    }

    // Replace the entry with the lowest RSSI or an empty slot
    if (min_rssi_index != -1 && (scan_result[min_rssi_index].ssid[0] == 0 || ap_info->rssi > scan_result[min_rssi_index].rssi))
    {
        strncpy((char *)scan_result[min_rssi_index].ssid, (char *)ap_info->ssid, sizeof(scan_result[min_rssi_index].ssid) - 1);
        scan_result[min_rssi_index].ssid[sizeof(scan_result[min_rssi_index].ssid) - 1] = '\0'; // Ensure null termination
        scan_result[min_rssi_index].rssi = ap_info->rssi;
        scan_result[min_rssi_index].authmode = ap_info->authmode;
    }
}
/**
 * @brief Process the result of a wifi scan event
 *
 * @param arg
 * @param event_base
 * @param event_id
 * @param event_data
 */
static void wifi_scan_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    int total = 0;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE)
    {
        wifi_ap_record_t ap_info;

        memset((void *)scan_result, 0, sizeof(scan_result));
        while (esp_wifi_scan_get_ap_record(&ap_info) == ESP_OK)
        {
            // ESP_LOGI(TAG, "SSID '%s' RSSI: %d Channel %d Authmode %d", ap_info.ssid, ap_info.rssi, ap_info.primary, ap_info.authmode);
            store_ap_info(&ap_info);
        }

        for (int i = 0; i < CONFIG_CAPTIVE_PORTAL_MAX_SCAN_SSIDS; i++)
        {
            if (scan_result[i].ssid[0])
            {
                ESP_LOGI(TAG, "---SSID '%s' RSSI: %d Authmode %d", scan_result[i].ssid, scan_result[i].rssi, scan_result[i].authmode);
                total++;
            }
        }
        ESP_LOGI(TAG, "%d SSIDs found", total);
    }
}

/**
 * @brief Initialize the captive portal
 * NOTE: use menuconfig to set the SSID/CHANNEL/PASSWORD/MAX_AP_CONNECTION values
 */
static void wifi_captive_portal_wifi_ap_init()
{
    char decoratedSSID[33];
    uint8_t wifiMac[6];
    esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, wifiMac);
    if (ret == ESP_OK)
    {
        sprintf(decoratedSSID, "%s%02x:%02x:%02x\n",
                CONFIG_CAPTIVE_PORTAL_WIFI_AP_SSID,
                wifiMac[3], wifiMac[4], wifiMac[5]);
    }
    else
    {
        sprintf((char *)decoratedSSID, "%snoMAC\n",
                CONFIG_CAPTIVE_PORTAL_WIFI_AP_SSID);
    };
    wifi_config_t wifi_config_ap = {
        .ap = {
            .ssid = "",
            .ssid_len = strlen(decoratedSSID),
            .channel = CONFIG_CAPTIVE_PORTAL_WIFI_AP_CHANNEL,
            .password = CONFIG_CAPTIVE_PORTAL_WIFI_AP_PASSWORD,
            .max_connection = CONFIG_CAPTIVE_PORTAL_MAX_AP_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };
    strncpy((char *)wifi_config_ap.ap.ssid, (char *)decoratedSSID, sizeof(wifi_config_ap.ap.ssid) - 1);
    wifi_config_ap.ap.ssid[sizeof(wifi_config_ap.ap.ssid) - 1] = '\0'; // Null-terminate the SSID

    if (strlen(CONFIG_CAPTIVE_PORTAL_WIFI_AP_PASSWORD) == 0)
    {
        wifi_config_ap.ap.authmode = WIFI_AUTH_OPEN;
    }
    // Set Wi-Fi mode to APSTA
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config_ap));
    ESP_LOGI(TAG, "starting WiFi access point: SSID: %s password:%s channel: %d",
             decoratedSSID, CONFIG_CAPTIVE_PORTAL_WIFI_AP_PASSWORD, CONFIG_CAPTIVE_PORTAL_WIFI_AP_CHANNEL);
    /** Wifi captive portal DNS init. */
    wifi_captive_portal_dns_init();
}

static void wifi_captive_portal_wifi_init()
{
    ESP_ERROR_CHECK(esp_netif_init());

    wifi_event_group = xEventGroupCreate();

    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
    assert(ap_netif);

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    esp_netif_ip_info_t ip_info;

    /** NOTE: This is where you set the access point (AP) IP address
        and gateway address. It has to be a class A internet address
        otherwise the captive portal sign-in prompt won't show up on
        Android when you connect to the access point. */
    IP4_ADDR(&ip_info.ip, 124, 213, 16, 29);
    IP4_ADDR(&ip_info.gw, 124, 213, 16, 29);
    IP4_ADDR(&ip_info.netmask, 255, 0, 0, 0);
    esp_netif_dhcps_stop(ap_netif);
    esp_netif_set_ip_info(ap_netif, &ip_info);
    esp_netif_dhcps_start(ap_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_captive_portal_wifi_ap_init();

    // Register event handler for handling network scans
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        WIFI_EVENT_SCAN_DONE,
                                                        &wifi_scan_event_handler,
                                                        NULL,
                                                        NULL));

    ESP_ERROR_CHECK(esp_wifi_start());
}

/** The WiFi task. */
void wifi_captive_portal_wifi_task(void *pvParameter)
{
    while (1)
    {
        while (1)
        {
            bool wifi_is_init = false;

            esp_err_t err = ESP_OK;

            /** Init only once. */
            if (!wifi_is_init)
            {
                ESP_LOGI(TAG, "wifi initializing...");

                /** Initialize wifi */
                wifi_captive_portal_wifi_init();

                wifi_is_init = true;
            }

            /** Send an event which says "this task is finished". */
            err = esp_event_post_to(wifi_captive_portal_wifi_event_loop_handle, WIFI_CAPTIVE_PORTAL_WIFI_EVENT, WIFI_CAPTIVE_PORTAL_WIFI_EVENT_FINISH, NULL, 0, portMAX_DELAY);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Sending event failed");
            }

            break;
        }

        vTaskDelete(NULL);
    }
}

void wifi_captive_portal_wifi_scan(bool block)
{
    // Perform Wi-Fi scan
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true};
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, block)); // Non-blocking scan
}

int wifi_captive_portal_get_scan_count(void)
{
    int count = 0;
    for (int i = 0; i < CONFIG_CAPTIVE_PORTAL_MAX_SCAN_SSIDS; i++)
    {
        if (scan_result[i].ssid[0])
        {
            count++;
        }
    }
    return count;
}

bool wifi_captive_portal_get_scan_result(int pos, wifi_ap_info_t *out)
{
    if (out == NULL ||
        pos < 0 ||
        pos >= CONFIG_CAPTIVE_PORTAL_MAX_SCAN_SSIDS ||
        scan_result[pos].ssid[0] == 0)
    {
        return false;
    }
    memcpy((void *)out, (void *)&scan_result[pos], sizeof(*out));
    return true;
}