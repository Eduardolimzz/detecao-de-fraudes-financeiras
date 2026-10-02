# Como rodar o projeto (do zero)

Guia de referência rápida: resumo do estado atual + passo a passo de
compilação e execução, assumindo uma máquina que nunca viu este projeto.

## 1. O que o projeto faz

Detecta padrões de fraude financeira (lavagem de dinheiro) modelando
transações bancárias como um **grafo dirigido**: cada conta é um vértice,
cada transação é uma aresta, e a Fase I usa busca em profundidade (DFS)
para encontrar ciclos de transações.

## 2. Módulos existentes hoje

| Módulo | Responsabilidade |
|---|---|
| `csv_parser` | Lê um CSV genérico linha a linha (sem saber o que as colunas significam), isolando aspas/vírgulas internas e contando linhas malformadas. |
| `label_map` | Hash table autoral: converte cada rótulo `"banco:conta"` em um índice inteiro e vice-versa. |
| `grafo` | Lista de adjacência dirigida autoral (um vértice aponta para sua lista de arestas de saída). |
| `matriz_grafo` | Matriz de adjacência dirigida autoral (conectividade apenas, sem guardar arestas paralelas). |
| `representacao` | Camada de abstração única sobre lista/matriz (RF02): os algoritmos só enxergam `RepGrafo`, nunca uma das duas diretamente. |
| `carregador` | Pipeline completo CSV → grafo: lê o arquivo, monta os rótulos, cria o `RepGrafo` e insere as arestas (RF01). |
| `bench` | Cronômetro e leitura de pico de memória do processo; grava um log CSV por execução (RF03). |
| `mem_track` | Contador autoral de bytes alocados/liberados, substitui `malloc`/`free` nos módulos do grafo. |
| `dfs` | Busca em profundidade **iterativa**, com pilha explícita no heap (nunca recursão). |
| `deteccao_ciclos` | Detecta ciclos a partir das arestas de retorno da DFS (esquema branco/cinza/preto) e reconstrói cada ciclo com os rótulos originais das contas. |
| `validacao_lavagem` | Cruza os ciclos encontrados com a coluna `Is Laundering` do CSV, para saber se algum ciclo "bate" com um caso de lavagem conhecido. |
| `cli` | Interpreta os argumentos de linha de comando (`--repr`, `--dataset`, `--limit`, `--help`). |

## 3. O que já está pronto (com teste passando) vs. o que falta

**Pronto e validado:**
- Carga do grafo a partir do CSV (RF01) — `tests/test_carregador.c`
- Lista e matriz de adjacência, alternáveis por flag (RF02) — `tests/test_grafo.c`, `tests/test_matriz_grafo.c`, `tests/test_representacao.c`
- Hash de rótulos — `tests/test_label_map.c`
- Log de tempo/memória por execução (RF03) — `tests/test_bench.c`, `tests/test_mem_track.c`
- DFS iterativa + detecção de ciclos, validadas num grafo-brinquedo com ciclo conhecido antes do dataset real — `tests/test_deteccao_ciclos.c`
- Subgrafo induzido via `--limit` para testes de estresse — `tests/test_limit.sh`

**Ainda não existe (Fase II):**
- Bellman-Ford (caminhos de menor custo)
- Investigação de Caminho Hamiltoniano
- Artigo em LaTeX (pasta `artigo/` ainda vazia)

## 4. Pré-requisitos

- Linux (testado em Ubuntu 24.04)
- `gcc` com suporte a C11 (testado com gcc 13.3; qualquer gcc/clang
  razoavelmente recente serve, não há uso de extensões além do C11 padrão
  + `clock_gettime` POSIX)
- `make`
- `valgrind` (opcional, só para quem quiser conferir vazamento de memória)

Nenhuma outra biblioteca é necessária — o projeto é C puro, sem
dependências externas (RNF01).

## 5. Clonar e compilar

```bash
git clone <url-do-repositorio>
cd detecao-de-fraudes-financeiras
make clean && make
```

Se tudo estiver certo, a saída é só a lista de comandos `gcc` sendo
executados, **sem nenhum warning**, terminando na linha que gera
`build/grafos`. O binário fica em `build/grafos`.

## 6. Rodar a suíte de testes

```bash
make test
```

Isso compila um executável por arquivo `tests/test_*.c` e roda todos em
sequência. Espere ver uma seção `== build/test_<nome>` para cada um, cada
uma terminando com uma linha de sucesso (ex.: "Pipeline de carga validado
com sucesso!", "Detecção de ciclos validada contra o grafo-brinquedo com
sucesso!"). Se qualquer `assert()` falhar, o teste correspondente aborta e
o `make test` para ali (não tenta rodar os seguintes).

Há também `tests/test_limit.sh`, rodado à parte:

```bash
bash tests/test_limit.sh
```

## 7. Rodar o binário principal no dataset real

O dataset (`data/dataset.csv`, 32.386 contas e 20.000 transações) já vem
versionado no repositório — não precisa baixar nada.

```bash
# representação em lista de adjacência (padrão)
./build/grafos --repr=lista

# representação em matriz de adjacência
./build/grafos --repr=matriz
```

Os dois imprimem o mesmo resumo de carga (|V|, |E|, tempo, memória) e em
seguida os ciclos encontrados. Os números de ciclos **não são idênticos
entre as duas representações** (2.396 na lista contra 2.371 na matriz, na
última execução registrada): a lista guarda cada transação separadamente,
então uma conta com várias transações para si mesma gera um auto-laço por
transação; a matriz só marca a conectividade, então transações repetidas
entre o mesmo par de contas colapsam em uma única aresta. Isso é
comportamento esperado, documentado em `include/representacao.h` e em
`docs/achados-fase1.md`.

Outras flags úteis:

```bash
./build/grafos --dataset=outro_arquivo.csv   # usar outro CSV
./build/grafos --limit=1000                  # subgrafo induzido pelos 1000 primeiros vértices
./build/grafos --help                        # lista todas as opções
```

## 8. Onde ficam os resultados

- `results/log_execucao.csv`: uma linha por execução (carga e detecção de
  ciclos), com timestamp, algoritmo, representação, `|V|`, `|E|`, tempo em
  ms e pico de memória em KB. É acrescentado automaticamente a cada vez
  que `./build/grafos` roda — não precisa de nenhuma flag extra.
- `docs/achados-fase1.md`: interpretação dos resultados da Fase I (quantos
  ciclos existem, comprimento típico, se batem com os casos de lavagem
  rotulados), cada afirmação com o comando exato que a reproduz.