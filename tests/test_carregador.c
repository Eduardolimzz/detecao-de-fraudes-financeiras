#include "carregador.h"

#include "mem_track.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

#define FIXTURE_BRINQUEDO "tests/fixtures/transacoes_brinquedo.csv"
#define FIXTURE_ARBITRAGEM "tests/fixtures/arbitragem_brinquedo.csv"
#define FIXTURE_SO_CABECALHO "tests/fixtures/so_cabecalho.csv"
#define DATASET "data/dataset.csv"

/* Grafo-brinquedo da fixture (índices na ordem em que as contas aparecem):
 *   0 = 010:AAA, 1 = 010:BBB, 2 = 020:AAA
 *   0 -> 1 (duas transações), 1 -> 2, 2 -> 0 e auto-laço 2 -> 2
 * mais uma linha quebrada que deve ser ignorada. */
static void testar_fixture(TipoRepr repr) {
    printf("Carregando fixture na representação '%s'...\n", rep_tipo_nome(repr));
    size_t base = mem_bytes_atuais();

    GrafoCarregado c;
    assert(carregador_carregar(FIXTURE_BRINQUEDO, repr, 0, &c) == CARGA_OK);

    assert(c.n_vertices == 3);
    assert(c.n_arestas == 5); /* paralela e auto-laço preservadas */
    assert(c.linhas_ignoradas == 1);
    assert(c.tempo_ms >= 0.0);
    assert(c.bytes_estruturas > 0);
    assert(c.bytes_pico >= c.bytes_estruturas);
    assert(rep_num_vertices(c.grafo) == 3);

    /* chave composta banco:conta, com zero à esquerda preservado */
    assert(label_map_obter_indice(c.rotulos, "010:AAA") == 0);
    assert(label_map_obter_indice(c.rotulos, "010:BBB") == 1);
    assert(label_map_obter_indice(c.rotulos, "020:AAA") == 2);
    assert(label_map_obter_indice(c.rotulos, "10:AAA") == -1);
    /* mesma conta "AAA" em bancos diferentes são vértices distintos */
    assert(label_map_obter_indice(c.rotulos, "010:AAA") !=
           label_map_obter_indice(c.rotulos, "020:AAA"));

    /* direção preservada */
    assert(rep_existe_aresta(c.grafo, 0, 1));
    assert(!rep_existe_aresta(c.grafo, 1, 0));
    assert(rep_existe_aresta(c.grafo, 1, 2));
    assert(rep_existe_aresta(c.grafo, 2, 0));
    assert(rep_existe_aresta(c.grafo, 2, 2));

    carregador_liberar(&c);
    assert(mem_bytes_atuais() == base); /* nada vazou */
}

static void testar_erros(void) {
    printf("Testando erros de carga...\n");
    size_t base = mem_bytes_atuais();
    GrafoCarregado c;

    assert(carregador_carregar("tests/fixtures/nao_existe.csv", REPR_LISTA, 0, &c)
           == CARGA_ARQUIVO_INEXISTENTE);
    carregador_liberar(&c);

    assert(carregador_carregar(FIXTURE_SO_CABECALHO, REPR_LISTA, 0, &c) == CARGA_SEM_DADOS);
    carregador_liberar(&c);

    assert(mem_bytes_atuais() == base);
}

/* Critério de aceite da issue #11: o dataset oficial bate com a
 * caracterização de docs/dataset.md. */
static void testar_dataset_real(void) {
    printf("Carregando %s (lista)...\n", DATASET);
    size_t base = mem_bytes_atuais();

    GrafoCarregado c;
    assert(carregador_carregar(DATASET, REPR_LISTA, 0, &c) == CARGA_OK);
    assert(c.n_vertices == 32386);
    assert(c.n_arestas == 20000);
    assert(c.linhas_ignoradas == 0);
    /* Fase II: todas as 20.000 transações têm valores positivos e viram
     * arestas ponderadas entre 32.709 estados banco:conta:moeda. */
    assert(c.n_vertices_estado == 32709);
    assert(c.n_arestas_bf == 20000);
    carregador_imprimir_sumario(&c, REPR_LISTA);

    carregador_liberar(&c);
    assert(mem_bytes_atuais() == base);
}

/* Regressão: as colunas de valor e moeda eram lidas nos índices errados
 * (Amount Received como moeda, Payment Format como valor) e nenhuma aresta
 * ponderada era gerada. A fixture tem uma arbitragem USD -> EUR -> BRL -> USD. */
static void testar_estados_e_pesos(void) {
    printf("Carregando fixture de arbitragem (estados e pesos)...\n");
    size_t base = mem_bytes_atuais();

    GrafoCarregado c;
    assert(carregador_carregar(FIXTURE_ARBITRAGEM, REPR_LISTA, 0, &c) == CARGA_OK);

    assert(c.n_vertices == 4);        /* contas: AAA, BBB, CCC, DDD */
    assert(c.n_vertices_estado == 4); /* AAA:USD, BBB:EUR, CCC:BRL, DDD:EUR */
    assert(c.n_arestas_bf == 4);

    /* estado de origem usa Payment Currency; o de destino, Receiving Currency */
    int a_usd = label_map_obter_indice(c.rotulos_estado, "001:AAA:USD");
    int b_eur = label_map_obter_indice(c.rotulos_estado, "002:BBB:EUR");
    int c_brl = label_map_obter_indice(c.rotulos_estado, "003:CCC:BRL");
    assert(a_usd >= 0 && b_eur >= 0 && c_brl >= 0);

    /* tx1: pagou 100 USD, recebeu 100 EUR => r = 1 => w = 0 */
    assert(c.arestas_bf[0].origem == a_usd && c.arestas_bf[0].destino == b_eur);
    assert(fabs(c.arestas_bf[0].peso) < 1e-12);
    /* tx2: pagou 100 EUR, recebeu 600 BRL => r = 6 => w = -ln(6) */
    assert(c.arestas_bf[1].origem == b_eur && c.arestas_bf[1].destino == c_brl);
    assert(fabs(c.arestas_bf[1].peso - (-log(6.0))) < 1e-12);
    /* tx3: pagou 600 BRL, recebeu 120 USD => r = 0,2 => w = -ln(0,2) */
    assert(c.arestas_bf[2].origem == c_brl && c.arestas_bf[2].destino == a_usd);
    assert(fabs(c.arestas_bf[2].peso - (-log(0.2))) < 1e-12);

    carregador_liberar(&c);
    assert(mem_bytes_atuais() == base);
}

int main(void) {
    testar_fixture(REPR_LISTA);
    testar_fixture(REPR_MATRIZ);
    testar_erros();
    testar_estados_e_pesos();
    testar_dataset_real();
    printf("Pipeline de carga validado com sucesso!\n");
    return 0;
}
