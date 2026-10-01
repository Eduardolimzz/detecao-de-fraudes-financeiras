#ifndef CARREGADOR_H
#define CARREGADOR_H

#include <stddef.h>

#include "label_map.h"
#include "representacao.h"

/*
 * Pipeline de carga do grafo a partir do CSV de transações (RF01).
 *
 * Segue o ADR 0001: cada conta é o vértice "banco:conta" (texto, preservando
 * zeros à esquerda) e cada linha do CSV vira uma aresta dirigida
 * origem -> destino. Auto-laços e transações repetidas são mantidos.
 * A coluna "Is Laundering" NÃO é lida: o rótulo não entra na topologia.
 *
 * A carga é feita em duas passadas, porque a matriz de adjacência precisa
 * saber |V| antes de ser criada:
 *   1. lê o CSV, atribui um índice a cada conta (LabelMap) e guarda as
 *      arestas em um vetor temporário;
 *   2. cria o grafo com |V| exato e insere as arestas.
 */

/* Posições das colunas no dataset (as duas colunas "Account" só se
 * distinguem pela posição: a primeira é origem, a segunda é destino). */
#define COL_BANCO_ORIGEM 1
#define COL_CONTA_ORIGEM 2
#define COL_BANCO_DESTINO 3
#define COL_CONTA_DESTINO 4
#define COL_VALOR_PAGO 7
#define NUM_COLUNAS_MINIMO 11

typedef struct {
    RepGrafo *grafo;
    LabelMap *rotulos;         /* índice <-> "banco:conta" (todas as contas do CSV) */
    int n_vertices;            /* |V| do grafo construído (subgrafo, se houver --limit) */
    int n_arestas;             /* |E| do grafo construído */
    int limite;                /* valor de --limit (0 = sem limite) */
    int n_vertices_total;      /* |V| do CSV completo */
    int n_arestas_total;       /* transações válidas do CSV completo */
    int linhas_ignoradas;      /* malformadas ou com menos colunas que o esperado */
    double tempo_ms;           /* tempo total da carga (leitura + construção) */
    size_t bytes_estruturas;   /* bytes vivos do grafo + rótulos após a carga */
    size_t bytes_pico;         /* pico durante a carga (inclui o vetor temporário) */
    size_t bytes_grafo;        /* só lista/matriz, sem rótulos nem vetor temporário */
} GrafoCarregado;

typedef enum {
    CARGA_OK = 0,
    CARGA_ARQUIVO_INEXISTENTE,
    CARGA_SEM_PERMISSAO,
    CARGA_SEM_DADOS,       /* nenhuma linha válida */
    CARGA_SEM_MEMORIA,
    CARGA_LIMITE_REPR      /* representação não comporta |V| (matriz) */
} StatusCarga;

/* Carrega o CSV em 'caminho' usando a representação pedida.
 * limite > 0: constrói o subgrafo induzido pelos 'limite' primeiros vértices
 * (ordem de primeira aparição no CSV), para os testes de estresse.
 * limite == 0: grafo completo. limite > |V| total equivale ao grafo completo.
 * Em caso de erro devolve o status; mesmo assim, chame carregador_liberar. */
StatusCarga carregador_carregar(const char *caminho, TipoRepr repr, int limite,
                                GrafoCarregado *saida);
                                
/* Texto curto em português para um StatusCarga. */
const char *carregador_status_texto(StatusCarga status);

/* Imprime |V|, |E|, tempo de carga e memória consumida. */
void carregador_imprimir_sumario(const GrafoCarregado *c, TipoRepr repr);

void carregador_liberar(GrafoCarregado *c);

#endif
