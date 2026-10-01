#include "label_map.h"
#include "mem_track.h"
#include "representacao.h"

#include <assert.h>
#include <stdio.h>

/* Como os módulos do grafo alocam via mem_*, o contador também serve como
 * detector simples de vazamento: depois de liberar tudo, deve voltar à base. */
static void testar_modulos_nao_vazam(void) {
    printf("Testando que os módulos do grafo devolvem toda a memória...\n");
    size_t base = mem_bytes_atuais();

    TipoRepr tipos[] = {REPR_LISTA, REPR_MATRIZ};
    for (int t = 0; t < 2; t++) {
        RepGrafo *g = rep_criar(tipos[t], 200);
        assert(g != NULL);
        assert(mem_bytes_atuais() > base);
        for (int i = 0; i < 199; i++) {
            assert(rep_inserir_aresta(g, i, i + 1, 1.0));
        }
        rep_liberar(g);
        assert(mem_bytes_atuais() == base);
    }

    LabelMap *map = label_map_criar(4);
    assert(map != NULL);
    char rotulo[32];
    for (int i = 0; i < 1000; i++) { /* força redimensionamentos */
        snprintf(rotulo, sizeof(rotulo), "conta-%d", i);
        assert(label_map_obter_ou_inserir(map, rotulo) == i);
    }
    label_map_destruir(map);
    assert(mem_bytes_atuais() == base);
}

int main(void) {
    testar_modulos_nao_vazam();
    printf("Módulos do grafo sem vazamento segundo o contador!\n");
    return 0;
}
