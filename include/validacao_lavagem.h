#ifndef VALIDACAO_LAVAGEM_H
#define VALIDACAO_LAVAGEM_H

#include "deteccao_ciclos.h"

/*
 * Validação SEMÂNTICA (não apenas estrutural) da detecção de ciclos:
 * verifica se os ciclos encontrados coincidem com transações realmente
 * marcadas como lavagem (coluna "Is Laundering"=1) no CSV de origem.
 *
 * Reaproveita o mesmo csv_parser.h e as colunas já definidas em
 * carregador.h — só acrescenta a leitura da última coluna, que o
 * carregador ignora de propósito (ADR 0001: o rótulo não entra na
 * topologia do grafo).
 */

typedef struct ConjuntoLavagem ConjuntoLavagem;

/* Lê 'caminho_csv' e guarda todo par (origem, destino) com
 * Is Laundering=1, nos mesmos rótulos "banco:conta" usados no grafo.
 * Retorna NULL em caso de erro de leitura ou falta de memória. */
ConjuntoLavagem *validacao_lavagem_carregar(const char *caminho_csv);

/* Quantas transações distintas estão marcadas como lavagem. */
int validacao_lavagem_tamanho(const ConjuntoLavagem *conj);

/* 1 se existe uma transação origem -> destino marcada como lavagem. */
int validacao_lavagem_contem_aresta(const ConjuntoLavagem *conj,
                                    const char *origem, const char *destino);

/* 1 se QUALQUER aresta do ciclo (incluindo a que fecha o ciclo, do último
 * rótulo de volta ao primeiro) está marcada como lavagem. */
int validacao_lavagem_ciclo_suspeito(const ConjuntoLavagem *conj, const Ciclo *ciclo);

void validacao_lavagem_liberar(ConjuntoLavagem *conj);

#endif
