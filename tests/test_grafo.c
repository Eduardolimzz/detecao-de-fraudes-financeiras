#include "grafo.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_VERTICES_TESTE 10000
#define NUM_ARESTAS_TESTE 50000

void testar_operacoes_basicas(void) {
    printf("Executando testes de operações básicas em grafo dirigido...\n");

    Grafo *g = grafo_criar(4);
    assert(g != NULL);

    /* Teste de inserção dirigida: 0 -> 1, 0 -> 2, 1 -> 2 */
    assert(grafo_inserir_aresta(g, 0, 1, 150.50));
    assert(grafo_inserir_aresta(g, 0, 2, 300.00));
    assert(grafo_inserir_aresta(g, 1, 2, 50.00));

    /* Graus de saída */
    assert(grafo_grau_saida(g, 0) == 2);
    assert(grafo_grau_saida(g, 1) == 1);
    assert(grafo_grau_saida(g, 2) == 0);
    assert(grafo_num_arestas(g) == 3);

    /* Navegação na lista de adjacência de 0 */
    const ArestaNode *adj = grafo_obter_adjacentes(g, 0);
    assert(adj != NULL);
    assert(adj->destino == 2); /* Inserção no topo da lista */
    assert(adj->proximo != NULL);
    assert(adj->proximo->destino == 1);

    grafo_liberar(g);
    printf("Operações básicas validadas com sucesso!\n");
}

void testar_redimensionamento_e_carga(void) {
    printf("Executando teste de carga com %d arestas...\n", NUM_ARESTAS_TESTE);

    Grafo *g = grafo_criar(10);
    assert(g != NULL);

    
    for (int i = 0; i < NUM_ARESTAS_TESTE; i++) {
        int u = i % NUM_VERTICES_TESTE;
        int v = (i * 7 + 3) % NUM_VERTICES_TESTE;
        double valor = (double)(i + 1) * 10.25;

        assert(grafo_inserir_aresta(g, u, v, valor));
    }

    assert(grafo_num_arestas(g) == NUM_ARESTAS_TESTE);

   
    int total_graus = 0;
    for (int i = 0; i < grafo_num_vertices(g); i++) {
        int grau = grafo_grau_saida(g, i);
        if (grau > 0) {
            total_graus += grau;

            int cont = 0;
            const ArestaNode *node = grafo_obter_adjacentes(g, i);
            while (node != NULL) {
                cont++;
                node = node->proximo;
            }
            assert(cont == grau);
        }
    }
    assert(total_graus == NUM_ARESTAS_TESTE);

    grafo_liberar(g);
    printf("Teste de carga finalizado com sucesso!\n");
}

int main(void) {
    testar_operacoes_basicas();
    testar_redimensionamento_e_carga();
    return 0;
}