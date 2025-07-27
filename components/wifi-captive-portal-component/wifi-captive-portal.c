/**	wifi-captive-portal-component

  Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

  This code is released under the license terms contained in the
  file named LICENSE, which is found in the top-level folder in
  this project. You must agree to follow those license terms,
  otherwise you aren't allowed to copy, distribute, or use any
  part of this project in any way.
*/
#include "wifi-captive-portal.h"

static const char *TAG = "wifi-captive-portal";
/**
 * @brief Task configuration for the wifi task
 *
 */
const char *wifi_captive_portal_wifi_task_name = "wifi_captive_portal_wifi_task";
const uint32_t wifi_captive_portal_wifi_task_stack_depth = 4096;
UBaseType_t wifi_captive_portal_wifi_task_priority = 5;
/**
 * @brief Task configuration for the http webserver task
 *
 */
const char *wifi_captive_portal_httpd_task_name = "wifi_captive_portal_httpd_task";
const uint32_t wifi_captive_portal_httpd_task_stack_depth = 4096;
UBaseType_t wifi_captive_portal_httpd_task_priority = 5;

/**
 * @brief
 *
 * @param handler_arg
 * @param base
 * @param id
 * @param event_data
 */
static void wifi_captive_portal_wifi_finish_event_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
    ESP_LOGI(TAG, "event received: WIFI_CAPTIVE_PORTAL_WIFI_EVENT_FINISH");

    xTaskCreate(&wifi_captive_portal_httpd_task,
                wifi_captive_portal_httpd_task_name,
                wifi_captive_portal_httpd_task_stack_depth * 8,
                handler_arg,
                wifi_captive_portal_httpd_task_priority,
                NULL);
    ESP_LOGI(TAG, "Task started: %s", wifi_captive_portal_httpd_task_name);
}

/**
 * @brief Create a captive portal
 * Creates a captive portal including a DNS server, Wifi network and HTTP server
 * Note: We assume that the esp_event_loop_create_default() has been called
 * @param wifi_config Configuration for the Wifi network
 * @param http_handler Handler to be called to process any HTTP requests
 */
void wifi_captive_portal(esp_err_t (*http_handler)(httpd_req_t *r))
{
    esp_event_loop_args_t wifi_captive_portal_wifi_event_loop_args = {
        .queue_size = 5,
        .task_name = "wifi_captive_portal_wifi_event_loop_task", // task will be created
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = wifi_captive_portal_wifi_task_stack_depth,
        .task_core_id = tskNO_AFFINITY};

    ESP_ERROR_CHECK(
        esp_event_loop_create(&wifi_captive_portal_wifi_event_loop_args,
                              &wifi_captive_portal_wifi_event_loop_handle));
    ESP_ERROR_CHECK(
        esp_event_handler_instance_register_with(wifi_captive_portal_wifi_event_loop_handle,    // event_loop
                                                 WIFI_CAPTIVE_PORTAL_WIFI_EVENT,                // event_base
                                                 WIFI_CAPTIVE_PORTAL_WIFI_EVENT_FINISH,         // event_id
                                                 wifi_captive_portal_wifi_finish_event_handler, // event_handler
                                                 (void *)http_handler,                          // event_handler_arg
                                                 wifi_captive_portal_wifi_event_loop_handle     // handler_ctx_arg
                                                 ));

    esp_event_loop_args_t wifi_captive_portal_httpd_event_loop_args = {
        .queue_size = 5,
        .task_name = "wifi_captive_portal_httpd_event_loop_task", // task will be created
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = wifi_captive_portal_httpd_task_stack_depth,
        .task_core_id = tskNO_AFFINITY};

    ESP_ERROR_CHECK(
        esp_event_loop_create(&wifi_captive_portal_httpd_event_loop_args,
                              &wifi_captive_portal_httpd_event_loop_handle));

    xTaskCreate(&wifi_captive_portal_wifi_task,
                wifi_captive_portal_wifi_task_name,
                wifi_captive_portal_wifi_task_stack_depth * 8,
                NULL,
                wifi_captive_portal_wifi_task_priority,
                NULL);

    ESP_LOGI(TAG, "Task started: %s", wifi_captive_portal_wifi_task_name);
}
