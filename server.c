#include "client.h" 

#define MAX_CLIENTS_ON          10
#define MAX_CHANNELS            10
#define NICKNAME_MAX_SIZE       50
#define CHANNEL_NAME_MAX_SIZE   200
#define MAX_CLIENTS_PER_CHANNEL 10

#define PORT 8081

typedef struct _server_side_channel_struct CHANNEL;
typedef struct _server_side_client_struct CLIENT;
typedef struct _server_side_struct SERVER_SIDE;
typedef struct _server_msg_pattern_struct SERVER_FRAME;

// Padrão de mensagens que serão recebidas pelo servidor
// Corresponde a 2 sends: 
// - Mensagem enviada pelo cliente
// - Operação desejada pelo cliente
// struct _server_msg_pattern_struct {
//     char message[MAX_BUFFER];
//     int opcode;
// };

// Struct que representa os canais dentro do servidor
struct _server_side_channel_struct {
    char channel_name[CHANNEL_NAME_MAX_SIZE]; // nome do canal 
    // TODO: Restricoes de nome de canal para serem tratadas:
        // Comecar com: '&' ou '#'
        // Caracteres proibidos: espaco, virgula, Ctrl+G (^G)  
    CLIENT* clients[MAX_CLIENTS_PER_CHANNEL]; // lista de clientes conectados a tal canal
    int num_clients; // Número atual de cliente no canal (deve sempre ser inferior a MAX_CLIENTS_PER_CHANNEL)
};

// Struct que representa o cliente dentro do servidor
struct _server_side_client_struct {
    char nickname[NICKNAME_MAX_SIZE]; // Nickname do usuário
    int socket_descriptor; // descritor da conexào com o cliente
    struct sockaddr_in addr; // endereço da conexão com o cliente 
    socklen_t addr_len; 
    
    bool is_admin; // boolean representativo se é adm no canal que está
    bool is_muted; // indica se o usuário está mutado no canal ao qual pertence
    
    CHANNEL *user_channel; // referência para o canal que o usuário está conectado
};

// Struct que compreende todas as informações do servidor
struct _server_side_struct {
    CLIENT* clients[MAX_CLIENTS_ON]; // todos os clientes
    CHANNEL* channels[MAX_CHANNELS]; // todos os canais

    int socket_descriptor; // descritor do socket da conexão
    struct sockaddr_in addr; // endereço da conexão
    socklen_t addr_len;
    
    int num_channels; // Total de canais
    int num_clients;  // Total de clientes
};

// função que dado um cliente e uma mensagem
void send_message_channel(CLIENT* sender, char* msg){
    if(!sender->user_channel){
        //todo: erro de não estar em um canal
        printf("Cliente não pertence a um canal");
        return;
    }
    for (int j = 0; j < MAX_CLIENTS_PER_CHANNEL; j++) {
        CLIENT* cur_client = sender->user_channel->clients[j];
        if (cur_client && cur_client->user_channel && cur_client->socket_descriptor == sender->socket_descriptor){
            // todo: colocar aqui envio de mensagem específica para o cur_cliente
            //! checar server_send_msg no do monici
        } 
    }
}

// função de desconectar clientes do SEVIDOR
void end_client_connection(CLIENT* cliente){
    // mensagem de saída
    printf("%s has left the server\n", cliente->nickname);
    
    //todo: remover cliente do canal
    //todo: encerrar socket
    //todo: desalocar memória
}

//todo: ping -> ao invés de fazer função de ping, só mandar um send(client->sock, "server >> pong", 14, 0)

// // função de envio de mensagem
// void *send_message(void *args) {
//     // Usa a socket e a comunicação criadas nas mains como parâmetro
//     SOCKET_DATA * connection_socket = (SOCKET_DATA *) args;
    
//     // cria um buffer com o tamanho máximo da mensagem
//     char * buffer = malloc(MAX * sizeof(char));

//     // loop que recebe da stdin e envia mensagens de tamanho máximo 4096 bytes até o chat ser encerrado
//     while (TRUE) {

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

