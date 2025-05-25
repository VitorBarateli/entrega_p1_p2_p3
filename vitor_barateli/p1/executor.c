#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MEM_SIZE 256    // Define o tamanho da memória da máquina Neander (256 bytes)
#define MAGIC_HEADER {0x03, 0x4E, 0x44, 0x52}   // // Define o cabeçalho mágico esperado nos arquivos .mem

// Retorna o mnemônico correspondente ao opcode
const char* get_mnemonic(uint8_t opcode) {
    switch (opcode & 0xF0) {
        case 0x00: return "NOP";
        case 0x10: return "STA";
        case 0x20: return "LDA";
        case 0x30: return "ADD";
        case 0x40: return "OR";
        case 0x50: return "AND";
        case 0x60: return "NOT";
        case 0x80: return "JMP";
        case 0x90: return "JN";
        case 0xA0: return "JZ";
        case 0xF0: return "HLT";
        default: return "";
    }
}

// Verifica se a próxima instrução ocupa 2 bytes (opcode + operando)
int should_skip_next(const char* mnemonic) {
    // Somente NOT, NOP e HLT são de 1 byte
    return (mnemonic[0] != '\0' && strcmp(mnemonic, "NOP") != 0 && strcmp(mnemonic, "HLT") != 0 && strcmp(mnemonic, "NOT") != 0);
}

// Carrega o conteúdo do arquivo .mem na memória simulada
void load_memory(const char *filename, uint8_t *memory) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    // Lê e valida o cabeçalho do arquivo
    uint8_t header[4];
    fread(header, 1, 4, file);
    uint8_t expected_header[] = MAGIC_HEADER;
    
    if (memcmp(header, expected_header, 4) != 0) {
        fprintf(stderr, "Erro: O arquivo fornecido nao e um arquivo .mem valido.\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Lê os dados de memória (somente os bytes de instruções, pula os espaços extras)
    int index = 0;
    uint8_t byte;
    while (fread(&byte, 1, 1, file) == 1) {
        if (index % 2 == 0) {
            memory[index / 2] = byte;
        }
        index++;
    }
    
    fclose(file);
}

// Imprime o conteúdo da memória
void print_memory(uint8_t *memory) {
    printf("Reg\tValor\tMnemonico\n");
    int show_mnemonic = 1;  // Flag para controlar exibição de mnemônicos
    
    for (int i = 0; i < MEM_SIZE; i++) {
        const char *mnemonic = show_mnemonic ? get_mnemonic(memory[i]) : "";
        
        if (should_skip_next(mnemonic) && i + 1 < MEM_SIZE) {
            printf("%02X\t%02X\t%s %02X\n", i, memory[i], mnemonic, memory[i + 1]);
            show_mnemonic = 0;
        } else {
            printf("%02X\t%02X\t%s\n", i, memory[i], mnemonic);
            
            // Controla se o próximo byte deve mostrar mnemônico ou não
            if (strcmp(mnemonic, "NOP") == 0 || strcmp(mnemonic, "HLT") == 0 || strcmp(mnemonic, "NOT") == 0) {
                show_mnemonic = 1;
            } else {
                show_mnemonic = (mnemonic[0] == '\0');
            }
        }
    }
}

// Executa o código carregado na memória simulada do Neander
void execute(uint8_t *memory) {
    uint8_t AC = 0;
    int PC = 0;
    uint8_t N = 0, Z = 0;

    while (PC < MEM_SIZE) {
        uint8_t opcode = memory[PC];
        uint8_t address = memory[PC + 1];
        
        // Verifica se é instrução HLT
        if ((opcode & 0xF0) == 0xF0) {
            PC += 1;
            break;
        }
        else if ((opcode & 0xF0) == 0x60) { // Verifica se é NOT
            PC += 1;
        } 
        else {  // Instruções de 2 bytes
            PC += 2;
        }
        
        // Executa a instrução com base no opcode
        switch (opcode & 0xF0) {
            case 0x10: memory[address] = AC; break;     // STA: armazena AC na memória
            case 0x20: AC = memory[address]; break;     // LDA: carrega valor da memória para AC
            case 0x30: AC += memory[address]; break;    // ADD: soma valor da memória ao AC
            case 0x40: AC |= memory[address]; break;    // OR: AC |= memória
            case 0x50: AC &= memory[address]; break;    // AND: AC &= memória
            case 0x60: AC = ~AC; break;                 // NOT: inverte bits do AC
            case 0x80: PC = address; break;             // JMP: desvia
            case 0x90: if (N) PC = address; break;      // JN: desvia se negativo
            case 0xA0: if (Z) PC = address; break;      // JZ: desvia se zero
        }
        
        // Atualiza flags N e Z
        N = (AC & 0x80) ? 1 : 0;
        Z = (AC == 0) ? 1 : 0;
    }

    printf("\n---------------------------------------------");
    printf("\n\nMemoria apos a execucao:\n");
    print_memory(memory);
    printf("\nFinal:\nAC: %02X\nPC: %02X\nN: %d\nZ: %d\n", AC, PC, N, Z);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.mem>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    uint8_t memory[MEM_SIZE] = {0};
    load_memory(argv[1], memory);

    printf("Memoria antes da execucao:\n");
    print_memory(memory);
    execute(memory);

    return EXIT_SUCCESS;
}