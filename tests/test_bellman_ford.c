#include "bellman_ford.h"

#include "mem_track.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

/* Confere que o ciclo devolvido é coerente com as arestas de entrada: cada
 * passo existe, os pesos batem, a soma é o peso_total e ele é negativo. */
static void verificar_ciclo(const ResultadoBellmanFord *res, const ArestaPonderada *arestas,
                            int m, int n_vertices) {
    assert(res->possui_ciclo_negativo == 1);
    assert(res->n_ciclos == 1);

    const CicloNegativo *c = &res->itens[0];
    assert(c->comprimento >= 1 && c->comprimento <= n_vertices);

    double soma = 0.0;
    for (int k = 0; k < c->comprimento; k++) {
        int de = c->vertice_indices[k];
        int para = c->vertice_indices[(k + 1) % c->comprimento];
        int achou = 0;
        for (int j = 0; j < m; j++) {
            if (arestas[j].origem == de && arestas[j].destino == para &&
                fabs(arestas[j].peso - c->pesos[k]) < 1e-12) {
                achou = 1;
                break;
            }
        }
        assert(achou);
        soma += c->pesos[k];
    }
    assert(fabs(soma - c->peso_total) < 1e-9);
    assert(c->peso_total < -res->tolerancia);
}

/* Roda Bellman-Ford pela interface de representação (lista ou matriz). */
static ResultadoBellmanFord *executar_em(TipoRepr tipo, int n, const ArestaPonderada *arestas,
                                         int m, const LabelMap *rotulos) {
    RepGrafo *g = bellman_ford_construir_grafo(tipo, n, arestas, m);
    assert(g != NULL);
    ResultadoBellmanFord *res = bellman_ford_executar_rep(g, rotulos, BF_TOLERANCIA_PADRAO);
    assert(res != NULL);
    bellman_ford_anotar_transacoes(res, arestas, m);
    rep_liberar(g);
    return res;
}

/* ---------- 1. ciclo negativo ---------- */
static void testar_ciclo_negativo(void) {
    printf("Caso 1: ciclo negativo (arbitragem USD -> EUR -> BRL -> USD)...\n");

    LabelMap *mapa = label_map_criar(0);
    int usd = label_map_obter_ou_inserir(mapa, "B1:C1:USD");
    int eur = label_map_obter_ou_inserir(mapa, "B2:C2:EUR");
    int brl = label_map_obter_ou_inserir(mapa, "B3:C3:BRL");

    /* r = 1, 6 e 0,2: produto 1,2 => peso total = -ln(1,2) */
    ArestaPonderada arestas[3] = {
        {usd, eur, calcular_peso_transacao(100.0, 100.0), 1},
        {eur, brl, calcular_peso_transacao(100.0, 600.0), 2},
        {brl, usd, calcular_peso_transacao(600.0, 120.0), 3},
    };

    /* direto sobre o vetor de arestas: traz rótulos e ids */
    ResultadoBellmanFord *res = bellman_ford_executar(3, arestas, 3, mapa);
    assert(res != NULL);
    verificar_ciclo(res, arestas, 3, 3);
    assert(res->itens[0].comprimento == 3);
    assert(fabs(res->itens[0].peso_total - (-log(1.2))) < 1e-9);
    assert(res->iteracoes >= 1 && res->iteracoes <= 3);

    /* cada rótulo é um dos três estados e cada transação aparece uma vez */
    int vistos[4] = {0, 0, 0, 0};
    for (int k = 0; k < 3; k++) {
        assert(res->itens[0].rotulos_estado[k] != NULL);
        int id = res->itens[0].id_transacoes[k];
        assert(id >= 1 && id <= 3);
        vistos[id]++;
    }
    assert(vistos[1] == 1 && vistos[2] == 1 && vistos[3] == 1);
    bellman_ford_imprimir_resultado(res, 3, 3);
    bellman_ford_liberar(res);

    /* pela camada de abstração, nas duas representações */
    TipoRepr tipos[] = {REPR_LISTA, REPR_MATRIZ};
    for (int t = 0; t < 2; t++) {
        res = executar_em(tipos[t], 3, arestas, 3, mapa);
        verificar_ciclo(res, arestas, 3, 3);
        assert(res->itens[0].comprimento == 3);
        for (int k = 0; k < 3; k++) {
            assert(res->itens[0].id_transacoes[k] >= 1); /* anotadas */
        }
        bellman_ford_liberar(res);
    }
    label_map_destruir(mapa);
}

