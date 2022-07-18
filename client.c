#include "utils.h"

// Função que lê a mensagem do cliente da stdin em blocos
char *get_client_message()
{
    char *buffer = malloc(MAX_BUFFER * sizeof(char));
    memset(buffer, 0, MAX_BUFFER * sizeof(char));

    if (!fgets(buffer, MAX_BUFFER, stdin))
        return NULL;

    buffer[strlen(buffer) - 1] = '\0';
    return buffer;
}

// Função de receber mensagens a partir da socket
void *recv_message(void *args)
{
    // Usa a socket e a comunicação criadas nas mains como parâmetro
    SOCKET_DATA *connection_socket = (SOCKET_DATA *)args;

    // cria um buffer com o tamanho máximo da mensagem
    char *buffer = malloc(MAX_BUFFER * sizeof(char));

    // loop que recebe do chat mensagens de tamanho máximo 4096 bytes e as mostra na tela
    while (true)
    {
        memset(buffer, 0, MAX_BUFFER * sizeof(char));
        if (recv(connection_socket->socket_descriptor, buffer, MAX_BUFFER * sizeof(char), 0) != -1)
        {
            printf("%s\n", buffer);
            send(connection_socket->socket_descriptor, "received", 9, 0); // handshake client side
        }
    }

    // encerrando variáveis
    close(connection_socket->socket_descriptor);
    printf("Exiting...\n");

    free(buffer);

    return 0;
}

// Função que verifica a presença de comandos em uma mensagem e retorna seu código de operação
// Ao invés de mandar as palavras de comando, mandamos um código int para dizer qual a operação deve ser enviada
int parse_for_cmd(char *message)
{
    // Para o caso do Ctrl + D (EOF -> retorna null no fgets, portanto msg == NULL)
    if (!message)
        return quit;

    if (message[0] != '/') // sem comandos, mensagem simples
        return msg;       

    else
    { // possíveis comandos na message

        if (strncmp(message, "/connect", 8) == 0)
        {
            return con;
        }
        else if (strncmp(message, "/quit", 5) == 0)
        {
            return quit;
        }
        else if (strncmp(message, "/ping", 5) == 0)
        {
            return ping;
        }
        else if (strncmp(message, "/join", 5) == 0)
        {
            return join;
        }
        else if (strncmp(message, "/nickname", 9) == 0)
        {
            return nickname;
        }
        else if (strncmp(message, "/kick", 5) == 0)
        {
            return kick;
        }
        else if (strncmp(message, "/mute", 5) == 0)
        {
            return mute;
        }
        else if (strncmp(message, "/unmute", 7) == 0)
        {
            return unmute;
        }
        else if (strncmp(message, "/whois", 6) == 0)
        {
            return whois;
        }
        else
        {                           
            return invalid_command; // Comando não identificado
        }
    }

    return msg;
}

// Mudando nick do usuário para o cliente
void set_client_nickname(char *client_nickname, char *client_message)
{
    char *chosen_nickname = strchr(client_message, ' ') + 1;
    if (strcmp(chosen_nickname, "user") == 0)
    {
        printf("Nickname cannot be 'user'\n");
        return;
    }

    strcpy(client_nickname, chosen_nickname);
    printf("New user nickname: %s\n", client_nickname);
}

