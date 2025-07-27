#ifndef __WIFI_CAPTIVE_PORTAL_COMPONENT_WIFI_CAPTIVE_PORTAL_WIFI_H_INCLUDED__
#define __WIFI_CAPTIVE_PORTAL_COMPONENT_WIFI_CAPTIVE_PORTAL_WIFI_H_INCLUDED__
/**	wifi-captive-portal-component

  Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

  This code is released under the license terms contained in the
  file named LICENSE, which is found in the top-level folder in
  this project. You must agree to follow those license terms,
  otherwise you aren't allowed to copy, distribute, or use any
  part of this project in any way.
*/

/** An event base type for "wifi-captive-portal-wifi". */
ESP_EVENT_DECLARE_BASE(WIFI_CAPTIVE_PORTAL_WIFI_EVENT);
enum /**< The events. */
{
    WIFI_CAPTIVE_PORTAL_WIFI_EVENT_FINISH,  /**< An event for "this task is finished". */
    WIFI_CAPTIVE_PORTAL_WIFI_EVENT_STOPPED, /**< Do something after the wifi interface has stopped. */
};

/** The event loop handle. */
extern esp_event_loop_handle_t wifi_captive_portal_wifi_event_loop_handle;

/** Task action values. */
struct wifi_captive_portal_wifi_task_action_value_t
{
    bool no_deep_sleep;        /**< If true, don't deep sleep after the wifi request. */
    uint64_t deep_sleep_usecs; /**< Deep sleep for this number of microseconds (seconds * 1000000). */
};

typedef struct
{
    uint8_t ssid[33];          /**< SSID of AP */
    int8_t rssi;               /**< signal strength of AP. Note that in some rare cases where signal strength is very strong, rssi values can be slightly positive */
    wifi_auth_mode_t authmode; /**< authmode of AP */
} wifi_ap_info_t;

#define _WIFI_CAPTIVE_PORTAL_WIFI_TASK_ACTION_VALUE_CAST_VOID_P(x) (wifi_captive_portal_wifi_task_action_value_void_p) & x
#define WIFI_CAPTIVE_PORTAL_WIFI_TASK_ACTION_VALUE_CAST_VOID_P(x) _WIFI_CAPTIVE_PORTAL_WIFI_TASK_ACTION_VALUE_CAST_VOID_P(x)

#define _WIFI_CAPTIVE_PORTAL_WIFI_TASK_ACTION_VALUE_COPY(x) *(struct wifi_captive_portal_wifi_task_action_value_t *)x
#define WIFI_CAPTIVE_PORTAL_WIFI_TASK_ACTION_VALUE_COPY(x) _WIFI_CAPTIVE_PORTAL_WIFI_TASK_ACTION_VALUE_COPY(x)

typedef void *wifi_captive_portal_wifi_task_action_void_p;

#define _WIFI_CAPTIVE_PORTAL_WIFI_TASK_ACTION_CAST_VOID_P(x) (wifi_captive_portal_wifi_task_action_void_p) & x
#define WIFI_CAPTIVE_PORTAL_WIFI_TASK_ACTION_CAST_VOID_P(x) _WIFI_CAPTIVE_PORTAL_WIFI_TASK_ACTION_CAST_VOID_P(x)

#ifdef __cplusplus
extern "C"
{
#endif

    /** The esp-idf task function. */
    void wifi_captive_portal_wifi_task(void *pvParameter);
    void wifi_captive_portal_wifi_scan(bool block);
    bool wifi_captive_portal_get_scan_result(int pos, wifi_ap_info_t *out);
    int wifi_captive_portal_get_scan_count(void);

#ifdef __cplusplus
}
#endif

#endif
