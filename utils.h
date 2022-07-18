#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <unistd.h>

#include <pthread.h>

#include <signal.h>

// definindo constantes para serem usadas na comunicação
#define PORT 8080

#define MAX_BUFFER 4096
#define NICKNAME_MAX_SIZE       50
#define MAX_CLIENTS_ON          10
#define MAX_CHANNELS            10
#define NICKNAME_MAX_SIZE       50
#define CHANNEL_NAME_MAX_SIZE   200
#define MAX_CLIENTS_PER_CHANNEL 10
#define MAX_ATTEMPTS 5

typedef enum { false, true } bool;

// structs para passar mais facilmente parâmetros da socket
typedef struct sockaddr_in socket_address;

typedef struct __client_data {
    char client_nickname[NICKNAME_MAX_SIZE];
    int socket_descriptor;
    struct sockaddr_in server_address;
    int addr_len;
    bool is_connected;
} SOCKET_DATA;

typedef enum _irc_options {
    msg = 0,
    
    // Comando da Segunda entrega
    con, // connect
    quit,
    ping,

    // Comandos da Terceira entrega
    join,
    nickname,
    kick,
    mute,
    unmute,
    whois,

    invalid_command
} IRC_OPTIONS;


typedef struct _server_side_channel_struct CHANNEL;
typedef struct _server_side_client_struct CLIENT;
typedef struct _server_side_struct SERVER_SIDE;
typedef struct _server_msg_pattern_struct SERVER_FRAME;

// Struct que representa os canais dentro do servidor
struct _server_side_channel_struct
{
    char channel_name[CHANNEL_NAME_MAX_SIZE]; // nome do canal
    CLIENT *clients[MAX_CLIENTS_PER_CHANNEL]; // lista de clientes conectados a tal canal
    int num_clients;                          // Número atual de cliente no canal (deve sempre ser inferior a MAX_CLIENTS_PER_CHANNEL)
};

// Struct que representa o cliente dentro do servidor
struct _server_side_client_struct
{
    char nickname[NICKNAME_MAX_SIZE]; // Nickname do usuário
    int socket_descriptor;            // descritor da conexào com o cliente
    struct sockaddr_in addr;          // endereço da conexão com o cliente
    socklen_t addr_len;

    bool is_admin; // boolean representativo se é adm no canal que está
    bool is_muted; // indica se o usuário está mutado no canal ao qual pertence

    CHANNEL *user_channel; // referência para o canal que o usuário está conectado
};

// Struct que compreende todas as informações do servidor
struct _server_side_struct
{
    CLIENT *clients[MAX_CLIENTS_ON]; 
    CHANNEL *channels[MAX_CHANNELS]; 

    int socket_descriptor;   // descritor do socket da conexão
    struct sockaddr_in addr; // endereço da conexão
    socklen_t addr_len;

    int num_channels; // Total de canais
    int num_clients;  // Total de clientes
};


#endif