#include "carregador.h"

#include "bench.h"
#include "csv_parser.h"
#include "mem_track.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPACIDADE_INICIAL_ARESTAS 1024
#define TAMANHO_CHAVE 256

/* Aresta guardada entre a primeira e a segunda passada. */
typedef struct {
    int origem;
    int destino;
    double valor;
} ArestaTemp;

typedef struct {
    ArestaTemp *itens;
    int tamanho;
    int capacidade;
} VetorArestas;

static int vetor_adicionar(VetorArestas *v, int origem, int destino, double valor) {
    if (v->tamanho >= v->capacidade) {
        int nova_cap = (v->capacidade == 0) ? CAPACIDADE_INICIAL_ARESTAS : v->capacidade * 2;
        ArestaTemp *novo = mem_realloc(v->itens, (size_t)nova_cap * sizeof(ArestaTemp));
        if (novo == NULL) {
            return 0;
        }
        v->itens = novo;
        v->capacidade = nova_cap;
    }
    v->itens[v->tamanho].origem = origem;
    v->itens[v->tamanho].destino = destino;
    v->itens[v->tamanho].valor = valor;
    v->tamanho++;
    return 1;
}

/* Monta "banco:conta". Retorna 0 se não couber no buffer. */
static int montar_chave(char *chave, size_t tam, const char *banco, const char *conta) {
    int n = snprintf(chave, tam, "%s:%s", banco, conta);
    return n > 0 && (size_t)n < tam;
}

/* Monta "banco:conta:moeda". Retorna 0 se não couber no buffer. */
static int montar_chave_estado(char *chave, size_t tam, const char *banco, const char *conta, const char *moeda) {
    int n = snprintf(chave, tam, "%s:%s:%s", banco, conta, moeda);
    return n > 0 && (size_t)n < tam;
}

/* O valor financeiro é lido com strtod; retorna 0.0 se não for numérico. */
static double ler_valor(const char *texto) {
    char *fim = NULL;
    double v = strtod(texto, &fim);
    return (fim == texto) ? 0.0 : v;
}

