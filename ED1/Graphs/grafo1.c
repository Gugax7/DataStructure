#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#define MAX 100

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

// e) Encontrar uma árvore geradora mínima: Implementar um algoritmo para encontrar a árvore geradora mínima.

// aqui ele vai retornar (1 quando a>b), (0 quando a=b) e (-1 quando a<b)
// que é o que precisamos para fazer o qsort funcionar.
int compararArestas(const void* a, const void* b){
    Aresta* arestaA = (Aresta*)a;
    Aresta* arestaB = (Aresta*)b;

    return(arestaA->peso > arestaB->peso) - (arestaA->peso > arestaB->peso);
}

// agora para definir os subconjuntos utilizados pelo algoritmo de kruskal deveremos
// utilizar uma definição de pai do subconjunto, sendo assim todo subconjunto possui
// um pai que deve ser buscado toda vez que preciso checar se dois vertices estão no
// mesmo subconjunto, por isso do struct rank e pai.

int encontrar(Subconjunto subconjuntos[], int i){
    if(subconjuntos[i].pai != i){
        subconjuntos[i].pai = encontrar(subconjuntos, subconjuntos[i].pai);
    }
    return subconjuntos[i].pai;
}

void unir(Subconjunto subconjuntos[], int p1_sub, int p2_sub){
    // isso aqui é só para garantir que estamos trabalhando com os pais de fato
    int raizX = encontrar(subconjuntos,p1_sub);
    int raizY = encontrar(subconjuntos,p2_sub);

    // agora preciso garantir que o com maior rank seja o pai dentre eles.
    if(subconjuntos[raizX].rank > subconjuntos[raizY].rank){
        subconjuntos[raizY].pai = raizX;
    }
    else if(subconjuntos[raizX].rank < subconjuntos[raizY].rank){
        subconjuntos[raizX].pai = raizY;
    }

    // e se forem iguais escolho x arbitrariamente para ser o pai de y e aumento seu rank.
    else{
        subconjuntos[raizY].pai = raizX;
        subconjuntos[raizX].rank++;
    }
}

// Eis aqui o metodo principal de kruskal vou explicar a logica dele abaixo.
void encontrarArvoreGeradoraMinima(Grafo* grafo){
    // checagem para impedir que grafos dirigidos entrem no sistema.
    if(grafo->tipo != 'G'){
        printf("o grafo não pode ser dirigido!");
        return;
    }

    // como kruskal vai analisando cada aresta e sempre pega as menores, fica mais facil se orde-
    // narmos essa lista de arestas (que ja vem com o grafo)
    qsort(grafo->arestas, grafo->num_arestas, sizeof(Aresta), compararArestas);

    // criamos um array de subconjuntos para utilizar na logica de kruskal onde só podemos unir
    // arestas de subconjuntos diferentes.
    Subconjunto *subconjuntos = (Subconjunto*)malloc(grafo->num_vertices * sizeof(Subconjunto));

    // setamos todo mundo (vertices) como um subconjunto proprio com pai sendo ele mesmo e tendo
    // rank 0.
    for(int v = 0; v < grafo->num_vertices; v++){
        subconjuntos[v].pai = v;
        subconjuntos[v].rank = 0;
    }

    float pesoTotal = 0;
    int arestasIncluidas = 0;

    // vamos percorrer todas as arestas do nosso grafo checando se ela ja pertence a arvore e
    // alocando ela para um subconjunto se possivel.

    // cabe dizer aqui também que o numero de arestas incluidas não pode ser maior que V - 1
    // pela definição de arvore, por isso entra aqui no for.
    for(int i = 0; i < grafo->num_arestas && arestasIncluidas < grafo->num_vertices -1; i++){

         Aresta proximaAresta = grafo->arestas[i];

         // encontramos o pai de cada vertice da aresta atual que esta sendo checada
         int x = encontrar(subconjuntos, proximaAresta.vi - 1);
         int y = encontrar(subconjuntos, proximaAresta.vj - 1);

         // se for o mesmo pai, então pertencem ao mesmo conjunto, então a aresta não pode ser
         // unida

         // agora se forem de pais diferentes, logo são de subcnojuntos diferentes, e podemos,
         // uni-la dessa vez.
         if(x!=y){
            printf("%d -- %d (Peso: %.2f)\n", proximaAresta.vi, proximaAresta.vj, proximaAresta.peso);
            pesoTotal+=proximaAresta.peso;
            unir(subconjuntos,x,y);
            arestasIncluidas++;
         }
    }

}


//f) Cálculo de caminho mais curto:
//Implementar o algoritmo de Dijkstra para encontrar o caminho mais curto.


