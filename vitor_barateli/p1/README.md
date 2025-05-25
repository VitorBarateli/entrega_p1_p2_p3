## Vitor Jorge Barateli
- Observações:
    - programa.txt sempre deverá ter uma variável com uma expressão matemática e a expressão precisa ser igual a que está na variável RES
        - Exemplo:
        ```
        x = a + b
        RES = a + b
        ```

- Limitações:
    - Compilador não verifica se as variáveis que estão na expressão matemática realmente foram inicializadas. Portanto, é possível criar uma expressão matemática com variáveis que não existem, gerando cálculos errados. Exemplo:
        ```
        A = 1
        X = B + C
        RES = B + C
        ```

- Comandos de Execução:
    - compilador.c: ./compilador <diretorio_programa.txt> <nome_arquivo_gerado.txt>
    - assembler.c ./assembler <diretorio_arquivo.txt> <nome_arquivo_gerado.mem>
    - executor.c: ./executor <diretorio_arquivo.mem>