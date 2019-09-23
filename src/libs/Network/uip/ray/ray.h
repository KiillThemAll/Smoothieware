#ifndef __RAY_H__
#define __RAY_H__

//struct uip_udp_conn;
#include <stdint.h>

#include "coap.h"
#include "coap_dump.h"

class Ray {
public:
    Ray();
    ~Ray();

    void init(struct uip_udp_conn *udp_rxonly, struct uip_udp_conn *udp_txonly);
    void appcall();

private:
    void process_packet(uint8_t *buf, uint16_t size);
    void send_response(uint16_t size);

    struct uip_udp_conn *m_udp_rxonly;
    struct uip_udp_conn *m_udp_txonly;
    bool m_notifications_enabled;
    uint16_t m_notify_remote_ip[2];
    uint16_t m_notify_remote_port;
};

#endif //__RAY_H__
