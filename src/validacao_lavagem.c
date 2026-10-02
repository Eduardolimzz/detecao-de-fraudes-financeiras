#include "validacao_lavagem.h"

#include "carregador.h"
#include "csv_parser.h"
#include "mem_track.h"

#include <stdio.h>
#include <string.h>

#define TAMANHO_CHAVE 256
#define COL_IS_LAUNDERING 10
#define CAPACIDADE_INICIAL 32

typedef struct {
    char origem[TAMANHO_CHAVE];
    char destino[TAMANHO_CHAVE];
} ParLavagem;

struct ConjuntoLavagem {
    ParLavagem *itens;
    int n;
    int capacidade;
};

static int montar_chave(char *chave, size_t tam, const char *banco, const char *conta) {
    int n = snprintf(chave, tam, "%s:%s", banco, conta);
    return n > 0 && (size_t)n < tam;
}

static int adicionar(ConjuntoLavagem *conj, const char *origem, const char *destino) {
    if (conj->n >= conj->capacidade) {
        int nova_cap = conj->capacidade * 2;
        ParLavagem *novo = mem_realloc(conj->itens, (size_t)nova_cap * sizeof(ParLavagem));
        if (novo == NULL) {
            return 0;
        }
        conj->itens = novo;
        conj->capacidade = nova_cap;
    }
    strcpy(conj->itens[conj->n].origem, origem);
    strcpy(conj->itens[conj->n].destino, destino);
    conj->n++;
    return 1;
}

ConjuntoLavagem *validacao_lavagem_carregar(const char *caminho_csv) {
    int erro = 0;
    CsvParser *parser = csv_parser_abrir(caminho_csv, &erro);
    if (parser == NULL) {
        return NULL;
    }

    ConjuntoLavagem *conj = mem_malloc(sizeof(ConjuntoLavagem));
    if (conj == NULL) {
        csv_parser_fechar(parser);
        return NULL;
    }
    conj->itens = mem_malloc(CAPACIDADE_INICIAL * sizeof(ParLavagem));
    conj->n = 0;
    conj->capacidade = CAPACIDADE_INICIAL;
    if (conj->itens == NULL) {
        mem_free(conj);
        csv_parser_fechar(parser);
        return NULL;
    }

    int falhou = 0;
    LinhaCSV linha;
    char chave_origem[TAMANHO_CHAVE];
    char chave_destino[TAMANHO_CHAVE];

    while (!falhou && csv_parser_proxima_linha(parser, &linha)) {
        if (linha.n_campos > COL_IS_LAUNDERING &&
            strcmp(linha.campos[COL_IS_LAUNDERING], "1") == 0 &&
            montar_chave(chave_origem, sizeof(chave_origem),
                        linha.campos[COL_BANCO_ORIGEM], linha.campos[COL_CONTA_ORIGEM]) &&
            montar_chave(chave_destino, sizeof(chave_destino),
                        linha.campos[COL_BANCO_DESTINO], linha.campos[COL_CONTA_DESTINO])) {
            falhou = !adicionar(conj, chave_origem, chave_destino);
        }
        csv_parser_liberar_linha(&linha);
    }

    csv_parser_fechar(parser);
    if (falhou) {
        validacao_lavagem_liberar(conj);
        return NULL;
    }
    return conj;
}

int validacao_lavagem_tamanho(const ConjuntoLavagem *conj) {
    return conj->n;
}

int validacao_lavagem_contem_aresta(const ConjuntoLavagem *conj,
                                    const char *origem, const char *destino) {
    for (int i = 0; i < conj->n; i++) {
        if (strcmp(conj->itens[i].origem, origem) == 0 &&
            strcmp(conj->itens[i].destino, destino) == 0) {
            return 1;
        }
    }
    return 0;
}

int validacao_lavagem_ciclo_suspeito(const ConjuntoLavagem *conj, const Ciclo *ciclo) {
    for (int i = 0; i < ciclo->comprimento; i++) {
        const char *origem = ciclo->rotulos[i];
        const char *destino = ciclo->rotulos[(i + 1) % ciclo->comprimento];
        if (validacao_lavagem_contem_aresta(conj, origem, destino)) {
            return 1;
        }
    }
    return 0;
}

void validacao_lavagem_liberar(ConjuntoLavagem *conj) {
    if (conj == NULL) {
        return;
    }
    mem_free(conj->itens);
    mem_free(conj);
}
