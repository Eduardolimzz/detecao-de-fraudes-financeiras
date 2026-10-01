#ifndef REPRESENTACAO_H
#define REPRESENTACAO_H

/*
 * Camada de abstração sobre as duas representações de grafo (RF02).
 *
 * Os algoritmos (BFS, DFS, ciclos...) só enxergam o tipo RepGrafo e as
 * funções rep_*. A escolha entre lista e matriz de adjacência é feita uma
 * única vez, em rep_criar(), normalmente a partir da flag --repr.
 *
 * Decisão: enum + switch (em vez de ponteiros de função), por ser mais
 * simples de ler e depurar. O número de vértices é fixado na criação.
 */

typedef enum {
    REPR_LISTA,
    REPR_MATRIZ
} TipoRepr;

typedef struct RepGrafo RepGrafo;

/* Iterador sobre os vizinhos de saída de um vértice. Pode ser pausado e
 * retomado, o que permite escrever DFS iterativo. Não deve ser alterado
 * diretamente: use apenas rep_iter_inicio e rep_iter_proximo. */
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

/* Insere a transação origem -> destino.
 * Lista: guarda cada transação (arestas paralelas e auto-laços incluídos).
 * Matriz: marca apenas a conectividade; o valor é ignorado e transações
 * repetidas colapsam na mesma célula (ver ADR 0001).
 * Retorna 1 se ok, 0 se vértice inválido ou falta de memória. */
int rep_inserir_aresta(RepGrafo *g, int origem, int destino, double valor);

/* Retorna 1 se existe ao menos uma aresta origem -> destino. */
int rep_existe_aresta(const RepGrafo *g, int origem, int destino);

TipoRepr rep_tipo(const RepGrafo *g);
int rep_num_vertices(const RepGrafo *g);

/* Número de transações inseridas com sucesso. É igual nas duas
 * representações, mesmo que a matriz colapse arestas paralelas. */
int rep_num_arestas(const RepGrafo *g);

void rep_iter_inicio(IteradorVizinhos *it, const RepGrafo *g, int vertice);

/* Devolve 1 e grava o próximo vizinho em *destino_out, ou 0 quando acabou.
 * Na lista, um vizinho com arestas paralelas aparece várias vezes. */
int rep_iter_proximo(IteradorVizinhos *it, int *destino_out);

void rep_liberar(RepGrafo *g);

#endif
