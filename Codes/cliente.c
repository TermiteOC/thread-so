#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>  /// O_RDONLY, O_WRONLY, open
#include <unistd.h> /// read, write, close, mkfifo

#include "banco.h"

void clear() {
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}

int main() {
    mkfifo(FIFO_REQUISICAO, 0666);
    mkfifo(FIFO_RESPOSTA, 0666);

    char requisicao[TAM_BUFFER];
    char resposta[TAM_BUFFER];

    while (1) {
        printf("Cliente iniciado. Digite as requisições no seguinte formato:\n");
        printf("INSERT id=* nome=*\n");
        printf("DELETE id=*\n");
        printf("SELECT atributo WHERE atributo=*\n");
        printf("UPDATE id=* nome=*\n");
        printf("sair\n");

        fgets(requisicao, TAM_BUFFER, stdin); /// requisicao inclui \n em vez de \0
        requisicao[strcspn(requisicao, "\n")] = '\0';

        if (strcmp(requisicao, "sair") == 0) {
            break;
        }

        int fd_req = open(FIFO_REQUISICAO, O_WRONLY);
        if (fd_req < 0) {
            clear();
            printf("Erro ao abrir o pipe de requisição");
            exit(1);
        }
        write(fd_req, requisicao, strlen(requisicao) + 1); /// +1 para permitir o byte do \0 ser escrito
        close(fd_req);

        int fd_resp = open(FIFO_RESPOSTA, O_RDONLY);
        if (fd_resp < 0) {
            clear();
            printf("Erro ao abrir o pipe de resposta");
            exit(1);
        }
        ssize_t chars_lidos = read(fd_resp, resposta, TAM_BUFFER); /// não inclui o \0
        resposta[chars_lidos] = '\0';
        close(fd_resp);

        printf("Servidor: %s\n", resposta);
        printf("Pressione Enter para continuar...");
        getchar();
        clear();
    }

    clear();
    printf("Cliente finalizado.\n");
    return 0;
}
