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
    TOKEN_INT,
    TOKEN_BOOL,
    TOKEN_STRING,
    TOKEN_VOID,
    TOKEN_CHAR,
    TOKEN_FLOAT,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_RETURN,
    TOKEN_MAIN,
    TOKEN_IDENTIFICADOR,
    TOKEN_NUMERO,
    TOKEN_FLOAT_LITERAL,
    TOKEN_BOOLEANO,
    TOKEN_STRING_LITERAL,
    TOKEN_CHAR_LITERAL,

    TOKEN_MAIS,
    TOKEN_MENOS,
    TOKEN_MULT,
    TOKEN_DIV,
    TOKEN_IGUAL,

    TOKEN_PARENTESE_ESQ,
    TOKEN_PARENTESE_DIR,
    TOKEN_CHAVE_ESQ,
    TOKEN_CHAVE_DIR,
    TOKEN_PONTO_VIRGULA,
    TOKEN_VIRGULA,

    TOKEN_NOVA_LINHA,
    TOKEN_EOF,
    TOKEN_ERRO
} TokenType;

// Estrutura para representar um token com o seu tipo e valor
typedef struct
{
    TokenType type;
    char lexeme[MAX_TOKEN_LEN];
} Token;

FILE *file;

// Função para exibir mensagens de erro e encerrar o programa
void error(const char *msg)
{
    fprintf(stderr, "Erro: %s\n", msg);
    exit(EXIT_FAILURE);
}

// Verifica se um caractere pode iniciar um identificador (letra ou '_')
bool is_ident_start(char c)
{
    return isalpha(c) || c == '_';
}

