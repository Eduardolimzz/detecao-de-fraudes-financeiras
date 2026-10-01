#include "representacao.h"

#include <assert.h>
#include <stdio.h>

#define N_BRINQUEDO 5

/* Grafo-brinquedo dirigido:
 *   0 -> 1, 0 -> 2, 1 -> 3, 2 -> 3, 3 -> 3 (auto-laço), 3 -> 4
 * e uma aresta paralela 0 -> 1 repetida. */
static void montar_brinquedo(RepGrafo *g) {
    assert(rep_inserir_aresta(g, 0, 1, 10.0));
    assert(rep_inserir_aresta(g, 0, 2, 20.0));
    assert(rep_inserir_aresta(g, 1, 3, 30.0));
    assert(rep_inserir_aresta(g, 2, 3, 40.0));
    assert(rep_inserir_aresta(g, 3, 3, 50.0));
    assert(rep_inserir_aresta(g, 3, 4, 60.0));
    assert(rep_inserir_aresta(g, 0, 1, 70.0));
}

/* Marca em visto[] todos os vizinhos de v e devolve quantos "passos"
 * o iterador deu (inclui repetições da lista). */
static int coletar_vizinhos(const RepGrafo *g, int v, int visto[N_BRINQUEDO]) {
    IteradorVizinhos it;
    int destino;
    int passos = 0;

    for (int i = 0; i < N_BRINQUEDO; i++) {
        visto[i] = 0;
    }
    rep_iter_inicio(&it, g, v);
    while (rep_iter_proximo(&it, &destino)) {
        assert(destino >= 0 && destino < N_BRINQUEDO);
        visto[destino] = 1;
        passos++;
    }
    return passos;
}

/* O mesmo conjunto de testes roda nas duas representações: é exatamente
 * isso que a abstração promete aos algoritmos. */
static void testar_representacao(TipoRepr tipo) {
    printf("Testando representação '%s'...\n", rep_tipo_nome(tipo));

    RepGrafo *g = rep_criar(tipo, N_BRINQUEDO);
    assert(g != NULL);
    assert(rep_tipo(g) == tipo);
    assert(rep_num_vertices(g) == N_BRINQUEDO);
    assert(rep_num_arestas(g) == 0);

    montar_brinquedo(g);
    assert(rep_num_arestas(g) == 7);

    /* direção preservada */
    assert(rep_existe_aresta(g, 0, 1));
    assert(!rep_existe_aresta(g, 1, 0));
    assert(rep_existe_aresta(g, 3, 3));
    assert(!rep_existe_aresta(g, 4, 3));

    /* vizinhos de 0 = {1, 2}; de 3 = {3, 4}; de 4 = {} */
    int visto[N_BRINQUEDO];
    int passos = coletar_vizinhos(g, 0, visto);
    assert(visto[1] && visto[2] && !visto[0] && !visto[3] && !visto[4]);
    /* lista repete a paralela (3 passos); matriz colapsa (2 passos) */
    assert(passos == (tipo == REPR_LISTA ? 3 : 2));

    coletar_vizinhos(g, 3, visto);
    assert(visto[3] && visto[4] && !visto[0] && !visto[1] && !visto[2]);

    assert(coletar_vizinhos(g, 4, visto) == 0);

    /* entradas inválidas são rejeitadas sem alterar o grafo */
    assert(!rep_inserir_aresta(g, -1, 0, 1.0));
    assert(!rep_inserir_aresta(g, 0, N_BRINQUEDO, 1.0));
    assert(rep_num_arestas(g) == 7);
    assert(!rep_existe_aresta(g, 0, N_BRINQUEDO));

    IteradorVizinhos it;
    int d;
    rep_iter_inicio(&it, g, 99);
    assert(!rep_iter_proximo(&it, &d));

    rep_liberar(g);
}

static void testar_selecao_por_texto(void) {
    TipoRepr t;
    printf("Testando conversão de texto para TipoRepr...\n");
    assert(rep_tipo_de_texto("lista", &t) && t == REPR_LISTA);
    assert(rep_tipo_de_texto("matriz", &t) && t == REPR_MATRIZ);
    assert(!rep_tipo_de_texto("hash", &t));
    assert(!rep_tipo_de_texto(NULL, &t));
}

static void testar_criacao_invalida(void) {
    printf("Testando criação inválida...\n");
    assert(rep_criar(REPR_LISTA, 0) == NULL);
    assert(rep_criar(REPR_MATRIZ, 0) == NULL);
    assert(rep_num_vertices(NULL) == 0);
    assert(rep_num_arestas(NULL) == 0);
    rep_liberar(NULL);
}

int main(void) {
    testar_selecao_por_texto();
    testar_criacao_invalida();
    testar_representacao(REPR_LISTA);
    testar_representacao(REPR_MATRIZ);
    printf("Camada de abstração validada com sucesso!\n");
    return 0;
}
