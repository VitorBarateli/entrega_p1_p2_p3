#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>

#define MAX_TOKEN_LEN 100 // Definição de tamanho máximo para tokens

// Enumeração para os tipos de tokens reconhecidos
typedef enum
{
    TOKEN_LABEL,
    TOKEN_NUMERO,
    TOKEN_IGUAL,
    TOKEN_OPERADOR,
    TOKEN_PARENTESE_ESQ,
    TOKEN_PARENTESE_DIR,
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
FILE *output_file;
Token current_token;

// Função para exibir mensagens de erro e encerrar o programa
void error(const char *msg)
{
    fprintf(stderr, "Erro: %s\n", msg);
    exit(EXIT_FAILURE);
}

// Função para fazer análise léxica (tokenização)
Token lexer()
{
    Token token;
    char c;

    while ((c = fgetc(file)) != EOF)
    {
        // Ignora espaços em branco
        if (isspace(c))
        {
            if (c == '\n')
            {
                token.type = TOKEN_NOVA_LINHA;
                strcpy(token.lexeme, "\n");
                return token;
            }
            continue;
        }

        // Reconhece labels com emojis ou caracteres UTF-8
        if ((unsigned char)c >= 0x80 || isalpha((unsigned char)c))
        {
            int i = 0;
            token.lexeme[i++] = c;

            while (1)
            {
                c = fgetc(file);
                if (c == EOF)
                    break;

                // Continua a leitura enquanto for válido (letra, dígito, underscore ou UTF-8)
                if ((unsigned char)c >= 0x80 || isalnum((unsigned char)c) || c == '_')
                {
                    if (i < MAX_TOKEN_LEN - 1)
                        token.lexeme[i++] = c;
                }
                else
                {
                    ungetc(c, file);
                    break;
                }
            }

            token.lexeme[i] = '\0';
            token.type = TOKEN_LABEL;
            return token;
        }

        // Reconhece números
        if (isdigit(c))
        {
            int i = 0;
            token.lexeme[i++] = c;
            while (isdigit(c = fgetc(file)))
                token.lexeme[i++] = c;
            token.lexeme[i] = '\0';
            ungetc(c, file); // Devolve o caractere não numérico
            token.type = TOKEN_NUMERO;
            return token;
        }

        // Reconhece símbolos especiais e operadores
        switch (c)
        {
        case '=':
            token.type = TOKEN_IGUAL;
            strcpy(token.lexeme, "=");
            return token;
        case '+':
        case '-':
        case '*':
        case '/':
            token.type = TOKEN_OPERADOR;
            token.lexeme[0] = c;
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
            token.type = TOKEN_ERRO;
            snprintf(token.lexeme, sizeof(token.lexeme), "%c", c);
            return token;
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
        advance();
    else
        error("Token inesperado");
}

int expr();

// Analisa um fator: uma variável ou uma expressão entre parênteses; e retorna o valor
int fator()
{
    int val = 0;
    if (current_token.type == TOKEN_NUMERO)
    {
        val = atoi(current_token.lexeme);
        advance();
    }
    else if (current_token.type == TOKEN_PARENTESE_ESQ)
    {
        advance();
        val = expr();
        expect(TOKEN_PARENTESE_DIR);
    }
    else
    {
        error("Esperado número ou parêntese");
    }
    return val;
}

// Analisa um termo: multiplicações e divisões
int termo()
{
    int val = fator();
    while (current_token.type == TOKEN_OPERADOR &&
           (current_token.lexeme[0] == '*' || current_token.lexeme[0] == '/'))
    {
        char op = current_token.lexeme[0];
        advance();
        int right = fator();
        if (op == '*')
            val *= right;
        else
        {
            if (right == 0)
                error("Divisão por zero");
            val /= right;
        }
    }
    return val;
}

// Analisa uma expressão: soma e subtração
int expr()
{
    int val = termo();
    while (current_token.type == TOKEN_OPERADOR &&
           (current_token.lexeme[0] == '+' || current_token.lexeme[0] == '-'))
    {
        char op = current_token.lexeme[0];
        advance();
        int right = termo();
        if (op == '+')
            val += right;
        else
            val -= right;
    }
    return val;
}

void gerar_brainfuck(const char *str, FILE *out)
{
    int current = 0;
    fprintf(out, ">"); // Move para célula 1

    for (const unsigned char *p = (const unsigned char *)str; *p; p++)
    {
        int delta = *p - current;

        // Ajusta valor da célula com '+' ou '-'
        if (delta > 0)
            for (int i = 0; i < delta; i++)
                fputc('+', out);
        else
            for (int i = 0; i < -delta; i++)
                fputc('-', out);

        fputc('.', out);
        current = *p;
    }

    fputc('\n', out);
}

// Analisa linha no formato: LABEL = EXPRESSAO
void parse_linha()
{
    if (current_token.type == TOKEN_IGUAL)
    {
        error("Linha iniciada com '=' sem um label. Operação abortada.");
    }

    if (current_token.type == TOKEN_LABEL)
    {
        char label[MAX_TOKEN_LEN];
        strcpy(label, current_token.lexeme);
        advance();

        expect(TOKEN_IGUAL);
        int resultado = expr();

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%s=%d", label, resultado);
        gerar_brainfuck(buffer, output_file);

        if (current_token.type == TOKEN_NOVA_LINHA)
            advance();
    }
    else
    {
        error("Esperado um label antes de '='.");
    }
}

int main()
{
    setlocale(LC_ALL, ""); // Suporte a caracteres Unicode

    file = stdin;
    output_file = stdout;

    advance();
    while (current_token.type != TOKEN_EOF)
    {
        parse_linha();
    }

    return 0;
}
