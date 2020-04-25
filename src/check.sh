#!/bin/bash

tsv_orig=`./tsvsim 10000 40 1 1`
tsv_ref=`echo $tsv_orig | ./tsvtree -o comp | ./tsvtree --tree -o tsv`
tsv_from_tree=`echo $tsv_ref | ./tsvtree --indent-with-tab -o tree | ./tsvtree --tree -o tsv`
tsv_from_comp=`echo $tsv_ref | ./tsvtree -o comp | ./tsvtree --tree -o tsv`

#echo $tsv_orig
#echo $tsv_ref
#echo $tsv_from_tree
#echo $tsv_from_comp

if [[ $tsv_ref != $tsv_from_tree ]]
then
   echo "Fail"
   exit 1
fi

if [[ $tsv_ref != $tsv_from_comp ]]
then
   echo "Fail"
   exit 1
fi

echo "OK"
