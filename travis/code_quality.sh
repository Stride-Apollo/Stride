#!/bin/bash

if [ "${quality_dir}" = "src/main" ]; then
	# Be very strict on normal code
	MINLINES=5
	LIMIT=2
else
	MINLINES=15
	LIMIT=5
fi

jscpd \
	--path ${quality_dir:=.} \
	--skip-comments \
	--languages clike,javascript,python,css,htmlmixed \
	--exclude "**/resources/**" \
	--min-lines $MINLINES \
	--blame \
	--limit $LIMIT
