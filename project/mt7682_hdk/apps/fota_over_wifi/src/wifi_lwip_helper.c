/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "os.h"
#include "semphr.h"
#include "wifi_api.h"
#include "lwip/ip4_addr.h"
#include "lwip/inet.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
#include "portmacro.h"
#include "dhcpd.h"
#include "wifi_lwip_helper.h"
#include "network_default_config.h"
#include "wifi_private_api.h"

static SemaphoreHandle_t wifi_connected;

static SemaphoreHandle_t ip_ready;

static void ip_ready_callback(struct netif *netif);

static int32_t wifi_station_port_secure_event_handler(wifi_event_t event, uint8_t *payload, uint32_t length);
static int32_t wifi_station_disconnected_event_handler(wifi_event_t event, uint8_t *payload, uint32_t length);

/**
  * @brief  dhcp got ip will callback this function.
  * @param[in] struct netif *netif: which network interface got ip.
  * @retval None
  */
static void ip_ready_callback(struct netif *netif)
{
    if (!ip4_addr_isany_val(netif->ip_addr)) {
        char ip_addr[17] = {0};
        if (NULL != inet_ntoa(netif->ip_addr)) {
            strcpy(ip_addr, inet_ntoa(netif->ip_addr));
            LOG_I(common, "************************");
            LOG_I(common, "DHCP got IP:%s", ip_addr);
            LOG_I(common, "************************");

            xSemaphoreGive(ip_ready);
        } else {
            LOG_E(common, "DHCP got Failed");
        }
#ifdef MTK_WIFI_REPEATER_ENABLE
#ifndef MTK_WIFI_DHCPD_DHCP_COEXIST_ENABLE
        uint8_t op_mode = 0;
        struct netif *ap_if = netif_find_by_type(NETIF_TYPE_AP);
        if (wifi_config_get_opmode(&op_mode) < 0) {
            return;
        }

        if (WIFI_MODE_REPEATER == op_mode) {
            netif_set_addr(ap_if, &netif->ip_addr, &netif->netmask, &netif->gw);
        }
#endif
#endif
    }
}

/**
  * @brief  wifi connected will call this callback function. set lwip status in this function
  * @param[in] wifi_event_t event: not used.
  * @param[in] uint8_t *payload: not used.
  * @param[in] uint32_t length: not used.
  * @retval None
  */
static int32_t wifi_station_port_secure_event_handler(wifi_event_t event,
        uint8_t *payload,
        uint32_t length)
{
    struct netif *sta_if;

    sta_if = netif_find_by_type(NETIF_TYPE_STA);
    netif_set_link_up(sta_if);

    xSemaphoreGive(wifi_connected);
    LOG_I(common, "wifi connected");
    return 0;
}

/**
  * @brief  wifi disconnected will call this callback function. set lwip status in this function
  * @param[in] wifi_event_t event: not used.
  * @param[in] uint8_t *payload: not used.
  * @param[in] uint32_t length: not used.
  * @retval None
  */
static int32_t wifi_station_disconnected_event_handler(wifi_event_t event,
        uint8_t *payload,
        uint32_t length)
{
    uint8_t opmode  = 0;

    if (wifi_config_get_opmode(&opmode) < 0) {
        return 0;
    }

    if ((WIFI_MODE_AP_ONLY != opmode) && WIFI_EVENT_IOT_DISCONNECTED == event) {
        uint8_t link_status = 1;
        //should check link status, it will emit this event when sp disconnect with host under repeater mode.
        if (wifi_connection_get_link_status(&link_status) < 0) {
            return 0;
        }

        if (link_status == 0) {
            struct netif *sta_if;
            sta_if = netif_find_by_type(NETIF_TYPE_STA);
            netif_set_link_down(sta_if);
            if (dhcp_config_init() == STA_IP_MODE_DHCP) {
                netif_set_addr(sta_if, IP4_ADDR_ANY, IP4_ADDR_ANY, IP4_ADDR_ANY);
            }
            LOG_I(common, "wifi disconnected");
        }
    }
    return 1;
}

/**
  * @brief  network init function. initial wifi and lwip config
  * @param None
  * @retval None
  */

void lwip_network_init(uint8_t opmode)
{
    lwip_tcpip_config_t tcpip_config = {0, {0}, {0}, {0}, {0}, {0}, {0}};

    if (0 != tcpip_config_init(&tcpip_config)) {
        LOG_E(common, "tcpip config init fail");
        return;
    }
    wifi_connected = xSemaphoreCreateBinary();
    if (dhcp_config_init() == STA_IP_MODE_DHCP) {
        ip_ready = xSemaphoreCreateBinary();
    }
    lwip_tcpip_init(&tcpip_config, opmode);
}

