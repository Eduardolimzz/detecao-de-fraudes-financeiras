#ifndef CLI_H
#define CLI_H

#include "representacao.h"

#define DATASET_PADRAO "data/dataset.csv"

/* Opções de linha de comando do programa. */
typedef struct {
    TipoRepr repr;       /* --repr=lista | --repr=matriz (padrão: lista) */
    const char *dataset; /* --dataset=CAMINHO (padrão: data/dataset.csv) */
    int limite;          /* --limit=N: subgrafo com os N primeiros vértices
                            (0 = grafo completo, o padrão) */
    int pedir_ajuda;     /* --help */
} Opcoes;

/* Preenche *op com os valores padrão. */
void cli_opcoes_padrao(Opcoes *op);

/* Interpreta argv. Retorna 1 se tudo ok, 0 se alguma opção for inválida
 * (a mensagem de erro já é impressa em stderr). */
int cli_interpretar(int argc, char **argv, Opcoes *op);

void cli_imprimir_uso(const char *nome_programa);

#endif