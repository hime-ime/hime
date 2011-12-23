#!/bin/bash

opt="-nbad -bap -nbc -bbo -hnl -br -brs -c33 -cd33 -ncdb -ce -ci4
-cli0 -d0 -di1 -nfc1 -i8 -ip0 -l80 -lp -npcs -nprs -npsl -sai
-saf -saw -ncs -nsc -sob -nfca -cp33 -ss -ts8 -il1"

for file in $(find . -name *.h -o -name *.c); do
  indent "$file" $opt -o "$file".new
  mv "$file".new "$file"
done