void dijkstra(Grafo* grafo, int origem){
    if(!grafo){
        printf("o grafo é invalido!");
        return;
    }
    if(grafo->tipo != 'D' && grafo->tipo != 'G'){
        printf("tipo de grafo invalido!");
        return;
    }

    int V = grafo->num_vertices;
    float dist[MAX];
    int visitado[MAX];

    // aqui definimos as distancias todas como um numero bem grande
    // e todos visitados como 0 que significa falso;
    for(int i = 0; i < V; i++){
        dist[i] = FLT_MAX;
        visitado[i] = 0;
    }
    // aqui definimos a distancia da origem pra ela mesmo como 0
    dist[origem - 1] = 0;

    for(int contador = 0; contador < V - 1; contador++){
        int u = -1;
        float menorDist = FLT_MAX;

        // aqui definimos u como  vertice que possui 
        // a menor distancia da origem e ainda não foi visitado
        for(int v = 0; v < V; v++){
            if(visitado[v] == 0 && dist[v] < menorDist){
                menorDist = dist[v];
                u = v;
            }
        }

        visitado[u] = 1;

        for(int i = 0; i < grafo ->num_arestas; i++){
            int vi = grafo->arestas[i].vi - 1;
            int vj = grafo->arestas[i].vj - 1;
            float peso = grafo->arestas[i].peso;
            if(peso < 0){
                printf("Arestas não podem possuir peso menor que 0");
                return;
            }

            // agora precisamos considerar os dois tipos de grafos que estamos abrangendo nesse programa
            if(grafo->tipo== 'G'){
                //Para grafos não dirigidos devemos considerar ambos os sentidos.
                if((vi == u || vj == u)){
                    int adj = (vi == u) ? vj : vi;

                    // checamos aqui se o vertice adjacente possui distancia maior do que a distancia atual + peso
                    // se tiver nós trocamos essa distancia.
                    if(visitado[adj] == 0 && dist[u] + peso < dist[adj]){
                        dist[adj] = dist[u] + peso;
                    }


                }
            }else{

                // aqui o grafo dirigido
                // como so tem um caminho, não existe duvida qual é o atual e qual é o adj
                // então é só fazer a mesma verificação que o ultimo
                if(vi == u && peso > 0){
                    if(visitado[vj] == 0 && dist[u] + peso < dist[vj]){
                        dist[vj] = dist[u] + peso;
                    }
                }
            }
        }
    }

    // Exibir as distâncias calculadas
    printf("Distâncias do vértice %d:\n", origem);
    for (int i = 0; i < V; i++) {
        if (dist[i] == FLT_MAX) {
            printf("Vértice %d: Infinito\n", i + 1);
        } else {
            printf("Vértice %d: %.2f\n", i + 1, dist[i]);
        }
    }
}

void buscaEmLargura(Grafo* grafo, int origem){
    if(!grafo) {
        printf("Grafo inválido!\n");
        return;
    }
    int V = grafo->num_vertices;
    int visitado[MAX];
    int fila[MAX];

    // inicio e fim aqui é basicamente para criarmos uma fila
    // desse modo colocamos o vertice novo achado sempre no fim
    // e removemos o antigo do inicio.
    int inicio = 0, fim = 0;

    for (int i = 0; i < V; i++) visitado[i] = 0;

    // Ajuste para índices baseados em 1
    origem = origem - 1;
    fila[fim++] = origem;
    visitado[origem] = 1;

    printf("BFS a partir do vértice %d:\n", origem + 1);

    while(inicio < fim){
        int u = fila[inicio++];
        printf("%d ", u+1);

        for(int i = 0; i < grafo->num_arestas; i++){
            int vi = grafo->arestas[i].vi - 1;
            int vj = grafo->arestas[i].vj - 1;
            if(grafo->tipo == 'G'){
                if(vi == u && !visitado[vj]){
                    fila[fim++] = vj;
                    visitado[vj] = 1;
                }
                else if(vj == u && !visitado[vi]){
                    fila[fim++] = vi;
                    visitado[vi] = 1;
                }
            }
            else if(grafo->tipo == 'D'){
                if(vi == u && !visitado[vj]){
                    fila[fim++] = vj;
                    visitado[vj] = 1;
                }
            }
        }
    }

    printf("\n");
}

int main() {
    const char* nome_arquivo = "grafo.txt"; // Nome do arquivo para salvar e ler o grafo

    // Passo 1: Criar um grafo e salvá-lo no arquivo
    printf("Criando um grafo...\n");
    //criarGrafoESalvar(nome_arquivo);

    // Passo 2: Ler o grafo do arquivo
    printf("\nLendo o grafo do arquivo '%s'...\n", nome_arquivo);
    Grafo* grafo = lerGrafoDeArquivo(nome_arquivo);

    gerarMatrizAdjacencias(grafo, "matriz_adjacencia.txt");

    // Passo 3: Exibir o grafo no terminal
    if (grafo) {
        printf("\nExibindo o grafo:\n");
        exibirGrafo(grafo);
        mostraGrau(grafo);
        encontrarArvoreGeradoraMinima(grafo);
        dijkstra(grafo, 1);
        buscaEmLargura(grafo, 1);

        // Liberar a memória alocada para o grafo
        free(grafo->arestas);
        free(grafo);
    } 
    else {
        printf("Erro ao ler o grafo do arquivo.\n");
    }

    return 0;
}
