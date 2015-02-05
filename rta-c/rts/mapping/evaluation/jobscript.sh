#!/bin/bash
#
#PBS -N processmapping
#PBS -A m.2013-1-213
#PBS -l walltime=02:30:00,mppwidth=384,mppnppn=24
#PBS -j oe

##PBS -l hostlist=emil.1504+emil.638+emil.128+emil.1406+emil.30+emil.798+emil.1280+emil.352+emil.736+emil.254+emil.1182+emil.512+emil.1120+emil.896+emil.414+emil.1022
##PBS -l hostlist=emil.30+emil.128+emil.254+emil.352+emil.414+emil.512+emil.638+emil.736+emil.798+emil.896+emil.1022+emil.1120+emil.1182+emil.1280+emil.1406+emil.1504

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

# --------------------------
# How to submit a job:
#
# NPROCS=144
# PPN=12
# qsub -l mppwidth=${NPROCS},mppnppn=${PPN} -v "NPROCS=${NPROCS},PPN=${PPN}" ./jobscript-michs.sh
#
# --------------------------


#NPROCS=120
#PPN=24
TIMESTEPS=1000

cd $PBS_O_WORKDIR

#main loop
#arguments are: Problem size in three numbers, name of target file, directory to place mappings ( '/' omitted)
function evaluate_loop(){
	local PROBLEMSIZE=$1
	local TARGETFILE=$2
	local MAPPINGDIR=$3
	unset PROCMAP
	echo "Running for mapping: \"${MAPPINGDIR}\""

	for iteration in {0..4}
	do
		I_STR=001
		if [ -n "${MAPPINGDIR}" ] ; then
			mkdir -p ${MAPPINGDIR}
			echo "aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./proc_map_eval source_graphs/${PROBLEMSIZE}_000.grf ${TARGETFILE} $MAPPINGDIR/${PROBLEMSIZE}_${I_STR}.map ${PPN}"
			aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./proc_map_eval source_graphs/${PROBLEMSIZE}_000.grf ${TARGETFILE} $MAPPINGDIR/${PROBLEMSIZE}_${I_STR}.map ${PPN}
			local PROCMAP="-pmap $MAPPINGDIR/${PROBLEMSIZE}_${I_STR} ${PPN}"
		fi
		echo "aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./MDbase -restart ${PROBLEMSIZE}/coll1-${I_STR} -bench ${TIMESTEPS} ${PROCMAP} >res_${PROBLEMSIZE}_${iteration}/${PROBLEMSIZE}_${I_STR}_${MAPPINGDIR}.txt 2>&1"
		aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./MDbase -restart ${PROBLEMSIZE}/coll1-${I_STR} -bench ${TIMESTEPS} ${PROCMAP} >res_${PROBLEMSIZE}_${iteration}/${PROBLEMSIZE}_${I_STR}_${MAPPINGDIR}.txt 2>&1
		for i in {2..20}
		do
			PRIOR=`expr $i - 1`
			I_STR=$(printf "%03d" $i)
			PRIOR_STR=$(printf "%03d" $PRIOR)
			if [ -n "${MAPPINGDIR}" ] ; then
				echo "aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./proc_map_refine_eval source_graphs/${PROBLEMSIZE}_${PRIOR_STR}.grf ${TARGETFILE} $MAPPINGDIR/${PROBLEMSIZE}_${PRIOR_STR}.map $MAPPINGDIR/${PROBLEMSIZE}_${I_STR}.map ${PPN}"
				aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./proc_map_refine_eval source_graphs/${PROBLEMSIZE}_${PRIOR_STR}.grf ${TARGETFILE} $MAPPINGDIR/${PROBLEMSIZE}_${PRIOR_STR}.map $MAPPINGDIR/${PROBLEMSIZE}_${I_STR}.map ${PPN}
				local PROCMAP="-pmap $MAPPINGDIR/${PROBLEMSIZE}_${I_STR} ${PPN}"
			fi
			echo "aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./MDbase -restart ${PROBLEMSIZE}/coll1-${I_STR} -bench ${TIMESTEPS} ${PROCMAP} >res_${PROBLEMSIZE}_${iteration}/${PROBLEMSIZE}_${I_STR}_${MAPPINGDIR}.txt 2>&1"
			aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./MDbase -restart ${PROBLEMSIZE}/coll1-${I_STR} -bench ${TIMESTEPS} ${PROCMAP} >res_${PROBLEMSIZE}_${iteration}/${PROBLEMSIZE}_${I_STR}_${MAPPINGDIR}.txt 2>&1
		done;
	done;
}

