#include "grafo.h"
#include "mem_track.h"

#include <stdio.h>
#include <stdlib.h>

#define CAPACIDADE_PADRAO_VERTICES 16

struct Grafo {
    ArestaNode **listas; 
    int *graus_saida;   
    int num_vertices;   
    int num_arestas;    
};

static int garantir_capacidade(Grafo *g, int vertice_max) {
    if (vertice_max < g->num_vertices) {
        return 1;
    }

    int nova_cap = (g->num_vertices == 0) ? CAPACIDADE_PADRAO_VERTICES : g->num_vertices;
    while (nova_cap <= vertice_max) {
        nova_cap *= 2;
    }

    ArestaNode **novas_listas = mem_realloc(g->listas, (size_t)nova_cap * sizeof(ArestaNode *));
    if (novas_listas == NULL) {
        return 0;
    }
    g->listas = novas_listas;

    int *novos_graus = mem_realloc(g->graus_saida, (size_t)nova_cap * sizeof(int));
    if (novos_graus == NULL) {
        return 0;
    }
    g->graus_saida = novos_graus;

    for (int i = g->num_vertices; i < nova_cap; i++) {
        g->listas[i] = NULL;
        g->graus_saida[i] = 0;
    }

    g->num_vertices = nova_cap;
    return 1;
}

Grafo *grafo_criar(int num_vertices_inicial) {
    Grafo *g = mem_malloc(sizeof(Grafo));
    if (g == NULL) {
        return NULL;
    }

    int cap = (num_vertices_inicial > 0) ? num_vertices_inicial : CAPACIDADE_PADRAO_VERTICES;

    g->listas = mem_calloc((size_t)cap, sizeof(ArestaNode *));
    if (g->listas == NULL) {
        mem_free(g);
        return NULL;
    }

    g->graus_saida = mem_calloc((size_t)cap, sizeof(int));
    if (g->graus_saida == NULL) {
        mem_free(g->listas);
        mem_free(g);
        return NULL;
    }

    g->num_vertices = cap;
    g->num_arestas = 0;

    return g;
}

int grafo_inserir_aresta(Grafo *g, int origem, int destino, double valor) {
    if (g == NULL || origem < 0 || destino < 0) {
        return 0;
    }

    int vertice_max = (origem > destino) ? origem : destino;
    if (!garantir_capacidade(g, vertice_max)) {
        return 0;
    }

    ArestaNode *novo_no = mem_malloc(sizeof(ArestaNode));
    if (novo_no == NULL) {
        return 0;
    }

    novo_no->destino = destino;
    novo_no->valor = valor;
    novo_no->proximo = g->listas[origem];
    g->listas[origem] = novo_no;

    g->graus_saida[origem]++;
    g->num_arestas++;

    return 1;
}

int grafo_grau_saida(const Grafo *g, int vertice) {
    if (g == NULL || vertice < 0 || vertice >= g->num_vertices) {
        return -1;
    }
    return g->graus_saida[vertice];
}

const ArestaNode *grafo_obter_adjacentes(const Grafo *g, int vertice) {
    if (g == NULL || vertice < 0 || vertice >= g->num_vertices) {
        return NULL;
    }
    return g->listas[vertice];
}

int grafo_num_vertices(const Grafo *g) {
    if (g == NULL) return 0;
    return g->num_vertices;
}

int grafo_num_arestas(const Grafo *g) {
    if (g == NULL) return 0;
    return g->num_arestas;
}

void grafo_liberar(Grafo *g) {
    if (g == NULL) {
        return;
    }

    for (int i = 0; i < g->num_vertices; i++) {
        ArestaNode *atual = g->listas[i];
        while (atual != NULL) {
            ArestaNode *temp = atual;
            atual = atual->proximo;
            mem_free(temp);
        }
    }

    mem_free(g->listas);
    mem_free(g->graus_saida);
    mem_free(g);
}