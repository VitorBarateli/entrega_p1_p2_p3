# Comandos
- Comandos de Compilação:
    - **bfc**: gcc -o bfc bfc.c
    - **bfe**: gcc -o bfe bfe.c

- Comandos de Execução:
    - **bfc**: echo "label=expressao_matematica" | ./bfc
    - **bfe**: echo "label=expressao_matematica" | ./bfc | ./bfe
    - **Exemplo 1**: echo "crédito=5*4/2+2-1" | ./bfc
    - **Exemplo 2**: echo "😀=5*4/2+2-1" | ./bfc | ./bfe

# Observações
- O programa não realiza multiplicação ou divisão com números negativos
- A divisão funciona somente com divisões exatas