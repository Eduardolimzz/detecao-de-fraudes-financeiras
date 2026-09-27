#include "csv_parser.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAMANHO_BUFFER 4096
#define CAMPO_MAX (TAMANHO_BUFFER)
#define CAPACIDADE_INICIAL_CAMPOS 8

struct CsvParser {
    FILE *fp;
    int n_campos_esperado;
    int contador_erros;
    char buffer[TAMANHO_BUFFER];
};

static void remover_quebra_de_linha(char *linha) {
    size_t len = strlen(linha);
    while (len > 0 && (linha[len - 1] == '\n' || linha[len - 1] == '\r')) {
        linha[--len] = '\0';
    }
}

/* Se a linha lida com fgets nao terminou em quebra de linha e ainda nao
 * chegamos ao fim do arquivo, a linha fisica era maior que o buffer fixo.
 * Descarta o restante dela para nao desalinhar as proximas leituras. */
static void descartar_resto_da_linha(FILE *fp) {
    char resto[TAMANHO_BUFFER];
    while (fgets(resto, sizeof(resto), fp) != NULL) {
        if (strchr(resto, '\n') != NULL) {
            return;
        }
    }
}

static void liberar_campos_parciais(char **campos, int n) {
    for (int i = 0; i < n; i++) {
        free(campos[i]);
    }
    free(campos);
}

/*
 * Divide uma linha em campos respeitando aspas (campos entre aspas podem
 * conter virgulas internas; "" dentro de um campo com aspas representa
 * uma aspa literal). Retorna o numero de campos, ou -1 se a linha estiver
 * malformada (aspas nao fechadas ou lixo apos o fechamento das aspas).
 */
static int dividir_campos(const char *linha, char ***campos_out) {
    int capacidade = CAPACIDADE_INICIAL_CAMPOS;
    char **campos = malloc((size_t)capacidade * sizeof(char *));
    if (campos == NULL) {
        return -1;
    }
    int n = 0;

    size_t len = strlen(linha);
    size_t i = 0;
    char campo_buf[CAMPO_MAX];

    while (1) {
        size_t campo_len = 0;

        if (i < len && linha[i] == '"') {
            int fechou = 0;
            i++;
            while (i < len) {
                if (linha[i] == '"') {
                    if (i + 1 < len && linha[i + 1] == '"') {
                        if (campo_len < CAMPO_MAX - 1) {
                            campo_buf[campo_len++] = '"';
                        }
                        i += 2;
                    } else {
                        i++;
                        fechou = 1;
                        break;
                    }
                } else {
                    if (campo_len < CAMPO_MAX - 1) {
                        campo_buf[campo_len++] = linha[i];
                    }
                    i++;
                }
            }
            if (!fechou) {
                liberar_campos_parciais(campos, n);
                return -1;
            }
            if (i < len && linha[i] != ',') {
                liberar_campos_parciais(campos, n);
                return -1;
            }
        } else {
            while (i < len && linha[i] != ',') {
                if (campo_len < CAMPO_MAX - 1) {
                    campo_buf[campo_len++] = linha[i];
                }
                i++;
            }
        }
        campo_buf[campo_len] = '\0';

        if (n >= capacidade) {
            capacidade *= 2;
            char **novo = realloc(campos, (size_t)capacidade * sizeof(char *));
            if (novo == NULL) {
                liberar_campos_parciais(campos, n);
                return -1;
            }
            campos = novo;
        }
        campos[n] = malloc(campo_len + 1);
        if (campos[n] == NULL) {
            liberar_campos_parciais(campos, n);
            return -1;
        }
        memcpy(campos[n], campo_buf, campo_len + 1);
        n++;

        if (i < len && linha[i] == ',') {
            i++;
            continue;
        }
        break;
    }

    *campos_out = campos;
    return n;
}

CsvParser *csv_parser_abrir(const char *caminho, int *erro) {
    FILE *fp = fopen(caminho, "r");
    if (fp == NULL) {
        if (erro != NULL) {
            *erro = (errno == EACCES) ? 2 : 1;
        }
        return NULL;
    }

    CsvParser *p = malloc(sizeof(CsvParser));
    if (p == NULL) {
        fclose(fp);
        if (erro != NULL) {
            *erro = 1;
        }
        return NULL;
    }

    p->fp = fp;
    p->contador_erros = 0;
    p->n_campos_esperado = 0;

    if (fgets(p->buffer, TAMANHO_BUFFER, p->fp) != NULL) {
        remover_quebra_de_linha(p->buffer);
        char **campos_cabecalho = NULL;
        int n = dividir_campos(p->buffer, &campos_cabecalho);
        if (n > 0) {
            p->n_campos_esperado = n;
            liberar_campos_parciais(campos_cabecalho, n);
        }
    }

    if (erro != NULL) {
        *erro = 0;
    }
    return p;
}

int csv_parser_proxima_linha(CsvParser *p, LinhaCSV *linha_out) {
    while (fgets(p->buffer, TAMANHO_BUFFER, p->fp) != NULL) {
        int linha_truncada = (strchr(p->buffer, '\n') == NULL) && !feof(p->fp);
        remover_quebra_de_linha(p->buffer);

        if (p->buffer[0] == '\0') {
            continue; /* linha vazia: ignora silenciosamente, sem erro */
        }

        if (linha_truncada) {
            descartar_resto_da_linha(p->fp);
            p->contador_erros++;
            continue;
        }

        char **campos = NULL;
        int n = dividir_campos(p->buffer, &campos);

        if (n < 0 || n != p->n_campos_esperado) {
            if (n > 0) {
                liberar_campos_parciais(campos, n);
            }
            p->contador_erros++;
            continue;
        }

        linha_out->campos = campos;
        linha_out->n_campos = n;
        linha_out->malformada = 0;
        return 1;
    }
    return 0;
}

int csv_parser_contador_erros(const CsvParser *p) {
    return p->contador_erros;
}

void csv_parser_liberar_linha(LinhaCSV *linha) {
    if (linha == NULL || linha->campos == NULL) {
        return;
    }
    liberar_campos_parciais(linha->campos, linha->n_campos);
    linha->campos = NULL;
    linha->n_campos = 0;
}

void csv_parser_fechar(CsvParser *p) {
    if (p == NULL) {
        return;
    }
    fclose(p->fp);
    free(p);
}
