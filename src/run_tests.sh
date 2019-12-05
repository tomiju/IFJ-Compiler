#!/bin/bash
testNumber=1
succes=0
failed=0

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

printf "************* TESTING ************\n\n"

for file in ./tests/*; do
testing_file=${file##*/};

printf "TEST $testNumber: $testing_file ";

COMPILER_RETURN_CODE="$(awk -F: -v src=$testing_file '{ if ($1 == src) print $2}' outputs/outputs)";

# spustí preklad v našom programe
./ifj2019 < "${file}" >output 2> /dev/null;
# náš návratový kód
return_code="$?";

# spustí preklad v pythone
PYTHON_OUTPUT="$(python3 < "${file}" 2> /dev/null)";
# návratový kód pythonu

if [ $return_code == $COMPILER_RETURN_CODE ]; then
  	INTERPRED_OUTPUT="$(./ic19int output)";
  	interpret_ret_code="$?";

  	INTERPRETER_RETURN_CODE="$(awk -F: -v src=$testing_file '{ if ($1 == src) print $3}' outputs/outputs)";

  	if [ "$interpret_ret_code" == "$INTERPRETER_RETURN_CODE" ]; then
  		if [ "$INTERPRED_OUTPUT" == "$PYTHON_OUTPUT" ]; then
  			printf "===> ${GREEN}SUCCESS\n${NC}";
			let "succes += 1";
  		else
  			printf "===> ${RED}FAILED\n${NC}";
			printf "Expected output: $PYTHON_OUTPUT get: $INTERPRED_OUTPUT\n";
			let "failed += 1";
  		fi
  	else
  		echo "nie"
  		#printf "Expected: $INTERPRETER_RETURN_CODE Interpred returned: $interpret_return_code";
  	fi
else
	printf "===> ${RED}FAILED\n${NC}";
	printf "Expected return code: $COMPILER_RETURN_CODE get: $return_code\n";
	let "failed += 1";
fi

let "testNumber += 1";
done;

let "testNumber -= 1";

printf "\n********** TESTING ENDED **********\n\n";
printf "TOTAL: $testNumber -> ${GREEN}SUCCESSED${NC}: $succes ${RED}FAILED${NC}: $failed\n";