#include "utils.h"

// ambas as funções são utilizadas tanto pelo server quanto pelo cliente para participarem do chat

// função de envio de mensagem
void *send_message(void *args) {
    // Usa a socket e a comunicação criadas nas mains como parâmetro
    SOCKET_DATA * connection_socket = (SOCKET_DATA *) args;
    
    // cria um buffer com o tamanho máximo da mensagem
    char * buffer = malloc(MAX * sizeof(char));

    // loop que recebe da stdin e envia mensagens de tamanho máximo 4096 bytes até o chat ser encerrado
    while (TRUE) {

        memset(buffer, 0, MAX * sizeof(char));
        fgets(buffer, MAX, stdin);

        if (buffer == NULL) break;
        
        buffer[strlen(buffer) - 1] = '\0';
        send(connection_socket->socket_descriptor, buffer, strlen(buffer) * sizeof(char), 0);
    }

    // encerrando variáveis
    free(buffer);
    
    close(connection_socket->socket_descriptor);

    return 0;
}

// função de leitura de mensagens
void *recv_message(void * args) {
    // Usa a socket e a comunicação criadas nas mains como parâmetro
    SOCKET_DATA * connection_socket = (SOCKET_DATA *) args;
    
    // cria um buffer com o tamanho máximo da mensagem
    char *buffer = malloc(MAX * sizeof(char));

    // loop que recebe do chat mensagens de tamanho máximo 4096 bytes e as mostra na tela
    while (TRUE) {
        memset(buffer, 0, MAX * sizeof(char));
        if (recv(connection_socket->socket_descriptor, buffer, MAX * sizeof(char), 0) > 0) {
            printf(">> %s\n", buffer);
        }
        else {
            break;
        }
    }

    // encerrando variáveis
    close(connection_socket->socket_descriptor);
    printf("Exiting...\n");
    
    free(buffer);

    return 0;
}