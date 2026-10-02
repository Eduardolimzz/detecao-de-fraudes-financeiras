#include "deteccao_ciclos.h"

#include "carregador.h"
#include "mem_track.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define FIXTURE_BRINQUEDO "tests/fixtures/grafo_brinquedo.csv"

/* Gabarito documentado em tests/fixtures/grafo_brinquedo.md:
 *   ciclo de 3: 001:AAA -> 001:BBB -> 001:CCC -> 001:AAA
 *   ciclo de 4: 001:DDD -> 001:EEE -> 001:FFF -> 001:GGG -> 001:DDD
 *   acíclicos:  001:HHH, 001:III (não podem aparecer em nenhum ciclo) */
static const char *CICLO_3[] = {"001:AAA", "001:BBB", "001:CCC"};
static const char *CICLO_4[] = {"001:DDD", "001:EEE", "001:FFF", "001:GGG"};
static const char *ACICLICOS[] = {"001:HHH", "001:III"};

static int ciclo_contem_rotulo(const Ciclo *c, const char *rotulo) {
    for (int i = 0; i < c->comprimento; i++) {
        if (strcmp(c->rotulos[i], rotulo) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Verifica que 'esperados' (tamanho n) aparecem todos no ciclo e que o
 * ciclo não tem nenhum rótulo além desses. */
static void assert_ciclo_igual(const Ciclo *c, const char **esperados, int n) {
    assert(c->comprimento == n);
    for (int i = 0; i < n; i++) {
        assert(ciclo_contem_rotulo(c, esperados[i]));
    }
}

static void testar_grafo_brinquedo(TipoRepr repr) {
    printf("Detectando ciclos no grafo-brinquedo (representação '%s')...\n",
           rep_tipo_nome(repr));
    size_t base = mem_bytes_atuais();

    GrafoCarregado c;
    assert(carregador_carregar(FIXTURE_BRINQUEDO, repr, 0, &c) == CARGA_OK);
    assert(c.n_vertices == 9);

    ResultadoCiclos *res = deteccao_ciclos_executar(c.grafo, c.rotulos);
    assert(res != NULL);
    assert(res->n_ciclos == 2);

    /* Os dois ciclos esperados devem aparecer, um com 3 e outro com 4
     * vértices (a ordem dos dois em res->itens não é garantida pela
     * interface, só a existência e o conteúdo de cada um). */
    const Ciclo *ciclo3 = NULL;
    const Ciclo *ciclo4 = NULL;
    for (int i = 0; i < res->n_ciclos; i++) {
        if (res->itens[i].comprimento == 3) {
            ciclo3 = &res->itens[i];
        } else if (res->itens[i].comprimento == 4) {
            ciclo4 = &res->itens[i];
        }
    }
    assert(ciclo3 != NULL);
    assert(ciclo4 != NULL);
    assert_ciclo_igual(ciclo3, CICLO_3, 3);
    assert_ciclo_igual(ciclo4, CICLO_4, 4);

    /* Nenhum vértice acíclico pode aparecer em nenhum ciclo reportado. */
    for (int i = 0; i < res->n_ciclos; i++) {
        for (size_t j = 0; j < sizeof(ACICLICOS) / sizeof(ACICLICOS[0]); j++) {
            assert(!ciclo_contem_rotulo(&res->itens[i], ACICLICOS[j]));
        }
    }

    deteccao_ciclos_liberar(res);
    carregador_liberar(&c);
    assert(mem_bytes_atuais() == base); /* nada vazou */
}

int main(void) {
    testar_grafo_brinquedo(REPR_LISTA);
    testar_grafo_brinquedo(REPR_MATRIZ);
    printf("Detecção de ciclos validada contra o grafo-brinquedo com sucesso!\n");
    return 0;
}
