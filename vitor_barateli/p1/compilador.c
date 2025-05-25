#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

// Definições de tamanho máximo para tokens e linhas
#define MAX_TOKEN_LEN 100
#define MAX_LINE_LEN 256

// Enumeração para os tipos de tokens reconhecidos
typedef enum
{
    TOKEN_PROGRAMA,
    TOKEN_LABEL,
    TOKEN_DOIS_PONTOS,
    TOKEN_INICIO,
    TOKEN_VARIAVEL,
    TOKEN_IGUAL,
    TOKEN_NUMERO,
    TOKEN_OPERADOR,
    TOKEN_RES,
    TOKEN_FIM,
    TOKEN_PARENTESE_ESQ,
    TOKEN_PARENTESE_DIR,
    TOKEN_NOVA_LINHA,
    TOKEN_EOF,
    TOKEN_ASPAS,
    TOKEN_ERRO
} TokenType;

// Estrutura para representar um token com o seu tipo e valor
typedef struct
{
    TokenType type;
    char lexeme[MAX_TOKEN_LEN];
} Token;

FILE *file;
FILE *output_file;
Token current_token;

char current_var[MAX_TOKEN_LEN];  // Variável atual da expressão matemática
int lines = 0;                    // Contador de linhas para instruções do neander
bool first = true;                // Flag de controle para primeira expressão matemática

// Função para exibir mensagens de erro e encerrar o programa
void error(const char *msg)
{
    fprintf(stderr, "Erro: %s\n", msg);
    exit(EXIT_FAILURE);
}

// Função para fazer análise léxica (tokenização)
Token lexer()
{
    char current;
    Token token;

    while ((current = fgetc(file)) != EOF)
    {
        // Ignora espaços em branco
        if (isspace(current))
        {
            if (current == '\n')
            {
                token.type = TOKEN_NOVA_LINHA;
                strcpy(token.lexeme, "\n");
                return token;
            }
            continue;
        }

        // Reconhece aspas
        if (current == '"')
        {
            token.type = TOKEN_ASPAS;
            strcpy(token.lexeme, "\"");
            return token;
        }

        // Reconhece palavras (identificadores e palavras-chave)
        if (isalpha(current))
        {
            int i = 0;
            token.lexeme[i++] = current;
            while (isalnum(current = fgetc(file)))
            {
                if (i < MAX_TOKEN_LEN - 1)
                    token.lexeme[i++] = current;
            }
            ungetc(current, file);  // Devolve o último caractere não alfanumérico
            token.lexeme[i] = '\0';

            // Verifica se é uma palavra-chave ou um label
            if (strcmp(token.lexeme, "PROGRAMA") == 0)
                token.type = TOKEN_PROGRAMA;
            else if (strcmp(token.lexeme, "INICIO") == 0)
                token.type = TOKEN_INICIO;
            else if (strcmp(token.lexeme, "RES") == 0)
                token.type = TOKEN_RES;
            else if (strcmp(token.lexeme, "FIM") == 0)
                token.type = TOKEN_FIM;
            else
                token.type = TOKEN_LABEL;
            return token;
        }

        // Reconhece números
        if (isdigit(current))
        {
            int i = 0;
            token.lexeme[i++] = current;
            while (isdigit(current = fgetc(file)))
            {
                if (i < MAX_TOKEN_LEN - 1)
                    token.lexeme[i++] = current;
            }
            ungetc(current, file);
            token.lexeme[i] = '\0';
            token.type = TOKEN_NUMERO;
            return token;
        }

        // Reconhece símbolos especiais e operadores
        switch (current)
        {
        case '=':
            token.type = TOKEN_IGUAL;
            strcpy(token.lexeme, "=");
            return token;
        case ':':
            token.type = TOKEN_DOIS_PONTOS;
            strcpy(token.lexeme, ":");
            return token;
        case '+':
        case '-':
        case '*':
        case '/':
            token.type = TOKEN_OPERADOR;
            token.lexeme[0] = current;
            token.lexeme[1] = '\0';
            return token;
        case '(':
            token.type = TOKEN_PARENTESE_ESQ;
            strcpy(token.lexeme, "(");
            return token;
        case ')':
            token.type = TOKEN_PARENTESE_DIR;
            strcpy(token.lexeme, ")");
            return token;
        default:
            fprintf(stderr, "Erro: Token desconhecido '%c'\n", current);
            error("Token desconhecido.");
        }
    }
    token.type = TOKEN_EOF;
    return token;
}

// Avança para o próximo token
void advance()
{
    current_token = lexer();
}

// Verifica se o token atual é o esperado, senão emite erro
void expect(TokenType type)
{
    if (current_token.type == type)
    {
        advance();
    }
    else
    {
        error("Erro de sintaxe.");
    }
}

void expr();

