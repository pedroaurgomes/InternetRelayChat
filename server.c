#include "utils.h"

//envio de informação com handshake, e 5 tentativas
bool send_to_client(int socket_descriptor, char *message, int str_length)
{
    for (int i = 0; i < MAX_ATTEMPTS; ++i)
    {
        send(socket_descriptor, message, str_length, 0);

        char check_message[9];
        recv(socket_descriptor, check_message, 9, 0);
        if (strncmp("received", check_message, 8) == 0)
        {
            return true;
        }
    }

    printf("Failure on sending a message to client. Disconnecting client from server...");
    return false;
}

// retorna um canal dado um nome
CHANNEL *search_channel_by_name(char *name, SERVER_SIDE *server)
{
    // Busca sequencial por todos os clientes a partir do nickname dado
    for (int i = 0; i < MAX_CHANNELS; ++i)
    {
        if (!server->channels[i])
            continue;
        if (strcmp(name, server->channels[i]->channel_name) == 0)
            return server->channels[i];
    }

    return NULL;
}

// Função que verifica se o nome do canal está de acordo com as regras impostas pelo RFC 1459
bool check_channel_name(char *channel_name)
{

    // A channel name must start with either '#' or '&'
    if ((channel_name[0] != '#') && (channel_name[0] != '&'))
    {
        return false;
    }

    // Forbidden character: ' ' ',' '^G' ( == ASCII 7)
    if ((strchr(channel_name, ' ') != NULL) || (strchr(channel_name, ',') != NULL) || (strchr(channel_name, 7) != NULL))
        return false;

    return true;
}

// função para deletar um canal do server
void delete_channel(SERVER_SIDE *server, CHANNEL *channel)
{
    for (int i = 0; i < MAX_CHANNELS; ++i)
    {
        if (server->channels[i] != NULL && server->channels[i] == channel)
        {
            printf("Deleting channel %s\n", server->channels[i]->channel_name);
            server->channels[i] = NULL;
            server->num_channels--;

            free(channel);
        }
    }
}

// Dado um cliente e um canal, adiciona o cliente a esse canal e remove do antigo
bool insert_client_channel(SERVER_SIDE *server, CHANNEL *channel, CLIENT *client)
{
    // caso não seja possível fazer a busca
    if (channel == NULL || client == NULL || channel->num_clients == MAX_CLIENTS_PER_CHANNEL)
        return false;

    // se estava em um canal antes, tratamento de erros
    if (client->user_channel != NULL)
    {
        // tira cliente do canal antigo
        printf("User already on channel %s\n", client->user_channel->channel_name);
        for (int i = 0; i < MAX_CLIENTS_PER_CHANNEL; ++i) {
            if (client->user_channel->clients[i] == client) {
                client->user_channel->clients[i] = NULL;
                client->user_channel->num_clients--;
            }
        }

        // Cliente é o último usuário do canal - matar canal
        if (client->user_channel->num_clients == 0)
        {
            delete_channel(server, client->user_channel);
            client->user_channel = NULL;
        }

        // Cliente é administrador - passar adm para outro cliente
        else if (client->is_admin)
        {
            for (int i = 0; i < MAX_CLIENTS_PER_CHANNEL; ++i) {
                if (client->user_channel->clients[i] != NULL && client != client->user_channel->clients[i]) {
                    client->user_channel->clients[i]->is_admin = true;
                    break;
                }
            }
        }
    }

    // adição do cliente no canal
    for (int i = 0; i < MAX_CLIENTS_PER_CHANNEL; i++)
    {
        if (!channel->clients[i])
        {
            channel->clients[i] = client;
            break;
        }
    }

    channel->num_clients++;
    
    client->is_muted = false;
    client->user_channel = channel;
    
    return true;
}

