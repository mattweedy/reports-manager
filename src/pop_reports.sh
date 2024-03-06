#!/bin/bash

echo "Creating files in ../reports/:"
ls ../reports/

for dir in ../reports/*/
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

echo "Removing files in ../dashboard/:"
for dir in ../dashboard/*/
do
    echo "dir: ${dir}"
    rm -rf "${dir}"*
done