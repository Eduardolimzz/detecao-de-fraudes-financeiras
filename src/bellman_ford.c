#include "bellman_ford.h"
#include "mem_track.h"

#include <stdio.h>
#include <string.h>

static char *duplicar_rotulo(const char *s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s) + 1;
    char *copia = mem_malloc(len);
    if (copia != NULL) {
        memcpy(copia, s, len);
    }
    return copia;
}

static int arestas_validas(int num_vertices, const ArestaPonderada *arestas, int num_arestas) {
    for (int j = 0; j < num_arestas; j++) {
        if (arestas[j].origem < 0 || arestas[j].origem >= num_vertices ||
            arestas[j].destino < 0 || arestas[j].destino >= num_vertices) {
            return 0;
        }
    }
    return 1;
}

/* Reconstrói o ciclo a partir de 'ultimo', o último vértice relaxado na
 * n-ésima passada. Devolve 1 e preenche *saida se o peso total for menor
 * que -tolerancia, 0 se for ruído numérico e -1 se faltar memória. */
static int reconstruir_ciclo(int num_vertices, const ArestaPonderada *arestas,
                             const int *pred_aresta, int ultimo,
                             const LabelMap *rotulos_estado, double tolerancia,
                             CicloNegativo *saida) {
    int x = ultimo;
    for (int i = 0; i < num_vertices; i++) {
        if (pred_aresta[x] < 0) {
            return 0;
        }
        x = arestas[pred_aresta[x]].origem;
    }

    /* Percorre o ciclo de trás para frente, guardando os índices das arestas. */
    int *pilha = mem_malloc((size_t)num_vertices * sizeof(int));
    if (pilha == NULL) {
        return -1;
    }
    int tam = 0;
    int atual = x;
    do {
        if (pred_aresta[atual] < 0 || tam >= num_vertices) {
            mem_free(pilha);
            return 0;
        }
        pilha[tam++] = pred_aresta[atual];
        atual = arestas[pred_aresta[atual]].origem;
    } while (atual != x);

    double total = 0.0;
    for (int k = 0; k < tam; k++) {
        total += arestas[pilha[k]].peso;
    }
    if (!(total < -tolerancia)) {
        mem_free(pilha);
        return 0; /* ruído de ponto flutuante, não é ciclo negativo */
    }

    saida->comprimento = tam;
    saida->peso_total = total;
    saida->vertice_indices = mem_malloc((size_t)tam * sizeof(int));
    saida->id_transacoes = mem_malloc((size_t)tam * sizeof(int));
    saida->pesos = mem_malloc((size_t)tam * sizeof(double));
    saida->rotulos_estado = mem_calloc((size_t)tam, sizeof(char *));
    if (saida->vertice_indices == NULL || saida->id_transacoes == NULL ||
        saida->pesos == NULL || saida->rotulos_estado == NULL) {
        mem_free(pilha);
        return -1;
    }

    /* A pilha está em ordem inversa; k-ésima transação em ordem direta. */
    for (int k = 0; k < tam; k++) {
        const ArestaPonderada *a = &arestas[pilha[tam - 1 - k]];
        saida->vertice_indices[k] = a->origem;
        saida->id_transacoes[k] = a->id_transacao;
        saida->pesos[k] = a->peso;
        if (rotulos_estado != NULL) {
            saida->rotulos_estado[k] =
                duplicar_rotulo(label_map_obter_rotulo(rotulos_estado, a->origem));
        }
    }
    mem_free(pilha);
    return 1;
}

