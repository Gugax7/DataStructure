#include <stdio.h>

void demonstrar_diferenca() {
    // Criar arquivo pequeno
    FILE *f = fopen("demo.txt", "w");
    fprintf(f, "ABC");
    fclose(f);
    
    printf("=== TESTE COM rewind() ===\n");
    f = fopen("demo.txt", "r");
    
    // Forçar EOF
    while (fgetc(f) != EOF);
    printf("Apos ler ate EOF:\n");
    printf("  posicao: %ld\n", ftell(f));
    printf("  feof(): %d\n", feof(f));
    printf("  ferror(): %d\n", ferror(f));
    
    // Usar rewind()
    rewind(f);
    printf("Apos rewind():\n");
    printf("  posicao: %ld\n", ftell(f));
    printf("  feof(): %d\n", feof(f));      // ← Limpo!
    printf("  ferror(): %d\n", ferror(f));  // ← Limpo!
    
    fclose(f);
    
    printf("\n=== TESTE COM fseek() ===\n");
    f = fopen("demo.txt", "r");
    
    // Forçar EOF novamente
    while (fgetc(f) != EOF);
    printf("Após ler ate EOF:\n");
    printf("  posicao: %ld\n", ftell(f));
    printf("  feof(): %d\n", feof(f));
    printf("  ferror(): %d\n", ferror(f));
    
    // Usar fseek()
    fseek(f, 0, SEEK_SET);
    printf("Após fseek(f, 0, SEEK_SET):\n");
    printf("  posicao: %ld\n", ftell(f));
    printf("  feof(): %d\n", feof(f));      // ← Ainda marcado!
    printf("  ferror(): %d\n", ferror(f));  // ← Ainda marcado!
    
    fclose(f);
}

int main() {
  demonstrar_diferenca();
}

// Therefore here we can se they are the same... yeah basically my friend is wrong