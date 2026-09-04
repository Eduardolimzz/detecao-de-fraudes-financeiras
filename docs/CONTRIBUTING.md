# Guia de Contribuição

## Fluxo de branches

- `main` é protegida: nenhum push direto é permitido.
- Todo trabalho novo deve ser feito em uma branch `feature/nn-descricao`
  (ex.: `feature/01-lista-adjacencia`, `feature/02-dfs-ciclos`).
- Ao concluir, abra um **Pull Request** para `main`.
- O PR exige **pelo menos 1 review** aprovado antes do merge.

## Definition of Done

Um PR só pode ser mergeado quando:

- [ ] O código compila com `make` **sem nenhum warning**.
- [ ] Não há vazamento de memória (`valgrind --leak-check=full`).
- [ ] A funcionalidade foi validada contra pelo menos um grafo-brinquedo em
      `tests/`.
- [ ] O PR está vinculado a uma issue (`Closes #nn` na descrição).
