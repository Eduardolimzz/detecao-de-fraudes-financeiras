#ifndef LABEL_MAP_H
#define LABEL_MAP_H

#include <stddef.h>

typedef struct LabelMap LabelMap;

/**
 * @param capacidade_inicial 
 * @return 
 */
LabelMap *label_map_criar(size_t capacidade_inicial);

/**
 * @param map 
 * @param rotulo 
 * @return 
 */
int label_map_obter_ou_inserir(LabelMap *map, const char *rotulo);

/**
 * @param map 
 * @param rotulo 
 * @return
 */
int label_map_obter_indice(const LabelMap *map, const char *rotulo);

/**
 * 
 *
 * @param map 
 * @param indice 
 * @return 
 */
const char *label_map_obter_rotulo(const LabelMap *map, int indice);


int label_map_tamanho(const LabelMap *map);


void label_map_destruir(LabelMap *map);

#endif 