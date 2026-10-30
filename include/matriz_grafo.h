#ifndef MATRIZ_GRAFO_H
#define MATRIZ_GRAFO_H

#include <stddef.h>

/* Limite máximo de vértices para evitar estouro de memória com a matriz V x V */
#define MAX_VERTICES_MATRIZ 50000

typedef struct MatrizGrafo MatrizGrafo;

/**
 * Cria uma matriz de adjacência para num_vertices vértices.
 * @param num_vertices Número total de vértices (0..n-1).
 * @return Ponteiro para a MatrizGrafo alocada ou NULL em caso de erro/limite excedido.
 */
MatrizGrafo *matriz_grafo_criar(int num_vertices);

/**
 * Insere a aresta origem -> destino com o peso especificado.
 * Se a aresta já existir, retém a de menor peso w (w_novo < w_atual).
 * @param m Ponteiro para a matriz de adjacência.
 * @param origem Vértice de origem.
 * @param destino Vértice de destino.
 * @param peso Valor numérico do peso da aresta (w = -ln(r)).
 * @return 1 em caso de sucesso, 0 se os vértices forem inválidos ou m for NULL.
 */
int matriz_grafo_inserir_aresta(MatrizGrafo *m, int origem, int destino, double peso);

/**
 * Retorna 1 se existe a aresta origem -> destino, 0 caso contrário.
 * @param m Ponteiro para a matriz.
 * @param origem Vértice de origem.
 * @param destino Vértice de destino.
 * @return 1 se a aresta existir, 0 se não existir ou se os índices forem inválidos.
 */
int matriz_grafo_existe_aresta(const MatrizGrafo *m, int origem, int destino);

/**
 * Consulta o peso da aresta origem -> destino.
 * @param m Ponteiro para a matriz.
 * @param origem Vértice de origem.
 * @param destino Vértice de destino.
 * @param peso_out Ponteiro onde o peso da aresta será gravado (pode ser NULL).
 * @return 1 se a aresta existir (e grava o peso em *peso_out), 0 caso contrário.
 */
int matriz_grafo_obter_peso(const MatrizGrafo *m, int origem, int destino, double *peso_out);

/**
 * Retorna o número de vértices da matriz.
 */
int matriz_grafo_num_vertices(const MatrizGrafo *m);

/**
 * Libera toda a memória associada à matriz de adjacência.
 */
void matriz_grafo_liberar(MatrizGrafo *m);

#endif