ResultadoBellmanFord *bellman_ford_executar_tol(int num_vertices,
                                                const ArestaPonderada *arestas,
                                                int num_arestas,
                                                const LabelMap *rotulos_estado,
                                                double tolerancia) {
    if (num_vertices <= 0 || num_arestas < 0 || (num_arestas > 0 && arestas == NULL) ||
        !(tolerancia >= 0.0) || !arestas_validas(num_vertices, arestas, num_arestas)) {
        return NULL;
    }

    ResultadoBellmanFord *res = mem_calloc(1, sizeof(ResultadoBellmanFord));
    if (res == NULL) return NULL;
    res->tolerancia = tolerancia;

    double *dist = mem_malloc((size_t)num_vertices * sizeof(double));
    int *pred_aresta = mem_malloc((size_t)num_vertices * sizeof(int));
    if (dist == NULL || pred_aresta == NULL) {
        mem_free(dist);
        mem_free(pred_aresta);
        mem_free(res);
        return NULL;
    }

    /* Distância 0 em todos os vértices: fonte virtual ligada a todos. */
    for (int i = 0; i < num_vertices; i++) {
        dist[i] = 0.0;
        pred_aresta[i] = -1;
    }

    /* Com a fonte virtual, caminhos mínimos têm no máximo |V| - 1 arestas
     * reais. Se a |V|-ésima passada ainda relaxa algo, há ciclo negativo. */
    int ultimo_relaxado = -1;
    int mudou = 0;
    for (int passada = 1; passada <= num_vertices; passada++) {
        mudou = 0;
        for (int j = 0; j < num_arestas; j++) {
            int u = arestas[j].origem;
            int v = arestas[j].destino;
            double w = arestas[j].peso;

            if (isfinite(w) && dist[u] + w < dist[v] - tolerancia) {
                dist[v] = dist[u] + w;
                pred_aresta[v] = j;
                ultimo_relaxado = v;
                mudou = 1;
            }
        }
        res->iteracoes = passada;
        if (!mudou) break; /* convergiu */
    }

    if (mudou) {
        CicloNegativo ciclo;
        memset(&ciclo, 0, sizeof(ciclo));
        int status = reconstruir_ciclo(num_vertices, arestas, pred_aresta, ultimo_relaxado,
                                       rotulos_estado, tolerancia, &ciclo);
        if (status == 1) {
            res->itens = mem_malloc(sizeof(CicloNegativo));
            if (res->itens != NULL) {
                res->itens[0] = ciclo;
                res->n_ciclos = 1;
                res->possui_ciclo_negativo = 1;
            } else {
                status = -1;
            }
        }
        if (status == -1) {
            /* libera o que foi parcialmente alocado para o ciclo */
            if (ciclo.rotulos_estado != NULL) {
                for (int k = 0; k < ciclo.comprimento; k++) mem_free(ciclo.rotulos_estado[k]);
            }
            mem_free(ciclo.rotulos_estado);
            mem_free(ciclo.vertice_indices);
            mem_free(ciclo.id_transacoes);
            mem_free(ciclo.pesos);
            mem_free(dist);
            mem_free(pred_aresta);
            mem_free(res);
            return NULL;
        }
    }

    mem_free(dist);
    mem_free(pred_aresta);
    return res;
}

ResultadoBellmanFord *bellman_ford_executar(int num_vertices,
                                            const ArestaPonderada *arestas,
                                            int num_arestas,
                                            const LabelMap *rotulos_estado) {
    return bellman_ford_executar_tol(num_vertices, arestas, num_arestas, rotulos_estado,
                                     BF_TOLERANCIA_PADRAO);
}

ResultadoBellmanFord *bellman_ford_executar_rep(const RepGrafo *g,
                                                const LabelMap *rotulos_estado,
                                                double tolerancia) {
    if (g == NULL) return NULL;

    int n = rep_num_vertices(g);
    int capacidade = rep_num_arestas(g);
    ArestaPonderada *arestas =
        mem_malloc((size_t)(capacidade > 0 ? capacidade : 1) * sizeof(ArestaPonderada));
    if (arestas == NULL) return NULL;

    /* Extrai as arestas pela interface comum: a lista devolve cada
     * transação; a matriz, uma aresta por par (a de menor peso). */
    int contagem = 0;
    for (int v = 0; v < n; v++) {
        IteradorVizinhos it;
        int destino;
        double peso;
        rep_iter_inicio(&it, g, v);
        while (rep_iter_proximo(&it, &destino, &peso)) {
            if (contagem < capacidade) {
                arestas[contagem].origem = v;
                arestas[contagem].destino = destino;
                arestas[contagem].peso = peso;
                arestas[contagem].id_transacao = 0;
                contagem++;
            }
        }
    }

    ResultadoBellmanFord *res =
        bellman_ford_executar_tol(n, arestas, contagem, rotulos_estado, tolerancia);
    mem_free(arestas);
    return res;
}