// inserção de novo canal no server
bool insert_channel_server(CHANNEL *channel, SERVER_SIDE *server)
{
    if (!channel || !server || server->num_channels == MAX_CHANNELS)
        return false;
    for (int i = 0; i < MAX_CHANNELS; i++)
    {
        if (!server->channels[i])
        {
            server->channels[i] = channel;
            break;
        }
    }

    server->num_channels++;
    return true;
}

// aceitando conexões novas, passa pela lista de clientes no server, checa posições vazias, e cria conexão
// roda em uma thread separada para sempre lidar com clientes novos
void *connect_server_client(void *args)
{
    SERVER_SIDE *server = (SERVER_SIDE *)args;

    while (true)
    {
        for (int i = 0; i < MAX_CLIENTS_ON; i++)
        {
            if (!server->clients[i])
            {
                CLIENT *cur_client = calloc(1, sizeof(CLIENT));
                cur_client->socket_descriptor = accept(server->socket_descriptor, (struct sockaddr *)&cur_client->addr, &cur_client->addr_len);

                recv(cur_client->socket_descriptor, cur_client->nickname, NICKNAME_MAX_SIZE, 0);

                char *welcome_message = calloc(8 + strlen(cur_client->nickname), sizeof(char));

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

// encerrando conexão de um cliente específico
bool quit_cmd(SERVER_SIDE *server, CLIENT *cur_client)
{
    //remoção do cliente no canal atual
    for (int j = 0; j < MAX_CLIENTS_PER_CHANNEL; j++)
    {
        if (cur_client->user_channel != NULL && cur_client->user_channel->clients[j] == cur_client)
        {
            cur_client->user_channel->clients[j] = NULL;
            cur_client->user_channel->num_clients--;
        }
    }

    // se canal esvaziar apaga eles
    if (cur_client->user_channel->num_clients == 0)
    {
        delete_channel(server, cur_client->user_channel);
        cur_client->user_channel = NULL;
    }
    else
    {
        // Definindo um novo admin para o canal se preciso
        if (cur_client->is_admin)
        {
            for (int j = 0; j < MAX_CLIENTS_PER_CHANNEL; j++)
            {
                if (cur_client->user_channel->clients[j] != NULL)
                {
                    cur_client->user_channel->clients[j]->is_admin = true;
                }
            }
        }
    }

    // tira cliente dos servers
    for (int j = 0; j < MAX_CLIENTS_ON; j++)
    {
        if (cur_client == server->clients[j])
        {
            server->clients[j] = NULL;
            server->num_clients--;
        }
    }

    close(cur_client->socket_descriptor);
    printf("Client %s disconected\n", cur_client->nickname);
    free(cur_client);
    return false;
}

// envio de mensagem de um cliente
bool message_cmd(SERVER_SIDE *server, CLIENT *cur_client)
{
    //pegando mensagem
    char message[MAX_BUFFER];
    recv(cur_client->socket_descriptor, message, MAX_BUFFER, MSG_DONTWAIT);

    // checando se é possível enviar mensagem
    if (!cur_client->user_channel)
    {
        return send_to_client(cur_client->socket_descriptor, "Please connect to a channel to send a msg", 42);
    }

    if (cur_client->is_muted)
    {
        return send_to_client(cur_client->socket_descriptor, "Error: user is muted", 21);
    }

    //envio das mensagens
    printf("Forwarding message: '%s' from '%s' to channel '%s'\n", message, cur_client->nickname, cur_client->user_channel->channel_name);

    for (int j = 0; j < MAX_CLIENTS_PER_CHANNEL; ++j)
    {
        if (cur_client->user_channel->clients[j] != NULL && cur_client != cur_client->user_channel->clients[j])
        {
            printf("Forwarding to %s\n", cur_client->user_channel->clients[j]->nickname);

            char *forward_message = calloc(MAX_BUFFER, sizeof(char));
            strcat(forward_message, "[");
            strcat(forward_message, cur_client->user_channel->channel_name);
            strcat(forward_message, "]");
            strcat(forward_message, cur_client->nickname);
            strcat(forward_message, ": ");
            strcat(forward_message, message);

            // se não conseguir enviar mensagem em 5 tentativas, desconecta
            if (!send_to_client(cur_client->user_channel->clients[j]->socket_descriptor, forward_message, MAX_BUFFER))
            {
                quit_cmd(server, cur_client->user_channel->clients[j]);
            }
        }
    }

    return true;
}

// resposta do ping
bool ping_cmd(CLIENT *cur_client)
{
    return send_to_client(cur_client->socket_descriptor, "[ * server * ] pong", 20);
}

//inserção em novo canal dado um cliente
bool join_cmd(SERVER_SIDE *server, CLIENT *cur_client)
{
    //entrada do nick do canal
    char channel_name[CHANNEL_NAME_MAX_SIZE];
    recv(cur_client->socket_descriptor, channel_name, CHANNEL_NAME_MAX_SIZE, MSG_DONTWAIT);

    printf("Printing Channel name (not verified): %s\n", channel_name);

    // Verificando se o nome do canal enviado pelo cliente segue as restrições impostas pelo RFC 1459
    if (!check_channel_name(channel_name))
    {
        return send_to_client(cur_client->socket_descriptor, "Invalid channel name. Please follow the RFC 1459 restrictions", 62);
    }

    CHANNEL *cur_channel = search_channel_by_name(channel_name, server);
    if (!cur_channel) //criação de canal se não existir
    {
        cur_channel = calloc(1, sizeof(CHANNEL));
        strcpy(cur_channel->channel_name, channel_name);
        if (!insert_channel_server(cur_channel, server)) //tratamento de erro
        {
            free(cur_channel);
            return send_to_client(cur_client->socket_descriptor, "Error: max number of channels was reached.", 43);
        }
        cur_client->is_admin = true;

        printf("Channel with name '%s' created by '%s'\n", cur_channel->channel_name, cur_client->nickname);
    }

    else
    {
        cur_client->is_admin = false; //se entrar em um canal que já existir, não será admin
    }

    if (!insert_client_channel(server, cur_channel, cur_client)) //tratamento de erro
    {
        printf("Failure in the creation of channel with name '%s' by '%s'\n", cur_channel->channel_name, cur_client->nickname);
        return send_to_client(cur_client->socket_descriptor, "Error: max number of clients was reached.", 42);
    }

    return send_to_client(cur_client->socket_descriptor, "Successufuly entered channel.", 30);
}

// mudança de apelido
bool nickname_cmd(CLIENT *cur_client)
{
    recv(cur_client->socket_descriptor, cur_client->nickname, NICKNAME_MAX_SIZE, MSG_DONTWAIT);
    return send_to_client(cur_client->socket_descriptor, "Successfuly changed nickname.", 31);
}

// expulsa usuário de um canal
bool kick_cmd(SERVER_SIDE *server, CLIENT *cur_client)
{
    //recebe nick de quem for pra ser expulso
    char name[NICKNAME_MAX_SIZE];
    recv(cur_client->socket_descriptor, name, NICKNAME_MAX_SIZE, MSG_DONTWAIT);

    if (!cur_client->is_admin) //checa permissões
    {
        return send_to_client(cur_client->socket_descriptor, "Permission denied - not channel admin.", 39);
    }

    // expulsa todos os usuários com nick do canal
    bool found_user = false;
    for (int j = 0; j < MAX_CLIENTS_PER_CHANNEL; j++)
    {
        if (cur_client->user_channel->clients[j] == NULL || cur_client == cur_client->user_channel->clients[j])
            continue;
        if (strcmp(cur_client->user_channel->clients[j]->nickname, name) == 0)
        {
            //expulsão do usuário e tratamento de erro
            if (!send_to_client(cur_client->user_channel->clients[j]->socket_descriptor, "You got kicked out from the channel.", 37))
            {
                quit_cmd(server, cur_client->user_channel->clients[j]);
            }

            cur_client->user_channel->clients[j]->user_channel = NULL;
            cur_client->user_channel->clients[j]->is_muted = false;
            cur_client->user_channel->clients[j] = NULL;
            cur_client->user_channel->num_clients--;

            found_user = true;
        }
    }

    // retorno da função
    if (found_user)
        return send_to_client(cur_client->socket_descriptor, "User(s) succesfuly kicked from channel.", 40);
    return send_to_client(cur_client->socket_descriptor, "User(s) not found.", 19);
}

// silenciando cliente em um canal
bool mute_cmd(SERVER_SIDE *server, CLIENT *cur_client)
{
    //entrada do nickname
    char name[NICKNAME_MAX_SIZE];
    recv(cur_client->socket_descriptor, name, NICKNAME_MAX_SIZE, MSG_DONTWAIT);

    // checa de permissões
    if (!cur_client->is_admin)
    {
        return send_to_client(cur_client->socket_descriptor, "Permission denied - not channel admin.", 39);
    }

    // procura clientes com nick
    for (int j = 0; j < MAX_CLIENTS_PER_CHANNEL; j++)
    {
        if (cur_client->user_channel->clients[j] == NULL || cur_client == cur_client->user_channel->clients[j])
            continue;
        if (strcmp(cur_client->user_channel->clients[j]->nickname, name) == 0)
        {
            cur_client->user_channel->clients[j]->is_muted = true; //muda permissão de fala

            //tratamento de erro
            if (!send_to_client(cur_client->user_channel->clients[j]->socket_descriptor, "You are now muted on the channel.", 34))
            {
                quit_cmd(server, cur_client->user_channel->clients[j]);
            }
        }
    }
    return send_to_client(cur_client->socket_descriptor, "User(s) succesfuly muted.", 26);
}

bool unmute_cmd(SERVER_SIDE *server, CLIENT *cur_client)
{
    //entrada do nickname
    char name[NICKNAME_MAX_SIZE];
    recv(cur_client->socket_descriptor, name, NICKNAME_MAX_SIZE, MSG_DONTWAIT);

    // checa de permissões
    if (!cur_client->is_admin)
    {
        return send_to_client(cur_client->socket_descriptor, "Permission denied - not channel admin.", 39);
    }

    // procura clientes com nick
    for (int j = 0; j < MAX_CLIENTS_PER_CHANNEL; j++)
    {
        if (cur_client->user_channel->clients[j] == NULL || cur_client == cur_client->user_channel->clients[j])
            continue;
        if (strcmp(cur_client->user_channel->clients[j]->nickname, name) == 0)
        {
            cur_client->user_channel->clients[j]->is_muted = false;//muda permissão de fala

            //tratamento de erro
            if (!send_to_client(cur_client->user_channel->clients[j]->socket_descriptor, "You are now unmuted on the channel.", 36))
            {
                quit_cmd(server, cur_client->user_channel->clients[j]);
            }
        }
    }

    return send_to_client(cur_client->socket_descriptor, "User succesfuly unmuted.", 25);
}

bool whois_cmd(SERVER_SIDE *server, CLIENT *cur_client)
{
    //entrada do nickname
    char name[NICKNAME_MAX_SIZE];
    recv(cur_client->socket_descriptor, name, NICKNAME_MAX_SIZE, MSG_DONTWAIT);

    // checa de permissões
    if (!cur_client->is_admin)
    {
        return send_to_client(cur_client->socket_descriptor, "Permission denied - not channel admin.", 39);
    }

    // procura clientes com nick
    for (int j = 0; j < MAX_CLIENTS_PER_CHANNEL; j++)
    {
        if (cur_client->user_channel->clients[j] == NULL || cur_client == cur_client->user_channel->clients[j])
            continue;
        if (strcmp(cur_client->user_channel->clients[j]->nickname, name) == 0)
        {
            //recuperando ip
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &cur_client->addr, client_ip, INET_ADDRSTRLEN);

            //tratamento de erro
            if (!send_to_client(cur_client->socket_descriptor, client_ip, INET_ADDRSTRLEN))
            {
                quit_cmd(server, cur_client);
                return false;
            }
        }
    }

    return true;
}

int main(int argc, char *argv[])
{
    // criação da socket para realizar comunicação sockaddr_in
    socket_address server_address;

    int server_socket_descriptor;
    if ((server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    { // no caso de falha retorna erro
        printf("Socket initiation failed!\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Socket successfully created!\n");
    }

    // vinculação da socket com a porta que acontecerá a comunicação
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);

    // Criando a struct do servidor e inicia ela com valores nulos
    SERVER_SIDE server = {
        .socket_descriptor = server_socket_descriptor,
        .addr = server_address,
        .addr_len = sizeof(server_address),
        .num_channels = 0,
        .num_clients = 0,
    };

    for (int i = 0; i < MAX_CHANNELS; i++)
        server.channels[i] = NULL;
    for (int i = 0; i < MAX_CLIENTS_ON; i++)
        server.clients[i] = NULL;

    if ((bind(server.socket_descriptor, (const struct sockaddr *)&server.addr, server.addr_len)) < 0)
    {
        printf("Socket binding failed!\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Socket successfully binded to port %d\n", PORT);
    }

    // Coloca o socket do servidor em modo passivo, ou seja, espera pelo cliente estabelecer uma conexão
    // Segundo parametro (backlog): Números máximo de conexões pendentes na fila
    if ((listen(server.socket_descriptor, MAX_CLIENTS_ON)) != 0)
    {
        printf("Socket listenning failed!\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Socket successfully listening on port %d\n", PORT);
    }

    //thread de aceitar novos clientes
    pthread_t connection_thread;
    pthread_create(&connection_thread, NULL, connect_server_client, (void *)&server);

    bool end_server = false;
    while (!end_server)
    {
        //passa por todos os clientes e realiza a operação necessária se for requisitafa
        for (int i = 0; i < MAX_CLIENTS_ON; ++i)
        {
            if (server.clients[i] == NULL)
                continue;
            int opcode = -1;

            CLIENT *cur_client = server.clients[i];
            if (recv(cur_client->socket_descriptor, &opcode, sizeof(int), MSG_DONTWAIT) == -1)
                continue;
            if (opcode == -1)
                continue;

            printf("Request from %s with opcode %d\n", cur_client->nickname, opcode);
            bool mantain_user_connection = true;
            if (opcode == msg)
            {
                mantain_user_connection = message_cmd(&server, cur_client);
            }
            else if (opcode == ping)
            {
                mantain_user_connection = ping_cmd(cur_client);
            }
            else if (opcode == join)
            {
                mantain_user_connection = join_cmd(&server, cur_client);
            }
            else if (opcode == kick)
            {
                mantain_user_connection = kick_cmd(&server, cur_client);
            }
            else if (opcode == mute)
            {
                mantain_user_connection = mute_cmd(&server, cur_client);
            }
            else if (opcode == unmute)
            {
                mantain_user_connection = unmute_cmd(&server, cur_client);
            }
            else if (opcode == whois)
            {
                mantain_user_connection = whois_cmd(&server, cur_client);
            }
            else if (opcode == nickname)
            {
                mantain_user_connection = nickname_cmd(cur_client);
            }
            else if (opcode != quit)
            {
                printf("Command not supported");
                mantain_user_connection = send_to_client(cur_client->socket_descriptor, "Error - Command not Supported", 30);
            }
            // respostas das operações para se preciso desconectar cliente
            if (opcode == quit || mantain_user_connection == false)
            {
                quit_cmd(&server, cur_client);
            }
        }
    }

    close(server.socket_descriptor);

    return 0;
}