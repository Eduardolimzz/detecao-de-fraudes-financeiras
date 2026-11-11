#ifndef BELLMAN_FORD_H
#define BELLMAN_FORD_H

#include "label_map.h"
#include "representacao.h"
#include <math.h>

/*
 * Bellman-Ford para ciclos de peso negativo (Fase II). Modelagem em
 * docs/adr/0004-modelagem-ponderada-bellman-ford.md.
 *
 * O vértice é o estado "banco:conta:moeda" e cada transação é uma aresta com
 * peso w = -ln(recebido / pago). Um ciclo de peso total negativo tem produto
 * de taxas maior que 1: é um CANDIDATO a arbitragem, não uma classificação
 * de fraude. Complexidade: O(|V| x |E|) no pior caso.
 */

/* Ciclos com peso total entre -tolerancia e 0 são ruído de ponto flutuante. */
#define BF_TOLERANCIA_PADRAO 1e-9

/* Aresta ponderada entre estados (um registro por transação). */
typedef struct {
    int origem;        /* índice do estado de origem (banco, conta, moeda paga) */
    int destino;       /* índice do estado de destino (banco, conta, moeda recebida) */
    double peso;       /* w = -ln(r) */
    int id_transacao;  /* ordem da transação válida no CSV (1 = primeira); 0 = desconhecida */
} ArestaPonderada;

/* Um ciclo negativo. Para 0 <= k < comprimento, a k-ésima transação do
 * ciclo vai do estado vertice_indices[k] ao estado
 * vertice_indices[(k + 1) % comprimento]. Um laço negativo tem
 * comprimento 1. */
typedef struct {
    char **rotulos_estado;  /* rótulos "banco:conta:moeda" (NULL se não houver mapa) */
    int *vertice_indices;   /* sequência dos estados do ciclo */
    int comprimento;
    int *id_transacoes;     /* id_transacao de cada transação do ciclo (0 = desconhecida) */
    double *pesos;          /* peso w de cada transação do ciclo */
    double peso_total;      /* soma dos pesos; menor que -tolerancia */
} CicloNegativo;

typedef struct {
    CicloNegativo *itens;
    int n_ciclos;               /* 0 ou 1: um ciclo é reconstruído por execução */
    int possui_ciclo_negativo;
    int iteracoes;              /* passadas executadas sobre as arestas */
    double tolerancia;          /* tolerância usada na execução */
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

/* Executa Bellman-Ford sobre um vetor de arestas, com BF_TOLERANCIA_PADRAO.
 * Acha ciclos negativos em qualquer componente, alcançável ou não.
 * rotulos_estado pode ser NULL. Retorna NULL em argumentos inválidos ou
 * falta de memória. */
ResultadoBellmanFord *bellman_ford_executar(int num_vertices,
                                            const ArestaPonderada *arestas,
                                            int num_arestas,
                                            const LabelMap *rotulos_estado);

/* Igual a bellman_ford_executar, com tolerância explícita (>= 0). */
ResultadoBellmanFord *bellman_ford_executar_tol(int num_vertices,
                                                const ArestaPonderada *arestas,
                                                int num_arestas,
                                                const LabelMap *rotulos_estado,
                                                double tolerancia);

/* Executa Bellman-Ford sobre um RepGrafo (lista ou matriz). Na matriz as
 * paralelas já foram reduzidas à de menor peso, o que não muda o resultado.
 * Os ids das transações ficam 0 até bellman_ford_anotar_transacoes. */
ResultadoBellmanFord *bellman_ford_executar_rep(const RepGrafo *g,
                                                const LabelMap *rotulos_estado,
                                                double tolerancia);

/* Preenche id_transacoes buscando em 'arestas' a transação de mesma origem,
 * destino e peso de cada passo do ciclo. */
void bellman_ford_anotar_transacoes(ResultadoBellmanFord *res,
                                    const ArestaPonderada *arestas,
                                    int num_arestas);

/* Cria um RepGrafo de estados a partir das arestas ponderadas. Retorna NULL
 * se a representação não comportar o grafo. */
RepGrafo *bellman_ford_construir_grafo(TipoRepr tipo, int num_vertices,
                                       const ArestaPonderada *arestas,
                                       int num_arestas);

/* Imprime complexidade, passadas e, se houver, o ciclo candidato com cada
 * estado, transação, peso e taxa. */
void bellman_ford_imprimir_resultado(const ResultadoBellmanFord *res,
                                     int num_vertices, int num_arestas);

void bellman_ford_liberar(ResultadoBellmanFord *res);

#endif /* BELLMAN_FORD_H */
