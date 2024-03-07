#!/bin/bash

echo "Creating files in ../upload/:"
ls ../upload/

for dir in ../upload/*/
do
    dir=${dir%*/}
    echo "dir: ${dir}"
    file="${dir##*/}.xml"
    echo "file: ${file}"
    touch "${dir}/${file}"
done

echo "Removing files in ../backup/:"
for dir in ../backup/*/
do
    echo "dir: ${dir}"
    rm -rf "${dir}"*
done

echo "Removing files in ../reporting/:"
for dir in ../reporting/*/
do
    echo "dir: ${dir}"
    rm -rf "${dir}"*
done