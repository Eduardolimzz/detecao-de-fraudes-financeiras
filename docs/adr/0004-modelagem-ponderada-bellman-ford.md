# ADR 0004 — Modelagem ponderada da Fase II e Bellman-Ford

Status: proposto (validar no review do PR da issue #37)

## Contexto
A Fase II procura candidatos a arbitragem: transações encadeadas que
devolvem à conta de partida mais valor do que saiu. Isso exige pesos nas
arestas, ao contrário da Fase I (ADR 0001). A modelagem foi implementada na
issue #35, mas só estava documentada em comentários de código.

## Decisão
- O vértice é o estado `banco:conta:moeda`. Cada transação é uma aresta de
  (origem, Payment Currency) para (destino, Receiving Currency). Paralelas e
  auto-laços são mantidos. A Fase I continua usando `banco:conta`.
- O peso é `w = -ln(r)`, com `r = Amount Received / Amount Paid`. O produto
  das taxas vira soma de pesos: ciclo de peso total negativo equivale a
  produto das taxas maior que 1.
- Transações com valores não positivos ou ilegíveis não geram aresta. No
  sumário, `|E|` conta as transações e `|E_bf|`, as arestas ponderadas.
- Todos os vértices começam com distância 0 (fonte virtual), então o ciclo é
  achado em qualquer componente, alcançável ou não.
- O predecessor guarda o índice da aresta, para informar a transação
  (`id_transacao`, ordem no CSV) de cada passo do ciclo. Um ciclo é
  reconstruído por execução.
- Tolerância `BF_TOLERANCIA_PADRAO = 1e-9`: uma relaxação exige melhora maior
  que ela e o ciclo só vale se o peso total for menor que `-1e-9`. Isso evita
  que ruído de ponto flutuante (por exemplo `-1e-16`) vire ciclo negativo.
- O algoritmo roda sobre `RepGrafo`, em lista ou matriz. A matriz guarda só a
  paralela de menor peso, o que não muda a existência de ciclo negativo.
- Complexidade `O(|V| x |E|)`, com parada antecipada quando uma passada não
  altera nenhuma distância.
- A saída diz "candidato a arbitragem segundo a modelagem", nunca "fraude".
  `Is Laundering` não é usada (ADR 0001).

## Consequências
- No dataset oficial: 32.709 estados, 20.000 arestas ponderadas e nenhum ciclo
  negativo, com convergência em 3 passadas. Das 2.396 auto-transações de
  conta, 2.100 são laços na mesma moeda (`w = 0`) e 296 são conversões de
  moeda na própria conta, que ligam dois estados distintos.
- Um ciclo negativo só aparece na fixture `tests/fixtures/arbitragem_brinquedo.csv`,
  construída de propósito.
- A taxa é a implícita na transação, não uma taxa de mercado: tarifas e
  arredondamentos entram nela. O ciclo é um candidato a investigar, não uma
  prova. Não são criadas arestas de conversão de moeda além das do CSV.
- `--limit` vale para o grafo topológico; o Bellman-Ford usa todos os estados.
- A matriz ponderada usa 9 bytes por célula (cerca de 9,6 GB para 32.709
  estados). O `main` pula o Bellman-Ford em matriz acima de 2 GB e avisa.
