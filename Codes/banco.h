#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define TAM_BUFFER 100
#define ARQUIVO_BANCO "banco.txt"
#define FIFO_REQUISICAO "/tmp/fifo_requisicao"
#define FIFO_RESPOSTA   "/tmp/fifo_resposta"

typedef struct {
    int id;
    char nome[50];
} Registro;