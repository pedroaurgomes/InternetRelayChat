#include "client.h"


// (Abaixo)Funções traziadas da utils.c -> revisar para possíveis alterações ---------------

// função de envio de mensagem
void *send_message(void *args) {
    // Usa a socket e a comunicação criadas nas mains como parâmetro
    SOCKET_DATA * connection_socket = (SOCKET_DATA *) args;
    
    // cria um buffer com o tamanho máximo da mensagem
    char * buffer = malloc(MAX * sizeof(char));

    // loop que recebe da stdin e envia mensagens de tamanho máximo 4096 bytes até o chat ser encerrado
    while (TRUE) {
        // DUVIDA: acho que vamos ter que tirar esse while true daqui né ? Pq ele já vai estar na main do client 
        // Além disso, temos que retornar a msg para fazermos o parse dos comandos
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

// (Acima)Funções traziadas da utils.c -> revisar para possíveis alterações ---------------

// Função que verifica a presença de comandos em uma msg e retorna seu opcode
// OBS: Esta função ficaria bem menor e mais bonita se usássemos enum nos opcodes, mas aí ficaria
// muito igual a do Monici
int parse_for_cmd(char* msg) {

    int opcode = 0;

    if(msg[0] != '/') // nao ha comandos na msg
        opcode = 0; // msg 
    
    else { // há possíveis comandos na msg

        if(strncmp(msg,"/connect",8)){
            opcode = 1;
        }
        else if(strncmp(msg,"/quit",5)) {
            opcode = 2;
        }
        else if(strncmp(msg,"/ping",5)) {
            opcode = 3;
        }
        else if(strncmp(msg,"/join",5)) {
            opcode = 4;
        }
        else if(strncmp(msg,"/nickname",9)) {
            opcode = 5;
        }
        else if(strncmp(msg,"/kick",5)) {
            opcode = 6;
        }
        else if(strncmp(msg,"/mute",5)) {
            opcode = 7;
        }
        else if(strncmp(msg,"/unmute",7)) {
            opcode = 8;
        }
        else if(strncmp(msg,"/whois",6)) {
            opcode = 9;
        }
        else{ // Comando não identificado
            opcode = 10; // vai cair no default do switch case
        }
    }

    return opcode;
}



int main(int argc, char *argv[]) {
	socket_address server_address;

    // criação de socket do cliente
    int client_socket_descriptor;
    if ((client_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        printf("Client Socket initiation failed!\n");
        exit(EXIT_FAILURE);
    } 
    else {
        printf("Client Socket successfully created!\n");
    }
    
    server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_address.sin_port = htons(PORT); //porta definida no client.h para comunicação

    // ******* New code ***************
    
    // => Código que não estava na entrega 1
    char client_nickname[NICKNAME_MAX_SIZE] = "user";
    int opcode; // representa a operação (comando) que é enviado pelo cliente ao servidor.

    // Cliente tem que estar conectado com o servidor (??)
        // Ele vai mandar msgs (e comandos), mas um desses comandos é o próprio /connect (???)

    // Ao enviar mensagens, iremos verificar se elas se tratam de comandos ou nao: 
    // TODO: opcode = parse_msg(msg);

    while(client_connected()) { // função client_connected() ainda não existe

        // msg = send_message() (?) -> alterar a função send_message() para pegarmos a msg (?)

        // Verificamos se uma mensagem se trata de um comando - e qual -  ou não 
        opcode = parse_for_cmd(msg);

        if((nickname != 5) && strncasecmp(client_nickname, "user", 4) == 0) { 
            printf("Before any action, choose a nickname with command '/nickname <choosen nickname>'\n");
            continue; 
        }

        switch (opcode) {
            // opcode == 0
            case msg: // Não há comandos, o cliente está apenas enviando uma msg
                break;

            // opcode == 1
            case connect:

                break;

            // opcode == 2
            case quit:
                //close connection
                break;

            // opcode == 3
            case ping:
                break;

            // opcode == 4
            case join:
                // Verificar se um canal existe
                    // Se existir, o cliente junta-se ao canal 
                    // Se não, o canal é criado e o cliente torna-se o admin
                break;

            // opcode == 5
            case nickname:
                break;

            // opcode == 6
            case kick: // exclusiva de admin
                break;

            // opcode == 7
            case mute: // exclusiva de admin
                break;

            // opcode == 8
            case unmute: // exclusiva de admin
                break;

            // opcode == 9
            case whois: // exclusiva de admin
                break;

            default:
                printf("Command not supported");
        }   

    }


    // ******* end new code ***********

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