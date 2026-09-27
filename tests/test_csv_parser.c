#include <stdio.h>
#include <string.h>

#include "csv_parser.h"

#define CAMINHO_FIXTURE "tests/fixtures/csv_valido_10_linhas.csv"

static int falhas = 0;

static void verificar(int condicao, const char *descricao) {
    if (condicao) {
        printf("[OK] %s\n", descricao);
    } else {
        printf("[FALHOU] %s\n", descricao);
        falhas++;
    }
}

static void testar_arquivo_inexistente(void) {
    int erro = -1;
    CsvParser *p = csv_parser_abrir("tests/fixtures/nao_existe.csv", &erro);
    verificar(p == NULL, "abrir arquivo inexistente retorna NULL");
    verificar(erro == 1, "abrir arquivo inexistente sinaliza erro 1");
}

static void testar_fixture_10_linhas(void) {
    int erro = -1;
    CsvParser *p = csv_parser_abrir(CAMINHO_FIXTURE, &erro);
    verificar(p != NULL, "fixture de 10 linhas abre com sucesso");
    verificar(erro == 0, "abrir fixture nao sinaliza erro");
    if (p == NULL) {
        return;
    }

    int linhas_validas = 0;
    int encontrou_campo_com_virgula_interna = 0;
    LinhaCSV linha;

    while (csv_parser_proxima_linha(p, &linha)) {
        linhas_validas++;

        if (linha.n_campos >= 2 && strcmp(linha.campos[1], "Silva, Pedro") == 0) {
            encontrou_campo_com_virgula_interna = 1;
        }

        csv_parser_liberar_linha(&linha);
    }

    printf("Linhas validas lidas: %d\n", linhas_validas);
    printf("Linhas malformadas (erros): %d\n", csv_parser_contador_erros(p));

    verificar(linhas_validas == 8, "le exatamente 8 linhas de dados validas");
    verificar(csv_parser_contador_erros(p) == 1, "contabiliza exatamente 1 linha malformada");
    verificar(encontrou_campo_com_virgula_interna,
              "campo entre aspas com virgula interna e parseado corretamente");

    csv_parser_fechar(p);
}

int main(void) {
    testar_arquivo_inexistente();
    testar_fixture_10_linhas();

    if (falhas > 0) {
        printf("\n%d verificacao(oes) falharam.\n", falhas);
        return 1;
    }

    printf("\nTodas as verificacoes passaram.\n");
    return 0;
}
