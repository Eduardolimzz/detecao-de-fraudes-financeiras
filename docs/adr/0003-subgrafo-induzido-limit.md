# ADR 0003 — Subgrafo induzido com --limit=N

Status: aceito

## Contexto
O protocolo experimental exige testes de estresse com subconjuntos do grafo
(N = 100, 500, 1000, ...), reprodutíveis para o artigo.

## Decisão
- `--limit=N` cria o subgrafo induzido pelos N primeiros vértices distintos,
  na ordem de primeira aparição no CSV (origem antes de destino em cada linha).
- O CSV é lido por inteiro: uma transação entre dois vértices iniciais pode
  aparecer em qualquer ponto do arquivo e deve ser mantida.
- N conta vértices. Se N > |V| total, usa-se o grafo completo, com aviso.
- Não há aleatoriedade: o mesmo CSV e o mesmo N geram sempre o mesmo
  subgrafo, o que atende ao requisito de semente fixa sem PRNG.
- O sumário imprime |V| e |E| do subgrafo e o total do grafo completo.

## Consequências
- Os subgrafos são esparsos: com N=500, |E|=410 (grafo completo: |V|=32386,
  |E|=20000). Isso deve ser discutido no artigo.
- O tempo de carga é dominado pela leitura do CSV e não cresce com N; a análise
  assintótica deve usar o tempo dos algoritmos.
- `bytes_grafo` mede só a lista/matriz, para comparar memória entre
  representações sem o offset constante dos rótulos.