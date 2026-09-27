#ifndef CSV_PARSER_H
#define CSV_PARSER_H

/*
 * Parser de CSV genérico e resiliente (RNF01: 100% C puro, sem
 * bibliotecas externas de CSV). Não conhece o domínio do projeto —
 * apenas lê linhas e devolve campos de texto. A interpretação desses
 * campos (conta, valor, etc.) é responsabilidade de outro módulo.
 */

typedef struct {
    char **campos;
    int n_campos;
    int malformada;
} LinhaCSV;

typedef struct CsvParser CsvParser;

/* erro: 0 = ok, 1 = arquivo nao existe, 2 = sem permissao de leitura */
CsvParser *csv_parser_abrir(const char *caminho, int *erro);

/* Lê a próxima linha válida do arquivo, pulando internamente linhas
 * malformadas (e contabilizando-as). Retorna 1 se leu uma linha,
 * 0 se chegou ao fim do arquivo. */
int csv_parser_proxima_linha(CsvParser *p, LinhaCSV *linha_out);

/* Número de linhas malformadas ignoradas até o momento. */
int csv_parser_contador_erros(const CsvParser *p);

void csv_parser_liberar_linha(LinhaCSV *linha);
void csv_parser_fechar(CsvParser *p);

#endif
