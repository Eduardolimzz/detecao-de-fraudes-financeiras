#include "label_map.h"
#include "mem_track.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPACIDADE_PADRAO_BUCKETS 16
#define FATOR_CARGA_MAXIMO 0.75

typedef struct LabelNode {
    char *rotulo;
    int indice;
    struct LabelNode *next;
} LabelNode;

struct LabelMap {
    LabelNode **buckets;
    size_t num_buckets;
    char **rotulos_por_indice; 
    int capacidade_rotulos;
    int tamanho;
};

static char *copiar_string(const char *s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s);
    char *copia = mem_malloc(len + 1);
    if (copia != NULL) {
        memcpy(copia, s, len + 1);
    }
    return copia;
}


static unsigned int hash_fnv1a(const char *str) {
    unsigned int hash = 2166136261u;
    while (*str) {
        hash ^= (unsigned char)(*str++);
        hash *= 16777619u;
    }
    return hash;
}

static int redimensionar_buckets(LabelMap *map) {
    size_t nova_capacidade = map->num_buckets * 2;
    LabelNode **novos_buckets = mem_calloc(nova_capacidade, sizeof(LabelNode *));
    if (novos_buckets == NULL) {
        return 0;
    }

    for (size_t i = 0; i < map->num_buckets; i++) {
        LabelNode *no_atual = map->buckets[i];
        while (no_atual != NULL) {
            LabelNode *proximo = no_atual->next;
            unsigned int novo_idx = hash_fnv1a(no_atual->rotulo) % nova_capacidade;
            no_atual->next = novos_buckets[novo_idx];
            novos_buckets[novo_idx] = no_atual;
            no_atual = proximo;
        }
    }

    mem_free(map->buckets);
    map->buckets = novos_buckets;
    map->num_buckets = nova_capacidade;
    return 1;
}

LabelMap *label_map_criar(size_t capacidade_inicial) {
    LabelMap *map = mem_malloc(sizeof(LabelMap));
    if (map == NULL) {
        return NULL;
    }

    size_t buckets_count = (capacidade_inicial > 0) ? capacidade_inicial : CAPACIDADE_PADRAO_BUCKETS;
    map->buckets = mem_calloc(buckets_count, sizeof(LabelNode *));
    if (map->buckets == NULL) {
        mem_free(map);
        return NULL;
    }

    int cap_rotulos = (int)buckets_count;
    map->rotulos_por_indice = mem_malloc((size_t)cap_rotulos * sizeof(char *));
    if (map->rotulos_por_indice == NULL) {
        mem_free(map->buckets);
        mem_free(map);
        return NULL;
    }

    map->num_buckets = buckets_count;
    map->capacidade_rotulos = cap_rotulos;
    map->tamanho = 0;

    return map;
}

int label_map_obter_ou_inserir(LabelMap *map, const char *rotulo) {
    if (map == NULL || rotulo == NULL) {
        return -1;
    }

    unsigned int idx_bucket = hash_fnv1a(rotulo) % map->num_buckets;
    LabelNode *no = map->buckets[idx_bucket];
    while (no != NULL) {
        if (strcmp(no->rotulo, rotulo) == 0) {
            return no->indice;
        }
        no = no->next;
    }

    if ((double)map->tamanho / (double)map->num_buckets >= FATOR_CARGA_MAXIMO) {
        if (redimensionar_buckets(map)) {
            idx_bucket = hash_fnv1a(rotulo) % map->num_buckets;
        }
    }

    if (map->tamanho >= map->capacidade_rotulos) {
        int nova_cap = map->capacidade_rotulos * 2;
        char **novo_array = mem_realloc(map->rotulos_por_indice, (size_t)nova_cap * sizeof(char *));
        if (novo_array == NULL) {
            return -1;
        }
        map->rotulos_por_indice = novo_array;
        map->capacidade_rotulos = nova_cap;
    }

    char *copia = copiar_string(rotulo);
    if (copia == NULL) {
        return -1;
    }

    LabelNode *novo_no = mem_malloc(sizeof(LabelNode));
    if (novo_no == NULL) {
        mem_free(copia);
        return -1;
    }

    int novo_indice = map->tamanho;
    novo_no->rotulo = copia;
    novo_no->indice = novo_indice;
    novo_no->next = map->buckets[idx_bucket];
    map->buckets[idx_bucket] = novo_no;

    map->rotulos_por_indice[novo_indice] = copia;
    map->tamanho++;

    return novo_indice;
}

int label_map_obter_indice(const LabelMap *map, const char *rotulo) {
    if (map == NULL || rotulo == NULL) {
        return -1;
    }

    unsigned int idx_bucket = hash_fnv1a(rotulo) % map->num_buckets;
    LabelNode *no = map->buckets[idx_bucket];
    while (no != NULL) {
        if (strcmp(no->rotulo, rotulo) == 0) {
            return no->indice;
        }
        no = no->next;
    }

    return -1;
}

const char *label_map_obter_rotulo(const LabelMap *map, int indice) {
    if (map == NULL || indice < 0 || indice >= map->tamanho) {
        return NULL;
    }
    return map->rotulos_por_indice[indice];
}

int label_map_tamanho(const LabelMap *map) {
    if (map == NULL) {
        return 0;
    }
    return map->tamanho;
}

void label_map_destruir(LabelMap *map) {
    if (map == NULL) {
        return;
    }

    for (size_t i = 0; i < map->num_buckets; i++) {
        LabelNode *atual = map->buckets[i];
        while (atual != NULL) {
            LabelNode *temp = atual;
            atual = atual->next;
            mem_free(temp);
        }
    }
    mem_free(map->buckets);

    for (int i = 0; i < map->tamanho; i++) {
        mem_free(map->rotulos_por_indice[i]);
    }
    mem_free(map->rotulos_por_indice);

    mem_free(map);
}