// Verifica se um caractere pode estar no corpo de um identificador (letra, número ou '_')
bool is_ident_char(char c)
{
    return isalnum(c) || c == '_';
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

        // Reconhecimento de identificadores ou palavras-chave
        if (is_ident_start(current))
        {
            int i = 0;
            token.lexeme[i++] = current;
            while (is_ident_char(current = fgetc(file)))
            {
                if (i < MAX_TOKEN_LEN - 1)
                    token.lexeme[i++] = current;
            }
            ungetc(current, file);  // Retorna o último caractere que não pertence ao token
            token.lexeme[i] = '\0';

            // Verifica se é uma palavra-chave ou identificador
            if (strcmp(token.lexeme, "int") == 0)
                token.type = TOKEN_INT;
            else if (strcmp(token.lexeme, "bool") == 0)
                token.type = TOKEN_BOOL;
            else if (strcmp(token.lexeme, "string") == 0)
                token.type = TOKEN_STRING;
            else if (strcmp(token.lexeme, "void") == 0)
                token.type = TOKEN_VOID;
            else if (strcmp(token.lexeme, "char") == 0)
                token.type = TOKEN_CHAR;
            else if (strcmp(token.lexeme, "float") == 0)
                token.type = TOKEN_FLOAT;
            else if (strcmp(token.lexeme, "if") == 0)
                token.type = TOKEN_IF;
            else if (strcmp(token.lexeme, "else") == 0)
                token.type = TOKEN_ELSE;
            else if (strcmp(token.lexeme, "while") == 0)
                token.type = TOKEN_WHILE;
            else if (strcmp(token.lexeme, "return") == 0)
                token.type = TOKEN_RETURN;
            else if (strcmp(token.lexeme, "main") == 0)
                token.type = TOKEN_MAIN;
            else if (strcmp(token.lexeme, "true") == 0 || strcmp(token.lexeme, "false") == 0)
                token.type = TOKEN_BOOLEANO;
            else
                token.type = TOKEN_IDENTIFICADOR;

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

            if (current == '.')
            {
                token.lexeme[i++] = current;
                current = fgetc(file);
                if (!isdigit(current))
                    error("Número float malformado");

                do {
                    if (i < MAX_TOKEN_LEN - 1)
                        token.lexeme[i++] = current;
                    current = fgetc(file);
                } while (isdigit(current));

                ungetc(current, file);
                token.lexeme[i] = '\0';
                token.type = TOKEN_FLOAT_LITERAL;
                return token;
            }

            ungetc(current, file);
            token.lexeme[i] = '\0';
            token.type = TOKEN_NUMERO;
            return token;
        }

        // Reconhecimento de literais string entre aspas
        if (current == '"')
        {
            int i = 0;
            while ((current = fgetc(file)) != '"' && current != EOF)
            {
                if (i < MAX_TOKEN_LEN - 1)
                    token.lexeme[i++] = current;
            }
            if (current != '"')
                error("String sem aspas de fechamento");
            token.lexeme[i] = '\0';
            token.type = TOKEN_STRING_LITERAL;
            return token;
        }

        // Literais de caractere
        if (current == '\'')
        {
            int i = 0;
            current = fgetc(file);
            if (current == '\'' || current == EOF)
                error("Literal de caractere vazio ou inválido");

            token.lexeme[i++] = current;

            current = fgetc(file);
            if (current != '\'')
                error("Literal de caractere malformado (esperado fechamento com aspas simples)");

            token.lexeme[i] = '\0';
            token.type = TOKEN_CHAR_LITERAL;
            return token;
        }

        // Reconhece símbolos especiais e operadores
        switch (current)
        {
        case '+':
            token.type = TOKEN_MAIS;
            strcpy(token.lexeme, "+");
            return token;
        case '-':
            token.type = TOKEN_MENOS;
            strcpy(token.lexeme, "-");
            return token;
        case '*':
            token.type = TOKEN_MULT;
            strcpy(token.lexeme, "*");
            return token;
        case '/':
            token.type = TOKEN_DIV;
            strcpy(token.lexeme, "/");
            return token;
        case '=':
            token.type = TOKEN_IGUAL;
            strcpy(token.lexeme, "=");
            return token;
        case '(':
            token.type = TOKEN_PARENTESE_ESQ;
            strcpy(token.lexeme, "(");
            return token;
        case ')':
            token.type = TOKEN_PARENTESE_DIR;
            strcpy(token.lexeme, ")");
            return token;
        case '{':
            token.type = TOKEN_CHAVE_ESQ;
            strcpy(token.lexeme, "{");
            return token;
        case '}':
            token.type = TOKEN_CHAVE_DIR;
            strcpy(token.lexeme, "}");
            return token;
        case ';':
            token.type = TOKEN_PONTO_VIRGULA;
            strcpy(token.lexeme, ";");
            return token;
        case ',':
            token.type = TOKEN_VIRGULA;
            strcpy(token.lexeme, ",");
            return token;
        default:
            fprintf(stderr, "Erro: caractere inválido '%c'\n", current);
            error("Token inválido");
        }
    }

    token.type = TOKEN_EOF;
    strcpy(token.lexeme, "EOF");
    return token;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Uso: %s <arquivo_entrada.txt>\n", argv[0]);
        return EXIT_FAILURE;
    }

    file = fopen(argv[1], "r");
    if (!file)
    {
        perror("Erro ao abrir arquivo de entrada");
        return EXIT_FAILURE;
    }

    // Executa o lexer até encontrar EOF
    Token t;
    do {
        t = lexer();
        printf("Token: %-20s Lexema: '%s'\n",
            (t.type == TOKEN_IDENTIFICADOR ? "IDENTIFICADOR" :
             t.type == TOKEN_INT ? "INT" :
             t.type == TOKEN_BOOL ? "BOOL" :
             t.type == TOKEN_STRING ? "STRING" :
             t.type == TOKEN_VOID ? "VOID" :
             t.type == TOKEN_CHAR ? "CHAR" :
             t.type == TOKEN_FLOAT ? "FLOAT" :
             t.type == TOKEN_IF ? "IF" :
             t.type == TOKEN_ELSE ? "ELSE" :
             t.type == TOKEN_WHILE ? "WHILE" :
             t.type == TOKEN_RETURN ? "RETURN" :
             t.type == TOKEN_MAIN ? "MAIN" :
             t.type == TOKEN_BOOLEANO ? "BOOLEANO" :
             t.type == TOKEN_STRING_LITERAL ? "STRING_LITERAL" :
             t.type == TOKEN_CHAR_LITERAL ? "CHAR_LITERAL" :
             t.type == TOKEN_NUMERO ? "NUMERO" :
             t.type == TOKEN_FLOAT_LITERAL ? "FLOAT_LITERAL" :
             t.type == TOKEN_MAIS ? "MAIS" :
             t.type == TOKEN_MENOS ? "MENOS" :
             t.type == TOKEN_MULT ? "MULT" :
             t.type == TOKEN_DIV ? "DIV" :
             t.type == TOKEN_IGUAL ? "IGUAL" :
             t.type == TOKEN_PARENTESE_ESQ ? "PARENTESE_ESQ" :
             t.type == TOKEN_PARENTESE_DIR ? "PARENTESE_DIR" :
             t.type == TOKEN_CHAVE_ESQ ? "CHAVE_ESQ" :
             t.type == TOKEN_CHAVE_DIR ? "CHAVE_DIR" :
             t.type == TOKEN_PONTO_VIRGULA ? "PONTO_VIRGULA" :
             t.type == TOKEN_VIRGULA ? "VIRGULA" :
             t.type == TOKEN_NOVA_LINHA ? "NOVA_LINHA" :
             t.type == TOKEN_EOF ? "EOF" : "ERRO"),
            t.lexeme);
    } while (t.type != TOKEN_EOF);

    fclose(file);
    return EXIT_SUCCESS;
}
