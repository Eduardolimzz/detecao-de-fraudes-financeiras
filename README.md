# Detecção de Fraudes Financeiras em Grafos

**Tema sorteado:** Detecção de Fraudes Financeiras em Grafos.

Projeto Integrador do IESB que modela transações financeiras como um **grafo
dirigido**, onde cada **vértice** representa uma conta bancária e cada
**aresta** representa uma transação direcionada entre contas. O objetivo é
detectar padrões suspeitos de lavagem de dinheiro (fraudes financeiras)
utilizando algoritmos clássicos de teoria dos grafos.

- **Fase I**: detecção de ciclos via DFS (busca em profundidade), indicando
  possíveis esquemas de lavagem circular de valores.
- **Fase II**: detecção de caminhos de menor custo com Bellman-Ford e
  investigação de Caminho Hamiltoniano para rotas suspeitas que passam por
  todas as contas de uma rede.

## Integrantes

| Nome | Matrícula |
|------|--------|
| Eduardo Lima dos Santos | 2412130074 |
| Alessandro Ribeiro Moreira | 2412130120 |
| Alanna Tomaz | 2412130055 |
| Thaynara Ramos | 2312130186 |
| Heitor dos Santos Ribeiro | 2412130143 |

## Requisitos do ambiente

- Ubuntu (ou distribuição Linux equivalente)
- `gcc` (suporte a C11)
- `make`
- `valgrind` (verificação de vazamento de memória)

## Como compilar e executar

```bash
# Compilar (build otimizado, sem warnings)
make

# Compilar em modo debug (-g -O0 -DDEBUG)
make debug

# Compilar e executar
make run

# Limpar artefatos de build
make clean
```

O binário gerado fica em `build/grafos`.

## Estrutura de diretórios

```
src/        # código-fonte .c
include/    # headers .h
data/       # datasets (não versionado, ver seção "Dataset")
scripts/    # scripts auxiliares (shell/python)
docs/       # ADRs, relatórios e documentação do processo
results/    # logs de benchmark e gráficos gerados
artigo/     # artigo científico em LaTeX (formato SBC)
tests/      # grafos-brinquedo e fixtures para validação
build/      # objetos e binário compilados (não versionado)
```

## Dataset

Utilizamos o dataset **IBM Transactions for Anti-Money Laundering (AML)**,
disponível no Kaggle:
<https://www.kaggle.com/datasets/ealtman2019/ibm-transactions-for-anti-money-laundering-aml>

O dataset **não é versionado** neste repositório (ver `.gitignore`). Baixe os
arquivos manualmente e coloque-os em `data/`.

## Convenção de commits

Utilizamos [Conventional Commits](https://www.conventionalcommits.org/pt-br/).

Exemplos reais esperados neste projeto:

```
feat(dfs): implementa detecção de ciclos em grafo dirigido
fix(makefile): corrige regra de dependência de headers
docs(readme): adiciona instruções de compilação
test(dfs): adiciona grafo-brinquedo com ciclo conhecido
```

## Status

### Fase I
- [ ] Estrutura de lista de adjacência autoral
- [ ] Leitura e parsing do dataset
- [ ] Implementação de DFS
- [ ] Detecção de ciclos
- [ ] Validação com grafos-brinquedo

### Fase II
- [ ] Implementação de Bellman-Ford
- [ ] Investigação de Caminho Hamiltoniano
- [ ] Benchmarks e relatório de resultados
- [ ] Artigo final (formato SBC)
