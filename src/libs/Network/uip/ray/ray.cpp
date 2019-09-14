/**
 * UDP Telemetry Streamer
 *
 * by Roman Isaikin <roman@vhrd.tech>
 */

#include "ray.h"

#include "Kernel.h"
#include "utils.h"
#include "uip.h"

#include <string.h>
#include <stdio.h>

#define ipv4_header ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

Ray::Ray() :
        m_notifications_enabled(false)
{
}
Ray::~Ray() {}

void Ray::init(struct uip_udp_conn *udp_rxonly, struct uip_udp_conn *udp_txonly)
{
    m_udp_rxonly = udp_rxonly;
    m_udp_txonly = udp_txonly;
}

void Ray::appcall()
{
    static int i = 0;
    i++;
    if (uip_newdata()) {
        char *data = (char *)uip_appdata;
        if (data[0] == 'E') {
            printf("Enabling counter\n");
            m_notifications_enabled = true;
            m_notify_remote_ip[0] = ipv4_header->srcipaddr[0];
            m_notify_remote_ip[1] = ipv4_header->srcipaddr[1];
            m_notify_remote_port = ipv4_header->srcport;
        } else if (data[0] == 'D') {
            printf("Disabling counter\n");
            m_notifications_enabled = false;
        }

        uint8_t buf[64];
        sprintf((char *)buf, "counter: %d\n", i);

        uint32_t len = strlen((const char *)buf);

        //memcpy(buf + len, uip_udp_conn, 9);

        memcpy(uip_appdata, buf, len);

        // switch to tx conn and send
        uip_udp_conn = m_udp_txonly;
        uip_udp_conn->ripaddr[0] = ipv4_header->srcipaddr[0];
        uip_udp_conn->ripaddr[1] = ipv4_header->srcipaddr[1];
        uip_udp_conn->rport = ipv4_header->srcport;
        uip_udp_send(len);
    } else if (m_notifications_enabled) {
        uint8_t buf[64];
        sprintf((char *)buf, "counter: %d\n", i);
        uint32_t len = strlen((const char *)buf);
        memcpy(uip_appdata, buf, len);

        // switch to tx conn and send
        uip_udp_conn = m_udp_txonly;
        uip_udp_conn->ripaddr[0] = m_notify_remote_ip[0];
        uip_udp_conn->ripaddr[1] = m_notify_remote_ip[1];
        uip_udp_conn->rport = m_notify_remote_port;
        uip_udp_send(len);
    }
}
