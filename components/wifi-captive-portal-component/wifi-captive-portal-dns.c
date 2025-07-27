/**	wifi-captive-portal-component

  Copyright (c) 2021 Jeremy Carter <jeremy@jeremycarter.ca>

  This code is released under the license terms contained in the
  file named LICENSE, which is found in the top-level folder in
  this project. You must agree to follow those license terms,
  otherwise you aren't allowed to copy, distribute, or use any
  part of this project in any way.

  Contains some modified example code from here:
  https://github.com/cornelis-61/esp32_Captdns/blob/master/main/captdns.c

  Original Example Code Header:
* ----------------------------------------------------------------------------
* "THE BEER-WARE LICENSE" (Revision 42):
* Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain
* this notice you can do whatever you want with this stuff. If we meet some day,
* and you think this stuff is worth it, you can buy me a beer in return.
*
* modified for ESP32 by Cornelis
*
* ----------------------------------------------------------------------------
*/
#include "wifi-captive-portal.h"

static const char *TAG = "wifi-captive-portal-dns";

static int sock_fd;

// Parses a label into a C-string containing a dotted
// Returns pointer to start of next fields in packet
static char *label_to_str(char *packet, char *labelPtr, int packetSz, char *res, int resMaxLen)
{
    int i, j, k;
    char *endPtr = NULL;
    i = 0;
    do
    {
        if ((*labelPtr & 0xC0) == 0)
        {
            j = *labelPtr++; // skip past length
            // Add separator period if there already is data in res
            if (i < resMaxLen && i != 0)
                res[i++] = '.';
            // Copy label to res
            for (k = 0; k < j; k++)
            {
                if ((labelPtr - packet) > packetSz)
                    return NULL;
                if (i < resMaxLen)
                    res[i++] = *labelPtr++;
            }
        }
        else if ((*labelPtr & 0xC0) == 0xC0)
        {
            // Compressed label pointer
            endPtr = labelPtr + 2;
            int offset = ntohs(*((uint16_t *)labelPtr)) & 0x3FFF;
            // Check if offset points to somewhere outside of the packet
            if (offset > packetSz)
                return NULL;
            labelPtr = &packet[offset];
        }
        // check for out-of-bound-ness
        if ((labelPtr - packet) > packetSz)
            return NULL;
    } while (*labelPtr != 0);
    res[i] = 0; // zero-terminate
    if (endPtr == NULL)
        endPtr = labelPtr + 1;
    return endPtr;
}

// Converts a dotted hostname to the weird label form dns uses.
static char *str_to_label(char *str, char *label, int maxLen)
{
    char *len = label;   // ptr to len byte
    char *p = label + 1; // ptr to next label byte to be written
    while (1)
    {
        if (*str == '.' || *str == 0)
        {
            *len = ((p - len) - 1); // write len of label bit
            len = p;                // pos of len for next part
            p++;                    // data ptr is one past len
            if (*str == 0)
                break; // done
            str++;
        }
        else
        {
            *p++ = *str++; // copy byte
                           // if ((p-label)>maxLen) return NULL;	// check out of bounds
        }
    }
    *len = 0;
    return p; // ptr to first free byte in resp
}

/**
 * @brief Get the ap netif object
 * Find the network object of the active network.
 * Since there might be more than one network AP object, we have to ignore the default one.
 * @return esp_netif_t* Network object
 */
esp_netif_t *get_ap_netif()
{
    esp_netif_t *netif = NULL;
    while ((netif = esp_netif_next_unsafe(netif)) != NULL)
    {
        if (esp_netif_get_ifkey(netif) && strcmp(esp_netif_get_ifkey(netif), "WIFI_AP_DEF") == 0)
        {
            return netif;
        }
    }
    return NULL; // Return NULL if the AP interface is not found
}