void bellman_ford_anotar_transacoes(ResultadoBellmanFord *res,
                                    const ArestaPonderada *arestas,
                                    int num_arestas) {
    if (res == NULL || arestas == NULL) return;

    for (int c = 0; c < res->n_ciclos; c++) {
        CicloNegativo *ciclo = &res->itens[c];
        for (int k = 0; k < ciclo->comprimento; k++) {
            int origem = ciclo->vertice_indices[k];
            int destino = ciclo->vertice_indices[(k + 1) % ciclo->comprimento];
            for (int j = 0; j < num_arestas; j++) {
                if (arestas[j].origem == origem && arestas[j].destino == destino &&
                    fabs(arestas[j].peso - ciclo->pesos[k]) <= 1e-12) {
                    ciclo->id_transacoes[k] = arestas[j].id_transacao;
                    break;
                }
            }
        }
    }
}

RepGrafo *bellman_ford_construir_grafo(TipoRepr tipo, int num_vertices,
                                       const ArestaPonderada *arestas,
                                       int num_arestas) {
    RepGrafo *g = rep_criar(tipo, num_vertices);
    if (g == NULL) return NULL;

    for (int j = 0; j < num_arestas; j++) {
        if (!rep_inserir_aresta(g, arestas[j].origem, arestas[j].destino, arestas[j].peso)) {
            rep_liberar(g);
            return NULL;
        }
    }
    return g;
}

void bellman_ford_imprimir_resultado(const ResultadoBellmanFord *res,
                                     int num_vertices, int num_arestas) {
    printf("\nBellman-Ford (estados banco:conta:moeda, peso w = -ln(recebido/pago))\n");
    printf("  Estados |V|: %d, transações ponderadas |E|: %d\n", num_vertices, num_arestas);
    printf("  Complexidade esperada: O(|V| x |E|), cerca de %.2e relaxações no pior caso\n",
           (double)num_vertices * (double)num_arestas);
    printf("  Passadas executadas: %d (tolerância numérica: %.0e)\n",
           res->iteracoes, res->tolerancia);

    if (!res->possui_ciclo_negativo) {
        printf("  Resultado: nenhum ciclo negativo encontrado\n");
        return;
    }

    for (int c = 0; c < res->n_ciclos; c++) {
        const CicloNegativo *ciclo = &res->itens[c];
        double taxa_total = exp(-ciclo->peso_total);

        printf("  Resultado: ciclo negativo encontrado, candidato a arbitragem segundo a\n"
               "             modelagem (não é classificação automática de fraude)\n");
        printf("  Comprimento: %d transação(ões)\n", ciclo->comprimento);
        printf("  Peso total: %.9f (produto das taxas %.6f, variação implícita %+.2f%%)\n",
               ciclo->peso_total, taxa_total, (taxa_total - 1.0) * 100.0);

        for (int k = 0; k < ciclo->comprimento; k++) {
            int prox = (k + 1) % ciclo->comprimento;
            const char *de = ciclo->rotulos_estado != NULL ? ciclo->rotulos_estado[k] : NULL;
            const char *para = ciclo->rotulos_estado != NULL ? ciclo->rotulos_estado[prox] : NULL;
            printf("    %d. ", k + 1);
            if (de != NULL) printf("%s", de); else printf("estado %d", ciclo->vertice_indices[k]);
            printf(" -> ");
            if (para != NULL) printf("%s", para); else printf("estado %d", ciclo->vertice_indices[prox]);
            if (ciclo->id_transacoes[k] > 0) {
                printf("  [transação #%d, ", ciclo->id_transacoes[k]);
            } else {
                printf("  [transação ?, ");
            }
            /* + 0.0 troca -0.0 (de -ln(1)) por +0.0 na impressão */
            printf("w=%+.6f, taxa=%.6f]\n", ciclo->pesos[k] + 0.0, exp(-ciclo->pesos[k]));
        }
    }
}

void bellman_ford_liberar(ResultadoBellmanFord *res) {
    if (res == NULL) return;

    for (int i = 0; i < res->n_ciclos; i++) {
        CicloNegativo *cn = &res->itens[i];
        if (cn->rotulos_estado != NULL) {
            for (int k = 0; k < cn->comprimento; k++) {
                mem_free(cn->rotulos_estado[k]);
            }
            mem_free(cn->rotulos_estado);
        }
        mem_free(cn->vertice_indices);
        mem_free(cn->id_transacoes);
        mem_free(cn->pesos);
    }
    mem_free(res->itens);
    mem_free(res);
}
