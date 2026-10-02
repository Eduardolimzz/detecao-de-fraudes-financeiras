#ifndef DFS_H
#define DFS_H

#include "representacao.h"

/*
 * Busca em profundidade (DFS) sobre a camada de abstração RepGrafo.
 *
 * Implementação ITERATIVA, com pilha explícita alocada no heap — nunca
 * recursão. O dataset real tem ~32.386 vértices; uma chamada recursiva por
 * vértice visitado arriscaria estourar a stack do processo (o tamanho da
 * pilha de chamadas do SO é limitado e não é controlado pelo programa),
 * especialmente em grafos com uma longa cadeia de dependência. A pilha
 * explícita cresce no heap, sem esse limite.
 *
 * Cada frame da pilha guarda um IteradorVizinhos (ver representacao.h),
 * que foi desenhado justamente para ser pausado e retomado: ao encontrar
 * um vizinho branco, o frame atual fica parado no meio da iteração
 * enquanto o novo vértice é explorado, e é retomado de onde parou quando a
 * subárvore volta.
 */

typedef enum {
    DFS_BRANCO, /* ainda não descoberto */
    DFS_CINZA,  /* descoberto, com descendentes ainda sendo explorados */
    DFS_PRETO   /* descoberto e totalmente explorado */
} CorDFS;

/* Uma aresta de retorno (u -> v) encontrada durante a busca: v estava
 * CINZA no momento em que u tentou visitá-lo, ou seja, v é ancestral de u
 * na árvore de DFS. É exatamente essa aresta que fecha um ciclo. */
typedef struct {
    int origem;
    int destino;
} ArestaRetornoDFS;

typedef struct {
    int n;
    int *descoberta;    /* tempo em que o vértice ficou CINZA, -1 se não alcançado */
    int *finalizacao;   /* tempo em que o vértice ficou PRETO, -1 se não alcançado */
    int *predecessor;   /* pai na árvore/floresta de DFS, -1 se raiz ou não alcançado */
    CorDFS *cor;        /* cor final de cada vértice (sempre BRANCO ou PRETO ao final) */
    ArestaRetornoDFS *arestas_retorno;
    int n_arestas_retorno;
} ResultadoDFS;

/* Percorre todos os vértices de g (floresta de DFS, cobre também vértices
 * desconectados), visitando-os em ordem crescente de índice como raiz de
 * uma nova árvore sempre que um vértice branco é encontrado.
 * Retorna NULL em caso de falta de memória. */
ResultadoDFS *dfs_executar(const RepGrafo *g);

void dfs_liberar(ResultadoDFS *r);

#endif