/* ---------- 2. nenhum ciclo negativo ---------- */
static void testar_sem_ciclo_negativo(void) {
    printf("Caso 2: ciclo com produto de taxas < 1 (nenhum ciclo negativo)...\n");

    /* r = 0,9, 5,5555... e 0,19: produto 0,95 => peso total positivo */
    ArestaPonderada arestas[3] = {
        {0, 1, calcular_peso_transacao(100.0, 90.0), 1},
        {1, 2, calcular_peso_transacao(90.0, 500.0), 2},
        {2, 0, calcular_peso_transacao(500.0, 95.0), 3},
    };

    ResultadoBellmanFord *res = bellman_ford_executar(3, arestas, 3, NULL);
    assert(res != NULL);
    assert(res->possui_ciclo_negativo == 0);
    assert(res->n_ciclos == 0 && res->itens == NULL);
    assert(res->iteracoes >= 1 && res->iteracoes <= 3);
    bellman_ford_liberar(res);

    TipoRepr tipos[] = {REPR_LISTA, REPR_MATRIZ};
    for (int t = 0; t < 2; t++) {
        res = executar_em(tipos[t], 3, arestas, 3, NULL);
        assert(res->possui_ciclo_negativo == 0);
        bellman_ford_liberar(res);
    }
}

/* ---------- 3. ciclo negativo desconectado ---------- */
static void testar_ciclo_desconectado(void) {
    printf("Caso 3: ciclo negativo em componente desconectado...\n");

    /* Componente A (0,1,2): caminho positivo, sem ciclo.
     * Componente B (4,5,6): ciclo de peso total -1.
     * Vértice 3 isolado. Nada em B é alcançável a partir de A. */
    ArestaPonderada arestas[] = {
        {0, 1, 2.0, 1}, {1, 2, 3.0, 2},
        {4, 5, 1.0, 3}, {5, 6, -3.0, 4}, {6, 4, 1.0, 5},
    };
    int m = 5;

    ResultadoBellmanFord *res = bellman_ford_executar(7, arestas, m, NULL);
    assert(res != NULL);
    verificar_ciclo(res, arestas, m, 7);
    assert(res->itens[0].comprimento == 3);
    for (int k = 0; k < 3; k++) {
        int v = res->itens[0].vertice_indices[k];
        assert(v >= 4 && v <= 6); /* só vértices do componente B */
    }
    assert(fabs(res->itens[0].peso_total - (-1.0)) < 1e-12);
    bellman_ford_liberar(res);

    TipoRepr tipos[] = {REPR_LISTA, REPR_MATRIZ};
    for (int t = 0; t < 2; t++) {
        res = executar_em(tipos[t], 7, arestas, m, NULL);
        verificar_ciclo(res, arestas, m, 7);
        bellman_ford_liberar(res);
    }
}

/* ---------- 4. aresta negativa sem ciclo negativo ---------- */
static void testar_aresta_negativa_sem_ciclo(void) {
    printf("Caso 4: arestas negativas sem ciclo negativo...\n");

    /* DAG com pesos negativos: relaxa bastante mas nunca forma ciclo. */
    ArestaPonderada arestas[] = {
        {0, 1, -5.0, 1}, {1, 2, -2.0, 2}, {0, 2, 1.0, 3}, {2, 3, -1.0, 4},
    };
    int m = 4;

    ResultadoBellmanFord *res = bellman_ford_executar(4, arestas, m, NULL);
    assert(res != NULL);
    assert(res->possui_ciclo_negativo == 0);
    bellman_ford_liberar(res);

    /* também um ciclo de peso total positivo contendo aresta negativa */
    ArestaPonderada ciclo_positivo[] = {{0, 1, -1.0, 1}, {1, 0, 2.0, 2}};
    TipoRepr tipos[] = {REPR_LISTA, REPR_MATRIZ};
    for (int t = 0; t < 2; t++) {
        res = executar_em(tipos[t], 4, arestas, m, NULL);
        assert(res->possui_ciclo_negativo == 0);
        bellman_ford_liberar(res);

        res = executar_em(tipos[t], 2, ciclo_positivo, 2, NULL);
        assert(res->possui_ciclo_negativo == 0);
        bellman_ford_liberar(res);
    }
}

