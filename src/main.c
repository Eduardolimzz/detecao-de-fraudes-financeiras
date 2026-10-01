#include <stdio.h>

#include "bench.h"
#include "carregador.h"
#include "cli.h"

#define PROJECT_NAME "Detecção de Fraudes Financeiras em Grafos"
#define PROJECT_VERSION "0.4.0"

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
    printf("Dataset: %s\n", op.dataset);
    if (op.limite > 0) {
        printf("Limite: %d vértices\n", op.limite);
    }

    GrafoCarregado carga;
    StatusCarga status = carregador_carregar(op.dataset, op.repr, op.limite, &carga);
    if (status != CARGA_OK) {
        fprintf(stderr, "Erro ao carregar '%s': %s.\n", op.dataset,
                carregador_status_texto(status));
        carregador_liberar(&carga);
        return 1;
    }

    carregador_imprimir_sumario(&carga, op.repr);

    /* O log é gravado a cada execução, sem flag extra (RF03). */
    bench_registrar_log("carga_csv", rep_tipo_nome(op.repr), carga.n_vertices,
                        carga.n_arestas, carga.tempo_ms);

    carregador_liberar(&carga);
    return 0;
}