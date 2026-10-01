#ifndef BENCH_H
#define BENCH_H

/*
 * Instrumentação de tempo e memória (RF03).
 *
 * Uso típico:
 *     Cronometro c;
 *     bench_cronometro_iniciar(&c);
 *     ... algoritmo ...
 *     double ms = bench_cronometro_ms(&c);
 *     bench_registrar_log("dfs_ciclos", "lista", nv, ne, ms);
 */

#define BENCH_LOG_PADRAO "results/log_execucao.csv"

typedef struct {
    double inicio_ms;
} Cronometro;

/* Relógio monotônico: clock_gettime(CLOCK_MONOTONIC) em Linux/POSIX
 * (QueryPerformanceCounter no Windows). */
void bench_cronometro_iniciar(Cronometro *c);

/* Milissegundos decorridos desde bench_cronometro_iniciar. */
double bench_cronometro_ms(const Cronometro *c);

/* Pico de memória residente do processo, em KB (VmHWM de
 * /proc/self/status no Linux; PeakWorkingSetSize no Windows).
 * Retorna -1 se não for possível ler. */
long bench_pico_rss_kb(void);

/* Acrescenta uma linha ao log CSV, criando o arquivo (com cabeçalho) se
 * ele não existir:
 *   timestamp,algoritmo,representacao,n_vertices,n_arestas,tempo_ms,memoria_kb
 * tempo_ms é gravado com 3 casas decimais e memoria_kb é o pico de RSS
 * no momento da chamada. Retorna 1 se gravou, 0 se falhou. */
int bench_registrar_log_em(const char *caminho, const char *algoritmo,
                           const char *representacao, int n_vertices,
                           int n_arestas, double tempo_ms);

/* Igual a bench_registrar_log_em, usando BENCH_LOG_PADRAO. */
int bench_registrar_log(const char *algoritmo, const char *representacao,
                        int n_vertices, int n_arestas, double tempo_ms);

#endif
