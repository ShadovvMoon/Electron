//
//  client.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef __client__
#define __client__

#include "defines.h"
#include "game.h"
#define kPlayerNameLength 255

// Networking
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include "md5.h"

// Definitions
#define BUFFER_SIZE 4096
#define HALO_VER 616000


class Client {
private:
    Game *game;
    
    // TEA encryption
    uint8_t enckey1[16];
    uint8_t deckey2[16];
    uint8_t deckey1[16];
    
    uint8_t *build_buffer;
    uint8_t *send_buffer;
    uint8_t *buffer;
    
    uint8_t tick_counter = 0;
    int t = 0;
    
    // Socket
    int sock;
    struct  sockaddr_in host;
    
    // Client structures
    uint16_t server_position;
    uint16_t client_position;
    uint8_t  p_no;
    char *challenge;
    
    // Player structures
    Player *me;

    bool handshake();
    void connect_packet();
    long recv_bits();
    void spawn();
    void send_acknowledgement();
    void send_bit_packet(uint8_t *data, int b);
    void send_packet(uint8_t *data, uint8_t length);
    void join();
    void main();
    
    // Player information
    char name[kPlayerNameLength];
    
    // Packets
    void send_game_update();
    void send_pong();
    
    // Threading
    void start();
public:
    ~Client();
    Client(const char *name, const char *ip, short port);
};
Client *start_client(const char *ip, short port);
#endif /* defined(____render__) */