// // função de leitura de mensagens
// void *recv_message(void * args) {
//     // Usa a socket e a comunicação criadas nas mains como parâmetro
//     SOCKET_DATA * connection_socket = (SOCKET_DATA *) args;
    
//     // cria um buffer com o tamanho máximo da mensagem
//     char *buffer = malloc(MAX * sizeof(char));

//     // loop que recebe do chat mensagens de tamanho máximo 4096 bytes e as mostra na tela
//     while (TRUE) {
//         memset(buffer, 0, MAX * sizeof(char));
//         if (recv(connection_socket->socket_descriptor, buffer, MAX * sizeof(char), 0) > 0) {
//             printf(">> %s\n", buffer);
//         }
//         else {
//             break;
//         }
//     }
// }



struct _server_side_client_struct *search_client_by_nickname(char* nickname, SERVER_SIDE *server){
    // Busca sequencial por todos os clientes a partir do nickname dado    
    for (int i = 0; i < MAX_CLIENTS_ON; ++i) {
        if (!server->clients[i]) continue;
        if (strcmp(nickname, server->clients[i]->nickname) == 0)
            return server->clients[i];
    }

    return NULL;
}

// aceitando conexões novas, passa pela lista de clientes no server, checa posições vazias, e cria conexão
void * connect_server_client(void * args) {
    SERVER_SIDE * server = (SERVER_SIDE *) args;

    // SERVER_SIDE * server = (SERVER_SIDE *) args;
    while (TRUE) {
        for (int i = 0; i < MAX_CLIENTS_ON; i++) {
            if(!server->clients[i]){
                CLIENT * cur_client = calloc(1, sizeof(CLIENT));
                cur_client->socket_descriptor = accept(server->socket_descriptor, (struct sockaddr *) &cur_client->addr, &cur_client->addr_len);
                
                recv(cur_client->socket_descriptor, cur_client->nickname, NICKNAME_MAX_SIZE, 0);

                char * welcome_message = calloc(8 + strlen(cur_client->nickname), sizeof(char));
                
                printf("%s has joined the server\n", cur_client->nickname);
                strcpy(welcome_message, "Hello, ");
                strcat(welcome_message, cur_client->nickname);
                
                server->clients[i] = cur_client;
                server->num_clients++;

                send(cur_client->socket_descriptor, welcome_message, strlen(welcome_message) + 1, 0);
            }
        }
    }
}

// SERVER_FRAME * recv_message(CLIENT * cur_client) {
//     SERVER_FRAME * client_message = calloc(1, sizeof(SERVER_FRAME));
    
//     recv(cur_client->socket_descriptor, & client_message->opcode, sizeof(int), 0);
//     recv(cur_client->socket_descriptor, client_message->message, MAX_BUFFER, 0);
    
//         printf("%s %s %d\n", cur_client->nickname, client_message->message, client_message->opcode);

//     return client_message;
// }


