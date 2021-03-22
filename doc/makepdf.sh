#!/bin/sh

compile() {
	set -e
	while true
	do
		platex nusdas > platex.log
		grep -q Rerun platex.log || break
	done
	dvipdfmx nusdas > dvipdf.log 2>&1
}

if ! compile
then
	rc=$?
	echo fail: $rc
	exit $rc
fi
