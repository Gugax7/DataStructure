#include <stdio.h>
#include <stdlib.h>

typedef struct{
    int vi,vj;
    float peso;
}Aresta;

typedef struct{
    int num_vertices;
    int num_arestas;
    char tipo;
    int valorado;
    Aresta* arestas;
} Grafo;

Grafo* lerGrafoDeArquivo(const char* nome_arquivo){
    FILE *arquivo = fopen(nome_arquivo, "r");
    if(!arquivo){
        perror("Erro abrindo o arquivo!");
        return NULL;
    }

    Grafo* grafo = (Grafo*)malloc(sizeof(Grafo));
    if(!grafo){
        perror("Erro alocando memoria para o grafo");
        fclose(arquivo);
        return NULL;
    }

    fscanf(arquivo, "%d %d %c %d", &grafo->num_vertices, &grafo->num_arestas, &grafo->tipo, &grafo->valorado);
    grafo->arestas = (Aresta*) malloc(grafo->num_arestas * sizeof(Aresta));
    if(!grafo->arestas){
        perror("erro alocando memoria para as arestas do grafo!");
        free(grafo);
        fclose(arquivo);
        return NULL;
    }

    for(int i = 0; i < grafo->num_arestas; i++){
        if(grafo->valorado){
            fscanf(arquivo, "%d %d %f", &grafo->arestas[i].vi, &grafo->arestas[i].vj, &grafo->arestas[i].peso);
        }
        else{
            fscanf(arquivo, "%d %d", &grafo->arestas[i].vi, &grafo->arestas[i].vj);
            grafo->arestas[i].peso = 0;
        }
    }
    fclose(arquivo);
    return grafo;
    
}

void criarGrafoESalvar(const char* nome_arquivo){
    FILE* arquivo = fopen(nome_arquivo, "w");
    if(!arquivo){
        perror("Erro ao abrir o arquivo para escrita!");
        return;
    }

    int num_vertices, num_arestas, valorado;
    char tipo;

    printf("Digite o número de vértices: ");
    scanf("%d", &num_vertices);

    printf("Digite o número de arestas/arcos: ");
    scanf("%d", &num_arestas);

    printf("Digite o tipo do grafo ('G' para não dirigido, 'D' para dirigido): ");
    scanf(" %c", &tipo);

    printf("O grafo é valorado? (1 para sim, 0 para não): ");
    scanf("%d", &valorado);

    fprintf(arquivo, "%d %d %c %d\n", num_vertices, num_arestas, tipo, valorado);

    printf("Insira as arestas/arcos no formato:\n");
    if (valorado) {
        printf("<vi> <vj> <peso>\n");
    } else {
        printf("<vi> <vj>\n");
    }

    for (int i = 0; i < num_arestas; i++) {
        int vi, vj;
        float peso = 0;

        if (valorado) {
            printf("Aresta/Arco %d: ", i + 1);
            scanf("%d %d %f", &vi, &vj, &peso);
            fprintf(arquivo, "%d %d %.2f\n", vi, vj, peso);
        } else {
            printf("Aresta/Arco %d: ", i + 1);
            scanf("%d %d", &vi, &vj);
            fprintf(arquivo, "%d %d\n", vi, vj);
        }
    }

    fclose(arquivo);
    printf("Grafo salvo com sucesso no arquivo '%s'.\n", nome_arquivo);

}

void exibirGrafo(Grafo* grafo) {
    if (!grafo) return;

    printf("Número de vértices: %d\n", grafo->num_vertices);
    printf("Número de arestas: %d\n", grafo->num_arestas);
    printf("Tipo: %c\n", grafo->tipo);
    printf("Valorado: %d\n", grafo->valorado);

    for (int i = 0; i < grafo->num_arestas; i++) {
        printf("Aresta %d: %d -> %d", i + 1, grafo->arestas[i].vi, grafo->arestas[i].vj);
        if (grafo->valorado) {
            printf(" (Peso: %.2f)", grafo->arestas[i].peso);
        }
        printf("\n");
    }
}



int main() {
    const char* nome_arquivo = "grafo.txt"; // Nome do arquivo para salvar e ler o grafo

    // Passo 1: Criar um grafo e salvá-lo no arquivo
    printf("Criando um grafo...\n");
    criarGrafoESalvar(nome_arquivo);

    // Passo 2: Ler o grafo do arquivo
    printf("\nLendo o grafo do arquivo '%s'...\n", nome_arquivo);
    Grafo* grafo = lerGrafoDeArquivo(nome_arquivo);

    // Passo 3: Exibir o grafo no terminal
    if (grafo) {
        printf("\nExibindo o grafo:\n");
        exibirGrafo(grafo);

        // Liberar a memória alocada para o grafo
        free(grafo->arestas);
        free(grafo);
    } else {
        printf("Erro ao ler o grafo do arquivo.\n");
    }

    return 0;
}
