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
OPTIONS="--verbose --human-readable --recursive --update"
IGNORE="--exclude-from=.$REMOTEHOST.ignore"

# Push to both servers, and build.
echo "pushing to $REMOTEUSER@$REMOTELOCATION via rsync with arguments $OPTIONS and ignoring $IGNORE"
