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

extern void resource_setup(const coap_resource_t *resources);
extern coap_resource_t resources[];

Ray::Ray() :
        m_notifications_enabled(false)
{
}
Ray::~Ray() {}

void Ray::process_packet(uint8_t *buf, uint16_t size) {
    int rc;
    coap_packet_t pkt;

#ifdef YACOAP_DEBUG
    printf("Received: ");
        coap_dump(buf, size, true);
        printf("\n");
#endif

    if ((rc = coap_parse(buf, size, &pkt)) > COAP_ERR)
        printf("Bad packet rc=%d\n", rc);
    else {
        size_t buflen = 256;//sizeof(buf);
        coap_packet_t rsppkt;
#ifdef YACOAP_DEBUG
        coap_dump_packet(&pkt);
#endif
        coap_handle_request(resources, &pkt, &rsppkt);

        if ((rc = coap_build(&rsppkt, buf, &buflen)) > COAP_ERR)
            printf("coap_build failed rc=%d\n", rc);
        else {
#ifdef YACOAP_DEBUG
            printf("Sending: ");
                coap_dump(buf, buflen, true);
                printf("\n");
#endif
#ifdef YACOAP_DEBUG
            coap_dump_packet(&rsppkt);
#endif

            send_response(buflen);
        }
    }
}

void Ray::send_response(uint16_t size)
{
    uip_udp_conn = m_udp_txonly;
    uip_udp_conn->ripaddr[0] = ipv4_header->srcipaddr[0];
    uip_udp_conn->ripaddr[1] = ipv4_header->srcipaddr[1];
    uip_udp_conn->rport = ipv4_header->srcport;
    uip_udp_send(size);
}

void Ray::init(struct uip_udp_conn *udp_rxonly, struct uip_udp_conn *udp_txonly)
{
    m_udp_rxonly = udp_rxonly;
    m_udp_txonly = udp_txonly;

    resource_setup(resources);
}

void Ray::appcall()
{
    static int i = 0;
    i++;
    if (uip_newdata()) {
        //char *data = (char *)uip_appdata;
        /*if (data[0] == 'E') {
            printf("Enabling counter\n");
            m_notifications_enabled = true;
            m_notify_remote_ip[0] = ipv4_header->srcipaddr[0];
            m_notify_remote_ip[1] = ipv4_header->srcipaddr[1];
            m_notify_remote_port = ipv4_header->srcport;
        } else if (data[0] == 'D') {
            printf("Disabling counter\n");
            m_notifications_enabled = false;
        }*/

        //uint8_t buf[64];
        //sprintf((char *)buf, "counter: %d\n", i);
        //uint32_t len = strlen((const char *)buf);
        //memcpy(uip_appdata, buf, len);

        process_packet((uint8_t *)uip_appdata, uip_len);

        // switch to tx conn and send
        /*uip_udp_conn = m_udp_txonly;
        uip_udp_conn->ripaddr[0] = ipv4_header->srcipaddr[0];
        uip_udp_conn->ripaddr[1] = ipv4_header->srcipaddr[1];
        uip_udp_conn->rport = ipv4_header->srcport;
        uip_udp_send(len);*/
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
