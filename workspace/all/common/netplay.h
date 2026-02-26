#ifndef __NETPLAY_H__
#define __NETPLAY_H__

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#define NETPLAY_PORT 55435
#define NETPLAY_BROADCAST_PORT 55436
#define NETPLAY_MAX_PEERS 4
#define NETPLAY_DEVICE_NAME_MAX 64
#define NETPLAY_MAX_INPUT_STATE 32
#define NETPLAY_PROTOCOL_VERSION 1

// Netplay states
typedef enum {
    NETPLAY_STATE_IDLE = 0,
    NETPLAY_STATE_HOSTING,
    NETPLAY_STATE_CONNECTING,
    NETPLAY_STATE_CONNECTED,
    NETPLAY_STATE_ERROR
} netplay_state_t;

// Netplay roles
typedef enum {
    NETPLAY_ROLE_NONE = 0,
    NETPLAY_ROLE_HOST,
    NETPLAY_ROLE_CLIENT
} netplay_role_t;

// Device info for discovery
typedef struct {
    char device_name[NETPLAY_DEVICE_NAME_MAX];
    char ip_address[16];
    uint16_t port;
    bool is_host;
    int player_count;
} netplay_device_t;

// Input state
typedef struct {
    uint8_t port;
    uint16_t buttons;
    int16_t analog_x;
    int16_t analog_y;
    int16_t analog_lx;
    int16_t analog_ly;
    int16_t analog_rx;
    int16_t analog_ry;
} netplay_input_t;

// Netplay packet types
typedef enum {
    NETPACKET_DISCOVERY = 0,
    NETPACKET_DISCOVERY_RESPONSE,
    NETPACKET_CONNECT_REQUEST,
    NETPACKET_CONNECT_RESPONSE,
    NETPACKET_DISCONNECT,
    NETPACKET_INPUT_STATE,
    NETPACKET_STATE_SYNC,
    NETPACKET_PING,
    NETPACKET_PONG
} netpacket_type_t;

// Netplay packet header
typedef struct {
    uint32_t magic;         // "NPAY"
    uint8_t type;           // netpacket_type_t
    uint8_t version;        // protocol version
    uint16_t length;        // data length
    uint32_t sequence;      // packet sequence number
} netpacket_header_t;

#define NETPLAY_MAGIC 0x4E504159  // "NPAY"

// Netplay API
typedef struct {
    pthread_mutex_t mutex;  // Mutex for thread safety
    
    netplay_state_t state;
    netplay_role_t role;
    char device_name[NETPLAY_DEVICE_NAME_MAX];
    
    // Hosting
    int server_socket;
    int client_sockets[NETPLAY_MAX_PEERS];
    int client_count;
    
    // Client
    int server_socket;
    char server_ip[16];
    
    // Discovery
    int broadcast_socket;
    bool discovery_enabled;
    
    // Input state
    netplay_input_t local_input;
    netplay_input_t remote_inputs[NETPLAY_MAX_PEERS];
    
    // State
    uint32_t frame_count;
    uint32_t last_ping_time;
    uint32_t latency_ms;
    uint32_t last_ping_sent_time;  // Timestamp when last ping was sent
    
    void (*on_input_received)(netplay_input_t *input);
    void (*on_state_received)(const void *state_data, int size, uint32_t frame);
    void (*on_device_discovered)(netplay_device_t *device);
    void (*on_connection_state_changed)(netplay_state_t state);
} netplay_context_t;

// API functions
void NET_init(netplay_context_t *ctx, const char *device_name);
void NET_quit(netplay_context_t *ctx);

// Discovery
int NET_start_discovery(netplay_context_t *ctx);
void NET_stop_discovery(netplay_context_t *ctx);
int NET_broadcast_discovery(netplay_context_t *ctx);
int NET_handle_discovery_packet(netplay_context_t *ctx, const char *data, int len);

// Hosting
int NET_start_hosting(netplay_context_t *ctx);
void NET_stop_hosting(netplay_context_t *ctx);
int NET_poll_host(netplay_context_t *ctx);

// Client
int NET_connect_to_host(netplay_context_t *ctx, const char *host_ip);
void NET_disconnect(netplay_context_t *ctx);
int NET_poll_client(netplay_context_t *ctx);

// Input
void NET_set_local_input(netplay_context_t *ctx, const netplay_input_t *input);
void NET_send_input(netplay_context_t *ctx);
netplay_input_t *NET_get_remote_input(netplay_context_t *ctx, int player_index);

// State sync
void NET_send_state(netplay_context_t *ctx, const void *state_data, int size);
int NET_receive_state(netplay_context_t *ctx, void *state_data, int max_size);

// Ping/Latency
uint32_t NET_get_latency(netplay_context_t *ctx);
void NET_send_ping(netplay_context_t *ctx);

#endif // __NETPLAY_H__