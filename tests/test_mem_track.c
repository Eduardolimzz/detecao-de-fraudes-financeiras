#include "mem_track.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void testar_malloc_e_free(void) {
    printf("Testando mem_malloc/mem_free...\n");
    size_t base = mem_bytes_atuais();

    char *a = mem_malloc(100);
    assert(a != NULL);
    assert(mem_bytes_atuais() == base + 100);
    assert((uintptr_t)a % sizeof(void *) == 0); /* alinhamento */
    memset(a, 0xAB, 100);

    char *b = mem_malloc(50);
    assert(mem_bytes_atuais() == base + 150);

    mem_free(a);
    assert(mem_bytes_atuais() == base + 50);
    mem_free(b);
    assert(mem_bytes_atuais() == base);

    mem_free(NULL); /* não pode quebrar nem alterar o contador */
    assert(mem_bytes_atuais() == base);
}

static void testar_calloc_zera(void) {
    printf("Testando mem_calloc...\n");
    size_t base = mem_bytes_atuais();

    int *v = mem_calloc(10, sizeof(int));
    assert(v != NULL);
    assert(mem_bytes_atuais() == base + 10 * sizeof(int));
    for (int i = 0; i < 10; i++) {
        assert(v[i] == 0);
    }
    mem_free(v);
    assert(mem_bytes_atuais() == base);

    assert(mem_calloc((size_t)-1, 16) == NULL); /* overflow */
    assert(mem_bytes_atuais() == base);
}

static void testar_realloc(void) {
    printf("Testando mem_realloc...\n");
    size_t base = mem_bytes_atuais();

    int *v = mem_realloc(NULL, 4 * sizeof(int)); /* equivale a malloc */
    assert(v != NULL);
    for (int i = 0; i < 4; i++) {
        v[i] = i * 10;
    }
    assert(mem_bytes_atuais() == base + 4 * sizeof(int));

    v = mem_realloc(v, 1000 * sizeof(int)); /* cresce preservando dados */
    assert(v != NULL);
    for (int i = 0; i < 4; i++) {
        assert(v[i] == i * 10);
    }
    assert(mem_bytes_atuais() == base + 1000 * sizeof(int));

    v = mem_realloc(v, 2 * sizeof(int)); /* encolhe */
    assert(v != NULL);
    assert(mem_bytes_atuais() == base + 2 * sizeof(int));

    mem_free(v);
    assert(mem_bytes_atuais() == base);
}

static void testar_pico_e_total(void) {
    printf("Testando pico e total acumulado...\n");
    mem_resetar_pico();
    size_t base = mem_bytes_atuais();
    assert(mem_bytes_pico() == base);
    assert(mem_bytes_total_alocado() == 0);

    void *a = mem_malloc(1000);
    void *b = mem_malloc(500);
    assert(mem_bytes_pico() == base + 1500);

    mem_free(a);
    mem_free(b);
    assert(mem_bytes_atuais() == base);
    assert(mem_bytes_pico() == base + 1500);        /* pico não desce */
    assert(mem_bytes_total_alocado() == 1500);      /* total não desce */

    void *c = mem_malloc(200);
    assert(mem_bytes_pico() == base + 1500);        /* 200 < pico anterior */
    assert(mem_bytes_total_alocado() == 1700);
    mem_free(c);

    mem_resetar_pico();
    assert(mem_bytes_pico() == base);
    assert(mem_bytes_total_alocado() == 0);
}

int main(void) {
    testar_malloc_e_free();
    testar_calloc_zera();
    testar_realloc();
    testar_pico_e_total();
    printf("Contador de memória validado com sucesso!\n");
    return 0;
}
