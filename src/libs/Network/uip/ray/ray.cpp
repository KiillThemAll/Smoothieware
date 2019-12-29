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

#include "Robot.h"
#include "Player.h"
#include "StepperMotor.h"

#define ipv4_header ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

extern void resource_setup(const coap_resource_t *resources);
extern coap_resource_t resources[];

Ray::Ray() :
        m_notifications_enabled(false),
        m_robot(0),
        m_player(0)
{
    m_payload.mcs_x = 0.0;
    m_payload.mcs_y = 0.0;
    m_payload.mcs_z = 0.0;
    m_payload.mcs_b = 0.0;
    m_payload.state = 5;
    m_payload.played = 0;
    m_payload.total = 0;
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
    } //else if (m_notifications_enabled) {
        //uint8_t buf[64];
        //sprintf((char *)buf, "counter: %d\n", i);
        //uint32_t len = strlen((const char *)buf);
        if (m_robot) {
            m_payload.mcs_x = m_robot->actuators[X_AXIS]->get_current_position();
            m_payload.mcs_y = m_robot->actuators[Y_AXIS]->get_current_position();
            m_payload.mcs_z = m_robot->actuators[Z_AXIS]->get_current_position();
            m_payload.mcs_b = m_robot->actuators[B_AXIS]->get_current_position();
        }

        if (m_player) {
            m_payload.state = m_player->state();
            m_payload.played = (uint32_t)m_player->played_cnt;
            m_payload.total = (uint32_t)m_player->file_size;
        }

        memcpy(uip_appdata, (void *)&m_payload, sizeof(ray_payload_t));

        // switch to tx conn and send
        uip_udp_conn = m_udp_txonly;
        uip_ipaddr_t addr;
        uip_ipaddr(&addr, 192, 168, 88, 240);
        uip_udp_conn->ripaddr[0] = addr[0];
        uip_udp_conn->ripaddr[1] = addr[1];
        uip_udp_conn->rport = HTONS(45454);
        uip_udp_send(sizeof(ray_payload_t));
    //}
}

void Ray::setRobot(Robot *robot)
{
    m_robot = robot;
}

void Ray::setPlayer(Player *player)
{
    m_player = player;
}