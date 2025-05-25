#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MEM_SIZE 256
#define HEADER_SIZE 4
#define FILE_HEADER {0x03, 0x4E, 0x44, 0x52}

uint8_t memory[MEM_SIZE * 2] = {0}; // Memória com dobro de tamanho, pois o binario do neander possui separadores 0x00 apos cada byte
int code_size = 0;                  // Tamanho do .CODE para saber em qual endereço começar a salvar as variaveis

// Estrutura para armazenar variáveis
typedef struct {
    char name[10];
    int address;
    int value;
    int initialized;
} Variable;

Variable variables[MEM_SIZE];
int variable_count = 0;

// Função para fornecer o opcode do mnemonico
uint8_t get_opcode(char *mnemonic, int *has_operand) {
    struct Mnemonic {
        char *mnemonic;
        uint8_t opcode;
        int has_operand;    // Verificar se o mnemonico ocupa 2 endereços ou não, como no caso do NOP, NOT e HLT
    };

    struct Mnemonic mnemonics[] = {
        {"NOP", 0x00, 0}, {"STA", 0x10, 1}, {"LDA", 0x20, 1}, {"ADD", 0x30, 1},
        {"OR", 0x40, 1}, {"AND", 0x50, 1}, {"NOT", 0x60, 0}, {"JMP", 0x80, 1},
        {"JN", 0x90, 1}, {"JZ", 0xA0, 1}, {"HLT", 0xF0, 0}, {NULL, 0, 0}
    };

    for (int i = 0; mnemonics[i].mnemonic != NULL; i++) {
        if (strcmp(mnemonic, mnemonics[i].mnemonic) == 0) {
            *has_operand = mnemonics[i].has_operand;
            return mnemonics[i].opcode;
        }
    }
    return 0xFF;
}

// Função para fornecer o endereço da variavel
int get_variable_address(char *name) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].address;
        }
    }
    return -1;
}

// Função para adicionar a variavel na lista de variaveis
void add_variable(char *name, int value, int initialized) {
    strcpy(variables[variable_count].name, name);
    variables[variable_count].address = code_size + variable_count;
    variables[variable_count].value = value;
    variables[variable_count].initialized = initialized;
    variable_count++;
}

// Função para processar o arquivo
void parse_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir arquivo");
        exit(EXIT_FAILURE);
    }
    
    char line[256];
    int parsing_code = 0;   // Verificar se esta na seção .DATA ou .CODE
    int addr = 0;
    
    // Primeira passagem: calcular tamanho da seção .CODE
    // para saber em qual endereço o valor das variaveis devem começar a ser salvos
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == ';' || line[0] == '\n') continue;    // Se a linha for comentario, entao ignora
        
        if (strstr(line, ".DATA")) {
            parsing_code = 0;   // Se estiver na seção .DATA, então valor é 0
            continue;
        }
        if (strstr(line, ".CODE")) {
            parsing_code = 1;   // // Se estiver na seção .CODE, então valor é 1
            continue;
        }
        
        // Contar quantidade de endereços usados pela seção .CODE
        if (parsing_code) { 
            char mnemonic[10], operand_str[10];
            int has_operand;
            if (sscanf(line, "%s %s", mnemonic, operand_str) == 2 || sscanf(line, "%s", mnemonic) == 1) {
                uint8_t opcode = get_opcode(mnemonic, &has_operand);
                if (opcode != 0xFF) {
                    addr += 2;  // Soma 2, pois cada byte é seguido por um separador 00 no arquivo do neander
                    if (has_operand) addr += 2;
                }
            }
        }
    }
    
    code_size = addr / 2;   // Divide o endereço por 2, pois cada byte é seguido por um separador 00 no arquivo binario
    rewind(file);
    parsing_code = 0;
    addr = 0;
    
    // Segunda passagem: armazenar código e variáveis corretamente
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == ';' || line[0] == '\n') continue;
        
        if (strstr(line, ".DATA")) {
            parsing_code = 0;
            continue;
        }
        if (strstr(line, ".CODE")) {
            parsing_code = 1;
            continue;
        }
        
        if (!parsing_code) { // .DATA
            char var_name[10], value_str[10];
            int value, initialized = 1;
            if (sscanf(line, "%s DB %s", var_name, value_str) == 2) {   // Verifica se a linha esta no formato: Nome DB Valor
                if (strcmp(value_str, "?") == 0) {  // Verifica se a variavel tem um valor ou não
                    value = 0;
                    initialized = 0;
                } else {
                    value = atoi(value_str);
                }
                add_variable(var_name, value, initialized);
            }
        } else { // .CODE
            char mnemonic[10], operand_str[10];
            int operand;
            int has_operand;
            
            if (sscanf(line, "%s %s", mnemonic, operand_str) == 2) {    // Verifica se a linha esta no formato: Mnemonico Operando
                uint8_t opcode = get_opcode(mnemonic, &has_operand);    // Converte o mnemonico em opcode
                if (opcode != 0xFF) {
                    memory[addr++] = opcode;    // Adiciona o opcode na memoria
                    memory[addr++] = 0x00;      // Adiciona o separador 00 que o arquivo do neander possui após cada byte
                    
                    if (has_operand) {
                        operand = get_variable_address(operand_str);
                        if (operand == -1) operand = atoi(operand_str);
                        memory[addr++] = (uint8_t)operand;  // Adiciona o operando na memoria
                        memory[addr++] = 0x00;              // Adiciona o separador 00 que o arquivo do neander possui após cada byte
                    }
                }
            } else if (sscanf(line, "%s", mnemonic) == 1) {
                uint8_t opcode = get_opcode(mnemonic, &has_operand);
                if (opcode != 0xFF) {
                    memory[addr++] = opcode;
                    memory[addr++] = 0x00;
                }
            }
        }
    }
    fclose(file);
    
    // Armazenar valores das variáveis na memória nos endereços corretos
    for (int i = 0; i < variable_count; i++) {
        memory[variables[i].address * 2] = variables[i].value;  // Endereço é multiplicado por 2, pois cada byte é seguido por um separador 00
    }
}

// Funçao para escrever o arquivo binario
void write_binary(const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Erro ao criar binário");
        exit(EXIT_FAILURE);
    }
    
    uint8_t header[] = FILE_HEADER;
    fwrite(header, 1, HEADER_SIZE, file);
    fwrite(memory, 1, MEM_SIZE * 2, file);
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <arquivo.txt> <saida.mem>\n", argv[0]);
        return 1;
    }

    parse_file(argv[1]);
    write_binary(argv[2]);

    printf("Arquivo %s gerado com sucesso!\n", argv[2]);
    return 0;
}