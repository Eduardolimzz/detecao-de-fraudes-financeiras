#include "bench.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define LOG_TESTE "build/log_teste.csv"

static void espera_ativa(double ms) {
    Cronometro c;
    bench_cronometro_iniciar(&c);
    while (bench_cronometro_ms(&c) < ms) {
        /* gira */
    }
}

static void testar_cronometro(void) {
    printf("Testando cronômetro...\n");

    Cronometro c;
    bench_cronometro_iniciar(&c);
    double t0 = bench_cronometro_ms(&c);
    assert(t0 >= 0.0);

    espera_ativa(20.0);
    double t1 = bench_cronometro_ms(&c);
    assert(t1 >= 20.0);   /* nunca mede menos do que esperou */
    assert(t1 < 2000.0);  /* e não está absurdamente errado */
    assert(t1 >= t0);     /* monotônico */
}

static void testar_pico_rss(void) {
    printf("Testando leitura do pico de RSS...\n");
    long kb = bench_pico_rss_kb();
    assert(kb > 0); /* um processo em execução sempre ocupa memória */
    printf("  pico de RSS atual: %ld KB\n", kb);
}

static int contar_linhas(const char *caminho) {
    FILE *f = fopen(caminho, "r");
    assert(f != NULL);
    int n = 0;
    char buf[512];
    while (fgets(buf, sizeof(buf), f) != NULL) {
        n++;
    }
    fclose(f);
    return n;
}

static void testar_log_csv(void) {
    printf("Testando escrita do log CSV...\n");
    remove(LOG_TESTE);

    assert(bench_registrar_log_em(LOG_TESTE, "dfs_ciclos", "lista", 32386, 20000, 12.34567));
    assert(contar_linhas(LOG_TESTE) == 2); /* cabeçalho + 1 registro */

    /* segunda chamada acrescenta, sem repetir o cabeçalho */
    assert(bench_registrar_log_em(LOG_TESTE, "bfs", "matriz", 100, 250, 0.5));
    assert(contar_linhas(LOG_TESTE) == 3);

    FILE *f = fopen(LOG_TESTE, "r");
    assert(f != NULL);
    char linha[512];

    assert(fgets(linha, sizeof(linha), f) != NULL);
    assert(strcmp(linha, "timestamp,algoritmo,representacao,n_vertices,"
                         "n_arestas,tempo_ms,memoria_kb\n") == 0);

    assert(fgets(linha, sizeof(linha), f) != NULL);
    assert(strstr(linha, ",dfs_ciclos,lista,32386,20000,12.346,") != NULL);

    assert(fgets(linha, sizeof(linha), f) != NULL);
    assert(strstr(linha, ",bfs,matriz,100,250,0.500,") != NULL);
    fclose(f);

    /* caminho inválido: falha sem derrubar o programa */
    assert(!bench_registrar_log_em("pasta/que/nao/existe/log.csv", "x", "y", 1, 1, 1.0));

    remove(LOG_TESTE);
}

int main(void) {
    testar_cronometro();
    testar_pico_rss();
    testar_log_csv();
    printf("Módulo de instrumentação validado com sucesso!\n");
    return 0;
}