/* Primeira passada: lê o CSV e preenche os rótulos topológicos, rótulos por estado e vetores de arestas. */
static StatusCarga primeira_passada(const char *caminho, LabelMap *rotulos_top, LabelMap *rotulos_est,
                                    VetorArestas *arestas_top, ArestaPonderada **arestas_bf_out,
                                    int *n_bf_out, int *ignoradas) {
    int erro = 0;
    CsvParser *parser = csv_parser_abrir(caminho, &erro);
    if (parser == NULL) {
        return (erro == 2) ? CARGA_SEM_PERMISSAO : CARGA_ARQUIVO_INEXISTENTE;
    }

    StatusCarga status = CARGA_OK;
    LinhaCSV linha;
    char chave_origem[TAMANHO_CHAVE], chave_destino[TAMANHO_CHAVE];
    char estado_origem[TAMANHO_CHAVE], estado_destino[TAMANHO_CHAVE];

    int cap_bf = CAPACIDADE_INICIAL_ARESTAS;
    int n_bf = 0;
    ArestaPonderada *arestas_bf = mem_malloc((size_t)cap_bf * sizeof(ArestaPonderada));
    int invalidas_proprias = 0;

    if (arestas_bf == NULL) {
        csv_parser_fechar(parser);
        return CARGA_SEM_MEMORIA;
    }

    while (csv_parser_proxima_linha(parser, &linha)) {
        if (linha.n_campos < NUM_COLUNAS_MINIMO ||
            !montar_chave(chave_origem, sizeof(chave_origem),
                          linha.campos[COL_BANCO_ORIGEM], linha.campos[COL_CONTA_ORIGEM]) ||
            !montar_chave(chave_destino, sizeof(chave_destino),
                          linha.campos[COL_BANCO_DESTINO], linha.campos[COL_CONTA_DESTINO]) ||
            !montar_chave_estado(estado_origem, sizeof(estado_origem),
                                 linha.campos[COL_BANCO_ORIGEM], linha.campos[COL_CONTA_ORIGEM], linha.campos[COL_MOEDA_PAGA]) ||
            !montar_chave_estado(estado_destino, sizeof(estado_destino),
                                 linha.campos[COL_BANCO_DESTINO], linha.campos[COL_CONTA_DESTINO], linha.campos[COL_MOEDA_RECEBIDA])) {
            invalidas_proprias++;
            csv_parser_liberar_linha(&linha);
            continue;
        }

        /* 1. Grafo Topológico da Fase I */
        int orig_top = label_map_obter_ou_inserir(rotulos_top, chave_origem);
        int dest_top = label_map_obter_ou_inserir(rotulos_top, chave_destino);
        double v_pago = ler_valor(linha.campos[COL_VALOR_PAGO]);
        double v_recebido = ler_valor(linha.campos[COL_VALOR_RECEBIDO]);

        /* 2. Grafo de Estados e Pesos logarítmicos da Fase II: w = -ln(r) */
        double peso_w = calcular_peso_transacao(v_pago, v_recebido);

        if (isfinite(peso_w)) {
            int orig_est = label_map_obter_ou_inserir(rotulos_est, estado_origem);
            int dest_est = label_map_obter_ou_inserir(rotulos_est, estado_destino);

            if (n_bf >= cap_bf) {
                cap_bf *= 2;
                ArestaPonderada *novo = mem_realloc(arestas_bf, (size_t)cap_bf * sizeof(ArestaPonderada));
                if (novo == NULL) {
                    status = CARGA_SEM_MEMORIA;
                    csv_parser_liberar_linha(&linha);
                    break;
                }
                arestas_bf = novo;
            }
            arestas_bf[n_bf].origem = orig_est;
            arestas_bf[n_bf].destino = dest_est;
            arestas_bf[n_bf].peso = peso_w;
            n_bf++;
        }

        csv_parser_liberar_linha(&linha);

        if (orig_top < 0 || dest_top < 0 || !vetor_adicionar(arestas_top, orig_top, dest_top, v_pago)) {
            status = CARGA_SEM_MEMORIA;
            break;
        }
    }

    *ignoradas = csv_parser_contador_erros(parser) + invalidas_proprias;
    csv_parser_fechar(parser);

    if (status == CARGA_OK) {
        *arestas_bf_out = arestas_bf;
        *n_bf_out = n_bf;
    } else {
        mem_free(arestas_bf);
    }

    if (status == CARGA_OK && arestas_top->tamanho == 0) {
        status = CARGA_SEM_DADOS;
    }
    return status;
}

