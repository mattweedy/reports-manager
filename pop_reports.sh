#!/bin/bash

echo "Directories in ./reports/:"
ls ./reports/

for dir in ./reports/*/
do
    dir=${dir%*/}
    echo "dir: ${dir}"
    file="${dir##*/}.xml"
    echo "file: ${file}"
    touch "${dir}/${file}"
done