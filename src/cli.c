#include "cli.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PREFIXO_REPR "--repr="
#define PREFIXO_DATASET "--dataset="
#define PREFIXO_LIMIT "--limit="

void cli_opcoes_padrao(Opcoes *op) {
    op->repr = REPR_LISTA;
    op->dataset = DATASET_PADRAO;
    op->limite = 0;
    op->pedir_ajuda = 0;
}

/* Converte o texto de --limit em inteiro positivo. Só aceita dígitos
 * (rejeita vazio, sinal, espaços, "5x", zero e overflow). */
static int interpretar_limite(const char *texto, int *limite_out) {
    if (!isdigit((unsigned char)texto[0])) {
        return 0;
    }
    char *fim = NULL;
    errno = 0;
    long v = strtol(texto, &fim, 10);
    if (errno != 0 || *fim != '\0' || v <= 0 || v > INT_MAX) {
        return 0;
    }
    *limite_out = (int)v;
    return 1;
}

int cli_interpretar(int argc, char **argv, Opcoes *op) {
    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];

        if (strncmp(arg, PREFIXO_REPR, strlen(PREFIXO_REPR)) == 0) {
            const char *valor = arg + strlen(PREFIXO_REPR);
            if (!rep_tipo_de_texto(valor, &op->repr)) {
                fprintf(stderr, "Erro: valor inválido para --repr: '%s' "
                                "(use 'lista' ou 'matriz').\n", valor);
                return 0;
            }
        } else if (strncmp(arg, PREFIXO_DATASET, strlen(PREFIXO_DATASET)) == 0) {
            op->dataset = arg + strlen(PREFIXO_DATASET);
            if (op->dataset[0] == '\0') {
                fprintf(stderr, "Erro: --dataset exige um caminho.\n");
                return 0;
            }
        } else if (strncmp(arg, PREFIXO_LIMIT, strlen(PREFIXO_LIMIT)) == 0) {
            const char *valor = arg + strlen(PREFIXO_LIMIT);
            if (!interpretar_limite(valor, &op->limite)) {
                fprintf(stderr, "Erro: valor inválido para --limit: '%s' "
                                "(use um inteiro positivo).\n", valor);
                return 0;
            }
        } else if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            op->pedir_ajuda = 1;
        } else {
            fprintf(stderr, "Erro: opção desconhecida: '%s'.\n", arg);
            return 0;
        }
    }
    return 1;
}

void cli_imprimir_uso(const char *nome_programa) {
    printf("Uso: %s [opções]\n\n", nome_programa);
    printf("Opções:\n");
    printf("  --repr=lista|matriz   representação do grafo (padrão: lista)\n");
    printf("  --dataset=CAMINHO     CSV de transações (padrão: %s)\n", DATASET_PADRAO);
    printf("  --limit=N             subgrafo induzido pelos N primeiros vértices\n"
           "                        (ordem de primeira aparição no CSV; padrão:\n"
           "                        grafo completo)\n");
    printf("  -h, --help            mostra esta ajuda\n");
}