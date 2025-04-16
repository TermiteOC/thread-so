#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>    /// O_RDONLY, O_WRONLY, open
#include <unistd.h>   /// read, write, close, mkfifo
#include <pthread.h>
#include <sys/stat.h> /// permissão do mkfifo

#define TAM_BUFFER 100
#define ARQUIVO_BANCO "banco.txt"
#define FIFO_REQUISICAO "/tmp/fifo_requisicao"
#define FIFO_RESPOSTA   "/tmp/fifo_resposta"

typedef struct {
    int id;
    char nome[50];
} Registro;