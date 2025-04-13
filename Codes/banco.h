#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int id;
    char nome[50];
} Registro;