/* ---------- 5. laço negativo ---------- */
static void testar_laco_negativo(void) {
    printf("Caso 5: laço (auto-aresta) negativo...\n");

    LabelMap *mapa = label_map_criar(0);
    label_map_obter_ou_inserir(mapa, "B1:C1:USD");
    label_map_obter_ou_inserir(mapa, "B2:C2:EUR");
    label_map_obter_ou_inserir(mapa, "B3:C3:BRL");

    ArestaPonderada arestas[] = {{0, 1, 0.5, 1}, {1, 1, -0.5, 2}, {1, 2, 0.25, 3}};
    ResultadoBellmanFord *res = bellman_ford_executar(3, arestas, 3, mapa);
    assert(res != NULL);
    verificar_ciclo(res, arestas, 3, 3);
    assert(res->itens[0].comprimento == 1);
    assert(res->itens[0].vertice_indices[0] == 1);
    assert(res->itens[0].id_transacoes[0] == 2);
    assert(fabs(res->itens[0].peso_total - (-0.5)) < 1e-12);
    bellman_ford_imprimir_resultado(res, 3, 3);
    bellman_ford_liberar(res);

    TipoRepr tipos[] = {REPR_LISTA, REPR_MATRIZ};
    for (int t = 0; t < 2; t++) {
        res = executar_em(tipos[t], 3, arestas, 3, mapa);
        verificar_ciclo(res, arestas, 3, 3);
        assert(res->itens[0].comprimento == 1);
        assert(res->itens[0].id_transacoes[0] == 2);
        bellman_ford_liberar(res);
    }
    label_map_destruir(mapa);

    /* grafo de um único vértice com laço negativo: antes não era reconstruído */
    ArestaPonderada so_laco[] = {{0, 0, -1.0, 1}};
    res = bellman_ford_executar(1, so_laco, 1, NULL);
    assert(res != NULL);
    verificar_ciclo(res, so_laco, 1, 1);
    bellman_ford_liberar(res);

    /* laços não negativos não são ciclos negativos */
    ArestaPonderada lacos_ok[] = {{0, 0, 0.0, 1}, {1, 1, 0.75, 2}};
    res = bellman_ford_executar(2, lacos_ok, 2, NULL);
    assert(res != NULL && res->possui_ciclo_negativo == 0);
    bellman_ford_liberar(res);
}

/* ---------- tolerância de ponto flutuante ---------- */
static void testar_tolerancia(void) {
    printf("Tolerância: ruído de ponto flutuante não vira ciclo negativo...\n");

    /* Taxas 3 e 1/3: produto exatamente 1 no papel; em ponto flutuante a
     * soma de logaritmos pode sobrar alguns 1e-16. Não é arbitragem. */
    ArestaPonderada ruido[] = {
        {0, 1, calcular_peso_transacao(1.0, 3.0), 1},
        {1, 0, calcular_peso_transacao(3.0, 1.0), 2},
    };
    ResultadoBellmanFord *res = bellman_ford_executar(2, ruido, 2, NULL);
    assert(res != NULL && res->possui_ciclo_negativo == 0);
    bellman_ford_liberar(res);

    /* Ciclo de ganho minúsculo, -1e-12: ruído com a tolerância padrão (1e-9),
     * mas detectado se a tolerância for zerada. */
    ArestaPonderada minusculo[] = {{0, 1, 1.0, 1}, {1, 0, -1.0 - 1e-12, 2}};
    res = bellman_ford_executar(2, minusculo, 2, NULL);
    assert(res != NULL && res->possui_ciclo_negativo == 0);
    assert(fabs(res->tolerancia - BF_TOLERANCIA_PADRAO) < 1e-20);
    bellman_ford_liberar(res);

    res = bellman_ford_executar_tol(2, minusculo, 2, NULL, 0.0);
    assert(res != NULL);
    verificar_ciclo(res, minusculo, 2, 2);
    bellman_ford_liberar(res);

    /* Ganho de 0,1% (-ln 1,001 ~ -9,995e-4) está bem acima da tolerância. */
    ArestaPonderada pequeno[] = {
        {0, 1, 0.0, 1}, {1, 0, calcular_peso_transacao(1000.0, 1001.0), 2},
    };
    res = bellman_ford_executar(2, pequeno, 2, NULL);
    assert(res != NULL);
    verificar_ciclo(res, pequeno, 2, 2);
    bellman_ford_liberar(res);

    /* Ciclo de pesos todos zero (taxa 1 em cada transação) */
    ArestaPonderada zeros[] = {{0, 1, 0.0, 1}, {1, 2, 0.0, 2}, {2, 0, 0.0, 3}};
    res = bellman_ford_executar(3, zeros, 3, NULL);
    assert(res != NULL && res->possui_ciclo_negativo == 0);
    bellman_ford_liberar(res);
}

