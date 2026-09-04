# Project e Backlog da Fase I

Este documento versiona a configuração usada no GitHub Project da Fase I para manter rastreabilidade do planejamento.

## Milestone

- **Nome**: `Fase I - Topologia e Conectividade`
- **Data de entrega**: `2026-10-31`

## Campos customizados do Project

- **Sprint** (single select): `Sprint 1`, `Sprint 2`, `Sprint 3`
- **Story Points** (number)
- **Área** (single select): `Dados`, `Arquitetura`, `Estruturas`, `Algoritmos`, `Observabilidade`
- **Requisito** (single select): `RF01`, `RF02`, `RF03`, `NFR`

## Backlog priorizado (Fase I)

| Prioridade | Issue | Sprint | Story Points | Área | Requisito |
|---|---|---|---:|---|---|
| P0 | #3 - docs(dataset): Aquisição e caracterização do dataset IBM AML | Sprint 1 | 3 | Dados | RF01 |
| P0 | #4 - docs(adr): Definir modelagem de vértices e arestas do grafo | Sprint 1 | 2 | Arquitetura | RF01 |
| P0 | #5 - feat(parser): Implementar leitura de CSV genérica e resiliente | Sprint 1 | 5 | Dados | RF01 |
| P0 | #6 - feat(core): Implementar tabela hash de mapeamento rótulo-índice | Sprint 1 | 8 | Estruturas | RF01 |
| P1 | #7 - feat(graph): Implementar Lista de Adjacência | Sprint 2 | 8 | Estruturas | RF01 |
| P1 | #8 - feat(graph): Implementar Matriz de Adjacência | Sprint 2 | 5 | Estruturas | RF02 |
| P1 | #9 - feat(graph): Criar camada de abstração para alternância de representação | Sprint 2 | 5 | Arquitetura | RF02 |
| P1 | #10 - feat(bench): Implementar módulo de instrumentação de tempo e memória | Sprint 2 | 5 | Observabilidade | NFR |
| P2 | #11 - feat(loader): Implementar pipeline de carga end-to-end do grafo | Sprint 3 | 8 | Dados | RF01 |
| P2 | #12 - feat(graph): Implementar subamostragem determinística do grafo | Sprint 3 | 3 | Algoritmos | RF03 |

## Vinculação ao repositório

- Repositório vinculado ao Project: `Eduardolimzz/detecao-de-fraudes-financeiras`
- Todas as issues da Fase I (#3 a #12) devem permanecer adicionadas ao backlog do Project e associadas à milestone `Fase I - Topologia e Conectividade`.
