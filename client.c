#include "client.h"


// // função de envio de mensagem
// void *send_message(void *args) {
//     // Usa a socket e a comunicação criadas nas mains como parâmetro
//     SOCKET_DATA * connection_socket = (SOCKET_DATA *) args;
    
//     // cria um buffer com o tamanho máximo da mensagem
//     char * buffer = malloc(MAX * sizeof(char));

//     // loop que recebe da stdin e envia mensagens de tamanho máximo 4096 bytes até o chat ser encerrado
//     while (TRUE) {
//         // DUVIDA: acho que vamos ter que tirar esse while true daqui né ? Pq ele já vai estar na main do client 
//         // Além disso, temos que retornar a msg para fazermos o parse dos comandos
//         memset(buffer, 0, MAX * sizeof(char));
//         fgets(buffer, MAX, stdin);

//         if (buffer == NULL) break;
        
//         buffer[strlen(buffer) - 1] = '\0';
//         send(connection_socket->socket_descriptor, buffer, strlen(buffer) * sizeof(char), 0);
//     }

//     // encerrando variáveis
//     free(buffer);
    
//     close(connection_socket->socket_descriptor);

//     return 0;
// }

// Função que lê a msg do cliente to stdin
char * get_client_message() {
    char * buffer = malloc(MAX_BUFFER * sizeof(char));
    memset(buffer, 0, MAX_BUFFER * sizeof(char));
    
    fgets(buffer, MAX_BUFFER, stdin);
    
    buffer[strlen(buffer) - 1] = '\0';
    return buffer;
}