// Receive a DNS packet and maybe send a response back
static void dns_recv(struct sockaddr_in *premote_addr, char *pusrdata, unsigned short length)
{
    char buff[WIFI_CAPTIVE_PORTAL_DNS_LEN];
    char reply[WIFI_CAPTIVE_PORTAL_DNS_LEN];
    int i;
    char *rend = &reply[length];
    char *p = pusrdata;
    DnsHeader *hdr = (DnsHeader *)p;
    DnsHeader *rhdr = (DnsHeader *)&reply[0];
    p += sizeof(DnsHeader);

    // Some sanity checks:
    if (length > WIFI_CAPTIVE_PORTAL_DNS_LEN)
        return; // Packet is longer than DNS implementation allows
    if (length < sizeof(DnsHeader))
        return; // Packet is too short
    if (hdr->ancount || hdr->nscount || hdr->arcount)
        return; // this is a reply, don't know what to do with it
    if (hdr->flags & WIFI_CAPTIVE_PORTAL_DNS_FLAG_TC)
        return; // truncated, can't use this
    // Reply is basically the request plus the needed data
    memcpy(reply, pusrdata, length);
    rhdr->flags |= WIFI_CAPTIVE_PORTAL_DNS_FLAG_QR;

    for (i = 0; i < ntohs(hdr->qdcount); i++)
    {
        // Grab the labels in the q string
        p = label_to_str(pusrdata, p, length, buff, sizeof(buff));
        if (p == NULL)
            return;
        DnsQuestionFooter *qf = (DnsQuestionFooter *)p;
        p += sizeof(DnsQuestionFooter);

        ESP_LOGI(TAG, "DNS: WIFI_CAPTIVE_PORTAL_DNS_Q (type 0x%X cl 0x%X) for %s\n", ntohs(qf->type), ntohs(qf->cl), buff);

        if (ntohs(qf->type) == WIFI_CAPTIVE_PORTAL_DNS_QTYPE_A)
        {
            // They want to know the IPv4 address of something.
            // Build the response.

            rend = str_to_label(buff, rend, sizeof(reply) - (rend - reply)); // Add the label
            if (rend == NULL)
                return;
            DnsResourceFooter *rf = (DnsResourceFooter *)rend;
            rend += sizeof(DnsResourceFooter);
            rf->type = htons(WIFI_CAPTIVE_PORTAL_DNS_QTYPE_A);
            rf->cl = htons(WIFI_CAPTIVE_PORTAL_DNS_QCLASS_IN);
            rf->ttl = htonl(0);
            rf->rdlength = htons(4); // IPv4 addr is 4 bytes;
            // Grab the current IP of the softap interface

            esp_netif_t *netif = get_ap_netif();
            esp_netif_ip_info_t info;
            info.ip.addr = 0;
            if (netif != NULL)
            {
                esp_netif_get_ip_info(netif, &info);
            }
            *rend++ = ip4_addr1(&info.ip);
            *rend++ = ip4_addr2(&info.ip);
            *rend++ = ip4_addr3(&info.ip);
            *rend++ = ip4_addr4(&info.ip);
            rhdr->ancount = htons(ntohs(rhdr->ancount) + 1);
        }
        else if (ntohs(qf->type) == WIFI_CAPTIVE_PORTAL_DNS_QTYPE_NS)
        {
            // Give ns server. Basically can be whatever we want because it'll get resolved to our IP later anyway.
            rend = str_to_label(buff, rend, sizeof(reply) - (rend - reply)); // Add the label
            DnsResourceFooter *rf = (DnsResourceFooter *)rend;
            rend += sizeof(DnsResourceFooter);
            rf->type = htons(WIFI_CAPTIVE_PORTAL_DNS_QTYPE_NS);
            rf->cl = htons(WIFI_CAPTIVE_PORTAL_DNS_QCLASS_IN);
            rf->ttl = htons(0);
            rf->rdlength = htons(4);
            *rend++ = 2;
            *rend++ = 'n';
            *rend++ = 's';
            *rend++ = 0;
            rhdr->ancount = htons(ntohs(rhdr->ancount) + 1);
        }
        else if (ntohs(qf->type) == WIFI_CAPTIVE_PORTAL_DNS_QTYPE_URI)
        {
            // Give uri to us
            rend = str_to_label(buff, rend, sizeof(reply) - (rend - reply)); // Add the label
            DnsResourceFooter *rf = (DnsResourceFooter *)rend;
            rend += sizeof(DnsResourceFooter);
            DnsUriHdr *uh = (DnsUriHdr *)rend;
            rend += sizeof(DnsUriHdr);
            rf->type = htons(WIFI_CAPTIVE_PORTAL_DNS_QTYPE_URI);
            rf->cl = htons(WIFI_CAPTIVE_PORTAL_DNS_QCLASS_URI);
            rf->ttl = htons(0);
            rf->rdlength = htons(4 + 16);
            uh->prio = htons(10);
            uh->weight = htons(1);
            memcpy(rend, "http://esp.nonet", 16);
            rend += 16;
            rhdr->ancount = htons(ntohs(rhdr->ancount) + 1);
        }
    }
    // Send the response
    sendto(sock_fd, (uint8_t *)reply, rend - reply, 0, (struct sockaddr *)premote_addr, sizeof(struct sockaddr_in));
}

static void dns_task(void *pvParameters)
{
    struct sockaddr_in server_addr;
    uint32_t ret;
    struct sockaddr_in from;
    socklen_t fromlen;
    char udp_msg[WIFI_CAPTIVE_PORTAL_DNS_LEN];

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(53);
    server_addr.sin_len = sizeof(server_addr);

    do
    {
        sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock_fd == -1)
        {
            ESP_LOGI(TAG, "dns_task failed to create sock!");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    } while (sock_fd == -1);

    do
    {
        ret = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (ret != 0)
        {
            ESP_LOGI(TAG, "dns_task failed to bind sock!");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    } while (ret != 0);

    ESP_LOGI(TAG, "DNS initialized.");

    while (1)
    {
        memset(&from, 0, sizeof(from));
        fromlen = sizeof(struct sockaddr_in);
        ret = recvfrom(sock_fd, (uint8_t *)udp_msg, WIFI_CAPTIVE_PORTAL_DNS_LEN, 0, (struct sockaddr *)&from, (socklen_t *)&fromlen);
        if (ret > 0)
            dns_recv(&from, udp_msg, ret);
    }

    close(sock_fd);
    vTaskDelete(NULL);
}

void wifi_captive_portal_dns_init(void)
{
    xTaskCreate(dns_task, (const char *)"dns_task", 10000, NULL, 3, NULL);
}
