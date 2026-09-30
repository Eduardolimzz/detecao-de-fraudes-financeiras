#include "label_map.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_INSERCOES 100000

void testar_mapeamento_100k(void) {
    printf("Iniciando teste unitário com %d inserções...\n", NUM_INSERCOES);

    LabelMap *map = label_map_criar(16);
    assert(map != NULL);

    char buffer_rotulo[32];


    for (int i = 0; i < NUM_INSERCOES; i++) {
        snprintf(buffer_rotulo, sizeof(buffer_rotulo), "0x%012X", i);
        int idx = label_map_obter_ou_inserir(map, buffer_rotulo);
        assert(idx == i); /* Garante contiguidade 0..V-1 */
    }

    assert(label_map_tamanho(map) == NUM_INSERCOES);


    for (int i = 0; i < NUM_INSERCOES; i++) {
        snprintf(buffer_rotulo, sizeof(buffer_rotulo), "0x%012X", i);

        int idx_encontrado = label_map_obter_indice(map, buffer_rotulo);
        assert(idx_encontrado == i);

        const char *rotulo_recuperado = label_map_obter_rotulo(map, i);
        assert(rotulo_recuperado != NULL);
        assert(strcmp(rotulo_recuperado, buffer_rotulo) == 0);
    }


    for (int i = 0; i < NUM_INSERCOES; i++) {
        snprintf(buffer_rotulo, sizeof(buffer_rotulo), "0x%012X", i);
        int idx_reinserido = label_map_obter_ou_inserir(map, buffer_rotulo);
        assert(idx_reinserido == i);
    }

    assert(label_map_tamanho(map) == NUM_INSERCOES);

    assert(label_map_obter_indice(map, "0xNAO_EXISTE") == -1);
    assert(label_map_obter_rotulo(map, -1) == NULL);
    assert(label_map_obter_rotulo(map, NUM_INSERCOES) == NULL);

    label_map_destruir(map);
    printf("Teste concluído com sucesso: 100.000 inserções validadas sem colisões lógicas!\n");
}

int main(void) {
    testar_mapeamento_100k();
    return 0;
}