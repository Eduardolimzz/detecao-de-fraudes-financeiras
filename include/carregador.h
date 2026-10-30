#ifndef CARREGADOR_H
#define CARREGADOR_H

#include <stddef.h>

#include "bellman_ford.h"
#include "label_map.h"
#include "representacao.h"

/* Posições das colunas no dataset do projeto */
#define COL_BANCO_ORIGEM 1
#define COL_CONTA_ORIGEM 2
#define COL_BANCO_DESTINO 3
#define COL_CONTA_DESTINO 4
#define COL_MOEDA_PAGA 5
#define COL_VALOR_PAGO 7
#define COL_MOEDA_RECEBIDA 8
#define COL_VALOR_RECEBIDO 9
#define COL_IS_LAUNDERING 10
#define NUM_COLUNAS_MINIMO 11

typedef struct {
    RepGrafo *grafo;
    LabelMap *rotulos;         /* índice <-> "banco:conta" (Fase I) */

    /* Estruturas ponderadas por estado para Bellman-Ford (Fase II) */
    LabelMap *rotulos_estado;  /* índice <-> "banco:conta:moeda" */
    ArestaPonderada *arestas_bf; /* vetor de arestas com peso w = -ln(r) */
    int n_arestas_bf;
    int n_vertices_estado;

    int n_vertices;            /* |V| do grafo construído */
    int n_arestas;             /* |E| do grafo construído */
    int limite;                /* valor de --limit (0 = sem limite) */
    int n_vertices_total;      /* |V| do CSV completo */
    int n_arestas_total;       /* transações válidas do CSV completo */
    int linhas_ignoradas;      /* malformadas ou com menos colunas */
    double tempo_ms;           /* tempo total da carga */
    size_t bytes_estruturas;   /* bytes vivos após a carga */
    size_t bytes_pico;         /* pico durante a carga */
    size_t bytes_grafo;        /* só lista/matriz */
} GrafoCarregado;

typedef enum {
    CARGA_OK = 0,
    CARGA_ARQUIVO_INEXISTENTE,
    CARGA_SEM_PERMISSAO,
    CARGA_SEM_DADOS,       /* nenhuma linha válida */
    CARGA_SEM_MEMORIA,
    CARGA_LIMITE_REPR      /* representação não comporta |V| */
} StatusCarga;

StatusCarga carregador_carregar(const char *caminho, TipoRepr repr, int limite,
                                GrafoCarregado *saida);

const char *carregador_status_texto(StatusCarga status);
void carregador_imprimir_sumario(const GrafoCarregado *c, TipoRepr repr);
void carregador_liberar(GrafoCarregado *c);

#endif