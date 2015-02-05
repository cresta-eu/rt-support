#
# rta-c - message passing communication routines library.
#  Copyright (C) 2014  Sebastian Tunstig
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

PROBLEMSIZES=("64" "100" "144" "225" "361")
MAPPINGTYPES=("" "all-to-all_mappings" "random_mappings" "manhattan_mappings")
#PROBLEMSIZES=("361")  #For testing
#MAPPINGTYPES=("")  #For testing
IFS=$'\n'

for problemsize in "${PROBLEMSIZES[@]}"
do
	for type in "${MAPPINGTYPES[@]}"
	do
		for i in $(seq -f "%03g" 1 19)
		do
			ARRAY_0=()
			ARRAY_1=()
			ARRAY_2=()
			ARRAY_3=()
			ARRAY_4=()
			PROBLEMSIZESTRING=$(printf "%03d" ${problemsize}) #e.g. "64" -> "064"
			for line in $(cat res_${PROBLEMSIZESTRING}_0/${PROBLEMSIZESTRING}_${i}_${type}.txt |grep "sec" |grep -v "Avg" |awk '{print $4}')
			do
				ARRAY_0+=(${line})
			done;
			for line in $(cat res_${PROBLEMSIZESTRING}_1/${PROBLEMSIZESTRING}_${i}_${type}.txt |grep "sec" |grep -v "Avg" |awk '{print $4}')
			do
				ARRAY_1+=(${line})
			done;
			for line in $(cat res_${PROBLEMSIZESTRING}_2/${PROBLEMSIZESTRING}_${i}_${type}.txt |grep "sec" |grep -v "Avg" |awk '{print $4}')
			do
				ARRAY_2+=(${line})
			done;
			for line in $(cat res_${PROBLEMSIZESTRING}_3/${PROBLEMSIZESTRING}_${i}_${type}.txt |grep "sec" |grep -v "Avg" |awk '{print $4}')
			do
				ARRAY_3+=(${line})
			done;
			for line in $(cat res_${PROBLEMSIZESTRING}_4/${PROBLEMSIZESTRING}_${i}_${type}.txt |grep "sec" |grep -v "Avg" |awk '{print $4}')
			do
				ARRAY_4+=(${line})
			done;

			ARRAY_0=($(printf '%s\n' "${ARRAY_0[@]}"|sort))
			ARRAY_1=($(printf '%s\n' "${ARRAY_1[@]}"|sort))
			ARRAY_2=($(printf '%s\n' "${ARRAY_2[@]}"|sort))
			ARRAY_3=($(printf '%s\n' "${ARRAY_3[@]}"|sort))
			ARRAY_4=($(printf '%s\n' "${ARRAY_4[@]}"|sort))
			TEMP=()
			TEMP+=(${ARRAY_0[${problemsize}-1]})
			TEMP+=(${ARRAY_1[${problemsize}-1]})
			TEMP+=(${ARRAY_2[${problemsize}-1]})
			TEMP+=(${ARRAY_3[${problemsize}-1]})
			TEMP+=(${ARRAY_4[${problemsize}-1]})
			TEMP=($(printf '%s\n' "${TEMP[@]}"|sort))
			RESULT=(${TEMP[0]}) #LOWEST (of the test rounds) MAXIMUM TIME FOR ONE PID IN THIS ITERATION: ${TEMP[0]}"
			echo "#${type}# ${problemsize} ${i}: ${RESULT[0]}"
	  done;
	done;
done;
