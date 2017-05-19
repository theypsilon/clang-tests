#!/usr/bin/env bash

set -euo pipefail

echo "CI suite starting..."
echo

if [[ "${1:-}" == "--no-clean" ]] || [[ "${1:-}" == "-nc" ]] ; then
    echo "WARNING! Skipping clean scripts because option '${1}' was given."
    echo
    must_clean=false
fi

for directory in * ; do
    if [ -d ${directory} ] ; then
        pushd ${directory} > /dev/null
        echo "CI dir: ${directory}"
        if [ -f clean.sh ] && ${must_clean:-true} ; then
            echo "CI script: clean.sh"
            ./clean.sh
        fi
        if [ -f compile.sh ] ; then
            echo "CI script: compile.sh"
            ./compile.sh
        fi
        if [ -f run.sh ] ; then
            echo "CI script: run.sh"
            ./run.sh
        fi
        popd > /dev/null
        echo
    fi
done

echo "CI DONE"