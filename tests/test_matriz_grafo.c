#include "matriz_grafo.h"
#include "grafo.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_VERTICES_TESTE 1000
#define NUM_ARESTAS_TESTE 5000
#define NUM_PARES_COMPARACAO 1000

static int lista_existe_aresta(const Grafo *g, int origem, int destino) {
    const ArestaNode *node = grafo_obter_adjacentes(g, origem);
    while (node != NULL) {
        if (node->destino == destino) {
            return 1;
        }
        node = node->proximo;
    }
    return 0;
}

void testar_limite_maximo(void) {
    printf("Testando rejeição para número de vértices acima do limite máximo...\n");
    MatrizGrafo *m_invalida = matriz_grafo_criar(MAX_VERTICES_MATRIZ + 1);
    assert(m_invalida == NULL);
    printf("Rejeição de limite validada com sucesso!\n");
}

void testar_comparacao_com_lista_adjacencia(void) {
    printf("Iniciando comparação entre Matriz de Adjacência e Lista de Adjacência...\n");

    srand(42); 

    Grafo *lista = grafo_criar(NUM_VERTICES_TESTE);
    assert(lista != NULL);

    MatrizGrafo *matriz = matriz_grafo_criar(NUM_VERTICES_TESTE);
    assert(matriz != NULL);

    for (int i = 0; i < NUM_ARESTAS_TESTE; i++) {
        int u = rand() % NUM_VERTICES_TESTE;
        int v = rand() % NUM_VERTICES_TESTE;
        double valor = (double)(rand() % 1000) + 1.0;

        grafo_inserir_aresta(lista, u, v, valor);
        matriz_grafo_inserir_aresta(matriz, u, v);
    }

 
    printf("Comparando 'existe_aresta' para %d pares aleatórios...\n", NUM_PARES_COMPARACAO);
    for (int i = 0; i < NUM_PARES_COMPARACAO; i++) {
        int u = rand() % NUM_VERTICES_TESTE;
        int v = rand() % NUM_VERTICES_TESTE;

        int existe_na_matriz = matriz_grafo_existe_aresta(matriz, u, v);
        int existe_na_lista = lista_existe_aresta(lista, u, v);

        assert(existe_na_matriz == existe_na_lista);
    }

    matriz_grafo_liberar(matriz);
    grafo_liberar(lista);

    printf("Comparação concluída: 1.000 pares validados com 100%% de correspondência!\n");
}

int main(void) {
    testar_limite_maximo();
    testar_comparacao_com_lista_adjacencia();
    return 0;
}