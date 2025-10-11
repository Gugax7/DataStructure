#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_FILE "images.dat"
#define INDEX_FILE "index.idx"

#define MAX_NAME_LEN 256

typedef struct {
    char name[MAX_NAME_LEN];
    long offset;
    int width;
    int height;
    int max_val;
    char magic[3];
} IndexRecord;

void negativePixels(unsigned char* pixels, int width, int height, int max_val) {
    size_t data_size = width * height;
    for (size_t i = 0; i < data_size; i++) {
        pixels[i] = max_val - pixels[i];
    }
}

void importImage(char* pgm_path) {
    FILE *pgm_fp = fopen(pgm_path, "rb");
    if (!pgm_fp) {
        perror("Erro ao abrir o arquivo PGM de entrada");
        return;
    }
    printf("Importando imagem: '%s'\n", pgm_path);

    char magic[3];
    int width, height, max_val;
    fscanf(pgm_fp, "%2s\n%d %d\n%d", magic, &width, &height, &max_val);

    if (strcmp(magic, "P5") != 0 && strcmp(magic, "P2") != 0) {
        fprintf(stderr, "Erro: O arquivo não é um PGM suportado (P2 ou P5).\n");
        fclose(pgm_fp);
        return;
    }

    fgetc(pgm_fp); 

    printf("Formato: %s, Largura: %d, Altura: %d, Max_val: %d\n", magic, width, height, max_val);

    size_t data_size = width * height;
    unsigned char* pixels = (unsigned char*)malloc(data_size);
    if (!pixels) {
        fprintf(stderr, "Erro de alocação de memória\n");
        fclose(pgm_fp);
        return;
    }

    if (strcmp(magic, "P5") == 0) {
        fread(pixels, 1, data_size, pgm_fp);
    } else {
        unsigned int pixel_val;
        for (size_t i = 0; i < data_size; i++) {
            if (fscanf(pgm_fp, "%u", &pixel_val) == 1) {
                pixels[i] = (unsigned char)pixel_val;
            }
        }
    }

    fclose(pgm_fp);

    FILE* data_fp = fopen(DATA_FILE, "ab");   
    if (!data_fp) {
        fprintf(stderr, "Erro: não foi possível abrir o arquivo de imagens.\n");
        free(pixels);
        return;
    }

    long offset = ftell(data_fp);
    fwrite(pixels, 1, data_size, data_fp);
    fclose(data_fp);
    free(pixels);

    IndexRecord record;
    strncpy(record.name, pgm_path, MAX_NAME_LEN-1);
    record.name[MAX_NAME_LEN-1] = '\0';
    record.offset = offset;
    record.height = height;
    record.width = width;
    record.max_val = max_val;
    strncpy(record.magic, magic, 3);

    FILE *index_fp = fopen(INDEX_FILE, "ab");
    if (!index_fp) {
        perror("Erro ao abrir o arquivo de índice");
        return;
    }

    fwrite(&record, sizeof(IndexRecord), 1, index_fp);
    fclose(index_fp);

    printf("Imagem '%s' importada com sucesso.\n", pgm_path);
}

void exportImage(char* image_name, char* output_path, int negative) {
    FILE* index_fp = fopen(INDEX_FILE, "rb");
    if (!index_fp) {
        perror("Erro: não foi possível abrir o arquivo de índice.");
        return;
    }

    IndexRecord record;
    int found = 0;

    while (fread(&record, sizeof(IndexRecord), 1, index_fp) == 1) {
        if (strcmp(record.name, image_name) == 0) {
            found = 1;
            break;
        }
    }
    fclose(index_fp);

    if (!found) {
        printf("A imagem '%s' não foi encontrada no banco.\n", image_name);
        return;
    }

    FILE* data_fp = fopen(DATA_FILE, "rb");
    if (!data_fp) {
        perror("Erro: não foi possível abrir o arquivo de dados.");
        return;
    }

    fseek(data_fp, record.offset, SEEK_SET);

    size_t data_size = record.width * record.height;
    unsigned char* pixels = (unsigned char*)malloc(data_size);
    if (!pixels) {
        fprintf(stderr, "Não foi possível alocar memória.\n");
        fclose(data_fp);
        return;
    }

    fread(pixels, 1, data_size, data_fp);
    fclose(data_fp);

    if (negative) {
        negativePixels(pixels, record.width, record.height, record.max_val);
    }

    FILE *output_fp = fopen(output_path, "wb");
    if (!output_fp) {
        perror("Erro: não foi possível criar o arquivo de saída.");
        free(pixels);
        return;        
    }
    
    fprintf(output_fp, "%s\n%d %d\n%d\n", record.magic, record.width, record.height, record.max_val);

    if (strcmp(record.magic, "P5") == 0) {
        fwrite(pixels, 1, data_size, output_fp);
    } else {
        for (size_t i = 0; i < data_size; i++) {
            fprintf(output_fp, "%d ", pixels[i]);
            if ((i + 1) % 15 == 0) {
                fprintf(output_fp, "\n");
            }
        }
    }

    fclose(output_fp);
    free(pixels);

    printf("Imagem '%s' exportada com sucesso para '%s'.\n", image_name, output_path);
}

void listImages() {
    FILE* index_fp = fopen(INDEX_FILE, "rb");
    if (!index_fp) {
        printf("Nenhuma imagem importada ainda ou arquivo de índice não encontrado.\n");
        return;
    }

    IndexRecord record;
    printf("Imagens armazenadas:\n");
    while (fread(&record, sizeof(IndexRecord), 1, index_fp) == 1) {
        printf("- %s (%s)\n", record.name, record.magic);
    }
    fclose(index_fp);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso:\n");
        fprintf(stderr, "  Para importar: %s import <caminho_imagem.pgm>\n", argv[0]);
        fprintf(stderr, "  Para exportar:  %s export <nome_imagem_no_banco> <arquivo_de_saida.pgm> [negative]\n", argv[0]);
        fprintf(stderr, "  Para listar: %s list\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "import") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Uso incorreto para 'import'.\n");
            return 1;
        }
        importImage(argv[2]);
    } else if (strcmp(argv[1], "export") == 0) {
        if (argc < 4 || argc > 5) {
            fprintf(stderr, "Uso incorreto para 'export'.\n");
            return 1;
        }

        int negative = 0;
        if (argc == 5 && strcmp(argv[4], "negative") == 0) {
            negative = 1;
        }

        exportImage(argv[2], argv[3], negative);
    } else if (strcmp(argv[1], "list") == 0) {
        listImages();
    } else {
        fprintf(stderr, "Comando desconhecido: '%s'.\n", argv[1]);
    }

    return 0;
}