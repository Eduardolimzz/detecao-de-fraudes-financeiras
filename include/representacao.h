#ifndef REPRESENTACAO_H
#define REPRESENTACAO_H

#include <stddef.h>

/*
 * Camada de abstração sobre as duas representações de grafo (RF02).
 *
 * Os algoritmos (BFS, DFS, Bellman-Ford, Hamiltoniano...) enxergam o tipo
 * RepGrafo e as funções rep_*. Suporta arestas ponderadas (pesos positivos,
 * zero e negativos), mantendo a diferenciação de peso zero e ausência de aresta.
 */

typedef enum {
    REPR_LISTA,
    REPR_MATRIZ
} TipoRepr;

typedef struct RepGrafo RepGrafo;

/* Iterador sobre os vizinhos de saída de um vértice. Permite recuperar
 * tanto o índice do vértice destino quanto o peso associado da aresta. */
typedef struct {
    const RepGrafo *g;
    int vertice;
    const void *no_atual; /* próximo nó (lista) */
    int coluna_atual;     /* próxima coluna (matriz) */
} IteradorVizinhos;

/* Converte "lista" / "matriz" em TipoRepr. Retorna 1 se válido, 0 se não. */
int rep_tipo_de_texto(const char *texto, TipoRepr *tipo_out);

/* Nome legível do tipo ("lista" ou "matriz"), usado em logs. */
const char *rep_tipo_nome(TipoRepr tipo);

/* Cria um grafo dirigido com exatamente num_vertices vértices (0..n-1).
 * Retorna NULL em caso de erro (inclusive matriz acima do limite). */
RepGrafo *rep_criar(TipoRepr tipo, int num_vertices);

/* Insere a transação origem -> destino com seu respectivo peso (w).
 * - Lista: guarda cada transação (arestas paralelas e auto-laços incluídos).
 * - Matriz: retém a aresta de menor peso w ao encontrar paralelas.
 * Retorna 1 se ok, 0 se vértice inválido ou falta de memória. */
int rep_inserir_aresta(RepGrafo *g, int origem, int destino, double peso);

/* Retorna 1 se existe ao menos uma aresta origem -> destino, 0 caso contrário. */
int rep_existe_aresta(const RepGrafo *g, int origem, int destino);

/* Consulta o peso da aresta origem -> destino.
 * Retorna 1 se a aresta existe e grava o peso em *peso_out.
 * Retorna 0 se a aresta não existir. */
int rep_obter_peso(const RepGrafo *g, int origem, int destino, double *peso_out);

TipoRepr rep_tipo(const RepGrafo *g);
int rep_num_vertices(const RepGrafo *g);

/* Número de transações/arestas inseridas com sucesso. */
int rep_num_arestas(const RepGrafo *g);

void rep_iter_inicio(IteradorVizinhos *it, const RepGrafo *g, int vertice);

/* Devolve 1 e grava o próximo vizinho em *destino_out e seu peso em *peso_out.
 * Se peso_out for NULL, o peso é ignorado. Devolve 0 quando a iteração acaba. */
int rep_iter_proximo(IteradorVizinhos *it, int *destino_out, double *peso_out);

void rep_liberar(RepGrafo *g);

#endif