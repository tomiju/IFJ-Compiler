#!/bin/bash
testNumber=1

rm ./tmp/*
rm ./outputs/*

for file in ./tests/*; do
echo "${file##*/}";
./ifj2019 < ./tests/"${file##*/}" > ./tmp/"output"$testNumber".txt";
if [ "$?" = 0 ]; then
  echo "soubor ""${file##*/}"" je ok: číslo: ""$testNumber";
  ./ic19int ./tmp/"output"$testNumber".txt" > ./outputs/"output"$testNumber".txt";
  if [ "$?" != 0 ]; then
    echo "not ok interpret ""$testNumber";
    echo "exit code interpret: "$? >>./outputs/"output"$testNumber".txt";
  fi;
else
  echo "not ok translator ""$testNumber";
  echo "exit code translator: "$? >>./outputs/"output"$testNumber".txt";
fi
#echo "exit code: "$?
let "testNumber += 1";
done;

#./ijc2019 ./tests/badtype6.py >test111
#./ic19int ./tmp/"output"$testNumber".txt" > ./outputs/"output"$testNumber".txt";
#testNumber=1
#for file in ./tmp/*; do
#./ifj2019 ./tmp/"${file##*/}" >> ./tmp/"output"$testNumber".txt";
#echo "exit code: "$? >>./outputs/"output"$testNumber".txt"
#echo "exit code: "$?
#let "testNumber += 1";
#done;
