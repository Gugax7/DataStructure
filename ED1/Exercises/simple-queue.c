#include <stdio.h>
#include <stdlib.h>
#define MAX 100

typedef struct {
    int dados[MAX];
    int frente;
    int tras;
} Fila;

// Inicializa a fila
void inicializarFila(Fila* f) {
    f->frente = 0;
    f->tras = -1;
}

// Verifica se a fila está cheia
int filaCheia(Fila* f) {
    return f->tras == MAX - 1;
}

// Verifica se a fila está vazia
int filaVazia(Fila* f) {
    return f->frente > f->tras;
}

// Insere um elemento no final da fila
int enfileirar(Fila* f, int valor) {
    if (filaCheia(f)) {
        printf("Fila cheia!\n");
        return 0;
    }
    f->tras++;
    f->dados[f->tras] = valor;
    return 1;
}

// Remove um elemento do início da fila
int desenfileirar(Fila* f, int* valorRemovido) {
    if (filaVazia(f)) {
        printf("Fila vazia!\n");
        return 0;
    }
    *valorRemovido = f->dados[f->frente];
    f->frente++;
    return 1;
}

// Imprime os elementos da fila
void imprimirFila(Fila* f) {
    if (filaVazia(f)) {
        printf("Fila vazia!\n");
        return;
    }
    printf("Fila: ");
    for (int i = f->frente; i <= f->tras; i++) {
        printf("%d ", f->dados[i]);
    }
    printf("\n");
}

// Função principal
int main() {
    Fila fila;
    inicializarFila(&fila);

    enfileirar(&fila, 10);
    enfileirar(&fila, 20);
    enfileirar(&fila, 30);
    imprimirFila(&fila);

    int removido;
    if (desenfileirar(&fila, &removido)) {
        printf("Removido: %d\n", removido);
    }

    imprimirFila(&fila);

    return 0;
}
