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

typedef struct{
    int pai, rank;
}Subconjunto;

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

void mostraGrau(Grafo* grafo){

    if(grafo->tipo == 'G'){
        for(int i = 0; i < grafo->num_vertices; i++){
            int grau = 0;
            for(int j = 0; j < grafo->num_arestas; j++){
                if(i + 1 == grafo->arestas[j].vi || i + 1 == grafo->arestas[j].vj){
                    grau++;
                }
            }
            printf("vertice %d possui grau %d\n",i+1,grau);
        }
    }
    else if(grafo->tipo == 'D'){
        for(int i = 0; i < grafo->num_vertices; i++){
            int grau_entrada = 0;
            int grau_saida = 0;
            for(int j = 0; j < grafo->num_arestas; j++){
                if(grafo->arestas[j].vj == i + 1) grau_entrada++;
                if(grafo->arestas[j].vi == i + 1) grau_saida++;
            }
            printf("Vértice %d possui grau de entrada %d e grau de saída %d\n", i + 1, grau_entrada, grau_saida);
        }
    }
}

void gerarMatrizAdjacencias(Grafo* grafo, const char* nome_arquivo){
    if(!grafo){
        printf("Grafo inválido. \n");
        return;
    }

    float** matriz = (float**)malloc(grafo->num_vertices * sizeof(float*));
    for(int i = 0; i < grafo->num_vertices; i++){
        matriz[i] = (float*) calloc(grafo->num_arestas, sizeof(float));
    }

    for(int i = 0; i < grafo->num_arestas; i++){
        int vi = grafo->arestas[i].vi - 1;
        int vj = grafo->arestas[i].vj - 1;
        float peso = grafo->arestas[i].peso;

        matriz[vi][vj] = peso ? peso : 1;
        if(grafo->tipo == 'G'){
            matriz[vj][vi] = peso ? peso : 1;
        }
    }

    FILE* arquivo = fopen(nome_arquivo, "w");
    if(!arquivo){
        perror("Erro ao abrir arquivo para salvar a matriz adjacencias");
        for(int i = 0; i < grafo->num_vertices; i++){
            free(matriz[i]);
        }
        free(matriz);
        return;
    }

    for(int i = 0; i< grafo->num_vertices; i++){
        for(int j = 0; j< grafo-> num_vertices; j++){
            fprintf(arquivo, "%.2f ", matriz[i][j]);
        }
        fprintf(arquivo,"\n");
    }

    fclose(arquivo);
    printf("Matriz adjacencia salva com sucesso no arquivo: %s\n", nome_arquivo);

    for(int i = 0; i < grafo->num_vertices; i++){
        free(matriz[i]);
    }
    free(matriz);

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

    gerarMatrizAdjacencias(grafo, "matriz_adjacencia.txt");

    // Passo 3: Exibir o grafo no terminal
    if (grafo) {
        printf("\nExibindo o grafo:\n");
        exibirGrafo(grafo);
        mostraGrau(grafo);

        // Liberar a memória alocada para o grafo
        free(grafo->arestas);
        free(grafo);
    } else {
        printf("Erro ao ler o grafo do arquivo.\n");
    }

    return 0;
}