function run_random_mapping(){
	local PROBLEMSIZE=$1
	local MAPPINGDIR=$2
	unset PROCMAP
	echo "Running for mapping: \"${MAPPINGDIR}\""
	if [ -n "${MAPPINGDIR}" ] ; then
		mkdir -p ${MAPPINGDIR}
		echo "aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./rand_map_eval $MAPPINGDIR/${PROBLEMSIZE}.map"
	aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./rand_map_eval $MAPPINGDIR/${PROBLEMSIZE}.map
	local PROCMAP="-pmap $MAPPINGDIR/${PROBLEMSIZE} ${PPN}"
	fi

	for iteration in {0..4}
	do
		I_STR=001
		echo "aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./MDbase -restart ${PROBLEMSIZE}/coll1-${I_STR} -bench ${TIMESTEPS} ${PROCMAP} >res_${PROBLEMSIZE}_${iteration}/${PROBLEMSIZE}_${I_STR}_${MAPPINGDIR}.txt 2>&1"
		aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./MDbase -restart ${PROBLEMSIZE}/coll1-${I_STR} -bench ${TIMESTEPS} ${PROCMAP} >res_${PROBLEMSIZE}_${iteration}/${PROBLEMSIZE}_${I_STR}_${MAPPINGDIR}.txt 2>&1
		for i in {2..20}
		do
			PRIOR=`expr $i - 1`
			I_STR=$(printf "%03d" $i)
			PRIOR_STR=$(printf "%03d" $PRIOR)
			echo "aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./MDbase -restart ${PROBLEMSIZE}/coll1-${I_STR} -bench ${TIMESTEPS} ${PROCMAP} >res_${PROBLEMSIZE}_${iteration}/${PROBLEMSIZE}_${I_STR}_${MAPPINGDIR}.txt 2>&1"
			aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./MDbase -restart ${PROBLEMSIZE}/coll1-${I_STR} -bench ${TIMESTEPS} ${PROCMAP} >res_${PROBLEMSIZE}_${iteration}/${PROBLEMSIZE}_${I_STR}_${MAPPINGDIR}.txt 2>&1
		done;
	done;

}

function run_suite(){
	#create target architecture files
	echo "aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./make_manhattan_target manhattan${NPROCS}.arch ${PPN}"
	aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./make_manhattan_target manhattan${NPROCS}.arch ${PPN}
	echo "aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./make_all-to-all_target all-to-all${NPROCS}.arch ${PPN}"
	aprun ${NODE_SPEC} -n ${NPROCS} -N ${PPN} ./make_all-to-all_target all-to-all${NPROCS}.arch ${PPN}

	#perform tests for both methods
	run_random_mapping ${SIZE_STR} random_mappings
	evaluate_loop ${SIZE_STR}
	evaluate_loop ${SIZE_STR} manhattan${NPROCS}.arch manhattan_mappings
	evaluate_loop ${SIZE_STR} all-to-all${NPROCS}.arch all-to-all_mappings
}

PPN=24
NPROCS=64
SIZE_STR=$(printf "%03d" $NPROCS)
run_suite

NPROCS=100
SIZE_STR=$(printf "%03d" $NPROCS)
run_suite

NPROCS=144
SIZE_STR=$(printf "%03d" $NPROCS)
run_suite

NPROCS=225
SIZE_STR=$(printf "%03d" $NPROCS)
run_suite

NPROCS=361
SIZE_STR=$(printf "%03d" $NPROCS)
run_suite
