# Achados da Fase I — DFS e detecção de ciclos

Resultados obtidos rodando a detecção de ciclos (`src/dfs.c` +
`src/deteccao_ciclos.c`) sobre `data/dataset.csv` (32.386 vértices, 20.000
transações), nas duas representações. Cada afirmação abaixo vem acompanhada
do comando que a produziu.

## 1. Quantos ciclos existem no grafo

```bash
./build/grafos --repr=lista
./build/grafos --repr=matriz
```

- **Representação lista**: 2.396 ciclos encontrados.
- **Representação matriz**: 2.371 ciclos encontrados.

A diferença (25 ciclos) é esperada e consistente com o ADR 0001: a lista de
adjacência preserva transações paralelas (a mesma conta reinvestindo em si
mesma várias vezes gera um auto-laço por transação), enquanto a matriz
colapsa arestas paralelas na mesma célula e conta cada par origem/destino
uma única vez. Não é uma divergência de algoritmo, e sim da representação
subjacente, documentada em `include/representacao.h`.

## 2. Comprimento típico dos ciclos encontrados

Saída de `./build/grafos --repr=lista`:

```
Distribuição por comprimento:
  comprimento 1: 2396 ciclo(s)
```

**100% dos ciclos encontrados são auto-laços (comprimento 1)** — uma conta
que envia uma transação para si mesma (no dataset, isso corresponde
principalmente a `Payment Format = Reinvestment`). **Nenhum ciclo de
comprimento ≥ 2 foi encontrado** em nenhuma das duas representações.

Isso é coerente com a esparsidade do grafo: 20.000 arestas para 32.386
vértices dão um grau de saída médio de ≈0,62, muito abaixo do necessário
para que ciclos multi-vértice apareçam com frequência numa amostra
aleatória de transações. A DFS com back-edges (BRANCO/CINZA/PRETO) detecta
corretamente a existência de ciclos; o resultado negativo para ciclos
maiores é uma característica do grafo amostrado, não uma falha do
algoritmo — confirmado também pelo grafo-brinquedo (`tests/test_deteccao_ciclos.c`),
onde os ciclos de 3 e 4 vértices são encontrados corretamente.

## 3. As contas em ciclos concentram rótulos de lavagem?

```bash
awk -F, '$11==1 {print $2":"$3" -> "$4":"$5}' data/dataset.csv
```

As 21 transações marcadas `Is Laundering=1` no dataset têm origem e destino
sempre em contas **distintas** (nenhuma é um auto-laço). Como os únicos
ciclos encontrados pela DFS são auto-laços, **nenhum ciclo detectado
contém uma transação marcada como lavagem**:

```
Transações marcadas Is Laundering=1 no CSV: 21
Ciclos com ao menos uma transação suspeita: 0 de 2396   (lista)
Ciclos com ao menos uma transação suspeita: 0 de 2371   (matriz)
```

**Resultado negativo, reportado explicitamente**: neste subconjunto de
20.000 transações, a assinatura "ciclo de transações" não coincide com os
casos de lavagem rotulados. Isso é esperado dado o padrão de lavagem
predominante no gabarito (`data/padroes_lavagem.txt`): os tipos mais
comuns ali são `FAN-OUT`, `GATHER-SCATTER`, `STACK`, `BIPARTITE` e
`RANDOM` — topologias de dispersão/concentração, não ciclos fechados. O
padrão `CYCLE` existe no gabarito, mas as contas desses casos específicos
aparentemente não caíram dentro da amostra de 20.000 transações fixada
(ADR 0002), ou os ciclos de lavagem reais têm contas que não fecham um
ciclo dentro do subconjunto amostrado. Validar isso com certeza exigiria
cruzar diretamente com as transações `CYCLE` de `padroes_lavagem.txt`
presentes no subconjunto — fica como trabalho futuro (Fase II, já que o
valor financeiro das transações poderá ajudar com Bellman-Ford).

## 4. Ilhas desconectadas de contas

**Pendente.** Não foi medido nesta etapa (exigiria compor fracamente/for­te­mente
componentes conexos, fora do escopo de DFS+ciclos). Fica para análise
futura.

## Reprodutibilidade

Todos os números acima vêm de:

```bash
make clean && make
./build/grafos --repr=lista
./build/grafos --repr=matriz
awk -F, '$11==1 {print $2":"$3" -> "$4":"$5}' data/dataset.csv
```
