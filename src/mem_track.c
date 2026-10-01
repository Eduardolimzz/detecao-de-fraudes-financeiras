#include "mem_track.h"

#include <stdlib.h>

/* O cabeçalho guarda o tamanho do bloco. A union com long double garante
 * que o ponteiro entregue ao usuário continue bem alinhado. */
typedef union {
    size_t tamanho;
    long double alinhamento;
} CabecalhoMem;

static size_t bytes_atuais = 0;
static size_t bytes_pico = 0;
static size_t bytes_total = 0;

static void registrar_alocacao(size_t bytes) {
    bytes_atuais += bytes;
    bytes_total += bytes;
    if (bytes_atuais > bytes_pico) {
        bytes_pico = bytes_atuais;
    }
}

static CabecalhoMem *cabecalho_de(void *ptr) {
    return (CabecalhoMem *)ptr - 1;
}

void *mem_malloc(size_t bytes) {
    if (bytes > (size_t)-1 - sizeof(CabecalhoMem)) {
        return NULL;
    }

    CabecalhoMem *cab = malloc(sizeof(CabecalhoMem) + bytes);
    if (cab == NULL) {
        return NULL;
    }

    cab->tamanho = bytes;
    registrar_alocacao(bytes);
    return cab + 1;
}

void *mem_calloc(size_t n, size_t tamanho) {
    if (tamanho != 0 && n > (size_t)-1 / tamanho) {
        return NULL; /* overflow na multiplicação */
    }

    size_t bytes = n * tamanho;
    CabecalhoMem *cab = calloc(1, sizeof(CabecalhoMem) + bytes);
    if (cab == NULL) {
        return NULL;
    }

    cab->tamanho = bytes;
    registrar_alocacao(bytes);
    return cab + 1;
}

void *mem_realloc(void *ptr, size_t bytes) {
    if (ptr == NULL) {
        return mem_malloc(bytes);
    }
    if (bytes > (size_t)-1 - sizeof(CabecalhoMem)) {
        return NULL;
    }

    CabecalhoMem *antigo = cabecalho_de(ptr);
    size_t tamanho_antigo = antigo->tamanho;

    CabecalhoMem *novo = realloc(antigo, sizeof(CabecalhoMem) + bytes);
    if (novo == NULL) {
        return NULL; /* bloco original continua válido e contabilizado */
    }

    novo->tamanho = bytes;
    bytes_atuais -= tamanho_antigo;
    bytes_total += bytes;
    bytes_atuais += bytes;
    if (bytes_atuais > bytes_pico) {
        bytes_pico = bytes_atuais;
    }
    return novo + 1;
}

void mem_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    CabecalhoMem *cab = cabecalho_de(ptr);
    bytes_atuais -= cab->tamanho;
    free(cab);
}

size_t mem_bytes_atuais(void) {
    return bytes_atuais;
}

size_t mem_bytes_pico(void) {
    return bytes_pico;
}

size_t mem_bytes_total_alocado(void) {
    return bytes_total;
}

void mem_resetar_pico(void) {
    bytes_pico = bytes_atuais;
    bytes_total = 0;
}
