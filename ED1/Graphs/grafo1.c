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
            fscanf(arquivo, "%d %d %d", &grafo->arestas[i].vi, &grafo->arestas[i].vj, &grafo->arestas[i].peso);
        }
        else{
            fscanf(arquivo, "%d %d", &grafo->arestas[i].vi, &grafo->arestas[i].vj);
            grafo->arestas[i].peso = 0;
        }
    }
    fclose(arquivo);
    return grafo;
    
}