#ifndef DETECCAO_CICLOS_H
#define DETECCAO_CICLOS_H

#include "label_map.h"
#include "representacao.h"

/*
 * Detecção de ciclos em grafo dirigido, construída sobre dfs_executar()
 * (esquema BRANCO/CINZA/PRETO): cada aresta de retorno encontrada pela DFS
 * fecha exatamente um ciclo, reconstruído andando pelo vetor de
 * predecessores até o destino da aresta de retorno.
 *
 * Os ciclos são devolvidos com os rótulos ORIGINAIS das contas
 * ("banco:conta"), não os índices internos da hash table, para que o
 * resultado seja legível fora do programa (relatório, comparação com o
 * gabarito de lavagem).
 */

typedef struct {
    char **rotulos;    /* rotulos[0..comprimento-1], na ordem do ciclo */
    int comprimento;   /* número de vértices distintos no ciclo (>= 1) */
} Ciclo;

typedef struct {
    Ciclo *itens;
    int n_ciclos;
} ResultadoCiclos;

/* Executa a DFS sobre g e reconstrói um Ciclo para cada aresta de retorno
 * encontrada. rotulos é usado só para traduzir índices em "banco:conta".
 * Retorna NULL em caso de falta de memória. */
ResultadoCiclos *deteccao_ciclos_executar(const RepGrafo *g, const LabelMap *rotulos);

void deteccao_ciclos_liberar(ResultadoCiclos *r);

#endif
