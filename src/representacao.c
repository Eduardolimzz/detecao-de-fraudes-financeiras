#include "representacao.h"

#include "grafo.h"
#include "matriz_grafo.h"

#include <stdlib.h>
#include <string.h>

struct RepGrafo {
    TipoRepr tipo;
    int num_vertices;
    int num_arestas;
    Grafo *lista;
    MatrizGrafo *matriz;
};

int rep_tipo_de_texto(const char *texto, TipoRepr *tipo_out) {
    if (texto == NULL || tipo_out == NULL) {
        return 0;
    }
    if (strcmp(texto, "lista") == 0) {
        *tipo_out = REPR_LISTA;
        return 1;
    }
    if (strcmp(texto, "matriz") == 0) {
        *tipo_out = REPR_MATRIZ;
        return 1;
    }
    return 0;
}

const char *rep_tipo_nome(TipoRepr tipo) {
    return (tipo == REPR_MATRIZ) ? "matriz" : "lista";
}

RepGrafo *rep_criar(TipoRepr tipo, int num_vertices) {
    if (num_vertices <= 0) {
        return NULL;
    }

    RepGrafo *g = calloc(1, sizeof(RepGrafo));
    if (g == NULL) {
        return NULL;
    }

    g->tipo = tipo;
    g->num_vertices = num_vertices;

    switch (tipo) {
        case REPR_LISTA:
            g->lista = grafo_criar(num_vertices);
            break;
        case REPR_MATRIZ:
            g->matriz = matriz_grafo_criar(num_vertices);
            break;
    }

    if (g->lista == NULL && g->matriz == NULL) {
        free(g);
        return NULL;
    }
    return g;
}

int rep_inserir_aresta(RepGrafo *g, int origem, int destino, double valor) {
    if (g == NULL || origem < 0 || origem >= g->num_vertices ||
        destino < 0 || destino >= g->num_vertices) {
        return 0;
    }

    int ok = 0;
    if (g->tipo == REPR_LISTA) {
        ok = grafo_inserir_aresta(g->lista, origem, destino, valor);
    } else {
        ok = matriz_grafo_inserir_aresta(g->matriz, origem, destino);
    }

    if (ok) {
        g->num_arestas++;
    }
    return ok;
}

int rep_existe_aresta(const RepGrafo *g, int origem, int destino) {
    if (g == NULL) {
        return 0;
    }

    if (g->tipo == REPR_MATRIZ) {
        return matriz_grafo_existe_aresta(g->matriz, origem, destino);
    }

    for (const ArestaNode *no = grafo_obter_adjacentes(g->lista, origem);
         no != NULL; no = no->proximo) {
        if (no->destino == destino) {
            return 1;
        }
    }
    return 0;
}

TipoRepr rep_tipo(const RepGrafo *g) {
    return g->tipo;
}

int rep_num_vertices(const RepGrafo *g) {
    return (g == NULL) ? 0 : g->num_vertices;
}

int rep_num_arestas(const RepGrafo *g) {
    return (g == NULL) ? 0 : g->num_arestas;
}

void rep_iter_inicio(IteradorVizinhos *it, const RepGrafo *g, int vertice) {
    it->g = g;
    it->vertice = vertice;
    it->no_atual = NULL;
    it->coluna_atual = 0;

    if (g == NULL || vertice < 0 || vertice >= g->num_vertices) {
        it->g = NULL; /* iterador vazio */
        return;
    }
    if (g->tipo == REPR_LISTA) {
        it->no_atual = grafo_obter_adjacentes(g->lista, vertice);
    }
}

int rep_iter_proximo(IteradorVizinhos *it, int *destino_out) {
    if (it->g == NULL) {
        return 0;
    }

    if (it->g->tipo == REPR_LISTA) {
        const ArestaNode *no = it->no_atual;
        if (no == NULL) {
            return 0;
        }
        *destino_out = no->destino;
        it->no_atual = no->proximo;
        return 1;
    }

    /* Matriz: varre a linha do vértice até achar a próxima célula marcada.
     * Custo O(V) por vértice, o que é esperado nesta representação. */
    while (it->coluna_atual < it->g->num_vertices) {
        int col = it->coluna_atual++;
        if (matriz_grafo_existe_aresta(it->g->matriz, it->vertice, col)) {
            *destino_out = col;
            return 1;
        }
    }
    return 0;
}

void rep_liberar(RepGrafo *g) {
    if (g == NULL) {
        return;
    }
    grafo_liberar(g->lista);
    matriz_grafo_liberar(g->matriz);
    free(g);
}
