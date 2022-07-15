# Trabalho de Redes de Computadores (SSC0142)
## Entrega 1: Sockets - 07/06/2022

### Grupo:
* Bernardo Marques Costa - nUSP: 11795551
* João Victor Sene Araújo - nUSP: 11796382
* Pedro Augusto Ribeiro Gomes - nUSP: 11819125
* Rodrigo Lopes Assaf - nUSP: 11795530

### Informações adicionais:
* Sistema Operacional: Linux Ubuntu 20.04 (WSL)
* Compilador: gcc, com flags: -Werror -Wall -pthread
    <br> Obs: Recomenda-se o uso do Makefile para compilar e rodar o programa.

### Utilização Makefile
Para compilar o código do cliente e do servidor, basta rodar no terminal o seguinte comando 
```make [all]```
Com este comando, serão criados os binários do servidor e do cliente, que devem ser rodados em terminais diferentes para que seja possível realizar uma comunicação entre eles.
Uma vez com os arquivos binários, será possível utilizar os comandos `make client_side` e `make server_side` para rodar os códigos de cliente e servidor, respectivamente.