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

/* O valor financeiro é guardado só como metadado para a Fase II; um valor
 * ilegível não invalida a topologia, então vira 0. */
static double ler_valor(const char *texto) {
    char *fim = NULL;
    double v = strtod(texto, &fim);
    return (fim == texto) ? 0.0 : v;
}

/* Primeira passada: lê o CSV e preenche rótulos + vetor de arestas. */
static StatusCarga primeira_passada(const char *caminho, LabelMap *rotulos,
                                    VetorArestas *arestas, int *ignoradas) {
    int erro = 0;
    CsvParser *parser = csv_parser_abrir(caminho, &erro);
    if (parser == NULL) {
        return (erro == 2) ? CARGA_SEM_PERMISSAO : CARGA_ARQUIVO_INEXISTENTE;
    }

    StatusCarga status = CARGA_OK;
    LinhaCSV linha;
    char chave_origem[TAMANHO_CHAVE];
    char chave_destino[TAMANHO_CHAVE];
    int invalidas_proprias = 0;

    while (csv_parser_proxima_linha(parser, &linha)) {
        if (linha.n_campos < NUM_COLUNAS_MINIMO ||
            !montar_chave(chave_origem, sizeof(chave_origem),
                          linha.campos[COL_BANCO_ORIGEM], linha.campos[COL_CONTA_ORIGEM]) ||
            !montar_chave(chave_destino, sizeof(chave_destino),
                          linha.campos[COL_BANCO_DESTINO], linha.campos[COL_CONTA_DESTINO])) {
            invalidas_proprias++;
            csv_parser_liberar_linha(&linha);
            continue;
        }

        int origem = label_map_obter_ou_inserir(rotulos, chave_origem);
        int destino = label_map_obter_ou_inserir(rotulos, chave_destino);
        double valor = ler_valor(linha.campos[COL_VALOR_PAGO]);
        csv_parser_liberar_linha(&linha);

        if (origem < 0 || destino < 0 || !vetor_adicionar(arestas, origem, destino, valor)) {
            status = CARGA_SEM_MEMORIA;
            break;
        }
    }

    *ignoradas = csv_parser_contador_erros(parser) + invalidas_proprias;
    csv_parser_fechar(parser);

    if (status == CARGA_OK && arestas->tamanho == 0) {
        status = CARGA_SEM_DADOS;
    }
    return status;
}

StatusCarga carregador_carregar(const char *caminho, TipoRepr repr, GrafoCarregado *saida) {
    memset(saida, 0, sizeof(*saida));

    size_t base_bytes = mem_bytes_atuais();
    mem_resetar_pico();

    Cronometro cron;
    bench_cronometro_iniciar(&cron);

    VetorArestas arestas = {NULL, 0, 0};
    StatusCarga status = CARGA_SEM_MEMORIA;

    saida->rotulos = label_map_criar(0);
    if (saida->rotulos != NULL) {
        status = primeira_passada(caminho, saida->rotulos, &arestas, &saida->linhas_ignoradas);
    }

    if (status == CARGA_OK) {
        saida->n_vertices = label_map_tamanho(saida->rotulos);
        saida->grafo = rep_criar(repr, saida->n_vertices);
        if (saida->grafo == NULL) {
            /* só falha aqui por limite da matriz ou falta de memória */
            status = (repr == REPR_MATRIZ) ? CARGA_LIMITE_REPR : CARGA_SEM_MEMORIA;
        }
    }

    if (status == CARGA_OK) {
        for (int i = 0; i < arestas.tamanho; i++) {
            const ArestaTemp *a = &arestas.itens[i];
            if (!rep_inserir_aresta(saida->grafo, a->origem, a->destino, a->valor)) {
                status = CARGA_SEM_MEMORIA;
                break;
            }
        }
        saida->n_arestas = rep_num_arestas(saida->grafo);
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
    printf("  |V| (contas distintas): %d\n", c->n_vertices);
    printf("  |E| (transações):       %d\n", c->n_arestas);
    printf("  Linhas ignoradas:       %d\n", c->linhas_ignoradas);
    printf("  Tempo de carga:         %.3f ms\n", c->tempo_ms);
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
    c->grafo = NULL;
    c->rotulos = NULL;
}
