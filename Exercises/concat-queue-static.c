#include <stdio.h>
#include <stdbool.h>

#define MAX 100

// Estrutura da fila estática
typedef struct {
    int dados[MAX];
    int inicio;
    int fim;
    int tamanho;
} Fila;

// Inicializa a fila
void inicializarFila(Fila* f) {
    f->inicio = 0;
    f->fim = -1;
    f->tamanho = 0;
}

// Verifica se a fila está cheia
bool filaCheia(Fila* f) {
    return f->tamanho == MAX;
}

// Verifica se a fila está vazia
bool filaVazia(Fila* f) {
    return f->tamanho == 0;
}

// Enfileira um elemento
bool enfileirar(Fila* f, int valor) {
    if (filaCheia(f)) return false;

    f->fim = (f->fim + 1) % MAX;
    f->dados[f->fim] = valor;
    f->tamanho++;
    return true;
}

// Imprime a fila
void imprimirFila(Fila* f) {
    int i, idx;
    for (i = 0; i < f->tamanho; i++) {
        idx = (f->inicio + i) % MAX;
        printf("%d ", f->dados[idx]);
    }
    printf("\n");
}

// Concatena f2 no final de f1
bool concatenarFilas(Fila* f1, Fila* f2) {
    if (f1->tamanho + f2->tamanho > MAX) return false;

    for (int i = 0; i < f2->tamanho; i++) {
        int idx = (f2->inicio + i) % MAX;
        enfileirar(f1, f2->dados[idx]);
    }
    return true;
}

// Função principal
int main() {
    Fila fila1, fila2;
    inicializarFila(&fila1);
    inicializarFila(&fila2);

    // Inserindo elementos na fila1
    enfileirar(&fila1, 1);
    enfileirar(&fila1, 2);
    enfileirar(&fila1, 3);

    // Inserindo elementos na fila2
    enfileirar(&fila2, 4);
    enfileirar(&fila2, 5);
    enfileirar(&fila2, 6);

    printf("Fila 1 antes da concatenação: ");
    imprimirFila(&fila1);

    printf("Fila 2 antes da concatenação: ");
    imprimirFila(&fila2);

    // Concatenar
    if (concatenarFilas(&fila1, &fila2)) {
        printf("Fila 1 após a concatenação: ");
        imprimirFila(&fila1);
    } else {
        printf("Não foi possível concatenar: espaço insuficiente.\n");
    }

    return 0;
}