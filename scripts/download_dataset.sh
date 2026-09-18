#!/usr/bin/env bash

set -euo pipefail

DATASET="ealtman2019/ibm-transactions-for-anti-money-laundering-aml"
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
DATA_DIR="${PROJECT_ROOT}/data"

if ! command -v kaggle >/dev/null 2>&1; then
    printf '%s\n' 'Erro: Kaggle CLI não encontrado.' >&2
    printf '%s\n' 'Instale com: python3 -m pip install --upgrade kaggle' >&2
    exit 1
fi

mkdir -p "${DATA_DIR}"

kaggle datasets download "${DATASET}" \
    --file HI-Small_Trans.csv \
    --path "${DATA_DIR}" \
    --unzip

kaggle datasets download "${DATASET}" \
    --file HI-Small_Patterns.txt \
    --path "${DATA_DIR}" \
    --unzip

printf 'Dataset preparado em: %s\n' "${DATA_DIR}"
