#include <stdio.h>

#include "bench.h"
#include "carregador.h"
#include "cli.h"
#include "deteccao_ciclos.h"
#include "mem_track.h"
#include "validacao_lavagem.h"

#define PROJECT_NAME "Detecção de Fraudes Financeiras em Grafos"
#define PROJECT_VERSION "0.5.0"

/* Imprime o caminho do ciclo fechando de volta no primeiro vértice:
 * "A -> B -> C -> A" (auto-laço: "A -> A"). Sem quebra de linha. */
static void imprimir_caminho_ciclo(const Ciclo *c) {
    for (int k = 0; k < c->comprimento; k++) {
        printf("%s -> ", c->rotulos[k]);
    }
    printf("%s", c->rotulos[0]);
}

/* Etapa 5: detecta ciclos no grafo carregado e cruza cada um com a coluna
 * "Is Laundering" do CSV de origem (validação semântica, não só
 * estrutural). Não aborta em caso de erro: a detecção de ciclos é um
 * extra sobre a carga, que já foi validada. */
static void executar_deteccao_ciclos(const GrafoCarregado *carga, TipoRepr repr,
                                     const char *caminho_csv, int mostrar_ciclos) {
    Cronometro cron;
    bench_cronometro_iniciar(&cron);

    ResultadoCiclos *ciclos = deteccao_ciclos_executar(carga->grafo, carga->rotulos);
    double tempo_ms = bench_cronometro_ms(&cron);

    if (ciclos == NULL) {
        fprintf(stderr, "Erro: falta de memória ao detectar ciclos.\n");
        return;
    }

    printf("\nDetecção de ciclos (DFS iterativo, 3 cores)\n");
    printf("  Ciclos encontrados: %d\n", ciclos->n_ciclos);

    /* Histograma do comprimento dos ciclos, em vez de uma linha por ciclo
     * (o dataset real pode ter milhares de ciclos triviais/auto-laços). */
    int maior_comprimento = 0;
    for (int i = 0; i < ciclos->n_ciclos; i++) {
        if (ciclos->itens[i].comprimento > maior_comprimento) {
            maior_comprimento = ciclos->itens[i].comprimento;
        }
    }
    int *histograma = (maior_comprimento > 0)
                           ? mem_calloc((size_t)maior_comprimento + 1, sizeof(int))
                           : NULL;

    ConjuntoLavagem *lavagem = validacao_lavagem_carregar(caminho_csv);
    int n_suspeitos = 0;

    for (int i = 0; i < ciclos->n_ciclos; i++) {
        const Ciclo *c = &ciclos->itens[i];
        if (histograma != NULL) {
            histograma[c->comprimento]++;
        }
        if (lavagem != NULL && validacao_lavagem_ciclo_suspeito(lavagem, c)) {
            n_suspeitos++;
            printf("  Ciclo suspeito (comprimento %d): ", c->comprimento);
            imprimir_caminho_ciclo(c);
            printf("\n");
        }
    }

    if (histograma != NULL) {
        printf("  Distribuição por comprimento:\n");
        for (int len = 1; len <= maior_comprimento; len++) {
            if (histograma[len] > 0) {
                printf("    comprimento %d: %d ciclo(s)\n", len, histograma[len]);
            }
        }
    }
    mem_free(histograma);

    /* --mostrar-ciclos=N|todos: caminho dos N primeiros ciclos, na ordem
     * em que a DFS os encontrou. Com 0 (padrão) nada é impresso aqui. */
    int n_mostrar = (mostrar_ciclos == MOSTRAR_CICLOS_TODOS || mostrar_ciclos > ciclos->n_ciclos)
                        ? ciclos->n_ciclos
                        : mostrar_ciclos;
    if (n_mostrar > 0) {
        printf("  Caminhos dos ciclos (%d de %d):\n", n_mostrar, ciclos->n_ciclos);
        for (int i = 0; i < n_mostrar; i++) {
            const Ciclo *c = &ciclos->itens[i];
            printf("    Ciclo %d (comprimento %d): ", i + 1, c->comprimento);
            imprimir_caminho_ciclo(c);
            if (lavagem != NULL && validacao_lavagem_ciclo_suspeito(lavagem, c)) {
                printf(" [SUSPEITO - contem transacao marcada como lavagem]");
            }
            printf("\n");
        }
    }

    if (lavagem == NULL) {
        printf("  Aviso: não foi possível cruzar com Is Laundering (%s).\n", caminho_csv);
    } else {
        printf("  Transações marcadas Is Laundering=1 no CSV: %d\n",
               validacao_lavagem_tamanho(lavagem));
        if (ciclos->n_ciclos > 0) {
            printf("  Ciclos com ao menos uma transação suspeita: %d de %d\n",
                   n_suspeitos, ciclos->n_ciclos);
        }
    }

    bench_registrar_log("deteccao_ciclos", rep_tipo_nome(repr), carga->n_vertices,
                        carga->n_arestas, tempo_ms);

    validacao_lavagem_liberar(lavagem);
    deteccao_ciclos_liberar(ciclos);
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

    executar_deteccao_ciclos(&carga, op.repr, op.dataset, op.mostrar_ciclos);

    carregador_liberar(&carga);
    return 0;
}