#include "matriz_grafo.h"
#include "mem_track.h"

#include <stdio.h>
#include <stdlib.h>

struct MatrizGrafo {
    unsigned char *matriz; 
    int num_vertices;
};

MatrizGrafo *matriz_grafo_criar(int num_vertices) {
    if (num_vertices <= 0) {
        fprintf(stderr, "Erro: O número de vértices deve ser maior que zero.\n");
        return NULL;
    }

  
    if (num_vertices > MAX_VERTICES_MATRIZ) {
        fprintf(stderr,
                "Erro: Número de vértices (%d) excede o limite prático suportado "
                "pela matriz de adjacência (%d vértices, que exige ~%.2f GB de RAM).\n",
                num_vertices,
                MAX_VERTICES_MATRIZ,
                ((double)num_vertices * (double)num_vertices) / (1024.0 * 1024.0 * 1024.0));
        return NULL;
    }

    MatrizGrafo *m = mem_malloc(sizeof(MatrizGrafo));
    if (m == NULL) {
        return NULL;
    }

    size_t bytes_totais = (size_t)num_vertices * (size_t)num_vertices;
    m->matriz = mem_calloc(bytes_totais, sizeof(unsigned char));
    if (m->matriz == NULL) {
        fprintf(stderr, "Erro: Falha ao alocar %zu bytes para a matriz de adjacência.\n", bytes_totais);
        mem_free(m);
        return NULL;
    }

    m->num_vertices = num_vertices;
    return m;
}

int matriz_grafo_inserir_aresta(MatrizGrafo *m, int origem, int destino) {
    if (m == NULL || origem < 0 || origem >= m->num_vertices || destino < 0 || destino >= m->num_vertices) {
        return 0;
    }

    size_t indice = (size_t)origem * (size_t)m->num_vertices + (size_t)destino;
    m->matriz[indice] = 1;
    return 1;
}

int matriz_grafo_existe_aresta(const MatrizGrafo *m, int origem, int destino) {
    if (m == NULL || origem < 0 || origem >= m->num_vertices || destino < 0 || destino >= m->num_vertices) {
        return 0;
    }

    size_t indice = (size_t)origem * (size_t)m->num_vertices + (size_t)destino;
    return m->matriz[indice] != 0;
}

int matriz_grafo_num_vertices(const MatrizGrafo *m) {
    if (m == NULL) return 0;
    return m->num_vertices;
}

void matriz_grafo_liberar(MatrizGrafo *m) {
    if (m == NULL) {
        return;
    }
    mem_free(m->matriz);
    mem_free(m);
}