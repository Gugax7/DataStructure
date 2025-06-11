#include <stdio.h>
#include <stdlib.h>

// Definição do nó da fila
typedef struct No {
    int valor;
    struct No* prox;
} No;

// Estrutura da fila
typedef struct {
    No* inicio;
    No* fim;
} Fila;

// Inicializa a fila
void inicializarFila(Fila* f) {
    f->inicio = NULL;
    f->fim = NULL;
}

// Insere no final da fila
void enfileirar(Fila* f, int valor) {
    No* novo = (No*)malloc(sizeof(No));
    novo->valor = valor;
    novo->prox = NULL;
    
    if (f->fim == NULL) {
        f->inicio = novo;
    } else {
        f->fim->prox = novo;
    }
    f->fim = novo;
}

// Imprime a fila
void imprimirFila(Fila* f) {
    No* atual = f->inicio;
    while (atual != NULL) {
        printf("%d ", atual->valor);
        atual = atual->prox;
    }
    printf("\n");
}

// Concatena fila2 ao final da fila1
void concatenarFilas(Fila* f1, Fila* f2) {
    if (f2->inicio == NULL) return; // fila2 vazia

    if (f1->fim == NULL) {
        // fila1 vazia, recebe toda a fila2
        f1->inicio = f2->inicio;
        f1->fim = f2->fim;
    } else {
        f1->fim->prox = f2->inicio;
        f1->fim = f2->fim;
    }

    // Esvazia f2
    f2->inicio = NULL;
    f2->fim = NULL;
}

// Libera memória da fila
void liberarFila(Fila* f) {
    No* atual = f->inicio;
    while (atual != NULL) {
        No* temp = atual;
        atual = atual->prox;
        free(temp);
    }
    f->inicio = NULL;
    f->fim = NULL;
}

// Função principal
int main() {
    Fila fila1, fila2;
    inicializarFila(&fila1);
    inicializarFila(&fila2);

    // Inserindo elementos
    enfileirar(&fila1, 1);
    enfileirar(&fila1, 2);
    enfileirar(&fila1, 3);

    enfileirar(&fila2, 4);
    enfileirar(&fila2, 5);
    enfileirar(&fila2, 6);

    printf("Fila 1 antes da concatenação: ");
    imprimirFila(&fila1);

    printf("Fila 2 antes da concatenação: ");
    imprimirFila(&fila2);

    // Concatenar
    concatenarFilas(&fila1, &fila2);

    printf("Fila 1 após a concatenação: ");
    imprimirFila(&fila1);

    // Liberar memória
    liberarFila(&fila1);
    liberarFila(&fila2);

    return 0;
}


// Estrutura do nó da fila
typedef struct No {
    int valor;
    struct No* prox;
} No;

// Enfileira no final (precisa percorrer tudo)
void enfileirar(No** fila, int valor) {
    No* novo = (No*) malloc(sizeof(No));
    novo->valor = valor;
    novo->prox = NULL;

    if (*fila == NULL) {
        // Fila vazia
        *fila = novo;
    } else {
        // Percorre até o fim
        No* atual = *fila;
        while (atual->prox != NULL) {
            atual = atual->prox;
        }
        atual->prox = novo;
    }
}

// Remove do início
int desenfileirar(No** fila, int* valor) {
    if (*fila == NULL) {
        return 0;
    }

    No* temp = *fila;
    *valor = temp->valor;
    *fila = temp->prox;
    free(temp);
    return 1;
}

// Imprime os elementos
void imprimirFila(No* fila) {
    No* atual = fila;
    printf("Fila: ");
    while (atual != NULL) {
        printf("%d ", atual->valor);
        atual = atual->prox;
    }
    printf("\n");
}

// Concatena f2 no fim de f1
void concatenarFilas(No** f1, No* f2) {
    if (*f1 == NULL) {
        *f1 = f2;
    } else {
        No* atual = *f1;
        while (atual->prox != NULL) {
            atual = atual->prox;
        }
        atual->prox = f2;
    }
}

// Libera toda a memória da fila
void liberarFila(No** fila) {
    No* atual = *fila;
    while (atual != NULL) {
        No* temp = atual;
        atual = atual->prox;
        free(temp);
    }
    *fila = NULL;
}

// Função principal
int main() {
    No* fila1 = NULL;
    No* fila2 = NULL;

    // Preenche fila1
    enfileirar(&fila1, 1);
    enfileirar(&fila1, 2);
    enfileirar(&fila1, 3);

    // Preenche fila2
    enfileirar(&fila2, 4);
    enfileirar(&fila2, 5);
    enfileirar(&fila2, 6);

    printf("Fila 1 antes da concatenação:\n");
    imprimirFila(fila1);

    printf("Fila 2 antes da concatenação:\n");
    imprimirFila(fila2);

    // Concatena
    concatenarFilas(&fila1, fila2);

    printf("Fila 1 após a concatenação:\n");
    imprimirFila(fila1);

    // Limpeza
    liberarFila(&fila1);
    // fila2 já está inclusa em fila1, não precisa liberar separadamente

    return 0;
}