// Analisa um fator: uma variável ou uma expressão entre parênteses
void fator()
{
    if (current_token.type == TOKEN_LABEL)
    {
        strcpy(current_var, current_token.lexeme);

        // Converte a variavel para maiúsculo para manter o mesmo padrão
        for (int i = 0; current_var[i]; i++)
        {
            current_var[i] = toupper(current_var[i]);
        }

        advance();
    }
    else if (current_token.type == TOKEN_PARENTESE_ESQ)
    {
        advance();
        expr();
        expect(TOKEN_PARENTESE_DIR);
    }
    else
    {
        error("Fator inválido.");
    }
}

// Analisa um termo: multiplicações e divisões
void termo()
{
    fator();

    while (current_token.type == TOKEN_OPERADOR && (current_token.lexeme[0] == '*' || current_token.lexeme[0] == '/'))
    {
        if (strcmp(current_token.lexeme, "*") == 0) {
            // Já que a multiplicação usa JZ e JMP, é necessario saber em qual linha de instrução começa e termina os comandos de multiplicação
            int start = 8;

            // Dependendo se a multiplicação é ou não a primeira operação matemática da expressão, é utilizado uma contagem diferente para cada cenário
            if (first == true) {
                lines += 32;
                first = false;
            } else {
                start += lines + 1;
                lines += 33;
            }
            
            int fim = lines + 1;
            char first_var[MAX_TOKEN_LEN];
            strcpy(first_var, current_var); // Salva a primeira variavel no first_var e avança para a proxima variavel
            advance();
            fator();

            // Como o resultado final ficará sempre no mesmo endereço de memória, 
            // logo é necessário copiar o valor dele para outro endereço, realizar as operações e depois salvar no endereço original
            fprintf(output_file, "LDA %s\n", first_var);
            fprintf(output_file, "STA TEMP\n");
            fprintf(output_file, "LDA AUX2\n");
            fprintf(output_file, "STA X\n");
            
            // Instruções para realizar a multiplicação
            fprintf(output_file, "LDA %s\n", current_var);
            fprintf(output_file, "JZ %d\n", fim);
            fprintf(output_file, "LDA X\n");
            fprintf(output_file, "ADD TEMP\n");
            fprintf(output_file, "STA X\n");
            fprintf(output_file, "LDA AUX2\n");
            fprintf(output_file, "ADD AUX1\n");
            fprintf(output_file, "STA AUX2\n");
            fprintf(output_file, "NOT\n");
            fprintf(output_file, "ADD AUX1\n");
            fprintf(output_file, "ADD %s\n", current_var);
            fprintf(output_file, "JZ %d\n", fim);
            fprintf(output_file, "JMP %d\n", start);
            strcpy(current_var, "X");
        }
        else if (strcmp(current_token.lexeme, "/") == 0) {
            // Já que a divisao usa JZ e JMP, é necessario saber em qual linha de instrução começa e termina os comandos de divisão
            int start = 15;

            // Dependendo se a divisão é ou não a primeira operação matemática da expressão, é utilizado uma contagem diferente para cada cenário
            if (first == true) {
                lines += 32;
                first = false;
            } else {

                start += lines + 1;
                lines += 33;
            }
            
            int fim = lines + 1;
            char first_var[MAX_TOKEN_LEN];
            strcpy(first_var, current_var);
            advance();
            fator();

            // Copiar o resultado final para outro endereço
            fprintf(output_file, "LDA %s\n", first_var);
            fprintf(output_file, "STA TEMP1\n");
            fprintf(output_file, "LDA AUX4\n");
            fprintf(output_file, "STA X\n");
            
            // Instruções para realizar a divisão
            fprintf(output_file, "LDA %s\n", current_var);
            fprintf(output_file, "NOT\n");
            fprintf(output_file, "ADD AUX3\n");
            fprintf(output_file, "STA %s\n", current_var);
            fprintf(output_file, "LDA TEMP1\n");
            fprintf(output_file, "ADD %s\n", current_var);
            fprintf(output_file, "STA TEMP1\n");
            fprintf(output_file, "LDA X\n");
            fprintf(output_file, "ADD AUX3\n");
            fprintf(output_file, "STA X\n");
            fprintf(output_file, "LDA TEMP1\n");
            fprintf(output_file, "JZ %d\n", fim);
            fprintf(output_file, "JMP %d\n", start);
            strcpy(current_var, "X");
        }
    }
}

