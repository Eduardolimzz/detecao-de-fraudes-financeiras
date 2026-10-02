#define _POSIX_C_SOURCE 199309L

#include "bench.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

#define CABECALHO_LOG \
    "timestamp,algoritmo,representacao,n_vertices,n_arestas,tempo_ms,memoria_kb\n"

static double agora_ms(void) {
#ifdef _WIN32
    LARGE_INTEGER freq, contador;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&contador);
    return (double)contador.QuadPart * 1000.0 / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
#endif
}

void bench_cronometro_iniciar(Cronometro *c) {
    c->inicio_ms = agora_ms();
}

double bench_cronometro_ms(const Cronometro *c) {
    return agora_ms() - c->inicio_ms;
}

long bench_pico_rss_kb(void) {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return (long)(pmc.PeakWorkingSetSize / 1024);
    }
    return -1;
#else
    FILE *f = fopen("/proc/self/status", "r");
    if (f == NULL) {
        return -1;
    }

    char linha[256];
    long kb = -1;
    while (fgets(linha, sizeof(linha), f) != NULL) {
        if (strncmp(linha, "VmHWM:", 6) == 0) {
            if (sscanf(linha + 6, "%ld", &kb) != 1) {
                kb = -1;
            }
            break;
        }
    }
    fclose(f);
    return kb;
#endif
}

/* Os nomes de algoritmo/representação são fixos no código, sem vírgulas;
 * aqui só protegemos contra NULL. */
static const char *campo_ou_vazio(const char *s) {
    return (s != NULL) ? s : "";
}

int bench_registrar_log_em(const char *caminho, const char *algoritmo,
                           const char *representacao, int n_vertices,
                           int n_arestas, double tempo_ms) {
    /* "ab+" cria se não existir e posiciona escritas sempre no fim */
    FILE *f = fopen(caminho, "ab+");
    if (f == NULL) {
        fprintf(stderr, "Aviso: não foi possível abrir o log '%s'.\n", caminho);
        return 0;
    }

    fseek(f, 0, SEEK_END);
    if (ftell(f) == 0) {
        fputs(CABECALHO_LOG, f);
    }

    char timestamp[32];
    time_t agora = time(NULL);
    struct tm *tm_local = localtime(&agora);
    if (tm_local == NULL ||
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_local) == 0) {
        strcpy(timestamp, "desconhecido");
    }

    int ok = fprintf(f, "%s,%s,%s,%d,%d,%.3f,%ld\n", timestamp,
                     campo_ou_vazio(algoritmo), campo_ou_vazio(representacao),
                     n_vertices, n_arestas, tempo_ms, bench_pico_rss_kb()) > 0;

    if (fclose(f) != 0) {
        ok = 0;
    }
    return ok;
}

int bench_registrar_log(const char *algoritmo, const char *representacao,
                        int n_vertices, int n_arestas, double tempo_ms) {
    return bench_registrar_log_em(BENCH_LOG_PADRAO, algoritmo, representacao,
                                  n_vertices, n_arestas, tempo_ms);
}
