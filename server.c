#include "utils.h"

int main(int argc, char *argv[]) {

    //criação da socket para realizar comunicação
	socket_address server_address, client_address;

    int server_socket_descriptor;
    if ((server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0) { //no caso de falha retorna erro
        printf("Socket initiation failed!\n");
        exit(EXIT_FAILURE);
    } 
    else {
        printf("Socket successfully created!\n");
    }
    

    // vinculação da socket com a porta que acontecerá a comunicação
    server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(PORT); //porta definida no utils.h para comunicação

	if ((bind(server_socket_descriptor, (const struct sockaddr *) & server_address, sizeof(server_address))) < 0) {
        printf("Socket binding failed!\n");
        exit(EXIT_FAILURE);
    }
    else {
        printf("Socket successfully binded to port %d\n", PORT);
    }

    // Coloca o socket do servidor em modo passivo, ou seja, espera pelo cliente estabelecer uma conexão
	// Segundo parametro (backlog): Números máximo de conexões pendentes na fila
	if ((listen(server_socket_descriptor, 10)) != 0) {
        printf("Socket listenning failed!\n");
        exit(EXIT_FAILURE);
    }
    else {
        printf("Socket successfully listening on port %d\n", PORT);
    }

    // verificação se conexão entre servidor e cliente foi aceita pelo cliente
    int len_client_address = sizeof(client_address);
    int connection_descriptor;
    if ((connection_descriptor = accept(server_socket_descriptor, (struct sockaddr *) & client_address, (socklen_t*) &len_client_address)) < 0) {
        printf("Server acception failed!\n");
        exit(EXIT_FAILURE);
    }
    else {
        printf("Server successfully accepted client\n");
    }
    
    SOCKET_DATA socket_data = {
        .socket_descriptor = connection_descriptor,
        .server_address = server_address,
        .addr_len = sizeof(server_address),
    };

    // chat entre servidor e cliente
    printf("[ Starting communication ]\n\n");
    
    // Usamos threads para poder realizar simultaneamente a escuta e o envio de mensagens entre o servidor e o cliente
    pthread_t thread;
    
    pthread_create(&thread, NULL, &send_message, (void *) &socket_data); //thread com função de envio de mensagens

    recv_message(&socket_data); // função para receber as mensagens
    
    //encerrando thread e socket para finalizar chat
    pthread_cancel(thread);
    
    close(server_socket_descriptor);

    return 0;
}