#include "matriz_grafo.h"
#include "mem_track.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct MatrizGrafo {
    double *pesos;           /* Vetor contíguo V x V de pesos (double) */
    unsigned char *existe;  /* Vetor contíguo V x V de presença (1 = existe, 0 = ausente) */
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
                ((double)num_vertices * (double)num_vertices * (sizeof(double) + sizeof(unsigned char))) / (1024.0 * 1024.0 * 1024.0));
        return NULL;
    }

    MatrizGrafo *m = mem_malloc(sizeof(MatrizGrafo));
    if (m == NULL) {
        return NULL;
    }

    size_t total_elementos = (size_t)num_vertices * (size_t)num_vertices;
    
    m->pesos = mem_calloc(total_elementos, sizeof(double));
    m->existe = mem_calloc(total_elementos, sizeof(unsigned char));

    if (m->pesos == NULL || m->existe == NULL) {
        fprintf(stderr, "Erro: Falha ao alocar memória para a matriz de adjacência ponderada.\n");
        if (m->pesos) mem_free(m->pesos);
        if (m->existe) mem_free(m->existe);
        mem_free(m);
        return NULL;
    }

    m->num_vertices = num_vertices;
    return m;
}

int matriz_grafo_inserir_aresta(MatrizGrafo *m, int origem, int destino, double peso) {
    if (m == NULL || origem < 0 || origem >= m->num_vertices || destino < 0 || destino >= m->num_vertices) {
        return 0;
    }

    size_t idx = (size_t)origem * (size_t)m->num_vertices + (size_t)destino;

    /* Regra para paralelas na matriz: retém a aresta de menor peso w (maior retorno) */
    if (!m->existe[idx] || peso < m->pesos[idx]) {
        m->pesos[idx] = peso;
        m->existe[idx] = 1;
    }

    return 1;
}

int matriz_grafo_existe_aresta(const MatrizGrafo *m, int origem, int destino) {
    if (m == NULL || origem < 0 || origem >= m->num_vertices || destino < 0 || destino >= m->num_vertices) {
        return 0;
    }

    size_t idx = (size_t)origem * (size_t)m->num_vertices + (size_t)destino;
    return m->existe[idx] != 0;
}

int matriz_grafo_obter_peso(const MatrizGrafo *m, int origem, int destino, double *peso_out) {
    if (m == NULL || origem < 0 || origem >= m->num_vertices || destino < 0 || destino >= m->num_vertices) {
        return 0;
    }

    size_t idx = (size_t)origem * (size_t)m->num_vertices + (size_t)destino;
    if (m->existe[idx]) {
        if (peso_out != NULL) {
            *peso_out = m->pesos[idx];
        }
        return 1;
    }
    return 0;
}

int matriz_grafo_num_vertices(const MatrizGrafo *m) {
    if (m == NULL) return 0;
    return m->num_vertices;
}

void matriz_grafo_liberar(MatrizGrafo *m) {
    if (m == NULL) {
        return;
    }
    if (m->pesos != NULL) {
        mem_free(m->pesos);
    }
    if (m->existe != NULL) {
        mem_free(m->existe);
    }
    mem_free(m);
}