/* ---------- arestas paralelas ---------- */
static void testar_arestas_paralelas(void) {
    printf("Arestas paralelas: vale a de menor peso nas duas representações...\n");

    /* 0 -> 1 com pesos 1,0 e -3,0; 1 -> 0 com peso 2,0.
     * Só a paralela de -3,0 forma ciclo negativo (-3 + 2 = -1). */
    ArestaPonderada arestas[] = {{0, 1, 1.0, 1}, {0, 1, -3.0, 2}, {1, 0, 2.0, 3}};

    TipoRepr tipos[] = {REPR_LISTA, REPR_MATRIZ};
    for (int t = 0; t < 2; t++) {
        ResultadoBellmanFord *res = executar_em(tipos[t], 2, arestas, 3, NULL);
        verificar_ciclo(res, arestas, 3, 2);
        assert(fabs(res->itens[0].peso_total - (-1.0)) < 1e-12);
        /* a transação #2 (a de peso -3) é a que aparece no ciclo */
        int tem_tx2 = 0;
        for (int k = 0; k < 2; k++) {
            if (res->itens[0].id_transacoes[k] == 2) tem_tx2 = 1;
        }
        assert(tem_tx2);
        bellman_ford_liberar(res);
    }
}

/* ---------- argumentos inválidos ---------- */
static void testar_argumentos_invalidos(void) {
    printf("Argumentos inválidos retornam NULL...\n");

    ArestaPonderada ok[] = {{0, 1, 1.0, 1}};
    ArestaPonderada fora[] = {{0, 5, 1.0, 1}};
    ArestaPonderada negativo[] = {{-1, 0, 1.0, 1}};

    assert(bellman_ford_executar(0, ok, 1, NULL) == NULL);
    assert(bellman_ford_executar(2, NULL, 1, NULL) == NULL);
    assert(bellman_ford_executar(2, fora, 1, NULL) == NULL);
    assert(bellman_ford_executar(2, negativo, 1, NULL) == NULL);
    assert(bellman_ford_executar_tol(2, ok, 1, NULL, -1.0) == NULL);
    assert(bellman_ford_executar_rep(NULL, NULL, BF_TOLERANCIA_PADRAO) == NULL);

    /* sem arestas: nada a relaxar */
    ResultadoBellmanFord *res = bellman_ford_executar(3, NULL, 0, NULL);
    assert(res != NULL && res->possui_ciclo_negativo == 0);
    bellman_ford_liberar(res);

    bellman_ford_liberar(NULL); /* não pode quebrar */
}

int main(void) {
    size_t base = mem_bytes_atuais();

    testar_ciclo_negativo();
    testar_sem_ciclo_negativo();
    testar_ciclo_desconectado();
    testar_aresta_negativa_sem_ciclo();
    testar_laco_negativo();
    testar_tolerancia();
    testar_arestas_paralelas();
    testar_argumentos_invalidos();

    assert(mem_bytes_atuais() == base); /* nenhum vazamento */
    printf("Bellman-Ford validado com sucesso!\n");
    return 0;
}