/**
  * @brief  Start lwip service in a certain operation mode.
  * @param[in] uint8_t opmode: the target operation mode.
  * @retval None
  */
void lwip_net_start(uint8_t opmode)
{
    struct netif *sta_if;
    struct netif *ap_if;

    switch (opmode) {
        case WIFI_MODE_STA_ONLY:
        case WIFI_MODE_REPEATER:
            wifi_connection_register_event_handler(WIFI_EVENT_IOT_PORT_SECURE, wifi_station_port_secure_event_handler);
            wifi_connection_register_event_handler(WIFI_EVENT_IOT_DISCONNECTED, wifi_station_disconnected_event_handler);
            if (dhcp_config_init() == STA_IP_MODE_DHCP) {
                sta_if = netif_find_by_type(NETIF_TYPE_STA);
                netif_set_default(sta_if);
                netif_set_status_callback(sta_if, ip_ready_callback);
                dhcp_start(sta_if);
            }
#ifdef MTK_WIFI_REPEATER_ENABLE
#ifdef MTK_WIFI_DHCPD_DHCP_COEXIST_ENABLE
            if (opmode == WIFI_MODE_REPEATER) {

                dhcpd_settings_t dhcpd_settings = {{0}, {0}, {0}, {0}, {0}, {0}, {0}};
                lwip_tcpip_config_t tcpip_config = {0, {0}, {0}, {0}, {0}, {0}, {0}};

                if (0 != tcpip_config_init(&tcpip_config)) {
                    LOG_E(common, "tcpip config init fail");
                    return;
                }

                dhcpd_settings_init(&tcpip_config, &dhcpd_settings);
                ap_if = netif_find_by_type(NETIF_TYPE_AP);
                netif_set_link_up(ap_if);
                dhcpd_start(&dhcpd_settings);
            }
#endif
#endif
            break;
        case WIFI_MODE_AP_ONLY: {
            dhcpd_settings_t dhcpd_settings = {{0}, {0}, {0}, {0}, {0}, {0}, {0}};
            lwip_tcpip_config_t tcpip_config = {0, {0}, {0}, {0}, {0}, {0}, {0}};

            if (0 != tcpip_config_init(&tcpip_config)) {
                LOG_E(common, "tcpip config init fail");
                return;
            }

            dhcpd_settings_init(&tcpip_config, &dhcpd_settings);
            ap_if = netif_find_by_type(NETIF_TYPE_AP);
            netif_set_default(ap_if);
            netif_set_link_up(ap_if);
            dhcpd_start(&dhcpd_settings);
            break;
        }
    }
}

/**
  * @brief  Stop lwip service in a certain operation mode.
  * @param[in] uint8_t opmode: the current operation mode.
  * @retval None
  */
void lwip_net_stop(uint8_t opmode)
{
    struct netif *sta_if;
    struct netif *ap_if;

    sta_if = netif_find_by_type(NETIF_TYPE_STA);
    ap_if = netif_find_by_type(NETIF_TYPE_AP);
    switch (opmode) {
        case WIFI_MODE_AP_ONLY:
            dhcpd_stop();
            netif_set_link_down(ap_if);
            break;
        case WIFI_MODE_STA_ONLY:
        case WIFI_MODE_REPEATER:
            netif_set_status_callback(sta_if, NULL);
            dhcp_release(sta_if);
            dhcp_stop(sta_if);
            netif_set_link_down(sta_if);
            break;
    }
}

/**
  * @brief  when wifi and ip ready will return, only used in station and repeater mode.
  * @param None
  * @retval None
  */
void lwip_net_ready()
{
    xSemaphoreTake(wifi_connected, portMAX_DELAY);
    if (dhcp_config_init() == STA_IP_MODE_DHCP) {
        xSemaphoreTake(ip_ready, portMAX_DELAY);
    }
}

/**
  * @brief  Change operation mode dynamically.
  * @param[in] uint8_t target_mode: the target switched operation mode.
  * @retval None
  */
uint8_t wifi_set_opmode(uint8_t target_mode)
{
    uint8_t origin_op_mode = 0;
    if (wifi_config_get_opmode(&origin_op_mode) < 0) {
        return 1;
    }

    if (target_mode == origin_op_mode) {
        LOG_I(wifi, "same opmode %d, do nothing", target_mode);
        return 0;
    }
    lwip_net_stop(origin_op_mode);

    if (wifi_config_set_opmode(target_mode) < 0) {
        return 1;
    }
    LOG_I(wifi, "set opmode to [%d]", target_mode);

    lwip_net_start(target_mode);
    return 0;
}

