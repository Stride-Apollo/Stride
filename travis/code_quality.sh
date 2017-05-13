#!/bin/bash

jscpd \
	--path ${quality_dir:=.} \
	--skip-comments \
	--languages clike,javascript,python,css,htmlmixed \
	--exclude "**/resources/**" \
	--min-lines 15 \
	--blame \
	--limit 5
