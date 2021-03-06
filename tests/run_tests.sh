#!/bin/bash
testNumber=1
succes=0
failed=0

#fixed lf

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

if [ $return_code = $COMPILER_RETURN_CODE ]; then
	if [ $return_code != 0 ]; then
    	printf "===> ${GREEN}SUCCESS\n${NC}";
    	let "succes += 1";
	else
		INTERPRED_OUTPUT="$(./ic19int output)";
  		interpret_ret_code="$?";

		INTERPRETER_RETURN_CODE="$(awk -F: -v src=$testing_file '{ if ($1 == src) print $3}' outputs/outputs)";

		if [ "$interpret_ret_code" = "$INTERPRETER_RETURN_CODE" ]; then
			i_out="$(echo $INTERPRED_OUTPUT | tr -d '\n')";
			p_out="$(echo $PYTHON_OUTPUT | tr -d '\n')";

			if [ "$i_out" = "$p_out" ]; then
				printf "===> ${GREEN}SUCCESS\n${NC}";
  				let "succes += 1";
			else
				printf "===> ${RED}FAILED\n${NC}";
  				printf "Expected output: $PYTHON_OUTPUT get: $INTERPRED_OUTPUT\n";
  				let "failed += 1";
			fi
		else
			printf "===> ${RED}FAILED\n${NC}";
  			let "failed += 1";
			printf "Expected: $INTERPRETER_RETURN_CODE Interpret returned: $interpret_ret_code\n";
		fi
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
