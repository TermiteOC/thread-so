#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define TAM_BUFFER 256
#define FIFO_REQUISICAO "/tmp/fifo_requisicao"
#define FIFO_RESPOSTA   "/tmp/fifo_resposta"

void clear() {
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}

int main() {
    mkfifo(FIFO_REQUISICAO, 0666);
    mkfifo(FIFO_RESPOSTA, 0666);

    char requisicao[TAM_BUFFER];
    char resultado[TAM_BUFFER];

    while (1) {
        printf("Cliente iniciado. Digite as requisições no seguinte formato:\n");
        printf("INSERT id=* nome=*\n");
        printf("DELETE id=*\n");
        printf("SELECT atributo WHERE atributo=*\n");
        printf("UPDATE id=* nome=*\n");
        printf("sair\n");

        fgets(requisicao, TAM_BUFFER, stdin);
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
        write(fd_req, requisicao, strlen(requisicao));
        close(fd_req);

        int fd_resp = open(FIFO_RESPOSTA, O_RDONLY);
        if (fd_resp < 0) {
            clear();
            printf("Erro ao abrir o pipe de resposta");
            exit(1);
        }
        read(fd_resp, resultado, TAM_BUFFER);
        close(fd_resp);

        printf("Servidor: %s\n", resultado);
        printf("Pressione Enter para continuar...");
        getchar();
        clear();
    }

    clear();
    printf("Cliente finalizado.\n");
    return 0;
}
