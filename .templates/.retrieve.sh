#!/bin/bash

# Create optstring so we can just pass one argument and everything's parameterized.
OPTSTRING="hpm"

while getopts ${OPTSTRING} opt; do
	case ${opt} in
		h)
			source .hopper && export $(grep --regexp ^[A-Z] .hopper | cut -d= -f1)
			;;
		p)
			source .pangolin && export export $(grep --regexp ^[A-Z] .pangolin | cut -d= -f1)
			;;
		m)
			source .megltower && export export $(grep --regexp ^[A-Z] .megltower | cut -d= -f1)
			;;
		?)
			echo "Invalid option."
			exit 1
			;;
	esac
done

# Set options and exchange files.
OPTIONS="--verbose --recursive --ignore-existing"
IGNORE="--exclude-from=.$REMOTEHOST."

echo "pulling via rsync from $REMOTEUSER@$REMOTELOCATION with arguments $OPTIONS"

