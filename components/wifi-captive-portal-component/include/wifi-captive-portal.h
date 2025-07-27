#ifndef __WIFI_CAPTIVE_PORTAL_COMPONENT_WIFI_CAPTIVE_PORTAL_H_INCLUDED__
#define __WIFI_CAPTIVE_PORTAL_COMPONENT_WIFI_CAPTIVE_PORTAL_H_INCLUDED__
/**	wifi-captive-portal-component - example - http-slideshow

  Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

  This code is released under the license terms contained in the
  file named LICENSE, which is found in the top-level folder in
  this project. You must agree to follow those license terms,
  otherwise you aren't allowed to copy, distribute, or use any
  part of this project in any way.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <fcntl.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>
#include <esp_system.h>
#include <esp_log.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_http_server.h>
#include <esp_event_base.h>
#include <esp_mac.h>
#include <esp_vfs_semihost.h>
#include <esp_vfs_fat.h>
#include <esp_vfs.h>
#include <esp_netif.h>
#include <lwip/inet.h>
#include <lwip/ip4_addr.h>
#include <lwip/dns.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <lwip/err.h>

#include "wifi-captive-portal-wifi.h"
#include "wifi-captive-portal-httpd.h"
#include "wifi-captive-portal-dns.h"

#ifdef __cplusplus
extern "C"
{
#endif

  void wifi_captive_portal(esp_err_t (*http_handler)(httpd_req_t *r));

#ifdef __cplusplus
}
#endif

#endif
