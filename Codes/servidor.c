#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "banco.h"

#define TAM_BUFFER 256
#define ARQUIVO_BANCO "banco.txt"
#define FIFO_REQUISICAO "/tmp/fifo_requisicao"
#define FIFO_RESPOSTA "/tmp/fifo_resposta"

pthread_mutex_t mutex_banco = PTHREAD_MUTEX_INITIALIZER;

void enviar_resposta(const char *resposta) {
    int fd_resp = open(FIFO_RESPOSTA, O_WRONLY);
    if (fd_resp >= 0) {
        write(fd_resp, resposta, strlen(resposta) + 1);
        close(fd_resp);
    }
}

void inserir(int id, const char *nome, char *resposta) {
    pthread_mutex_lock(&mutex_banco);

    // Primeiro, verifica se o ID já existe
    FILE *f = fopen(ARQUIVO_BANCO, "r");
    int id_existente = 0;
    if (f != NULL) {
        Registro r;
        while (fscanf(f, "%d %49s", &r.id, r.nome) == 2) {
            if (r.id == id) {
                id_existente = 1;
                break;
            }
        }
        fclose(f);
    }

    if (id_existente) {
        strcpy(resposta, "Erro: ID já existe no banco.");
    } else {
        f = fopen(ARQUIVO_BANCO, "a");
        if (f == NULL) {
            strcpy(resposta, "Erro ao abrir banco.");
        } else {
            fprintf(f, "%d %s\n", id, nome);
            fclose(f);
            strcpy(resposta, "Registro inserido com sucesso.");
        }
    }

    pthread_mutex_unlock(&mutex_banco);
}

void deletar(int id, char *resposta) {
    pthread_mutex_lock(&mutex_banco);
    FILE *f = fopen(ARQUIVO_BANCO, "r");
    FILE *tmp = fopen("tmp.txt", "w");
    int achou = 0;
    if (f && tmp) {
        Registro r;
        while (fscanf(f, "%d %49s", &r.id, r.nome) != EOF) {
            if (r.id != id) {
                fprintf(tmp, "%d %s\n", r.id, r.nome);
            } else {
                achou = 1;
            }
        }
        fclose(f);
        fclose(tmp);
        rename("tmp.txt", ARQUIVO_BANCO);
        if (achou)
            strcpy(resposta, "Registro deletado.");
        else
            strcpy(resposta, "Registro não encontrado.");
    } else {
        strcpy(resposta, "Erro ao abrir o banco.");
    }
    pthread_mutex_unlock(&mutex_banco);
}

void selecionar_por_id(int id, char *resposta) {
    pthread_mutex_lock(&mutex_banco);
    FILE *f = fopen(ARQUIVO_BANCO, "r");
    if (f) {
        Registro r;
        int achou = 0;
        while (fscanf(f, "%d %49s", &r.id, r.nome) != EOF) {
            if (r.id == id) {
                sprintf(resposta, "Registro encontrado: id=%d nome=%s", r.id, r.nome);
                achou = 1;
                break;
            }
        }
        if (!achou)
            strcpy(resposta, "Registro não encontrado.");
        fclose(f);
    } else {
        strcpy(resposta, "Erro ao abrir o banco.");
    }
    pthread_mutex_unlock(&mutex_banco);
}

void selecionar_por_nome(const char *nome, char *resposta) {
    pthread_mutex_lock(&mutex_banco);
    FILE *f = fopen(ARQUIVO_BANCO, "r");
    if (f) {
        Registro r;
        int achou = 0;
        while (fscanf(f, "%d %49s", &r.id, r.nome) != EOF) {
            if (strcmp(r.nome, nome) == 0) {
                sprintf(resposta, "Registro encontrado: id=%d nome=%s", r.id, r.nome);
                achou = 1;
                break;
            }
        }
        if (!achou)
            strcpy(resposta, "Registro não encontrado.");
        fclose(f);
    } else {
        strcpy(resposta, "Erro ao abrir o banco.");
    }
    pthread_mutex_unlock(&mutex_banco);
}

void atualizar(int id, const char *novo_nome, char *resposta) {
    pthread_mutex_lock(&mutex_banco);
    FILE *f = fopen(ARQUIVO_BANCO, "r");
    FILE *tmp = fopen("tmp.txt", "w");
    int achou = 0;
    if (f && tmp) {
        Registro r;
        while (fscanf(f, "%d %49s", &r.id, r.nome) != EOF) {
            if (r.id == id) {
                fprintf(tmp, "%d %s\n", id, novo_nome);
                achou = 1;
            } else {
                fprintf(tmp, "%d %s\n", r.id, r.nome);
            }
        }
        fclose(f);
        fclose(tmp);
        rename("tmp.txt", ARQUIVO_BANCO);
        if (achou)
            strcpy(resposta, "Registro atualizado.");
        else
            strcpy(resposta, "Registro não encontrado.");
    } else {
        strcpy(resposta, "Erro ao abrir o banco.");
    }
    pthread_mutex_unlock(&mutex_banco);
}

void *tratar_requisicao(void *arg) {
    char *req = (char *)arg;
    char resposta[TAM_BUFFER] = "Requisição inválida";

    int id;
    char nome[50];
    
    if (sscanf(req, "INSERT id=%d nome=%49[^\n]", &id, nome) == 2) {
        if (id < 0) {
            strcpy(resposta, "Erro: ID negativo não é permitido.");
        } else {
        inserir(id, nome, resposta);
        }
    }
    else if (sscanf(req, "DELETE id=%d", &id) == 1) {
        if (id < 0) {
            strcpy(resposta, "Erro: ID negativo não é permitido.");
        } else {
        deletar(id, resposta);
        }
    }
    else if (sscanf(req, "SELECT id WHERE nome=%49s", nome) == 1) {
        if (id < 0) {
            strcpy(resposta, "Erro: ID negativo não é permitido.");
        } else {
        selecionar_por_nome(nome, resposta);
        }
    }
    else if (sscanf(req, "SELECT nome WHERE id=%d", &id) == 1) {
        if (id < 0) {
            strcpy(resposta, "Erro: ID negativo não é permitido.");
        } else {
        selecionar_por_id(id, resposta);
        }
    }
    else if (sscanf(req, "UPDATE id=%d nome=%49s", &id, nome) == 2) {
        if (id < 0) {
            strcpy(resposta, "Erro: ID negativo não é permitido.");
        } else {
        atualizar(id, nome, resposta);
        }
    }

    enviar_resposta(resposta);
    free(req);
    return NULL;
}

int main() {
    char buffer[TAM_BUFFER];

    mkfifo(FIFO_REQUISICAO, 0666);
    mkfifo(FIFO_RESPOSTA, 0666);

    // threadpool thpool = thpool_init(5);

    printf("Servidor iniciado. Aguardando requisições...\n");

    while (1) {
        int fd_req = open(FIFO_REQUISICAO, O_RDONLY);
        if (fd_req < 0) {
            perror("Erro ao abrir pipe de requisição");
            exit(1);
        }

        ssize_t bytes_lidos = read(fd_req, buffer, sizeof(buffer));
        close(fd_req);

        if (bytes_lidos > 0) {
            char *requisicao = strdup(buffer);
            
            // for (int i=0; i<40; i++){
            //     thpool_add_work(thpool, tratar_requisicao, (void*)(uintptr_t)i);
            // };

            // thpool_wait(thpool);
            // thpool_destroy(thpool);
            pthread_t tid;
            pthread_create(&tid, NULL, tratar_requisicao, requisicao);
            pthread_detach(tid);
        }
    }

    return 0;
}
