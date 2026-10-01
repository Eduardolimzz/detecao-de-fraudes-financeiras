#include "carregador.h"

#include "mem_track.h"

#include <assert.h>
#include <stdio.h>

#define FIXTURE_BRINQUEDO "tests/fixtures/transacoes_brinquedo.csv"
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
    assert(carregador_carregar(FIXTURE_BRINQUEDO, repr, &c) == CARGA_OK);

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

    assert(carregador_carregar("tests/fixtures/nao_existe.csv", REPR_LISTA, &c)
           == CARGA_ARQUIVO_INEXISTENTE);
    carregador_liberar(&c);

    assert(carregador_carregar(FIXTURE_SO_CABECALHO, REPR_LISTA, &c) == CARGA_SEM_DADOS);
    carregador_liberar(&c);

    assert(mem_bytes_atuais() == base);
}

/* Critério de aceite da issue #11: o dataset oficial bate com a
 * caracterização de docs/dataset.md. */
static void testar_dataset_real(void) {
    printf("Carregando %s (lista)...\n", DATASET);
    size_t base = mem_bytes_atuais();

    GrafoCarregado c;
    assert(carregador_carregar(DATASET, REPR_LISTA, &c) == CARGA_OK);
    assert(c.n_vertices == 32386);
    assert(c.n_arestas == 20000);
    assert(c.linhas_ignoradas == 0);
    carregador_imprimir_sumario(&c, REPR_LISTA);

    carregador_liberar(&c);
    assert(mem_bytes_atuais() == base);
}

int main(void) {
    testar_fixture(REPR_LISTA);
    testar_fixture(REPR_MATRIZ);
    testar_erros();
    testar_dataset_real();
    printf("Pipeline de carga validado com sucesso!\n");
    return 0;
}
