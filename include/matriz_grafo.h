#ifndef MATRIZ_GRAFO_H
#define MATRIZ_GRAFO_H

#include <stddef.h>


#define MAX_VERTICES_MATRIZ 50000

typedef struct MatrizGrafo MatrizGrafo;

/**
 * @param num_vertices 
 * @return 
 */
MatrizGrafo *matriz_grafo_criar(int num_vertices);

/**
 * @param m 
 * @param origem 
 * @param destino 
 * @return
 */
int matriz_grafo_inserir_aresta(MatrizGrafo *m, int origem, int destino);

/**
 * @param m 
 * @param origem 
 * @param destino 
 * @return 
 */
int matriz_grafo_existe_aresta(const MatrizGrafo *m, int origem, int destino);


int matriz_grafo_num_vertices(const MatrizGrafo *m);


void matriz_grafo_liberar(MatrizGrafo *m);

#endif 