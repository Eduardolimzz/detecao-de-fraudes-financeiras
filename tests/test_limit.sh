#!/usr/bin/env bash
BIN=./build/grafos
CSV=tests/fixtures/limit.csv
LOG=results/log_execucao.csv
falhas=0

# O programa grava no log a cada execução: preserva o log real.
backup=$(mktemp)
if [ -f "$LOG" ]; then existia=1; cp "$LOG" "$backup"; else existia=0; fi
trap 'if [ $existia -eq 1 ]; then cp "$backup" "$LOG"; else rm -f "$LOG"; fi; rm -f "$backup"' EXIT

extrair() { printf '%s\n' "$1" | grep -F "$2" | head -1 | awk '{print $NF}'; }

checar() { # repr limite V_esperado E_esperado
    local repr=$1 limite=$2 v_esp=$3 e_esp=$4 saida v e
    local args=(--dataset="$CSV" --repr="$repr")
    [ -n "$limite" ] && args+=(--limit="$limite")
    saida=$("$BIN" "${args[@]}") || {
        echo "FALHA: execução (repr=$repr limit=${limite:-sem})"; falhas=$((falhas+1)); return; }
    v=$(extrair "$saida" '|V| (contas distintas):')
    e=$(extrair "$saida" '|E| (transações):')
    if [ "$v" = "$v_esp" ] && [ "$e" = "$e_esp" ]; then
        echo "ok:    repr=$repr limit=${limite:-sem} -> |V|=$v |E|=$e"
    else
        echo "FALHA: repr=$repr limit=${limite:-sem} -> |V|=$v |E|=$e (esperado $v_esp/$e_esp)"
        falhas=$((falhas+1))
    fi
}

for repr in lista matriz; do
    checar $repr ""  6 6
    checar $repr 1   1 0
    checar $repr 2   2 2
    checar $repr 3   3 3
    checar $repr 4   4 4
    checar $repr 100 6 6
done

for ruim in 0 -3 abc 5x; do
    if "$BIN" --dataset="$CSV" --limit="$ruim" >/dev/null 2>&1; then
        echo "FALHA: --limit=$ruim deveria ser rejeitado"; falhas=$((falhas+1))
    else
        echo "ok:    --limit=$ruim rejeitado"
    fi
done

# Reprodutibilidade no dataset real (compara |V| e |E|, não tempos).
if [ -f data/dataset.csv ]; then
    a=$("$BIN" --limit=500 | grep -F -e '|V| (' -e '|E| (')
    b=$("$BIN" --limit=500 | grep -F -e '|V| (' -e '|E| (')
    if [ "$a" = "$b" ]; then echo "ok:    reprodutível (--limit=500)"
    else echo "FALHA: --limit=500 não reprodutível"; falhas=$((falhas+1)); fi
fi

[ $falhas -eq 0 ] && echo "Todos os testes passaram." || echo "$falhas falha(s)."
exit $falhas