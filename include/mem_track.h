#ifndef MEM_TRACK_H
#define MEM_TRACK_H

#include <stddef.h>

/*
 * Contador autoral de bytes alocados (issue #10).
 *
 * Substitui malloc/calloc/realloc/free nos módulos que constroem o grafo.
 * Cada bloco recebe um cabeçalho com o seu tamanho, o que permite saber
 * quantos bytes estão vivos e qual foi o pico, sem depender do sistema
 * operacional. Os blocos devolvidos são alinhados como os do malloc.
 *
 * Importante: memória obtida com mem_* só pode ser liberada com mem_free.
 * Não é thread-safe (o projeto é single-thread).
 */

void *mem_malloc(size_t bytes);
void *mem_calloc(size_t n, size_t tamanho);
void *mem_realloc(void *ptr, size_t bytes);
void mem_free(void *ptr);

/* Bytes úteis (sem contar o cabeçalho) atualmente alocados. */
size_t mem_bytes_atuais(void);

/* Maior valor que mem_bytes_atuais() atingiu desde o início ou reset. */
size_t mem_bytes_pico(void);

/* Total acumulado de bytes pedidos (nunca diminui com free). */
size_t mem_bytes_total_alocado(void);

/* Zera o pico (passa a valer os bytes atuais) e o total acumulado.
 * Útil para medir uma fase isolada, como a construção do grafo. */
void mem_resetar_pico(void);

#endif
