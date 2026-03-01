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
#include <pthread.h>

#define MAX_PACKET_SIZE 4096
#define MAX_DATA_SIZE (MAX_PACKET_SIZE - sizeof(netpacket_header_t))

// Helper function to validate packet header
static int validate_packet_header(const netpacket_header_t *header, const char *source_desc) {
    if (header->magic != NETPLAY_MAGIC) {
        printf("NET: Invalid magic number from %s\n", source_desc);
        return -1;
    }
    
    if (header->version != NETPLAY_PROTOCOL_VERSION) {
        printf("NET: Protocol version mismatch from %s: got %u, expected %u\n", 
               source_desc, header->version, NETPLAY_PROTOCOL_VERSION);
        return -1;
    }
    
    if (header->length > MAX_DATA_SIZE) {
        printf("NET: Packet too large from %s: %u\n", source_desc, header->length);
        return -1;
    }
    
    return 0;
}

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
    
    // Initialize mutex for thread safety
    pthread_mutex_init(&ctx->mutex, NULL);
    
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
    
    // Destroy mutex
    pthread_mutex_destroy(&ctx->mutex);
    
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
    
    if (sent != offset) {
        printf("NET: Broadcast partial send: %d/%d bytes\n", sent, offset);
        return -1;
    }
    
    return sent;
}

