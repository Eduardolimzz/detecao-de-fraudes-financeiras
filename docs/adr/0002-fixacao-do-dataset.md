  # ADR 0002: Fixação do subconjunto de dataset como versão definitiva

  - **Status:** Aceito
  - **Data:** 2026-09-27
  - **Escopo:** Fase I — reprodutibilidade e gestão do dataset

  ## Contexto

  O dataset completo IBM AML (arquivo `HI-Small_Trans.csv`, com centenas de
  milhares de transações e ~475MB) excede o limite de 100MB para arquivos
  versionados no GitHub. Mantê-lo como fonte primária, com amostragem dinâmica
  via script de download e credenciais do Kaggle, dificulta a reprodutibilidade
  do projeto e a revisão pelo professor, que precisaria configurar uma conta e
  um token de API apenas para obter os dados.

  ## Decisão

  Adotar um subconjunto fixo de 20.000 transações (16.718 contas/vértices)
  como dataset definitivo do projeto, versionado diretamente no repositório em
  `data/dataset.csv` e `data/padroes_lavagem.txt`. O script de download
  (`scripts/download_dataset.sh`) e a dependência do Kaggle CLI foram
  removidos.

  ## Consequências

  - **Positivo:** qualquer pessoa reproduz o projeto com um simples
    `git clone`, sem necessidade de autenticação externa ou configuração
    adicional.
  - **Negativo:** a escala do subconjunto é menor que a do dataset original,
    o que limita os testes de estresse da Fase I a essa faixa (~20k arestas),
    a menos que o dataset completo seja obtido separadamente no futuro para
    testes de escala maior.