int main(int argc, char *argv[]) {
    //criação da socket para realizar comunicação
    // sockaddr_in
	socket_address server_address;

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
	server_address.sin_port = htons(PORT); 
    
    // Criando a struct do servidor
    SERVER_SIDE server = {
        .socket_descriptor = server_socket_descriptor,
        .addr = server_address,
        .addr_len = sizeof(server_address),
        // .channels = NULL,  //*tiramos pra n perder referencia
        .num_channels = 0,
        .num_clients = 0,
        // .clients = NULL,
    };

    for(int i = 0; i < MAX_CHANNELS; i++) server.channels[i] = NULL;
    for(int i = 0; i < MAX_CLIENTS_ON; i++) server.clients[i] = NULL;

	if ((bind(server.socket_descriptor, (const struct sockaddr *) &server.addr, server.addr_len)) < 0) {
        printf("Socket binding failed!\n");
        exit(EXIT_FAILURE);
    }
    else {
        printf("Socket successfully binded to port %d\n", PORT);
    }

 
    // Coloca o socket do servidor em modo passivo, ou seja, espera pelo cliente estabelecer uma conexão
	// Segundo parametro (backlog): Números máximo de conexões pendentes na fila
	if ((listen(server.socket_descriptor, MAX_CLIENTS_ON)) != 0) {
        printf("Socket listenning failed!\n");
        exit(EXIT_FAILURE);
    }
    else {
        printf("Socket successfully listening on port %d\n", PORT);
    }

    pthread_t connection_thread;
    pthread_create(&connection_thread, NULL, connect_server_client, (void *) &server);

    bool end_server = false;
    while(!end_server) {
        for (int i = 0; i < MAX_CLIENTS_ON; ++i) {
            if (server.clients[i] == NULL) continue;
            int opcode = -1;
            
            CLIENT * cur_client = server.clients[i];
            recv(cur_client->socket_descriptor, &opcode, sizeof(int), 0);
            // printf("opcode: %d\n", opcode);
            
            switch (opcode) {
                case msg:
                {
                    if(!cur_client->user_channel){
                        //todo colocar mensagem de erro para entrar em canal soh pra qm mandou
                    }
                    if(cur_client->is_muted){
                        //todo colocar mensagem de erro para avisar que esta mutado soh pra qm mandou
                    }
                    char message[MAX_BUFFER];
                    recv(cur_client->socket_descriptor, message, MAX_BUFFER, 0);
                    
                    break;
                }
                case quit:
                {
                    for (int i = 0; i < MAX_CLIENTS_PER_CHANNEL; i++) {
                        if (cur_client->user_channel->clients[i] == cur_client ) {
                            cur_client->user_channel->clients[i] = NULL;
                            cur_client->user_channel->num_clients--;
                        }
                    }
                    if(cur_client->is_admin){
                        for (int i = 0; i < MAX_CLIENTS_PER_CHANNEL; i++) {
                            if (cur_client->user_channel->clients[i] != NULL) {
                                cur_client->user_channel->clients[i]->is_admin = TRUE;
                            }
                        }
                    }
                    close(cur_client->socket_descriptor);

                    //todo: desalocar da estrutura
                    break;
                }                
                
                case ping:
                {
                    send(cur_client->socket_descriptor, "server >> pong", 14, 0);
                    break;
                }
                case join: 
                {
                    char channel[CHANNEL_NAME_MAX_SIZE];
                    recv(cur_client->socket_descriptor, channel, CHANNEL_NAME_MAX_SIZE, 0);

                    // TODO: responder a função de join do cliente
                    break;
                }
                case nickname: 
                {
                    char name[NICKNAME_MAX_SIZE];
                    recv(cur_client->socket_descriptor, name, NICKNAME_MAX_SIZE, 0);

                    // server não pode ter nickname
                    break;
                }
                case kick:
                {
                    if(!cur_client->is_admin) break;
                    char name[NICKNAME_MAX_SIZE];
                    recv(cur_client->socket_descriptor, name, NICKNAME_MAX_SIZE, 0);
                    /* pseudo:
                        verificar se cliente é admin (se for, o kick não será aplicado)
                        
                    
                    */
                    break;

                }
                case mute:
                {
                    if(!cur_client->is_admin) break;
                    char name[NICKNAME_MAX_SIZE];
                    recv(cur_client->socket_descriptor, name, NICKNAME_MAX_SIZE, 0);
                    break;
                }
                case unmute:
                {
                    if(!cur_client->is_admin) break;
                    char name[NICKNAME_MAX_SIZE];
                    recv(cur_client->socket_descriptor, name, NICKNAME_MAX_SIZE, 0);
                    break;
                }
                case whois:
                {
                    if(!cur_client->is_admin) break;
                    char name[NICKNAME_MAX_SIZE];
                    recv(cur_client->socket_descriptor, name, NICKNAME_MAX_SIZE, 0);
                    break;
                }
                default:
                    printf("Command not supported");
            } 
        }
    }

    return 0;
}