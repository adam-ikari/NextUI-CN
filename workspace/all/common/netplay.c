#include "netplay.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/select.h>

#define MAX_PACKET_SIZE 4096

static int create_tcp_socket(void) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("NET: Failed to create TCP socket: %s\n", strerror(errno));
        return -1;
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    return sock;
}

static int create_udp_socket(void) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        printf("NET: Failed to create UDP socket: %s\n", strerror(errno));
        return -1;
    }
    
    // Enable broadcast
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
    
    return sock;
}

static int set_nonblocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0) {
        return -1;
    }
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

void NET_init(netplay_context_t *ctx, const char *device_name) {
    memset(ctx, 0, sizeof(netplay_context_t));
    
    if (device_name) {
        strncpy(ctx->device_name, device_name, NETPLAY_DEVICE_NAME_MAX - 1);
    }
    
    ctx->state = NETPLAY_STATE_IDLE;
    ctx->role = NETPLAY_ROLE_NONE;
    
    // Initialize sockets to -1 (invalid)
    ctx->server_socket = -1;
    for (int i = 0; i < NETPLAY_MAX_PEERS; i++) {
        ctx->client_sockets[i] = -1;
    }
    ctx->broadcast_socket = -1;
    
    printf("NET: Initialized netplay context for device: %s\n", ctx->device_name);
}

void NET_quit(netplay_context_t *ctx) {
    NET_stop_hosting(ctx);
    NET_disconnect(ctx);
    NET_stop_discovery(ctx);
    
    printf("NET: Quit netplay\n");
}

