#ifndef __RAY_H__
#define __RAY_H__

//struct uip_udp_conn;
#include <stdint.h>

#include "coap.h"
#include "coap_dump.h"

class Robot;
class Player;

typedef struct {
    float mcs_x;
    float mcs_y;
    float mcs_z;
    float mcs_b;
    uint32_t state;
    uint32_t played;
    int32_t total;
} ray_payload_t;

class Ray {
public:
    Ray();
    ~Ray();

    void init(struct uip_udp_conn *udp_rxonly, struct uip_udp_conn *udp_txonly);
    void appcall();

    void setRobot(Robot *robot);
    void setPlayer(Player *player);

private:
    void process_packet(uint8_t *buf, uint16_t size);
    void send_response(uint16_t size);

    struct uip_udp_conn *m_udp_rxonly;
    struct uip_udp_conn *m_udp_txonly;
    bool m_notifications_enabled;
    uint16_t m_notify_remote_ip[2];
    uint16_t m_notify_remote_port;

    Robot *m_robot;
    Player *m_player;
    ray_payload_t m_payload;
};

#endif //__RAY_H__
