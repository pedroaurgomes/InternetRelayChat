#include "utils.h"

int main(int argc, char *argv[]) {
	socket_address server_address;

    // criação de socket do cliente
    int client_socket_descriptor;
    if ((client_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        printf("Socket initiation failed!\n");
        exit(EXIT_FAILURE);
    } 
    else {
        printf("Socket successfully created!\n");
    }
    
    server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_address.sin_port = htons(PORT); //porta definida no utils.h para comunicação

    // tentativa de conexão com servidor
    if (connect(client_socket_descriptor, (struct sockaddr *) & server_address, sizeof(server_address)) != 0) {
        printf("Connection with the server failed!\n");
        exit(EXIT_FAILURE);
    }
    
    SOCKET_DATA socket_data = {
        .socket_descriptor = client_socket_descriptor,
        .server_address = server_address,
        .addr_len = sizeof(server_address),
    };

    // início do chat com servidor
    printf("[ Starting communication ]\n\n");
    
    // Usamos threads para poder realizar simultaneamente a escuta e o envio de mensagens entre o servidor e o cliente
    pthread_t thread;
    pthread_create(&thread, NULL, &send_message, (void *) &socket_data); //thread com função de envio de mensagens

    recv_message(&socket_data);// função para receber as mensagens
    
    //encerrando thread para finalizar participação no chat
    
    pthread_cancel(thread);

    return 0;
}