// Analisa uma expressão: soma e subtração
void expr()
{
    termo();

    while (current_token.type == TOKEN_OPERADOR && (current_token.lexeme[0] == '+' || current_token.lexeme[0] == '-'))
    {
        if (strcmp(current_token.lexeme, "+") == 0)
        {
            char first_var[MAX_TOKEN_LEN];
            strcpy(first_var, current_var);
            advance();
            termo();

            // Dependendo se a soma é ou não a primeira operação matemática da expressão, é utilizado uma contagem diferente para cada cenário
            // É necessário aumentar o contador de linhas para multiplicação ou divisão
            if (first == true) {
                lines += 5;
                first = false;
            } else {
                lines += 6;
            }

            // Instruções para realizar a soma
            fprintf(output_file, "LDA %s\n", current_var);
            fprintf(output_file, "ADD %s\n", first_var);
            fprintf(output_file, "STA X\n");
            strcpy(current_var, "X");
        }
        else if (strcmp(current_token.lexeme, "-") == 0)
        {
            char first_var[MAX_TOKEN_LEN];
            strcpy(first_var, current_var);
            advance();
            termo();

            // Dependendo se a subtração é ou não a primeira operação matemática da expressão, é utilizado uma contagem diferente para cada cenário
            // É necessário aumentar o contador de linhas para multiplicação ou divisão
            if (first == true) {
                lines += 8;
                first = false;
            } else {
                lines += 9;
            }

            // Instruções para realizar a subtração
            fprintf(output_file, "LDA %s\n", current_var);
            fprintf(output_file, "NOT\n");
            fprintf(output_file, "ADD AUX\n");
            fprintf(output_file, "ADD %s\n", first_var);
            fprintf(output_file, "STA X\n");
            strcpy(current_var, "X");
        }
    }
}

// Parser da seção de variáveis e código principal
void parse_conteudo()
{
    fprintf(output_file, ".DATA\n");
    bool mult = false;
    bool sub = false;
    bool div = false;

    // Processa declarações de variáveis
    while (current_token.type == TOKEN_LABEL)
    {
        char var_name[MAX_TOKEN_LEN];
        strcpy(var_name, current_token.lexeme);

        // Converte a variavel para maiúsculo para manter o mesmo padrão
        for (int i = 0; var_name[i]; i++)
        {
            var_name[i] = toupper(var_name[i]);
        }

        advance();
        expect(TOKEN_IGUAL);

        // Verifica se a variavel está no formato NOME DB VALOR ou NOME DB EXPRESSAO
        // É necessario ter uma variavel com expressao matematica, pois é nesse momento que parte do .DATA é gerado
        if (current_token.type == TOKEN_NUMERO)
        {
            fprintf(output_file, "%s DB %s\n", var_name, current_token.lexeme);
            advance();
        }
        else
        {
            // Verifica presença de operadores nas expressões
            while (current_token.type != TOKEN_NOVA_LINHA)
            {
                if (strcmp(current_token.lexeme, "-") == 0)
                {
                    sub = true;
                }
                else if (strcmp(current_token.lexeme, "*") == 0)
                {
                    mult = true;
                }
                else if (strcmp(current_token.lexeme, "/") == 0)
                {
                    div = true;
                }

                advance();
            }

            // Declara auxiliares necessárias para cada tipo de operação
            if (mult == true)
            {
                fprintf(output_file, "AUX1 DB 1\n");
                fprintf(output_file, "AUX2 DB 0\n");
                fprintf(output_file, "TEMP DB 0\n");
            }
            if (sub == true)
            {
                fprintf(output_file, "AUX DB 1\n");
            }
            if ( div == true)
            {
                fprintf(output_file, "AUX3 DB 1\n");
                fprintf(output_file, "AUX4 DB 0\n");
                fprintf(output_file, "TEMP1 DB 0\n");
            }

            fprintf(output_file, "%s DB ?\n", var_name);
        }

        expect(TOKEN_NOVA_LINHA);
    }

    expect(TOKEN_RES);
    expect(TOKEN_IGUAL);

    fprintf(output_file, "\n.CODE\n.ORG 0\n");

    // É necessario que a expressao matematica no RES seja igual a da variavel, 
    // pois o passo anterior preparou o .DATA de acordo com a expressao qu está na variavel
    expr();

    fprintf(output_file, "HLT\n");

    expect(TOKEN_NOVA_LINHA);
}

// Parser do bloco INICIO...FIM
void parse_inicio()
{
    expect(TOKEN_INICIO);
    expect(TOKEN_NOVA_LINHA);
    parse_conteudo();
    expect(TOKEN_FIM);
}

// Parser do programa principal
void parse_programa()
{
    expect(TOKEN_PROGRAMA);
    expect(TOKEN_ASPAS);
    expect(TOKEN_LABEL);
    expect(TOKEN_ASPAS);
    expect(TOKEN_DOIS_PONTOS);
    expect(TOKEN_NOVA_LINHA);
    parse_inicio();
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Uso: %s <arquivo_entrada.txt> <arquivo_saida.txt>\n", argv[0]);
        return EXIT_FAILURE;
    }

    file = fopen(argv[1], "r");
    if (!file)
    {
        perror("Erro ao abrir arquivo de entrada");
        return EXIT_FAILURE;
    }

    output_file = fopen(argv[2], "w");
    if (!output_file)
    {
        perror("Erro ao criar arquivo de saida");
        return EXIT_FAILURE;
    }

    advance();
    parse_programa();
    printf("Compilacao bem-sucedida!\n");

    fclose(file);
    fclose(output_file);
    return EXIT_SUCCESS;
}