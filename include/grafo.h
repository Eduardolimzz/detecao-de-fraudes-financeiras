#ifndef GRAFO_H
#define GRAFO_H

#include <stddef.h>


typedef struct ArestaNode {
    int destino;             
    double valor;            
    struct ArestaNode *proximo;
} ArestaNode;


typedef struct Grafo Grafo;

/**
 * @param num_vertices_inicial 
 * @return 
 */
Grafo *grafo_criar(int num_vertices_inicial);

/**
 * @param g 
 * @param origem
 * @param destino 
 * @param valor 
 * @return 
 */
int grafo_inserir_aresta(Grafo *g, int origem, int destino, double valor);

/**
 * @param g 
 * @param vertice 
 * @return 
 */
int grafo_grau_saida(const Grafo *g, int vertice);

/**
 * @param g 
 * @param vertice 
 * @return 
 */
const ArestaNode *grafo_obter_adjacentes(const Grafo *g, int vertice);


int grafo_num_vertices(const Grafo *g);

int grafo_num_arestas(const Grafo *g);

void grafo_liberar(Grafo *g);

#endif 