#!/usr/bin/env bash
BIN=./build/grafos
for repr in lista matriz; do
    for n in 100 500 1000 5000 10000 20000; do
        echo "== repr=$repr N=$n"
        "$BIN" --repr=$repr --limit=$n || { echo "falhou em N=$n"; exit 1; }
    done
    echo "== repr=$repr (grafo completo)"
    "$BIN" --repr=$repr
done
# Quando os algoritmos (DFS, ciclos...) existirem, invoque-os aqui dentro do laço.