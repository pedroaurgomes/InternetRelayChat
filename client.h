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

// definindo constantes para serem usadas na comunicação
#define MAX_BUFFER 4096
#define NICKNAME_MAX_SIZE 50
#define PORT 8081

#define MAX_CLIENTS_ON          10
#define MAX_CHANNELS            10
#define NICKNAME_MAX_SIZE       50
#define CHANNEL_NAME_MAX_SIZE   200
#define MAX_CLIENTS_PER_CHANNEL 10

#define TRUE 1
#define FALSE 0

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


#endif