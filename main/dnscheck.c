#include "esp_log.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "lwip/netdb.h"

extern const char server_cert_pem_start[] asm("_binary_myothermind_pem_start");
extern const char server_cert_pem_end[] asm("_binary_myothermind_pem_end");

static const char *TAG = "DNS_CHECK";

void check_dns(void)
{
    const char *hostname = "myothermind.com";
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;

    int err = getaddrinfo(hostname, NULL, &hints, &res);
    if (err != 0 || res == NULL)
    {
        ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
        if (err == EAI_FAIL)
        {
            ESP_LOGE(TAG, "DNS lookup failed, check your network connection");
        }
        else if (err == EAI_NONAME)
        {
            ESP_LOGE(TAG, "DNS lookup failed, hostname not found");
        }
        return;
    }

    struct addrinfo *r;
    for (r = res; r != NULL; r = r->ai_next)
    {
        if (r->ai_family == AF_INET)
        {
            struct in_addr addr = ((struct sockaddr_in *)r->ai_addr)->sin_addr;
            ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(addr));
        }
    }

    freeaddrinfo(res);
}