int NET_handle_discovery_packet(netplay_context_t *ctx, const char *data, int len) {
    if (len < (int)sizeof(netpacket_header_t)) {
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
        
        if (sendto(ctx->broadcast_socket, response, offset, 0,
               (struct sockaddr*)&addr, addr_len) < 0) {
        printf("NET: Failed to send discovery response: %s\n", strerror(errno));
        return -1;
    }
        
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
    
    pthread_mutex_lock(&ctx->mutex);
    ctx->role = NETPLAY_ROLE_HOST;
    ctx->state = NETPLAY_STATE_HOSTING;
    ctx->client_count = 0;
    ctx->frame_count = 0;
    pthread_mutex_unlock(&ctx->mutex);
    
    printf("NET: Started hosting on port %d\n", NETPLAY_PORT);
    
    return 0;
}

void NET_stop_hosting(netplay_context_t *ctx) {
    pthread_mutex_lock(&ctx->mutex);
    
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
    
    pthread_mutex_unlock(&ctx->mutex);
    
    printf("NET: Stopped hosting\n");
}

int NET_poll_host(netplay_context_t *ctx) {
    int server_socket, client_count;
    int client_sockets[NETPLAY_MAX_PEERS];
    netplay_state_t state;
    
    pthread_mutex_lock(&ctx->mutex);
    server_socket = ctx->server_socket;
    state = ctx->state;
    client_count = ctx->client_count;
    memcpy(client_sockets, ctx->client_sockets, sizeof(client_sockets));
    pthread_mutex_unlock(&ctx->mutex);
    
    if (state != NETPLAY_STATE_HOSTING || server_socket < 0) {
        return -1;
    }
    
    // Accept new connections
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_sock = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
    
    if (client_sock >= 0) {
        pthread_mutex_lock(&ctx->mutex);
        if (ctx->client_count < NETPLAY_MAX_PEERS) {
            // Find empty slot
            for (int i = 0; i < NETPLAY_MAX_PEERS; i++) {
                if (ctx->client_sockets[i] < 0) {
                    ctx->client_sockets[i] = client_sock;
                    ctx->client_count++;
                    
                    char ip_str[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
                    printf("NET: Client %d connected from %s\n", i + 1, ip_str);
                    
                    pthread_mutex_unlock(&ctx->mutex);
                    
                    // Send connect response
                    uint8_t response[sizeof(netpacket_header_t)];
                    netpacket_header_t *header = (netpacket_header_t*)response;
                    header->magic = NETPLAY_MAGIC;
                    header->type = NETPACKET_CONNECT_RESPONSE;
                    header->version = 1;
                    header->length = 0;
                    header->sequence = 0;
                    
                    if (send(client_sock, response, sizeof(response), 0) < 0) {
                        printf("NET: Failed to send connect response to client %d: %s\n", i + 1, strerror(errno));
                    }
                    
                    pthread_mutex_lock(&ctx->mutex);
                    if (ctx->on_connection_state_changed) {
                        pthread_mutex_unlock(&ctx->mutex);
                        ctx->on_connection_state_changed(ctx->state);
                        pthread_mutex_lock(&ctx->mutex);
                    }
                    
                    break;
                }
            }
        } else {
            printf("NET: Connection rejected (max peers reached)\n");
            close(client_sock);
        }
        pthread_mutex_unlock(&ctx->mutex);
    }
    
    // Poll existing clients
    pthread_mutex_lock(&ctx->mutex);
    memcpy(client_sockets, ctx->client_sockets, sizeof(client_sockets));
    client_count = ctx->client_count;
    pthread_mutex_unlock(&ctx->mutex);
    
    for (int i = 0; i < NETPLAY_MAX_PEERS; i++) {
        if (client_sockets[i] >= 0) {
            uint8_t buffer[MAX_PACKET_SIZE];
            int received = recv(client_sockets[i], buffer, MAX_PACKET_SIZE, MSG_DONTWAIT);
            
            if (received <= 0) {
                // Client disconnected
                pthread_mutex_lock(&ctx->mutex);
                if (ctx->client_sockets[i] == client_sockets[i]) {
                    close(ctx->client_sockets[i]);
                    ctx->client_sockets[i] = -1;
                    ctx->client_count--;
                    
                    printf("NET: Client %d disconnected\n", i + 1);
                    
                    if (ctx->client_count == 0) {
                        ctx->state = NETPLAY_STATE_HOSTING;
                        if (ctx->on_connection_state_changed) {
                            pthread_mutex_unlock(&ctx->mutex);
                            ctx->on_connection_state_changed(ctx->state);
                            pthread_mutex_lock(&ctx->mutex);
                        }
                    }
                }
                pthread_mutex_unlock(&ctx->mutex);
            } else {
                // Process packet
                netpacket_header_t *header = (netpacket_header_t*)buffer;
                
                char source_desc[32];
                snprintf(source_desc, sizeof(source_desc), "client %d", i + 1);
                
                if (validate_packet_header(header, source_desc) < 0) {
                    continue;
                }
                
                if (header->type == NETPACKET_INPUT_STATE) {
                    // Parse input
                    if (header->length >= sizeof(netplay_input_t)) {
                        netplay_input_t *input = (netplay_input_t*)(buffer + sizeof(netpacket_header_t));
                        pthread_mutex_lock(&ctx->mutex);
                        memcpy(&ctx->remote_inputs[i], input, sizeof(netplay_input_t));
                        pthread_mutex_unlock(&ctx->mutex);
                        
                        if (ctx->on_input_received) {
                            ctx->on_input_received(input);
                        }
                    }
                } else if (header->type == NETPACKET_PING) {
                    // Send pong with the same ping_time payload
                    uint8_t pong[sizeof(netpacket_header_t) + sizeof(uint32_t)];
                    netpacket_header_t *pong_header = (netpacket_header_t*)pong;
                    pong_header->magic = NETPLAY_MAGIC;
                    pong_header->type = NETPACKET_PONG;
                    pong_header->version = 1;
                    pong_header->sequence = header->sequence;
                    
                    if (header->length == sizeof(uint32_t)) {
                        uint32_t ping_time = *(uint32_t*)(buffer + sizeof(netpacket_header_t));
                        pong_header->length = sizeof(ping_time);
                        memcpy(pong + sizeof(netpacket_header_t), &ping_time, sizeof(ping_time));
                    } else {
                        pong_header->length = 0;
                    }
                    
                    if (send(client_sockets[i], pong, sizeof(netpacket_header_t) + pong_header->length, 0) < 0) {
                        printf("NET: Failed to send pong to client %d: %s\n", i + 1, strerror(errno));
                    }
                } else if (header->type == NETPACKET_PONG) {
                    // Handle pong response and calculate latency
                    if (header->length == sizeof(uint32_t)) {
                        uint32_t ping_time = *(uint32_t*)(buffer + sizeof(netpacket_header_t));
                        struct timespec ts;
                        clock_gettime(CLOCK_MONOTONIC, &ts);
                        uint32_t current_time = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
                        pthread_mutex_lock(&ctx->mutex);
                        ctx->latency_ms = (current_time - ping_time) / 2;  // RTT / 2 = one-way latency
                        pthread_mutex_unlock(&ctx->mutex);
                        printf("NET: Latency updated to %u ms (from client %d)\n", ctx->latency_ms, i + 1);
                    }
                }
            }
        }
    }
    
    pthread_mutex_lock(&ctx->mutex);
    client_count = ctx->client_count;
    pthread_mutex_unlock(&ctx->mutex);
    
    return client_count;
}

// Client functions
int NET_connect_to_host(netplay_context_t *ctx, const char *host_ip) {
    ctx->server_socket = create_tcp_socket();
    if (ctx->server_socket < 0) {
        return -1;
    }
    
    strncpy(ctx->server_ip, host_ip, sizeof(ctx->server_ip) - 1);
    ctx->server_ip[sizeof(ctx->server_ip) - 1] = '\0';
    
    // Set socket to non-blocking first
    if (set_nonblocking(ctx->server_socket) < 0) {
        printf("NET: Failed to set client socket non-blocking\n");
        close(ctx->server_socket);
        ctx->server_socket = -1;
        return -1;
    }
    
    // Connect to host (non-blocking)
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(NETPLAY_PORT);
    inet_pton(AF_INET, host_ip, &addr.sin_addr);
    
    pthread_mutex_lock(&ctx->mutex);
    ctx->state = NETPLAY_STATE_CONNECTING;
    pthread_mutex_unlock(&ctx->mutex);
    
    int result = connect(ctx->server_socket, (struct sockaddr*)&addr, sizeof(addr));
    if (result < 0 && errno != EINPROGRESS) {
        printf("NET: Failed to connect to %s: %s\n", host_ip, strerror(errno));
        close(ctx->server_socket);
        ctx->server_socket = -1;
        pthread_mutex_lock(&ctx->mutex);
        ctx->state = NETPLAY_STATE_ERROR;
        pthread_mutex_unlock(&ctx->mutex);
        return -1;
    }
    
    // Wait for connection to complete using select
    if (result < 0) {
        // Connection in progress, wait for it to complete
        fd_set write_fds;
        FD_ZERO(&write_fds);
        FD_SET(ctx->server_socket, &write_fds);
        
        struct timeval timeout;
        timeout.tv_sec = 10;  // 10 second timeout
        timeout.tv_usec = 0;
        
        result = select(ctx->server_socket + 1, NULL, &write_fds, NULL, &timeout);
        if (result <= 0) {
            printf("NET: Connection timeout or error to %s\n", host_ip);
            close(ctx->server_socket);
            ctx->server_socket = -1;
            pthread_mutex_lock(&ctx->mutex);
            ctx->state = NETPLAY_STATE_ERROR;
            pthread_mutex_unlock(&ctx->mutex);
            return -1;
        }
        
        // Check for connection errors
        int error = 0;
        socklen_t len = sizeof(error);
        if (getsockopt(ctx->server_socket, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
            printf("NET: Connection failed to %s: %s\n", host_ip, strerror(error));
            close(ctx->server_socket);
            ctx->server_socket = -1;
            pthread_mutex_lock(&ctx->mutex);
            ctx->state = NETPLAY_STATE_ERROR;
            pthread_mutex_unlock(&ctx->mutex);
            return -1;
        }
    }
    
    // Wait for connect response with timeout
    uint8_t buffer[sizeof(netpacket_header_t)];
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(ctx->server_socket, &read_fds);
    
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    
    result = select(ctx->server_socket + 1, &read_fds, NULL, NULL, &timeout);
    if (result <= 0) {
        printf("NET: Timeout waiting for connect response from %s\n", host_ip);
        close(ctx->server_socket);
        ctx->server_socket = -1;
        pthread_mutex_lock(&ctx->mutex);
        ctx->state = NETPLAY_STATE_ERROR;
        pthread_mutex_unlock(&ctx->mutex);
        return -1;
    }
    
    int received = recv(ctx->server_socket, buffer, sizeof(buffer), MSG_DONTWAIT);
    if (received >= (int)sizeof(netpacket_header_t)) {
        netpacket_header_t *header = (netpacket_header_t*)buffer;
        if (header->magic == NETPLAY_MAGIC && header->type == NETPACKET_CONNECT_RESPONSE) {
            pthread_mutex_lock(&ctx->mutex);
            ctx->role = NETPLAY_ROLE_CLIENT;
            ctx->state = NETPLAY_STATE_CONNECTED;
            ctx->frame_count = 0;
            pthread_mutex_unlock(&ctx->mutex);
            
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
    pthread_mutex_lock(&ctx->mutex);
    ctx->state = NETPLAY_STATE_ERROR;
    pthread_mutex_unlock(&ctx->mutex);
    
    return -1;
}

void NET_disconnect(netplay_context_t *ctx) {
    int server_socket;
    
    pthread_mutex_lock(&ctx->mutex);
    server_socket = ctx->server_socket;
    pthread_mutex_unlock(&ctx->mutex);
    
    if (server_socket >= 0) {
        // Send disconnect packet
        uint8_t packet[sizeof(netpacket_header_t)];
        netpacket_header_t *header = (netpacket_header_t*)packet;
        header->magic = NETPLAY_MAGIC;
        header->type = NETPACKET_DISCONNECT;
        header->version = 1;
        header->length = 0;
        header->sequence = 0;
        
        if (send(server_socket, packet, sizeof(packet), 0) < 0) {
            printf("NET: Failed to send disconnect packet: %s\n", strerror(errno));
        }
        
        close(server_socket);
        
        pthread_mutex_lock(&ctx->mutex);
        ctx->server_socket = -1;
        pthread_mutex_unlock(&ctx->mutex);
    }
    
    pthread_mutex_lock(&ctx->mutex);
    ctx->role = NETPLAY_ROLE_NONE;
    ctx->state = NETPLAY_STATE_IDLE;
    pthread_mutex_unlock(&ctx->mutex);
    
    printf("NET: Disconnected\n");
}

int NET_poll_client(netplay_context_t *ctx) {
    int server_socket;
    netplay_state_t state;
    
    pthread_mutex_lock(&ctx->mutex);
    state = ctx->state;
    server_socket = ctx->server_socket;
    pthread_mutex_unlock(&ctx->mutex);
    
    if (state != NETPLAY_STATE_CONNECTED || server_socket < 0) {
        return -1;
    }
    
    uint8_t buffer[MAX_PACKET_SIZE];
    int received = recv(server_socket, buffer, MAX_PACKET_SIZE, MSG_DONTWAIT);
    
    if (received < 0) {
        return 0;
    }
    
    if (received < (int)sizeof(netpacket_header_t)) {
        return -1;
    }
    
    netpacket_header_t *header = (netpacket_header_t*)buffer;
    
    if (validate_packet_header(header, "server") < 0) {
        return -1;
    }
    
    if (header->type == NETPACKET_INPUT_STATE) {
        if (header->length >= sizeof(netplay_input_t)) {
            netplay_input_t *input = (netplay_input_t*)(buffer + sizeof(netpacket_header_t));
            pthread_mutex_lock(&ctx->mutex);
            memcpy(&ctx->remote_inputs[0], input, sizeof(netplay_input_t));
            pthread_mutex_unlock(&ctx->mutex);
            
            if (ctx->on_input_received) {
                ctx->on_input_received(input);
            }
        }
    } else if (header->type == NETPACKET_STATE_SYNC) {
        // Handle state synchronization
        if (header->length > 0 && ctx->on_state_received) {
            void *state_data = buffer + sizeof(netpacket_header_t);
            ctx->on_state_received(state_data, header->length, header->sequence);
        }
    } else if (header->type == NETPACKET_PING) {
        // Send pong with the same ping_time payload
        uint8_t pong[sizeof(netpacket_header_t) + sizeof(uint32_t)];
        netpacket_header_t *pong_header = (netpacket_header_t*)pong;
        pong_header->magic = NETPLAY_MAGIC;
        pong_header->type = NETPACKET_PONG;
        pong_header->version = 1;
        pong_header->sequence = header->sequence;
        
        if (header->length == sizeof(uint32_t)) {
            uint32_t ping_time = *(uint32_t*)(buffer + sizeof(netpacket_header_t));
            pong_header->length = sizeof(ping_time);
            memcpy(pong + sizeof(netpacket_header_t), &ping_time, sizeof(ping_time));
        } else {
            pong_header->length = 0;
        }
        
        if (send(server_socket, pong, sizeof(netpacket_header_t) + pong_header->length, 0) < 0) {
            printf("NET: Failed to send pong to server: %s\n", strerror(errno));
        }
    } else if (header->type == NETPACKET_PONG) {
        // Handle pong response and calculate latency
        if (header->length == sizeof(uint32_t)) {
            uint32_t ping_time = *(uint32_t*)(buffer + sizeof(netpacket_header_t));
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            uint32_t current_time = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
            pthread_mutex_lock(&ctx->mutex);
            ctx->latency_ms = (current_time - ping_time) / 2;  // RTT / 2 = one-way latency
            pthread_mutex_unlock(&ctx->mutex);
            printf("NET: Latency updated to %u ms (from server)\n", ctx->latency_ms);
        }
    }
    
    return received;
}

// Input functions
void NET_set_local_input(netplay_context_t *ctx, const netplay_input_t *input) {
    pthread_mutex_lock(&ctx->mutex);
    memcpy(&ctx->local_input, input, sizeof(netplay_input_t));
    pthread_mutex_unlock(&ctx->mutex);
}

void NET_send_input(netplay_context_t *ctx) {
    pthread_mutex_lock(&ctx->mutex);
    
    if (ctx->state != NETPLAY_STATE_CONNECTED) {
        pthread_mutex_unlock(&ctx->mutex);
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
    
    int server_socket = ctx->server_socket;
    netplay_role_t role = ctx->role;
    int client_sockets[NETPLAY_MAX_PEERS];
    memcpy(client_sockets, ctx->client_sockets, sizeof(client_sockets));
    
    ctx->frame_count++;
    
    pthread_mutex_unlock(&ctx->mutex);
    
    // Send packets outside of mutex to avoid holding lock during I/O
    if (role == NETPLAY_ROLE_CLIENT && server_socket >= 0) {
        if (send(server_socket, packet, offset, 0) < 0) {
            printf("NET: Failed to send input to server: %s\n", strerror(errno));
        }
    } else if (role == NETPLAY_ROLE_HOST) {
        // Broadcast to all clients
        for (int i = 0; i < NETPLAY_MAX_PEERS; i++) {
            if (client_sockets[i] >= 0) {
                if (send(client_sockets[i], packet, offset, 0) < 0) {
                    printf("NET: Failed to send input to client %d: %s\n", i + 1, strerror(errno));
                }
            }
        }
    }
}

netplay_input_t *NET_get_remote_input(netplay_context_t *ctx, int player_index) {
    if (player_index >= 0 && player_index < NETPLAY_MAX_PEERS) {
        pthread_mutex_lock(&ctx->mutex);
        netplay_input_t *input = &ctx->remote_inputs[player_index];
        pthread_mutex_unlock(&ctx->mutex);
        return input;
    }
    return NULL;
}

// State sync functions
void NET_send_state(netplay_context_t *ctx, const void *state_data, int size) {
    netplay_state_t state;
    netplay_role_t role;
    uint32_t frame_count;
    int client_sockets[NETPLAY_MAX_PEERS];
    
    pthread_mutex_lock(&ctx->mutex);
    state = ctx->state;
    role = ctx->role;
    frame_count = ctx->frame_count;
    memcpy(client_sockets, ctx->client_sockets, sizeof(client_sockets));
    pthread_mutex_unlock(&ctx->mutex);
    
    if (state != NETPLAY_STATE_CONNECTED || size > (int)MAX_DATA_SIZE) {
        return;
    }
    
    uint8_t packet[MAX_PACKET_SIZE];
    int offset = 0;
    
    netpacket_header_t *header = (netpacket_header_t*)packet;
    header->magic = NETPLAY_MAGIC;
    header->type = NETPACKET_STATE_SYNC;
    header->version = 1;
    header->sequence = frame_count;
    offset += sizeof(netpacket_header_t);
    
    memcpy(packet + offset, state_data, size);
    offset += size;
    
    header->length = offset - sizeof(netpacket_header_t);
    
    if (role == NETPLAY_ROLE_HOST) {
        // Broadcast to all clients
        for (int i = 0; i < NETPLAY_MAX_PEERS; i++) {
            if (client_sockets[i] >= 0) {
                if (send(client_sockets[i], packet, offset, 0) < 0) {
                    printf("NET: Failed to send state to client %d: %s\n", i + 1, strerror(errno));
                }
            }
        }
    }
}

int NET_receive_state(netplay_context_t *ctx, void *state_data, int max_size) {
    (void)ctx;
    (void)state_data;
    (void)max_size;
    // Note: State packets are received in poll_client() and passed via on_state_received callback
    // This function is kept for API compatibility but should not be used.
    // Use the on_state_received callback instead.
    printf("NET: NET_receive_state is deprecated, use on_state_received callback instead\n");
    return -1;
}

// Ping/Latency functions
uint32_t NET_get_latency(netplay_context_t *ctx) {
    pthread_mutex_lock(&ctx->mutex);
    uint32_t latency = ctx->latency_ms;
    pthread_mutex_unlock(&ctx->mutex);
    return latency;
}

void NET_send_ping(netplay_context_t *ctx) {
    netplay_state_t state;
    netplay_role_t role;
    uint32_t frame_count;
    int server_socket;
    int client_sockets[NETPLAY_MAX_PEERS];
    
    pthread_mutex_lock(&ctx->mutex);
    state = ctx->state;
    role = ctx->role;
    frame_count = ctx->frame_count;
    server_socket = ctx->server_socket;
    memcpy(client_sockets, ctx->client_sockets, sizeof(client_sockets));
    pthread_mutex_unlock(&ctx->mutex);
    
    if (state != NETPLAY_STATE_CONNECTED) {
        return;
    }
    
    uint8_t packet[sizeof(netpacket_header_t) + sizeof(uint32_t)];
    netpacket_header_t *header = (netpacket_header_t*)packet;
    header->magic = NETPLAY_MAGIC;
    header->type = NETPACKET_PING;
    header->version = 1;
    header->sequence = frame_count;
    
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint32_t ping_time = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    header->length = sizeof(ping_time);
    memcpy(packet + sizeof(netpacket_header_t), &ping_time, sizeof(ping_time));
    
    int packet_size = sizeof(netpacket_header_t) + sizeof(ping_time);
    
    if (role == NETPLAY_ROLE_CLIENT && server_socket >= 0) {
        if (send(server_socket, packet, packet_size, 0) < 0) {
            printf("NET: Failed to send ping to server: %s\n", strerror(errno));
        }
    } else if (role == NETPLAY_ROLE_HOST) {
        for (int i = 0; i < NETPLAY_MAX_PEERS; i++) {
            if (client_sockets[i] >= 0) {
                if (send(client_sockets[i], packet, packet_size, 0) < 0) {
                    printf("NET: Failed to send ping to client %d: %s\n", i + 1, strerror(errno));
                }
            }
        }
    }
}