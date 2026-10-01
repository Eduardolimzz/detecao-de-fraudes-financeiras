#include <stdio.h>

#include "bench.h"
#include "cli.h"
#include "mem_track.h"
#include "representacao.h"

#define PROJECT_NAME "Detecção de Fraudes Financeiras em Grafos"
#define PROJECT_VERSION "0.3.0"

/* Imprime os vizinhos de um vértice usando apenas a interface rep_*.
 * O código é idêntico para lista e matriz. */
static void imprimir_vizinhos(const RepGrafo *g, int v) {
    IteradorVizinhos it;
    int destino;

    printf("  vizinhos de %d:", v);
    rep_iter_inicio(&it, g, v);
    while (rep_iter_proximo(&it, &destino)) {
        printf(" %d", destino);
    }
    printf("\n");
}

int main(int argc, char **argv) {
    Opcoes op;
    cli_opcoes_padrao(&op);

    if (!cli_interpretar(argc, argv, &op)) {
        cli_imprimir_uso(argv[0]);
        return 1;
    }
    if (op.pedir_ajuda) {
        cli_imprimir_uso(argv[0]);
        return 0;
    }

    printf("%s\n", PROJECT_NAME);
    printf("Versão: %s\n", PROJECT_VERSION);
    printf("Representação: %s\n", rep_tipo_nome(op.repr));

    /* Demonstração com um grafo-brinquedo; a carga do dataset real é
     * feita no pipeline de carga (issue #11). */
    Cronometro cron;
    bench_cronometro_iniciar(&cron);

    RepGrafo *g = rep_criar(op.repr, 4);
    if (g == NULL) {
        fprintf(stderr, "Erro: não foi possível criar o grafo.\n");
        return 1;
    }
    rep_inserir_aresta(g, 0, 1, 1.0);
    rep_inserir_aresta(g, 0, 2, 1.0);
    rep_inserir_aresta(g, 2, 3, 1.0);

    double tempo_ms = bench_cronometro_ms(&cron);

    printf("Grafo-brinquedo: |V|=%d |E|=%d\n", rep_num_vertices(g), rep_num_arestas(g));
    for (int v = 0; v < rep_num_vertices(g); v++) {
        imprimir_vizinhos(g, v);
    }
    printf("Tempo de construção: %.3f ms\n", tempo_ms);
    printf("Memória alocada (autoral): %zu bytes\n", mem_bytes_atuais());
    printf("Pico de RSS: %ld KB\n", bench_pico_rss_kb());

    /* O log é gravado a cada execução, sem flag extra (RF03). */
    bench_registrar_log("demo_brinquedo", rep_tipo_nome(op.repr),
                        rep_num_vertices(g), rep_num_arestas(g), tempo_ms);

    rep_liberar(g);
    return 0;
}