// // função de leitura de mensagens
void * recv_message(void * args) {
    // Usa a socket e a comunicação criadas nas mains como parâmetro
    SOCKET_DATA * connection_socket = (SOCKET_DATA *) args;
    
    // cria um buffer com o tamanho máximo da mensagem
    char *buffer = malloc(MAX_BUFFER * sizeof(char));

    // loop que recebe do chat mensagens de tamanho máximo 4096 bytes e as mostra na tela
    while (TRUE) {
        memset(buffer, 0, MAX_BUFFER * sizeof(char));
        if (recv(connection_socket->socket_descriptor, buffer, MAX_BUFFER * sizeof(char), 0) > 0) {
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


// Função que verifica a presença de comandos em uma msg e retorna seu opcode
int parse_for_cmd(char* message) {

    int opcode = 0;
    printf("message: %s [%d]\n", message, con);
    if(message[0] != '/') // nao ha comandos na message
        opcode = msg; // message 
    
    else { // há possíveis comandos na message

        if(strncmp(message,"/connect",8) == 0){
            opcode = con;
        }
        else if(strncmp(message,"/quit",5) == 0) {
            opcode = quit;
        }
        else if(strncmp(message,"/ping",5) == 0) {
            opcode = ping;
        }
        else if(strncmp(message,"/join",5) == 0) {
            opcode = join;
        }
        else if(strncmp(message,"/nickname",9) == 0) {
            opcode = nickname;
        }
        else if(strncmp(message,"/kick",5) == 0) {
            opcode = kick;
        }
        else if(strncmp(message,"/mute",5) == 0) {
            opcode = mute;
        }
        else if(strncmp(message,"/unmute",7) == 0) {
            opcode = unmute;
        }
        else if(strncmp(message,"/whois",6) == 0) {
            opcode = whois;
        }
        else{ // Comando não identificado
            opcode = invalid_command; // vai cair no default do switch case
        }
    }

    return opcode;
}


void set_client_nickname(char * client_nickname, char * client_message) {
    char * chosen_nickname = strchr(client_message, ' ') + 1;
    if (strcmp(chosen_nickname, "user") == 0) {
        printf("Nickname cannot be 'user'\n");
        return;
    }

    strcpy(client_nickname, chosen_nickname);
    printf("New user nickname: %s\n", client_nickname);
}

bool connect_to_server(SOCKET_DATA *client_socket) {
    client_socket->socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if ((client_socket->socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        printf("Socket initiation failed!\n");
        return false;
    } 
    
    socket_address server_address;
    
    server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_address.sin_port = htons(PORT); //porta definida no client.h para comunicação
    
    client_socket->server_address = server_address;
    client_socket->addr_len = sizeof(client_socket->server_address);

    if (connect(client_socket->socket_descriptor, (const struct sockaddr *) &client_socket->server_address, client_socket->addr_len) != 0) {
        printf("Connection with the server failed\n");
        return false;
    }

    printf("[ Starting communication ]\n\n");
    send(client_socket->socket_descriptor, client_socket->client_nickname, NICKNAME_MAX_SIZE, 0); // Enviando 

    char * server_hello = calloc(8 + strlen(client_socket->client_nickname), sizeof(char)); 
    recv(client_socket->socket_descriptor, server_hello, 8 + strlen(client_socket->client_nickname), 0);
    
    printf("%s\n", server_hello);

    return true;
}

void send_full_message(SOCKET_DATA client_socket, int opcode, char* message, int size) {
    printf("%s, %s, %d\n", client_socket.client_nickname, message, opcode);

    send(client_socket.socket_descriptor, &opcode, sizeof(int), 0); 
    send(client_socket.socket_descriptor, message, size, 0);
}

int main(int argc, char *argv[]) {

    // ******* New code ***************
    
    // => Código que não estava na entrega 1
    SOCKET_DATA client_socket;
    stpcpy(client_socket.client_nickname, "user");
    client_socket.is_connected = false;

    pthread_t thread;

    int opcode; // representa a operação (comando) que é enviado pelo cliente ao servidor.
    // Cliente tem que estar conectado com o servidor (??)
        // Ele vai mandar msgs (e comandos), mas um desses comandos é o próprio /connect (???)

    // Ao enviar mensagens, iremos verificar se elas se tratam de comandos ou nao: 
    // TODO: opcode = parse_msg(msg);

    while (strncasecmp(client_socket.client_nickname, "user", 4) == 0) {
        printf("Before any action, choose a nickname with command '/nickname <choosen nickname>'\n");
        char * message = get_client_message();
        
        opcode = parse_for_cmd(message);
        
        printf("opcode: %d\n", opcode);
        if (opcode != nickname) {
            free(message);
            continue;
        }
        
        set_client_nickname(client_socket.client_nickname, message);
    }

    bool end_connection = false;
    while (!end_connection) { 
        // msg = send_message() (?) -> alterar a função send_message() para pegarmos a msg (?)
        char * message = get_client_message();

        // Verificamos se uma mensagem se trata de um comando - e qual -  ou não 
        opcode = parse_for_cmd(message);

        char * params = NULL;
        switch (opcode) {
            // opcode == 0
            case msg: // Não há comandos, o cliente está apenas enviando uma msg
                if (!client_socket.is_connected) {
                    printf("Client not connect to a server. Please type /connect\n");
                    continue;
                }

                send_full_message(client_socket, opcode, message, MAX_BUFFER);

                break;

            // opcode == 1
            case con:
                // todo - verificar se já está conectado
                if (!connect_to_server(&client_socket)) {
                    end_connection = true;
                    break;
                }

                client_socket.is_connected = true;
                pthread_create(&thread, NULL, &recv_message, (void *) &client_socket);
                break;

            // opcode == 2
            case quit:
                if (!client_socket.is_connected) {
                    printf("Client not connect to a server.");
                    end_connection = true;
                    continue;
                }

                send(client_socket.socket_descriptor, &opcode, sizeof(int), 0); 

                //closing connection

                // closing application
                printf("Application closed.\n");
                exit(0);

                break;

            // opcode == 3
            case ping:
                if (!client_socket.is_connected) {
                    printf("Client not connect to a server. Please type /connect");
                    continue;
                }
                send(client_socket.socket_descriptor, &opcode, sizeof(int), 0); 

                break;

            // opcode == 4
            case join:
                if (!client_socket.is_connected) {
                    printf("Client not connect to a server. Please type /connect");
                    continue;
                }
                params  = strchr(message, ' ') + 1;

                send_full_message(client_socket, opcode, params,CHANNEL_NAME_MAX_SIZE);

                // Verificar se um canal existe
                    // Se existir, o cliente junta-se ao canal 
                    // Se não, o canal é criado e o cliente torna-se o admin
                break;

            // opcode == 5
            case nickname:
                params  = strchr(message, ' ') + 1;

                if (client_socket.is_connected) {
                    send_full_message(client_socket, opcode, params, NICKNAME_MAX_SIZE);
                }
                
                memset(client_socket.client_nickname, 0, NICKNAME_MAX_SIZE * sizeof(char));
                strcpy(client_socket.client_nickname, params);

                break;

            // opcode == 6
            case kick: // exclusiva de admin
                params  = strchr(message, ' ') + 1;

                send_full_message(client_socket, opcode, params, NICKNAME_MAX_SIZE);

                break;

            // opcode == 7
            case mute: // exclusiva de admin
                params  = strchr(message, ' ') + 1;

                send_full_message(client_socket, opcode, params, NICKNAME_MAX_SIZE);

                break;

            // opcode == 8
            case unmute: // exclusiva de admin
                params  = strchr(message, ' ') + 1;

                send_full_message(client_socket, opcode, params, NICKNAME_MAX_SIZE);

                break;

            // opcode == 9
            case whois: // exclusiva de admin
                params  = strchr(message, ' ') + 1;

                send_full_message(client_socket, opcode, params, NICKNAME_MAX_SIZE);

                break;

            default:
                printf("Command not supported");
        }
        free(message);
    }
    // // ******* end new code ***********

    // // tentativa de conexão com servidor
    // if (connect(client_socket_descriptor, (struct sockaddr *) & server_address, sizeof(server_address)) != 0) {
    //     printf("Connection with the server failed!\n");
    //     exit(EXIT_FAILURE);
    // }
    
    // SOCKET_DATA socket_data = {
    //     .socket_descriptor = client_socket_descriptor,
    //     .server_address = server_address,
    //     .addr_len = sizeof(server_address),
    // };

    // // início do chat com servidor
    // printf("[ Starting communication ]\n\n");
    
    // // Usamos threads para poder realizar simultaneamente a escuta e o envio de mensagens entre o servidor e o cliente
    // pthread_t thread;
    // pthread_create(&thread, NULL, &send_message, (void *) &socket_data); //thread com função de envio de mensagens

    // recv_message(&socket_data);// função para receber as mensagens
    
    // //encerrando thread para finalizar participação no chat
    
    // pthread_cancel(thread);

    return 0;
}