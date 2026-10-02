# Grafo-brinquedo para validar DFS e detecção de ciclos

Arquivo: `tests/fixtures/grafo_brinquedo.csv` (mesmo formato do dataset real,
reaproveita `carregador_carregar`).

Contas (rótulo `banco:conta`, banco "001" fixo): **AAA, BBB, CCC, DDD, EEE,
FFF, GGG, HHH, III** — 9 vértices distintos.

> O enunciado pedia 8 vértices, mas dois ciclos disjuntos (3 + 4 vértices)
> mais pelo menos 2 vértices acíclicos exigem no mínimo 9 vértices
> distintos (3 + 4 + 2 = 9). Optou-se por manter os quatro critérios de
> cobertura (ciclo de 3, ciclo de 4, disjunção, acíclicos) em vez de forçar
> exatamente 8 vértices.

## Gabarito esperado

- **Ciclo de 3 vértices**: `001:AAA -> 001:BBB -> 001:CCC -> 001:AAA`
- **Ciclo de 4 vértices**: `001:DDD -> 001:EEE -> 001:FFF -> 001:GGG -> 001:DDD`
- Os dois ciclos são disjuntos (nenhum vértice em comum).
- **Vértices acíclicos** (não devem aparecer em nenhum ciclo reportado):
  `001:HHH` e `001:III` (ligados por uma única aresta `HHH -> III`, sem
  volta).

`Is Laundering` é sempre `0` neste arquivo — ele serve só para validar a
topologia (DFS e ciclos), não a validação semântica contra lavagem, que é
feita separadamente sobre `data/dataset.csv` + `data/padroes_lavagem.txt`.
