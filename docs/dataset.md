# Dataset

## Fonte

- **Nome:** IBM Transactions for Anti-Money Laundering (AML)
- **Link:** <https://www.kaggle.com/datasets/ealtman2019/ibm-transactions-for-anti-money-laundering-aml>
- **Licença:** Community Data License Agreement – Sharing – Version 1.0 (CDLA-Sharing-1.0)

## Estado atual

O projeto usa um subconjunto de **20.000 transações** como dataset
**definitivo** — não como uma amostra temporária a ser substituída depois.
O subconjunto foi obtido por amostragem aleatória (semente fixa) do arquivo
original `HI-Small_Trans.csv` do dataset IBM AML, preservando a distribuição
estatística das transações de lavagem.

O dataset completo (arquivo `HI-Small`, com centenas de milhares de
transações) **não é usado nem versionado** neste repositório, pois excede os
100MB de limite do GitHub para arquivos versionados normalmente. Os arquivos
definitivos do projeto são:

- `data/dataset.csv` — 20.000 transações
- `data/padroes_lavagem.txt` — padrões de lavagem rotulados (gabarito)

## Schema

O arquivo `data/dataset.csv` possui as seguintes colunas:

| Coluna | Descrição |
|---|---|
| `Timestamp` | data/hora da transação |
| `From Bank` | banco de origem |
| `Account` | conta de origem |
| `To Bank` | banco de destino |
| `Account.1` | conta de destino |
| `Amount Received` | valor recebido |
| `Receiving Currency` | moeda recebida |
| `Amount Paid` | valor pago |
| `Payment Currency` | moeda de pagamento |
| `Payment Format` | formato do pagamento |
| `Is Laundering` | rótulo experimental (1 = lavagem, 0 = normal) |

O CSV original possui duas colunas chamadas `Account`. Ao ser lido com
pandas, a segunda é renomeada automaticamente para `Account.1`. Portanto,
**`Account` é a conta de origem** e **`Account.1` é a conta de destino**.

## Caracterização estatística

- **Vértices (contas únicas):** 32.386
- **Arestas (transações):** 20.000
- **Densidade** (grafo dirigido, `arestas / (vértices * (vértices - 1))`):
  ≈ 0,00001907
- **Grau médio de saída:** 1,20
- **Grau máximo de saída:** 706
- **Distribuição de `Is Laundering`:** 19.979 transações marcadas como 0
  (normal) e 21 marcadas como 1 (lavagem)

## Metodologia de amostragem

O subconjunto foi obtido por amostragem aleatória simples (semente fixa 42,
`pandas.DataFrame.sample(n=20000, random_state=42)`) sobre as 5.078.345
transações do dataset original `HI-Small_Trans.csv`, seguida de reordenação
cronológica (`sort_values('Timestamp')`) para preservar a sequência temporal
das transações na amostra.

A proporção de casos de lavagem na amostra (21/20.000 ≈ 0,105%) é
estatisticamente consistente com a do dataset completo (5.177/5.078.345 ≈
0,102%), preservando o desbalanceamento natural do domínio de detecção de
fraude — diferente de uma versão anterior desta amostra, que usava as
primeiras 20.000 linhas ordenadas por tempo e continha quase nenhum caso
rotulado de lavagem.

## Observação sobre `padroes_lavagem.txt`

Este arquivo é mantido como gabarito de validação para a detecção de ciclos
(Fase I). Como o dataset foi reduzido para 20.000 transações, nem todos os
padrões listados nele necessariamente aparecem completos no subconjunto
atual. Isso deve ser validado quando o algoritmo de detecção de ciclos for
implementado.
