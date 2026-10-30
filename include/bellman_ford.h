#ifndef BELLMAN_FORD_H
#define BELLMAN_FORD_H

#include "label_map.h"
#include "representacao.h"
#include <math.h>

/* Estrutura para representação de aresta ponderada por estado (Bellman-Ford) */
typedef struct {
    int origem;     /* ID do nó de estado (banco, conta, moeda) */
    int destino;    /* ID do nó de estado destino */
    double peso;    /* w = -ln(r) */
} ArestaPonderada;

typedef struct {
    char **rotulos_estado;  /* rótulos originais "banco:conta:moeda" */
    int *vertice_indices;   /* sequência dos vértices do ciclo negativo */
    int comprimento;
} CicloNegativo;

typedef struct {
    CicloNegativo *itens;
    int n_ciclos;
    int possui_ciclo_negativo;
} ResultadoBellmanFord;

/**
 * Calcula o peso logarítmico w = -ln(Amount Received / Amount Paid).
 * Retorna INFINITY para entradas não positivas/inválidas.
 */
static inline double calcular_peso_transacao(double amount_paid, double amount_received) {
    if (amount_paid <= 0.0 || amount_received <= 0.0) {
        return INFINITY;
    }
    double r = amount_received / amount_paid;
    if (!isfinite(r) || r <= 0.0) {
        return INFINITY;
    }
    return -log(r);
}

/**
 * Executa o algoritmo de Bellman-Ford para detectar e reconstruir ciclos negativos.
 * Usa os rótulos de estado para tradução visual nos resultados.
 */
ResultadoBellmanFord *bellman_ford_executar(int num_vertices, 
                                            const ArestaPonderada *arestas, 
                                            int num_arestas, 
                                            const LabelMap *rotulos_estado);

void bellman_ford_liberar(ResultadoBellmanFord *res);

#endif /* BELLMAN_FORD_H */