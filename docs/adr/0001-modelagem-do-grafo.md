# ADR 0001: Modelagem do grafo de transações

- **Status:** Aceito
- **Data:** 2026-09-18
- **Escopo:** Fase I — topologia e conectividade

## Contexto

O projeto modela as transações do arquivo `HI-Small_Trans.csv` como um grafo
para investigar estruturas relacionadas a possíveis fraudes financeiras. O
arquivo possui, entre outras, as colunas `From Bank`, `Account`, `To Bank`,
`Account`, `Amount Received`, `Receiving Currency`, `Amount Paid`, `Payment
Currency`, `Payment Format` e `Is Laundering`. As duas colunas `Account` são
identificadas pela posição: a primeira pertence à origem e a segunda ao
destino.

Na Fase I, o objetivo é analisar topologia e conectividade. Portanto, a
construção inicial não deve depender de valores financeiros, moedas, rótulos
de fraude ou do arquivo de padrões. A modelagem também precisa representar
corretamente transações repetidas e evitar colisões entre contas com o mesmo
identificador em bancos diferentes.

## Decisão

### Tipo do grafo

O grafo será **dirigido**. Uma transação de uma conta para outra possui uma
origem e um destino, e a relação inversa não deve ser inferida
automaticamente.

### Vértices e identificação das contas

Cada conta bancária será representada por um vértice. O identificador lógico
do vértice será composto pelo banco e pela conta:

```text
banco:conta
```

Assim, `010:8000EBD30` e `03208:8000EBD30` são vértices distintos. A
identificação composta é obrigatória para evitar colisões quando o mesmo
número de conta aparecer em bancos diferentes. Os valores devem ser tratados
como texto, preservando zeros à esquerda dos bancos.

### Arestas

Cada linha de transação produzirá uma aresta direcionada do vértice de origem
para o vértice de destino:

```yaml
banco_origem:conta_origem -> banco_destino:conta_destino
```

Por exemplo:

```text
010:8000EBD30 -> 010:8000EBD30
```

### Arestas paralelas

O modelo de dados deverá preservar múltiplas transações entre o mesmo par de
contas. Portanto, o grafo lógico é um multigrafo dirigido: transações
distintas não devem ser silenciosamente descartadas durante o parsing ou o
armazenamento dos metadados.

Quando o objetivo de um algoritmo for somente conectividade, uma lista de
adjacência ou uma matriz poderá consultar a existência da ligação sem
duplicar o resultado da busca. A matriz poderá representar a conectividade
com uma célula não nula ou, se necessário para uma análise posterior, a
quantidade de transações. Essa simplificação não altera a decisão de
preservar as transações originais.

### Auto-laços

Uma transação cujo vértice de origem seja igual ao vértice de destino será
preservada como auto-laço. Ela não será removida durante a construção do
grafo, pois faz parte da topologia observada e pode ser relevante para a
detecção de ciclos. Algoritmos específicos poderão documentar separadamente
se ignoram auto-laços em uma métrica ou análise, mas essa não é a regra de
armazenamento.

### Pesos e metadados

Na Fase I, valores financeiros, moedas, formato de pagamento e demais
atributos não serão usados como pesos para decidir conectividade. A
conectividade depende somente dos vértices e da direção das arestas.

Os metadados da transação deverão ser preservados junto à aresta ou em uma
estrutura associada, quando forem necessários para rastreabilidade e análise
posterior. Esses metadados poderão ser usados na Fase II para pesos,
Bellman-Ford, arbitragem e outras análises, sem alterar a decisão topológica
da Fase I.

### Coluna `Is Laundering`

A coluna `Is Laundering` não será usada para construir a topologia do grafo.
Usá-la durante a construção introduziria o rótulo experimental na estrutura
que deveria ser derivada apenas das transações.

Posteriormente, ela poderá ser usada para validação dos resultados,
comparação com o *ground truth* e análise experimental. Essa separação evita
misturar o rótulo com a descoberta da estrutura do grafo.

### Arquivo `HI-Small_Patterns.txt`

O arquivo `HI-Small_Patterns.txt` será mantido como referência auxiliar para
validar ciclos ou padrões encontrados. Ele não fará parte da construção
inicial do grafo e não definirá vértices nem arestas.

### Representações

O projeto deverá suportar duas representações autorais:

- **lista de adjacência:** representação principal esperada para o dataset
  grande, por consumir memória proporcional aos vértices e às arestas
  armazenadas;
- **matriz de adjacência:** representação destinada à comparação de memória
  e a testes com subconjuntos menores, devido ao custo quadrático em relação
  ao número de vértices.

Ambas devem respeitar a direção das arestas. A lista deve permitir a
preservação de arestas paralelas e metadados; a matriz poderá representar a
conectividade agregada quando a análise não exigir distinguir transações
individuais.

## Consequências

- A direção das transações será preservada e relações inversas não serão
  criadas automaticamente.
- A chave composta banco-conta elimina colisões entre bancos distintos e
  exige preservar identificadores textuais, inclusive zeros à esquerda.
- O armazenamento deverá lidar com arestas paralelas e auto-laços sem
  descartá-los silenciosamente.
- A lista de adjacência será adequada ao dataset grande, enquanto a matriz
  será útil principalmente para experimentos controlados e comparação de
  desempenho.
- A construção do grafo permanecerá independente de `Is Laundering`, de
  `HI-Small_Patterns.txt` e dos valores financeiros, reduzindo risco de
  vazamento do rótulo para a topologia.
- A preservação de metadados aumenta o espaço necessário, mas mantém a
  possibilidade de auditoria e de uso nas análises da Fase II.

## Relação com as fases do projeto

### Fase I

Este ADR orienta o parser futuro, a criação das estruturas de lista e matriz,
as buscas BFS e DFS, a detecção de ciclos, a análise de componentes e os
benchmarks de memória e tempo. Nenhuma dessas funcionalidades é implementada
por este documento.

### Fase II

Os metadados preservados poderão fundamentar pesos, Bellman-Ford, análise de
arbitragem e Caminho Hamiltoniano. Essas funcionalidades não fazem parte
desta decisão e deverão respeitar a separação entre topologia e atributos
financeiros.

## Limites desta decisão

Este ADR registra a modelagem e as decisões arquiteturais do grafo. Ele não
implementa parser, lista de adjacência, matriz de adjacência, BFS, DFS,
detecção de ciclos ou qualquer outro algoritmo.
