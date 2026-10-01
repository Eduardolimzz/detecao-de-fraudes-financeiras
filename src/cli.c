#include "cli.h"

#include <stdio.h>
#include <string.h>

#define PREFIXO_REPR "--repr="
#define PREFIXO_DATASET "--dataset="

void cli_opcoes_padrao(Opcoes *op) {
    op->repr = REPR_LISTA;
    op->dataset = DATASET_PADRAO;
    op->pedir_ajuda = 0;
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
    printf("  -h, --help            mostra esta ajuda\n");
}