//função para conectar socket, baseada na primeira parte do trabalho
bool connect_to_server(SOCKET_DATA *client_socket)
{
    client_socket->socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if ((client_socket->socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        printf("Socket initiation failed!\n");
        return false;
    }

    socket_address server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(PORT); // porta definida no utils.h para comunicação

    client_socket->server_address = server_address;
    client_socket->addr_len = sizeof(client_socket->server_address);

    if (connect(client_socket->socket_descriptor, (const struct sockaddr *)&client_socket->server_address, client_socket->addr_len) != 0)
    {
        printf("Connection with the server failed\n");
        return false;
    }

    printf("[ Starting communication ]\n");
    send(client_socket->socket_descriptor, client_socket->client_nickname, NICKNAME_MAX_SIZE, 0); // Enviando

    char *server_hello = calloc(8 + strlen(client_socket->client_nickname), sizeof(char));
    recv(client_socket->socket_descriptor, server_hello, 8 + strlen(client_socket->client_nickname), 0);

    printf("%s\n", server_hello);

    return true;
}

// enviando mensagem e código da operação
void send_full_message(SOCKET_DATA *client_socket, int opcode, char *message, int size)
{
    send(client_socket->socket_descriptor, &opcode, sizeof(int), 0);
    send(client_socket->socket_descriptor, message, size, 0);
}

int main(int argc, char *argv[])
{

    // Ignora ctrl + C
    struct sigaction sa = { .sa_handler = SIG_IGN };
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1);

    SOCKET_DATA client_socket;
    stpcpy(client_socket.client_nickname, "user");
    client_socket.is_connected = false;

    pthread_t thread;

    int opcode;

    while (strncasecmp(client_socket.client_nickname, "user", 4) == 0)
    {
        printf("Before any action, choose a nickname with command '/nickname <choosen nickname>'\n");
        char *message = get_client_message();

        opcode = parse_for_cmd(message);

        if (opcode != nickname)
        {
            free(message);
            continue;
        }

        set_client_nickname(client_socket.client_nickname, message);
    }

    bool end_connection = false;
    while (!end_connection)
    {
        char *message = get_client_message();

        // Verificamos se uma mensagem se trata de um comando - e qual -  ou não
        opcode = parse_for_cmd(message);

        char *params = NULL;
        switch (opcode)
        {
        case msg: // Não há comandos, o cliente está apenas enviando uma msg
            if (!client_socket.is_connected)
            {
                printf("Client not connect to a server. Please type /connect\n");
                continue;
            }

            send_full_message(&client_socket, opcode, message, MAX_BUFFER);

            break;

        case con: //conexão da socket e uso de thread para enviar e receber mensagem simultaneamente
            if (!connect_to_server(&client_socket))
            {
                end_connection = true;
                break;
            }

            client_socket.is_connected = true;
            pthread_create(&thread, NULL, &recv_message, (void *)&client_socket);
            break;

        case quit:
            // encerrando conexão
            if (!client_socket.is_connected)
            {
                printf("Client not connect to a server.\n");
                end_connection = true;
                continue;
            }

            send(client_socket.socket_descriptor, &opcode, sizeof(int), 0);

            // encerrando aplicação
            printf("Application closed.\n");
            exit(0);

            break;

        case ping:
            if (!client_socket.is_connected)
            {
                printf("Client not connect to a server. Please type /connect\n");
                continue;
            }
            //envio do ping
            send(client_socket.socket_descriptor, &opcode, sizeof(int), 0);

            break;

        case join:
            if (!client_socket.is_connected)
            {
                printf("Client not connect to a server. Please type /connect\n");
                continue;
            }
            params = strchr(message, ' ') + 1;

            //envio da operação e o parametro
            send_full_message(&client_socket, opcode, params, CHANNEL_NAME_MAX_SIZE);

            // Verificar se um canal existe => está sendo feito do lado do servidor
            break;

        case nickname:
            params = strchr(message, ' ') + 1;

            if (client_socket.is_connected)
            {
                //envio da operação e o parametro
                send_full_message(&client_socket, opcode, params, NICKNAME_MAX_SIZE);
            }

            memset(client_socket.client_nickname, 0, NICKNAME_MAX_SIZE * sizeof(char));
            strcpy(client_socket.client_nickname, params);

            break;

        case kick: // exclusiva de admin
            params = strchr(message, ' ') + 1;

            //envio da operação e o parametro
            send_full_message(&client_socket, opcode, params, NICKNAME_MAX_SIZE);

            break;

        case mute: // exclusiva de admin
            params = strchr(message, ' ') + 1;

            //envio da operação e o parametro
            send_full_message(&client_socket, opcode, params, NICKNAME_MAX_SIZE);

            break;

        case unmute: // exclusiva de admin
            params = strchr(message, ' ') + 1;

            //envio da operação e o parametro
            send_full_message(&client_socket, opcode, params, NICKNAME_MAX_SIZE);

            break;

        case whois: // exclusiva de admin
            params = strchr(message, ' ') + 1;

            //envio da operação e o parametro
            send_full_message(&client_socket, opcode, params, NICKNAME_MAX_SIZE);

            break;

        default:
            printf("Command not supported\n");
        }
        free(message);
    }

    pthread_cancel(thread);

    return 0;
}