// Discovery functions
int NET_start_discovery(netplay_context_t *ctx) {
    ctx->broadcast_socket = create_udp_socket();
    if (ctx->broadcast_socket < 0) {
        return -1;
    }
    
    // Bind to broadcast port
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(NETPLAY_BROADCAST_PORT);
    
    if (bind(ctx->broadcast_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("NET: Failed to bind broadcast socket: %s\n", strerror(errno));
        close(ctx->broadcast_socket);
        ctx->broadcast_socket = -1;
        return -1;
    }
    
    if (set_nonblocking(ctx->broadcast_socket) < 0) {
        printf("NET: Failed to set broadcast socket non-blocking\n");
        close(ctx->broadcast_socket);
        ctx->broadcast_socket = -1;
        return -1;
    }
    
    ctx->discovery_enabled = true;
    printf("NET: Discovery started on port %d\n", NETPLAY_BROADCAST_PORT);
    
    return 0;
}

void NET_stop_discovery(netplay_context_t *ctx) {
    if (ctx->broadcast_socket >= 0) {
        close(ctx->broadcast_socket);
        ctx->broadcast_socket = -1;
    }
    ctx->discovery_enabled = false;
    printf("NET: Discovery stopped\n");
}

int NET_broadcast_discovery(netplay_context_t *ctx) {
    if (ctx->broadcast_socket < 0 || !ctx->discovery_enabled) {
        return -1;
    }
    
    // Build discovery packet
    uint8_t packet[MAX_PACKET_SIZE];
    int offset = 0;
    
    netpacket_header_t *header = (netpacket_header_t*)packet;
    header->magic = NETPLAY_MAGIC;
    header->type = NETPACKET_DISCOVERY;
    header->version = 1;
    header->length = NETPLAY_DEVICE_NAME_MAX;
    header->sequence = 0;
    offset += sizeof(netpacket_header_t);
    
    memcpy(packet + offset, ctx->device_name, NETPLAY_DEVICE_NAME_MAX);
    offset += NETPLAY_DEVICE_NAME_MAX;
    
    // Add role and player count
    uint8_t role = ctx->role;
    memcpy(packet + offset, &role, 1);
    offset += 1;
    
    uint8_t player_count = (ctx->role == NETPLAY_ROLE_HOST) ? 1 : 0;
    memcpy(packet + offset, &player_count, 1);
    offset += 1;
    
    header->length = offset - sizeof(netpacket_header_t);
    
    // Send broadcast
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_BROADCAST;
    addr.sin_port = htons(NETPLAY_BROADCAST_PORT);
    
    int sent = sendto(ctx->broadcast_socket, packet, offset, 0,
                       (struct sockaddr*)&addr, sizeof(addr));
    
    if (sent < 0) {
        printf("NET: Failed to send broadcast: %s\n", strerror(errno));
        return -1;
    }
    
    return sent;
}

int NET_handle_discovery_packet(netplay_context_t *ctx, const char *data, int len) {
    if (len < sizeof(netpacket_header_t)) {
        return -1;
    }
    
    netpacket_header_t *header = (netpacket_header_t*)data;
    if (header->magic != NETPLAY_MAGIC) {
        return -1;
    }
    
    if (header->type == NETPACKET_DISCOVERY) {
        // Build response packet
        uint8_t response[MAX_PACKET_SIZE];
        int offset = 0;
        
        netpacket_header_t *resp_header = (netpacket_header_t*)response;
        resp_header->magic = NETPLAY_MAGIC;
        resp_header->type = NETPACKET_DISCOVERY_RESPONSE;
        resp_header->version = 1;
        resp_header->sequence = header->sequence;
        offset += sizeof(netpacket_header_t);
        
        // Device info
        memcpy(response + offset, ctx->device_name, NETPLAY_DEVICE_NAME_MAX);
        offset += NETPLAY_DEVICE_NAME_MAX;
        
        uint8_t role = ctx->role;
        memcpy(response + offset, &role, 1);
        offset += 1;
        
        uint8_t player_count = (ctx->role == NETPLAY_ROLE_HOST) ? ctx->client_count + 1 : 0;
        memcpy(response + offset, &player_count, 1);
        offset += 1;
        
        resp_header->length = offset - sizeof(netpacket_header_t);
        
        // Send response back to sender
        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        if (recvfrom(ctx->broadcast_socket, NULL, 0, MSG_PEEK,
                    (struct sockaddr*)&addr, &addr_len) < 0) {
            return -1;
        }
        
        sendto(ctx->broadcast_socket, response, offset, 0,
               (struct sockaddr*)&addr, addr_len);
        
        return 0;
    }
    
    return -1;
}

// Hosting functions
int NET_start_hosting(netplay_context_t *ctx) {
    ctx->server_socket = create_tcp_socket();
    if (ctx->server_socket < 0) {
        return -1;
    }
    
    // Bind to netplay port
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(NETPLAY_PORT);
    
    if (bind(ctx->server_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("NET: Failed to bind server socket: %s\n", strerror(errno));
        close(ctx->server_socket);
        ctx->server_socket = -1;
        return -1;
    }
    
    if (listen(ctx->server_socket, NETPLAY_MAX_PEERS) < 0) {
        printf("NET: Failed to listen: %s\n", strerror(errno));
        close(ctx->server_socket);
        ctx->server_socket = -1;
        return -1;
    }
    
    if (set_nonblocking(ctx->server_socket) < 0) {
        printf("NET: Failed to set server socket non-blocking\n");
        close(ctx->server_socket);
        ctx->server_socket = -1;
        return -1;
    }
    
    ctx->role = NETPLAY_ROLE_HOST;
    ctx->state = NETPLAY_STATE_HOSTING;
    ctx->client_count = 0;
    ctx->frame_count = 0;
    
    printf("NET: Started hosting on port %d\n", NETPLAY_PORT);
    
    return 0;
}

void NET_stop_hosting(netplay_context_t *ctx) {
    if (ctx->server_socket >= 0) {
        close(ctx->server_socket);
        ctx->server_socket = -1;
    }
    
    for (int i = 0; i < NETPLAY_MAX_PEERS; i++) {
        if (ctx->client_sockets[i] >= 0) {
            close(ctx->client_sockets[i]);
            ctx->client_sockets[i] = -1;
        }
    }
    
    ctx->client_count = 0;
    ctx->role = NETPLAY_ROLE_NONE;
    ctx->state = NETPLAY_STATE_IDLE;
    
    printf("NET: Stopped hosting\n");
}

int NET_poll_host(netplay_context_t *ctx) {
    if (ctx->state != NETPLAY_STATE_HOSTING || ctx->server_socket < 0) {
        return -1;
    }
    
    // Accept new connections
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_sock = accept(ctx->server_socket, (struct sockaddr*)&client_addr, &addr_len);
    
    if (client_sock >= 0) {
        if (ctx->client_count < NETPLAY_MAX_PEERS) {
            // Find empty slot
            for (int i = 0; i < NETPLAY_MAX_PEERS; i++) {
                if (ctx->client_sockets[i] < 0) {
                    ctx->client_sockets[i] = client_sock;
                    ctx->client_count++;
                    
                    char *ip = inet_ntoa(client_addr.sin_addr);
                    printf("NET: Client %d connected from %s\n", i + 1, ip);
                    
                    // Send connect response
                    uint8_t response[sizeof(netpacket_header_t)];
                    netpacket_header_t *header = (netpacket_header_t*)response;
                    header->magic = NETPLAY_MAGIC;
                    header->type = NETPACKET_CONNECT_RESPONSE;
                    header->version = 1;
                    header->length = 0;
                    header->sequence = 0;
                    
                    send(client_sock, response, sizeof(response), 0);
                    
                    if (ctx->on_connection_state_changed) {
                        ctx->on_connection_state_changed(ctx->state);
                    }
                    
                    break;
                }
            }
        } else {
            printf("NET: Connection rejected (max peers reached)\n");
            close(client_sock);
        }
    }
    
    // Poll existing clients
    for (int i = 0; i < NETPLAY_MAX_PEERS; i++) {
        if (ctx->client_sockets[i] >= 0) {
            uint8_t buffer[MAX_PACKET_SIZE];
            int received = recv(ctx->client_sockets[i], buffer, MAX_PACKET_SIZE, MSG_DONTWAIT);
            
            if (received <= 0) {
                // Client disconnected
                close(ctx->client_sockets[i]);
                ctx->client_sockets[i] = -1;
                ctx->client_count--;
                
                printf("NET: Client %d disconnected\n", i + 1);
                
                if (ctx->client_count == 0) {
                    ctx->state = NETPLAY_STATE_HOSTING;
                    if (ctx->on_connection_state_changed) {
                        ctx->on_connection_state_changed(ctx->state);
                    }
                }
            } else {
                // Process packet
                netpacket_header_t *header = (netpacket_header_t*)buffer;
                if (header->magic == NETPLAY_MAGIC && header->type == NETPACKET_INPUT_STATE) {
                    // Parse input
                    if (header->length >= sizeof(netplay_input_t)) {
                        netplay_input_t *input = (netplay_input_t*)(buffer + sizeof(netpacket_header_t));
                        memcpy(&ctx->remote_inputs[i], input, sizeof(netplay_input_t));
                        
                        if (ctx->on_input_received) {
                            ctx->on_input_received(input);
                        }
                    }
                } else if (header->magic == NETPLAY_MAGIC && header->type == NETPACKET_PING) {
                    // Send pong
                    uint8_t pong[sizeof(netpacket_header_t)];
                    netpacket_header_t *pong_header = (netpacket_header_t*)pong;
                    pong_header->magic = NETPLAY_MAGIC;
                    pong_header->type = NETPACKET_PONG;
                    pong_header->version = 1;
                    pong_header->length = 0;
                    pong_header->sequence = header->sequence;
                    
                    send(ctx->client_sockets[i], pong, sizeof(pong), 0);
                }
            }
        }
    }
    
    return ctx->client_count;
}

// Client functions
int NET_connect_to_host(netplay_context_t *ctx, const char *host_ip) {
    ctx->server_socket = create_tcp_socket();
    if (ctx->server_socket < 0) {
        return -1;
    }
    
    strncpy(ctx->server_ip, host_ip, 15);
    ctx->server_ip[15] = '\0';
    
    // Connect to host
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(NETPLAY_PORT);
    inet_pton(AF_INET, host_ip, &addr.sin_addr);
    
    ctx->state = NETPLAY_STATE_CONNECTING;
    
    if (connect(ctx->server_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("NET: Failed to connect to %s: %s\n", host_ip, strerror(errno));
        close(ctx->server_socket);
        ctx->server_socket = -1;
        ctx->state = NETPLAY_STATE_ERROR;
        return -1;
    }
    
    if (set_nonblocking(ctx->server_socket) < 0) {
        printf("NET: Failed to set client socket non-blocking\n");
        close(ctx->server_socket);
        ctx->server_socket = -1;
        ctx->state = NETPLAY_STATE_ERROR;
        return -1;
    }
    
    // Wait for connect response
    uint8_t buffer[sizeof(netpacket_header_t)];
    int received = recv(ctx->server_socket, buffer, sizeof(buffer), 0);
    
    if (received >= sizeof(netpacket_header_t)) {
        netpacket_header_t *header = (netpacket_header_t*)buffer;
        if (header->magic == NETPLAY_MAGIC && header->type == NETPACKET_CONNECT_RESPONSE) {
            ctx->role = NETPLAY_ROLE_CLIENT;
            ctx->state = NETPLAY_STATE_CONNECTED;
            ctx->frame_count = 0;
            
            printf("NET: Connected to host %s\n", host_ip);
            
            if (ctx->on_connection_state_changed) {
                ctx->on_connection_state_changed(ctx->state);
            }
            
            return 0;
        }
    }
    
    printf("NET: Failed to receive connect response\n");
    close(ctx->server_socket);
    ctx->server_socket = -1;
    ctx->state = NETPLAY_STATE_ERROR;
    
    return -1;
}

void NET_disconnect(netplay_context_t *ctx) {
    if (ctx->server_socket >= 0) {
        // Send disconnect packet
        uint8_t packet[sizeof(netpacket_header_t)];
        netpacket_header_t *header = (netpacket_header_t*)packet;
        header->magic = NETPLAY_MAGIC;
        header->type = NETPACKET_DISCONNECT;
        header->version = 1;
        header->length = 0;
        header->sequence = 0;
        
        send(ctx->server_socket, packet, sizeof(packet), 0);
        
        close(ctx->server_socket);
        ctx->server_socket = -1;
    }
    
    ctx->role = NETPLAY_ROLE_NONE;
    ctx->state = NETPLAY_STATE_IDLE;
    
    printf("NET: Disconnected\n");
}

int NET_poll_client(netplay_context_t *ctx) {
    if (ctx->state != NETPLAY_STATE_CONNECTED || ctx->server_socket < 0) {
        return -1;
    }
    
    uint8_t buffer[MAX_PACKET_SIZE];
    int received = recv(ctx->server_socket, buffer, MAX_PACKET_SIZE, MSG_DONTWAIT);
    
    if (received < 0) {
        return 0;
    }
    
    if (received < sizeof(netpacket_header_t)) {
        return -1;
    }
    
    netpacket_header_t *header = (netpacket_header_t*)buffer;
    if (header->magic != NETPLAY_MAGIC) {
        return -1;
    }
    
    if (header->type == NETPACKET_INPUT_STATE) {
        if (header->length >= sizeof(netplay_input_t)) {
            netplay_input_t *input = (netplay_input_t*)(buffer + sizeof(netpacket_header_t));
            memcpy(&ctx->remote_inputs[0], input, sizeof(netplay_input_t));
            
            if (ctx->on_input_received) {
                ctx->on_input_received(input);
            }
        }
    } else if (header->type == NETPACKET_STATE_SYNC) {
        if (ctx->on_input_received) {
            ctx->on_input_received((netplay_input_t*)(buffer + sizeof(netpacket_header_t)));
        }
    } else if (header->type == NETPACKET_PING) {
        // Send pong
        uint8_t pong[sizeof(netpacket_header_t)];
        netpacket_header_t *pong_header = (netpacket_header_t*)pong;
        pong_header->magic = NETPLAY_MAGIC;
        pong_header->type = NETPACKET_PONG;
        pong_header->version = 1;
        pong_header->length = 0;
        pong_header->sequence = header->sequence;
        
        send(ctx->server_socket, pong, sizeof(pong), 0);
    }
    
    return received;
}

// Input functions
void NET_set_local_input(netplay_context_t *ctx, const netplay_input_t *input) {
    memcpy(&ctx->local_input, input, sizeof(netplay_input_t));
}

void NET_send_input(netplay_context_t *ctx) {
    if (ctx->state != NETPLAY_STATE_CONNECTED) {
        return;
    }
    
    uint8_t packet[MAX_PACKET_SIZE];
    int offset = 0;
    
    netpacket_header_t *header = (netpacket_header_t*)packet;
    header->magic = NETPLAY_MAGIC;
    header->type = NETPACKET_INPUT_STATE;
    header->version = 1;
    header->sequence = ctx->frame_count;
    offset += sizeof(netpacket_header_t);
    
    memcpy(packet + offset, &ctx->local_input, sizeof(netplay_input_t));
    offset += sizeof(netplay_input_t);
    
    header->length = offset - sizeof(netpacket_header_t);
    
    if (ctx->role == NETPLAY_ROLE_CLIENT && ctx->server_socket >= 0) {
        send(ctx->server_socket, packet, offset, 0);
    } else if (ctx->role == NETPLAY_ROLE_HOST) {
        // Broadcast to all clients
        for (int i = 0; i < NETPLAY_MAX_PEERS; i++) {
            if (ctx->client_sockets[i] >= 0) {
                send(ctx->client_sockets[i], packet, offset, 0);
            }
        }
    }
    
    ctx->frame_count++;
}

netplay_input_t *NET_get_remote_input(netplay_context_t *ctx, int player_index) {
    if (player_index >= 0 && player_index < NETPLAY_MAX_PEERS) {
        return &ctx->remote_inputs[player_index];
    }
    return NULL;
}

// State sync functions
void NET_send_state(netplay_context_t *ctx, const void *state_data, int size) {
    if (ctx->state != NETPLAY_STATE_CONNECTED || size > MAX_PACKET_SIZE - sizeof(netpacket_header_t)) {
        return;
    }
    
    uint8_t packet[MAX_PACKET_SIZE];
    int offset = 0;
    
    netpacket_header_t *header = (netpacket_header_t*)packet;
    header->magic = NETPLAY_MAGIC;
    header->type = NETPACKET_STATE_SYNC;
    header->version = 1;
    header->sequence = ctx->frame_count;
    offset += sizeof(netpacket_header_t);
    
    memcpy(packet + offset, state_data, size);
    offset += size;
    
    header->length = offset - sizeof(netpacket_header_t);
    
    if (ctx->role == NETPLAY_ROLE_HOST && ctx->server_socket >= 0) {
        // Broadcast to all clients
        for (int i = 0; i < NETPLAY_MAX_PEERS; i++) {
            if (ctx->client_sockets[i] >= 0) {
                send(ctx->client_sockets[i], packet, offset, 0);
            }
        }
    }
}

int NET_receive_state(netplay_context_t *ctx, void *state_data, int max_size) {
    if (ctx->state != NETPLAY_STATE_CONNECTED) {
        return -1;
    }
    
    // Note: State packets are received in poll_client() and passed via callback
    // This function is just for compatibility
    return -1;
}

// Ping/Latency functions
uint32_t NET_get_latency(netplay_context_t *ctx) {
    return ctx->latency_ms;
}

void NET_send_ping(netplay_context_t *ctx) {
    if (ctx->state != NETPLAY_STATE_CONNECTED) {
        return;
    }
    
    uint8_t packet[sizeof(netpacket_header_t)];
    netpacket_header_t *header = (netpacket_header_t*)packet;
    header->magic = NETPLAY_MAGIC;
    header->type = NETPACKET_PING;
    header->version = 1;
    header->length = 0;
    header->sequence = ctx->frame_count;
    
    uint32_t ping_time = time(NULL) * 1000;
    header->length = sizeof(ping_time);
    memcpy(packet + sizeof(netpacket_header_t), &ping_time, sizeof(ping_time));
    
    if (ctx->role == NETPLAY_ROLE_CLIENT && ctx->server_socket >= 0) {
        send(ctx->server_socket, packet, sizeof(packet) + sizeof(ping_time), 0);
    } else if (ctx->role == NETPLAY_ROLE_HOST) {
        for (int i = 0; i < NETPLAY_MAX_PEERS; i++) {
            if (ctx->client_sockets[i] >= 0) {
                send(ctx->client_sockets[i], packet, sizeof(packet) + sizeof(ping_time), 0);
            }
        }
    }
}