StatusCarga carregador_carregar(const char *caminho, TipoRepr repr, int limite,
                                GrafoCarregado *saida) {
    memset(saida, 0, sizeof(*saida));

    size_t base_bytes = mem_bytes_atuais();
    mem_resetar_pico();

    Cronometro cron;
    bench_cronometro_iniciar(&cron);

    VetorArestas arestas = {NULL, 0, 0};
    StatusCarga status = CARGA_SEM_MEMORIA;
    size_t antes_grafo = 0;
    int n = 0;

    saida->limite = limite;
    saida->rotulos = label_map_criar(0);
    saida->rotulos_estado = label_map_criar(0);

    if (saida->rotulos != NULL && saida->rotulos_estado != NULL) {
        status = primeira_passada(caminho, saida->rotulos, saida->rotulos_estado,
                                 &arestas, &saida->arestas_bf, &saida->n_arestas_bf,
                                 &saida->linhas_ignoradas);
    }

    if (status == CARGA_OK) {
        saida->n_vertices_total = label_map_tamanho(saida->rotulos);
        saida->n_vertices_estado = label_map_tamanho(saida->rotulos_estado);
        saida->n_arestas_total = arestas.tamanho;

        n = (limite > 0 && limite < saida->n_vertices_total) ? limite
                                                             : saida->n_vertices_total;
        saida->n_vertices = n;

        antes_grafo = mem_bytes_atuais();
        saida->grafo = rep_criar(repr, n);
        if (saida->grafo == NULL) {
            status = (repr == REPR_MATRIZ) ? CARGA_LIMITE_REPR : CARGA_SEM_MEMORIA;
        }
    }

    if (status == CARGA_OK) {
        for (int i = 0; i < arestas.tamanho; i++) {
            const ArestaTemp *a = &arestas.itens[i];
            if (a->origem >= n || a->destino >= n) {
                continue;
            }
            if (!rep_inserir_aresta(saida->grafo, a->origem, a->destino, a->valor)) {
                status = CARGA_SEM_MEMORIA;
                break;
            }
        }
        saida->n_arestas = rep_num_arestas(saida->grafo);
        saida->bytes_grafo = mem_bytes_atuais() - antes_grafo;
    }

    mem_free(arestas.itens);

    saida->tempo_ms = bench_cronometro_ms(&cron);
    saida->bytes_pico = mem_bytes_pico() - base_bytes;
    saida->bytes_estruturas = mem_bytes_atuais() - base_bytes;
    return status;
}

const char *carregador_status_texto(StatusCarga status) {
    switch (status) {
        case CARGA_OK: return "ok";
        case CARGA_ARQUIVO_INEXISTENTE: return "arquivo não encontrado";
        case CARGA_SEM_PERMISSAO: return "sem permissão de leitura";
        case CARGA_SEM_DADOS: return "o arquivo não tem nenhuma transação válida";
        case CARGA_SEM_MEMORIA: return "memória insuficiente";
        case CARGA_LIMITE_REPR:
            return "a representação escolhida não comporta este número de vértices";
    }
    return "erro desconhecido";
}

void carregador_imprimir_sumario(const GrafoCarregado *c, TipoRepr repr) {
    printf("Sumário da carga (representação: %s)\n", rep_tipo_nome(repr));
    if (c->limite > 0) {
        printf("  Subgrafo induzido pelos %d primeiros vértices (--limit=%d)\n",
               c->n_vertices, c->limite);
        if (c->limite > c->n_vertices_total) {
            printf("  Aviso: --limit maior que |V| total; usando o grafo completo\n");
        }
        printf("  Grafo completo:         |V|=%d, |E|=%d\n",
               c->n_vertices_total, c->n_arestas_total);
    }
    printf("  |V| (contas distintas): %d\n", c->n_vertices);
    printf("  |V_estado| (com moeda): %d\n", c->n_vertices_estado);
    printf("  |E| (transações):       %d\n", c->n_arestas);
    printf("  |E_bf| (com peso):      %d\n", c->n_arestas_bf);
    printf("  Linhas ignoradas:       %d\n", c->linhas_ignoradas);
    printf("  Tempo de carga:         %.3f ms\n", c->tempo_ms);
    printf("  Memória do grafo:       %.1f KB\n", (double)c->bytes_grafo / 1024.0);
    printf("  Memória das estruturas: %.1f KB (pico na carga: %.1f KB)\n",
           (double)c->bytes_estruturas / 1024.0, (double)c->bytes_pico / 1024.0);
    printf("  Pico de RSS do processo: %ld KB\n", bench_pico_rss_kb());
}

void carregador_liberar(GrafoCarregado *c) {
    if (c == NULL) {
        return;
    }
    rep_liberar(c->grafo);
    label_map_destruir(c->rotulos);
    
    if (c->rotulos_estado != NULL) {
        label_map_destruir(c->rotulos_estado);
    }
    if (c->arestas_bf != NULL) {
        mem_free(c->arestas_bf);
    }
    memset(c, 0, sizeof(*c));
}