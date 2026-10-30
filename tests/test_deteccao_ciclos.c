#include "bellman_ford.h"
#include "carregador.h"
#include "deteccao_ciclos.h"
#include "mem_track.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define FIXTURE_BRINQUEDO "tests/fixtures/grafo_brinquedo.csv"

/* Gabarito documentado em tests/fixtures/grafo_brinquedo.md:
-   ciclo de 3: 001:AAA -> 001:BBB -> 001:CCC -> 001:AAA
-   ciclo de 4: 001:DDD -> 001:EEE -> 001:FFF -> 001:GGG -> 001:DDD
-   acíclicos:  001:HHH, 001:III (não podem aparecer em nenhum ciclo) */
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
- ciclo não tem nenhum rótulo além desses. */
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
- vértices (a ordem dos dois em res->itens não é garantida pela
- interface, só a existência e o conteúdo de cada um). */
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

/* --- TESTES DA FASE II: PESOS E BELLMAN-FORD --- */

static void testar_calculo_pesos(void) {
    printf("Testando cálculo de pesos w = -ln(r)...\n");

    /* r = 100 / 100 = 1.0 => w = -ln(1.0) = 0.0 */
    double w1 = calcular_peso_transacao(100.0, 100.0);
    assert(fabs(w1 - 0.0) < 1e-6);

    /* r = 600 / 100 = 6.0 => w = -ln(6.0) approx -1.791759 */
    double w2 = calcular_peso_transacao(100.0, 600.0);
    assert(w2 < 0.0);
    assert(fabs(w2 - (-1.791759)) < 1e-4);

    /* Entradas inválidas / não positivas devem retornar INFINITY */
    assert(!isfinite(calcular_peso_transacao(0.0, 100.0)));
    assert(!isfinite(calcular_peso_transacao(100.0, -50.0)));
}

/* Exemplo 1: Ciclo Negativo (Ganho de Capital / Arbitragem) */
static void testar_bellman_ford_com_ciclo_negativo(void) {
    printf("Testando Bellman-Ford com Ciclo Negativo (Exemplo 1)...\n");

    LabelMap *mapa = label_map_criar(0);
    int u = label_map_obter_ou_inserir(mapa, "B1:C1:USD");
    int v = label_map_obter_ou_inserir(mapa, "B2:C2:EUR");
    int z = label_map_obter_ou_inserir(mapa, "B3:C3:BRL");

    ArestaPonderada arestas[3];
    /* Transação 1: Paid=100 USD, Rec=100 EUR -> w1 = 0.0 */
    arestas[0] = (ArestaPonderada){u, v, calcular_peso_transacao(100.0, 100.0)};
    /* Transação 2: Paid=100 EUR, Rec=600 BRL -> w2 = -1.7918 */
    arestas[1] = (ArestaPonderada){v, z, calcular_peso_transacao(100.0, 600.0)};
    /* Transação 3: Paid=600 BRL, Rec=120 USD -> w3 = 1.6094 */
    arestas[2] = (ArestaPonderada){z, u, calcular_peso_transacao(600.0, 120.0)};

    ResultadoBellmanFord *res = bellman_ford_executar(3, arestas, 3, mapa);
    assert(res != NULL);
    assert(res->possui_ciclo_negativo == 1);
    assert(res->n_ciclos == 1);
    assert(res->itens[0].comprimento == 3);

    bellman_ford_liberar(res);
    label_map_destruir(mapa);
}

/* Exemplo 2: Ciclo Sem Ciclo Negativo (Operação Normal) */
static void testar_bellman_ford_sem_ciclo_negativo(void) {
    printf("Testando Bellman-Ford Sem Ciclo Negativo (Exemplo 2)...\n");

    LabelMap *mapa = label_map_criar(0);
    int u = label_map_obter_ou_inserir(mapa, "B1:C1:USD");
    int v = label_map_obter_ou_inserir(mapa, "B2:C2:EUR");
    int z = label_map_obter_ou_inserir(mapa, "B3:C3:BRL");

    ArestaPonderada arestas[3];
    /* Transação 1: Paid=100 USD, Rec=90 EUR -> w1 = 0.1054 */
    arestas[0] = (ArestaPonderada){u, v, calcular_peso_transacao(100.0, 90.0)};
    /* Transação 2: Paid=90 EUR, Rec=500 BRL -> w2 = -1.7147 */
    arestas[1] = (ArestaPonderada){v, z, calcular_peso_transacao(90.0, 500.0)};
    /* Transação 3: Paid=500 BRL, Rec=95 USD -> w3 = 1.6607 */
    arestas[2] = (ArestaPonderada){z, u, calcular_peso_transacao(500.0, 95.0)};

    ResultadoBellmanFord *res = bellman_ford_executar(3, arestas, 3, mapa);
    assert(res != NULL);
    assert(res->possui_ciclo_negativo == 0);

    bellman_ford_liberar(res);
    label_map_destruir(mapa);
}

int main(void) {
    size_t base = mem_bytes_atuais();

    /* Validações da Fase I */
    testar_grafo_brinquedo(REPR_LISTA);
    testar_grafo_brinquedo(REPR_MATRIZ);

    /* Validações da Fase II */
    testar_calculo_pesos();
    testar_bellman_ford_com_ciclo_negativo();
    testar_bellman_ford_sem_ciclo_negativo();

    assert(mem_bytes_atuais() == base); /* Garante zero vazamentos de memória */
    printf("Detecção de ciclos (Fase I e Fase II) validada com sucesso!\n");
    return 0;
}