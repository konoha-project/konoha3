#!/bin/sh
target=Release
./${target}/logpoold &
for i in $(seq 1 100); do
	${target}/test_trace 100000 &
done
