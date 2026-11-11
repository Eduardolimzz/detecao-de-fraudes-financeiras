# ADR 0004: Modelagem ponderada da Fase II e Bellman-Ford

- **Status:** Proposto (validar no review do PR da issue #37)
- **Data:** 2026-11-11
- **Escopo:** Fase II — pesos, estados por moeda e ciclos negativos

## Contexto

A Fase II investiga **candidatos a arbitragem**: sequências de transações
que, encadeadas, devolvem à conta de partida mais valor do que saiu. Isso
exige pesos nas arestas, ao contrário da Fase I (ADR 0001), que só usa a
topologia. A modelagem ponderada foi implementada na issue #35, mas só
existia em comentários de código. Este ADR a documenta e registra as
decisões do Bellman-Ford da issue #37.

## Decisão

### Vértices: estados `banco:conta:moeda`

Uma mesma conta pode manter valor em moedas diferentes, e é a troca de
moeda que cria a possibilidade de ganho. Por isso, na Fase II o vértice é o
estado `banco:conta:moeda` (por exemplo `010:8000EBD30:Euro`), e não apenas
a conta do ADR 0001. A Fase I continua usando `banco:conta`.

### Arestas: uma por transação

Cada transação válida vira uma aresta dirigida:

```text
(banco_origem, conta_origem, Payment Currency)
    -> (banco_destino, conta_destino, Receiving Currency)
```

A origem usa a moeda **paga** e o destino usa a moeda **recebida**.
Transações repetidas entre os mesmos estados e auto-laços são preservados.

### Peso: `w = -ln(r)`, com `r = Amount Received / Amount Paid`

O peso é o logaritmo negativo da taxa implícita da transação. Assim, o
**produto** das taxas ao longo de um ciclo vira uma **soma** de pesos:

```text
soma(w) < 0   <=>   produto(r) > 1
```

Um ciclo de peso total negativo é, portanto, um ciclo em que o valor
recebido supera o valor pago ao longo da volta. É exatamente o que o
Bellman-Ford detecta. Pesos zero (`r = 1`, mesma moeda e mesmo valor),
positivos e negativos são todos legítimos.

Transações com `Amount Paid` ou `Amount Received` não positivos ou ilegíveis
não geram aresta (peso infinito). Elas continuam existindo no grafo
topológico da Fase I. A diferença aparece no sumário: `|E|` conta as
transações, `|E_bf|` conta as arestas ponderadas.

### Detecção de ciclos negativos

- Todos os vértices começam com distância 0, o que equivale a uma fonte
  virtual ligada a todos. Assim o ciclo é encontrado em **qualquer
  componente**, inclusive os que não são alcançáveis de um vértice inicial.
- Se a `|V|`-ésima passada ainda relaxa alguma aresta, há ciclo negativo. O
  ciclo é reconstruído andando `|V|` passos pelos predecessores e depois
  seguindo-os até fechar o ciclo.
- O predecessor guarda o **índice da aresta**, não só o vértice. Assim a saída
  informa qual transação (`id_transacao`, ordem da transação válida no CSV)
  foi usada em cada passo, mesmo havendo transações paralelas.
- Um ciclo é reconstruído por execução.
- Complexidade: `O(|V| x |E|)` no pior caso. Há parada antecipada quando uma
  passada não altera nenhuma distância; no dataset oficial isso ocorre na
  3ª passada.

### Tolerância de ponto flutuante

Os pesos são logaritmos e os totais são somas de `double`, então um ciclo
que no papel vale exatamente 0 pode sair como `-1e-16`. A tolerância
padrão é `BF_TOLERANCIA_PADRAO = 1e-9`:

- uma relaxação só ocorre se `dist[u] + w < dist[v] - tolerancia`;
- um ciclo só é aceito se o seu peso total for menor que `-tolerancia`.

Um ciclo com peso total entre `-1e-9` e `0` é tratado como ruído. Isso
equivale a ignorar ganhos menores que cerca de `1e-9` (0,0000001%). A
tolerância pode ser alterada por `bellman_ford_executar_tol`.

### Representações

O Bellman-Ford roda sobre `RepGrafo` (`bellman_ford_executar_rep`), na
lista ou na matriz escolhida em `--repr`. Na matriz, transações paralelas
já foram reduzidas à de menor peso; isso não altera a existência de ciclo
negativo, porque só a aresta mais barata entre dois estados importa para
ele. O ciclo reconstruído usa essa transação mais barata.

### Interpretação da saída

O resultado é sempre descrito como **candidato a arbitragem segundo esta
modelagem**. Ele não é classificado como fraude: a coluna `Is Laundering`
não é usada, em linha com o ADR 0001, e fica reservada à validação.

## Resultado no dataset oficial

Com `data/dataset.csv` (20.000 transações):

- 32.709 estados e 20.000 arestas ponderadas (nenhuma transação descartada);
- 301 transações em moedas diferentes (`r != 1`): 155 com `w < 0` e 146 com
  `w > 0`; as demais têm `r = 1` e `w = 0`;
- **nenhum ciclo negativo**, com convergência em 3 passadas.

O resultado é coerente com a Fase I, em que todos os 2.396 ciclos são
auto-laços de conta (comprimento 1). No grafo de estados, 2.100 deles são
laços na mesma moeda, com `w = 0`. Os outros 296 são conversões de moeda
dentro da própria conta (`w != 0`) e ligam dois estados distintos; como não
há ciclo negativo, nenhuma sequência delas fecha com ganho. Um ciclo
negativo só aparece nas fixtures de teste, construídas de propósito
(`tests/fixtures/arbitragem_brinquedo.csv`).

## Limitações

- **Não são criadas arestas de conversão de moeda.** Um ciclo só se fecha se
  cada passo for uma transação do CSV entre estados. Uma conversão dentro da
  mesma conta só existe no grafo quando consta no CSV como transação da conta
  para ela mesma (296 casos no dataset).
- **A taxa é a implícita na transação**, não uma taxa de mercado. Ela pode
  embutir tarifas e arredondamentos (os valores vêm com poucas casas), então
  um ciclo negativo é um candidato a ser investigado, não uma prova.
- **`--limit` não restringe as arestas ponderadas.** O subgrafo induzido
  vale para o grafo topológico; o Bellman-Ford usa todos os estados do CSV.
- **Memória da matriz ponderada:** cada célula guarda peso e presença
  (9 bytes). Com 32.709 estados seriam cerca de 9,6 GB. O `main` pula o
  Bellman-Ford em matriz acima de 2 GB e informa o motivo.
- Apenas um ciclo é reconstruído por execução.

## Consequências

- A Fase II tem pesos, estados e tolerância documentados, e o critério
  "candidato, não fraude" fica explícito na saída.
- O mesmo Bellman-Ford serve às duas representações, o que permite
  comparar tempo e memória entre elas (issue #39).
- O `id_transacao` permite voltar do ciclo ao CSV para auditoria.
