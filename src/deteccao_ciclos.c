#include "deteccao_ciclos.h"

#include "dfs.h"
#include "mem_track.h"

#include <string.h>

static char *duplicar_rotulo(const char *s) {
    size_t len = strlen(s) + 1;
    char *copia = mem_malloc(len);
    if (copia != NULL) {
        memcpy(copia, s, len);
    }
    return copia;
}

ResultadoCiclos *deteccao_ciclos_executar(const RepGrafo *g, const LabelMap *rotulos) {
    ResultadoDFS *dfs = dfs_executar(g);
    if (dfs == NULL) {
        return NULL;
    }

    ResultadoCiclos *res = mem_malloc(sizeof(ResultadoCiclos));
    if (res == NULL) {
        dfs_liberar(dfs);
        return NULL;
    }
    res->n_ciclos = dfs->n_arestas_retorno;
    res->itens = NULL;
    if (res->n_ciclos > 0) {
        res->itens = mem_calloc((size_t)res->n_ciclos, sizeof(Ciclo));
        if (res->itens == NULL) {
            mem_free(res);
            dfs_liberar(dfs);
            return NULL;
        }
    }

    /* Buffer reutilizado para reconstruir cada ciclo: nenhum ciclo tem mais
     * que n vértices distintos (são ancestrais na mesma árvore de DFS). */
    int *buffer = (dfs->n > 0) ? mem_malloc((size_t)dfs->n * sizeof(int)) : NULL;
    int falhou = (dfs->n > 0 && buffer == NULL);

    for (int i = 0; i < res->n_ciclos && !falhou; i++) {
        int u = dfs->arestas_retorno[i].origem;
        int v = dfs->arestas_retorno[i].destino;

        /* Anda de u até v pelos predecessores: u, pai(u), ..., v. Como v é
         * ancestral de u na árvore de DFS, essa cadeia sempre termina em v. */
        int tamanho = 0;
        int atual = u;
        buffer[tamanho++] = atual;
        while (atual != v) {
            atual = dfs->predecessor[atual];
            buffer[tamanho++] = atual;
        }

        Ciclo *ciclo = &res->itens[i];
        ciclo->comprimento = tamanho;
        ciclo->rotulos = mem_calloc((size_t)tamanho, sizeof(char *));
        if (ciclo->rotulos == NULL) {
            falhou = 1;
            break;
        }

        /* buffer está na ordem u -> ... -> v; invertido, fica v -> ... -> u,
         * a ordem em que o ciclo realmente ocorre (v -> ... -> u -> v). */
        for (int k = 0; k < tamanho; k++) {
            int indice_vertice = buffer[tamanho - 1 - k];
            const char *rotulo = label_map_obter_rotulo(rotulos, indice_vertice);
            ciclo->rotulos[k] = duplicar_rotulo(rotulo);
            if (ciclo->rotulos[k] == NULL) {
                falhou = 1;
                break;
            }
        }
    }

    mem_free(buffer);
    dfs_liberar(dfs);

    if (falhou) {
        deteccao_ciclos_liberar(res);
        return NULL;
    }
    return res;
}

void deteccao_ciclos_liberar(ResultadoCiclos *r) {
    if (r == NULL) {
        return;
    }
    for (int i = 0; i < r->n_ciclos; i++) {
        if (r->itens[i].rotulos != NULL) {
            for (int k = 0; k < r->itens[i].comprimento; k++) {
                mem_free(r->itens[i].rotulos[k]);
            }
            mem_free(r->itens[i].rotulos);
        }
    }
    mem_free(r->itens);
    mem_free(r);
}
