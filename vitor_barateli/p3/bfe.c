#include <stdio.h>
#include <stdlib.h>

// Define o tamanho da memória da linguagem Brainfuck
#define MEM_SIZE 30000

// Função que interpreta o código Brainfuck passado como string
void interpretar(const char *codigo) {
    unsigned char memoria[MEM_SIZE] = {0};
    unsigned char *ptr = memoria;  // Ponteiro para a célula atual
    const char *pc = codigo;       // Ponteiro para o código-fonte

    while (*pc) {
        switch (*pc) {
            // Move o ponteiro para a direita
            case '>': 
                ptr++; 
                if (ptr >= memoria + MEM_SIZE) 
                    ptr = memoria;  // Volta ao início se passar do limite
                break;

            // Move o ponteiro para a esquerda
            case '<': 
                if (ptr <= memoria) 
                    ptr = memoria + MEM_SIZE - 1;  // Vai para o final se for menor que o início
                else 
                    ptr--; 
                break;

            // Incrementa o valor na célula atual
            case '+': 
                (*ptr)++; 
                break;

            // Decrementa o valor na célula atual
            case '-': 
                (*ptr)--; 
                break;

            // Imprime o caractere correspondente ao valor da célula atual
            case '.': 
                putchar(*ptr); 
                break;

            // Lê um caractere da entrada padrão e armazena na célula atual
            case ',': 
                *ptr = getchar(); 
                break;

            // Início de um loop: se o valor da célula for 0, pula até o ']' correspondente
            case '[':
                if (*ptr == 0) {
                    int aberto = 1;
                    while (aberto > 0) {
                        pc++;
                        if (*pc == '[') aberto++;
                        else if (*pc == ']') aberto--;
                        if (*pc == '\0') {
                            fprintf(stderr, "Erro: laço '[' sem fechamento\n");
                            exit(1);
                        }
                    }
                }
                break;

            // Fim de um loop: se o valor da célula for diferente de 0, volta até o '[' correspondente
            case ']':
                if (*ptr != 0) {
                    int fechado = 1;
                    while (fechado > 0) {
                        pc--;
                        if (*pc == ']') fechado++;
                        else if (*pc == '[') fechado--;
                        if (pc < codigo) {
                            fprintf(stderr, "Erro: laço ']' sem abertura\n");
                            exit(1);
                        }
                    }
                }
                break;

            default: 
                break;
        }
        pc++;
    }
    
    putchar('\n');
}

// Função para ler o conteúdo de um arquivo para uma string
char *ler_arquivo(const char *nome_arquivo) {
    FILE *f = fopen(nome_arquivo, "r");
    if (!f) {
        perror("Erro ao abrir arquivo");
        exit(1);
    }

    // Move o ponteiro para o final e obtém o tamanho do arquivo
    fseek(f, 0, SEEK_END);
    long tamanho = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *codigo = malloc(tamanho + 1);
    if (!codigo) {
        fprintf(stderr, "Erro de alocação\n");
        exit(1);
    }

    fread(codigo, 1, tamanho, f);
    codigo[tamanho] = '\0';  // Adiciona terminador de string

    fclose(f);
    return codigo;
}

int main() {
    char *codigo = malloc(100000);
    if (!codigo) {
        fprintf(stderr, "Erro de alocação\n");
        return 1;
    }

    // Lê até 99.999 bytes da entrada padrão (stdin)
    size_t len = fread(codigo, 1, 99999, stdin);
    codigo[len] = '\0'; // Garante que a string esteja corretamente terminada

    interpretar(codigo);

    free(codigo);

    return 0;
}
