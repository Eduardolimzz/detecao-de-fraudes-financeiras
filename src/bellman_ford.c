#include "bellman_ford.h"
#include "mem_track.h"

#include <string.h>
#include <stdbool.h>

static char *duplicar_rotulo(const char *s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s) + 1;
    char *copia = mem_malloc(len);
    if (copia != NULL) {
        memcpy(copia, s, len);
    }
    return copia;
}

ResultadoBellmanFord *bellman_ford_executar(int num_vertices, 
                                            const ArestaPonderada *arestas, 
                                            int num_arestas, 
                                            const LabelMap *rotulos_estado) {
    if (num_vertices <= 0) return NULL;

    ResultadoBellmanFord *res = mem_malloc(sizeof(ResultadoBellmanFord));
    if (res == NULL) return NULL;

    res->itens = NULL;
    res->n_ciclos = 0;
    res->possui_ciclo_negativo = 0;

    double *dist = mem_malloc((size_t)num_vertices * sizeof(double));
    int *pred = mem_malloc((size_t)num_vertices * sizeof(int));

    if (dist == NULL || pred == NULL) {
        mem_free(dist);
        mem_free(pred);
        mem_free(res);
        return NULL;
    }

    /* Inicialização: distância zero para detectar ciclos em qualquer componente desconexo */
    for (int i = 0; i < num_vertices; i++) {
        dist[i] = 0.0;
        pred[i] = -1;
    }

    /* Relaxamento |V| - 1 vezes */
    for (int i = 1; i <= num_vertices - 1; i++) {
        bool mudou = false;
        for (int j = 0; j < num_arestas; j++) {
            int u = arestas[j].origem;
            int v = arestas[j].destino;
            double w = arestas[j].peso;

            if (isfinite(w) && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pred[v] = u;
                mudou = true;
            }
        }
        if (!mudou) break; /* Convergiu antecipadamente */
    }

    /* N-ésima iteração para encontrar nó afetado por ciclo negativo */
    int no_no_ciclo = -1;
    for (int j = 0; j < num_arestas; j++) {
        int u = arestas[j].origem;
        int v = arestas[j].destino;
        double w = arestas[j].peso;

        if (isfinite(w) && dist[u] + w < dist[v]) {
            no_no_ciclo = v;
            break;
        }
    }

    /* Se encontrou ciclo negativo, reconstrói o circuito */
    if (no_no_ciclo != -1) {
        res->possui_ciclo_negativo = 1;

        /* Anda |V| passos para trás nos predecessores para garantir entrar dentro do ciclo */
        int no_inicio = no_no_ciclo;
        for (int i = 0; i < num_vertices; i++) {
            if (pred[no_inicio] != -1) {
                no_inicio = pred[no_inicio];
            }
        }

        /* Reconstrução do caminho do ciclo */
        int *buffer = mem_malloc((size_t)num_vertices * sizeof(int));
        if (buffer != NULL) {
            int tam = 0;
            int curr = no_inicio;

            do {
                buffer[tam++] = curr;
                curr = pred[curr];
            } while (curr != no_inicio && curr != -1 && tam < num_vertices);

            if (curr == no_inicio && tam > 0) {
                res->n_ciclos = 1;
                res->itens = mem_calloc(1, sizeof(CicloNegativo));

                if (res->itens != NULL) {
                    CicloNegativo *cn = &res->itens[0];
                    cn->comprimento = tam;
                    cn->vertice_indices = mem_malloc((size_t)tam * sizeof(int));
                    cn->rotulos_estado = mem_calloc((size_t)tam, sizeof(char *));

                    if (cn->vertice_indices != NULL && cn->rotulos_estado != NULL) {
                        /* Inverte para ordem cronológica de ocorrência */
                        for (int k = 0; k < tam; k++) {
                            int idx = buffer[tam - 1 - k];
                            cn->vertice_indices[k] = idx;

                            if (rotulos_estado != NULL) {
                                const char *rot = label_map_obter_rotulo(rotulos_estado, idx);
                                cn->rotulos_estado[k] = duplicar_rotulo(rot);
                            }
                        }
                    }
                }
            }
            mem_free(buffer);
        }
    }

    mem_free(dist);
    mem_free(pred);

    return res;
}

void bellman_ford_liberar(ResultadoBellmanFord *res) {
    if (res == NULL) return;

    for (int i = 0; i < res->n_ciclos; i++) {
        CicloNegativo *cn = &res->itens[i];
        if (cn->rotulos_estado != NULL) {
            for (int k = 0; k < cn->comprimento; k++) {
                if (cn->rotulos_estado[k] != NULL) {
                    mem_free(cn->rotulos_estado[k]);
                }
            }
            mem_free(cn->rotulos_estado);
        }
        if (cn->vertice_indices != NULL) {
            mem_free(cn->vertice_indices);
        }
    }
    if (res->itens != NULL) {
        mem_free(res->itens);
    }
    mem_free(res);
}