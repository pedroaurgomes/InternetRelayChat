#ifndef __UTILS_H__
#define __UTILS_H__

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
#define MAX 4096

#define PORT 8080

#define TRUE 1
#define FALSE 0

// structs para passar mais facilmente parâmetros da socket
typedef struct sockaddr_in socket_address;

typedef struct _socket_data {
    int socket_descriptor;
    struct sockaddr_in server_address;
    int addr_len;
} SOCKET_DATA;

// definindo funções
void *recv_message(void * args);
void *send_message(void *args);

#endif