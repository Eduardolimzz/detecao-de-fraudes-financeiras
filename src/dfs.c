#include "dfs.h"

#include "mem_track.h"

#define CAPACIDADE_INICIAL_PILHA 64
#define CAPACIDADE_INICIAL_RETORNO 16

/* Frame da pilha explícita: o vértice sendo explorado e o ponto exato em
 * que a iteração de seus vizinhos parou (ou ainda não começou). */
typedef struct {
    int vertice;
    IteradorVizinhos it;
} FrameDFS;

typedef struct {
    FrameDFS *itens;
    int topo;        /* número de frames empilhados */
    int capacidade;
} PilhaDFS;

static int pilha_criar(PilhaDFS *p) {
    p->itens = mem_malloc(CAPACIDADE_INICIAL_PILHA * sizeof(FrameDFS));
    p->topo = 0;
    p->capacidade = CAPACIDADE_INICIAL_PILHA;
    return p->itens != NULL;
}

static int pilha_empilhar(PilhaDFS *p, int vertice, const RepGrafo *g) {
    if (p->topo >= p->capacidade) {
        int nova_cap = p->capacidade * 2;
        FrameDFS *novo = mem_realloc(p->itens, (size_t)nova_cap * sizeof(FrameDFS));
        if (novo == NULL) {
            return 0;
        }
        p->itens = novo;
        p->capacidade = nova_cap;
    }
    p->itens[p->topo].vertice = vertice;
    rep_iter_inicio(&p->itens[p->topo].it, g, vertice);
    p->topo++;
    return 1;
}

static void pilha_liberar(PilhaDFS *p) {
    mem_free(p->itens);
}

static int registrar_aresta_retorno(ResultadoDFS *r, int capacidade_atual_out[1],
                                    int origem, int destino) {
    if (r->n_arestas_retorno >= capacidade_atual_out[0]) {
        int nova_cap = capacidade_atual_out[0] * 2;
        ArestaRetornoDFS *novo =
            mem_realloc(r->arestas_retorno, (size_t)nova_cap * sizeof(ArestaRetornoDFS));
        if (novo == NULL) {
            return 0;
        }
        r->arestas_retorno = novo;
        capacidade_atual_out[0] = nova_cap;
    }
    r->arestas_retorno[r->n_arestas_retorno].origem = origem;
    r->arestas_retorno[r->n_arestas_retorno].destino = destino;
    r->n_arestas_retorno++;
    return 1;
}

ResultadoDFS *dfs_executar(const RepGrafo *g) {
    int n = rep_num_vertices(g);

    ResultadoDFS *r = mem_malloc(sizeof(ResultadoDFS));
    if (r == NULL) {
        return NULL;
    }
    r->n = n;
    r->descoberta = mem_malloc((size_t)n * sizeof(int));
    r->finalizacao = mem_malloc((size_t)n * sizeof(int));
    r->predecessor = mem_malloc((size_t)n * sizeof(int));
    r->cor = mem_malloc((size_t)n * sizeof(CorDFS));
    r->arestas_retorno = mem_malloc(CAPACIDADE_INICIAL_RETORNO * sizeof(ArestaRetornoDFS));
    r->n_arestas_retorno = 0;
    int capacidade_retorno = CAPACIDADE_INICIAL_RETORNO;

    if (r->descoberta == NULL || r->finalizacao == NULL || r->predecessor == NULL ||
        r->cor == NULL || r->arestas_retorno == NULL) {
        dfs_liberar(r);
        return NULL;
    }

    for (int v = 0; v < n; v++) {
        r->descoberta[v] = -1;
        r->finalizacao[v] = -1;
        r->predecessor[v] = -1;
        r->cor[v] = DFS_BRANCO;
    }

    PilhaDFS pilha;
    if (!pilha_criar(&pilha)) {
        dfs_liberar(r);
        return NULL;
    }

    int tempo = 0;
    int falhou = 0;

    for (int raiz = 0; raiz < n && !falhou; raiz++) {
        if (r->cor[raiz] != DFS_BRANCO) {
            continue;
        }
        r->cor[raiz] = DFS_CINZA;
        r->descoberta[raiz] = tempo++;
        if (!pilha_empilhar(&pilha, raiz, g)) {
            falhou = 1;
            break;
        }

        while (pilha.topo > 0 && !falhou) {
            FrameDFS *topo = &pilha.itens[pilha.topo - 1];
            int u = topo->vertice;
            int v;

            if (!rep_iter_proximo(&topo->it, &v)) {
                /* todos os vizinhos de u já foram explorados */
                r->cor[u] = DFS_PRETO;
                r->finalizacao[u] = tempo++;
                pilha.topo--;
                continue;
            }

            if (r->cor[v] == DFS_BRANCO) {
                r->cor[v] = DFS_CINZA;
                r->descoberta[v] = tempo++;
                r->predecessor[v] = u;
                if (!pilha_empilhar(&pilha, v, g)) {
                    falhou = 1;
                }
            } else if (r->cor[v] == DFS_CINZA) {
                /* v é ancestral de u na árvore de DFS: aresta de retorno,
                 * fecha um ciclo u -> ... -> v -> u. */
                if (!registrar_aresta_retorno(r, &capacidade_retorno, u, v)) {
                    falhou = 1;
                }
            }
            /* v PRETO: aresta de avanço ou cruzamento, não fecha ciclo. */
        }
    }

    pilha_liberar(&pilha);

    if (falhou) {
        dfs_liberar(r);
        return NULL;
    }
    return r;
}

void dfs_liberar(ResultadoDFS *r) {
    if (r == NULL) {
        return;
    }
    mem_free(r->descoberta);
    mem_free(r->finalizacao);
    mem_free(r->predecessor);
    mem_free(r->cor);
    mem_free(r->arestas_retorno);
    mem_free(r);
}
