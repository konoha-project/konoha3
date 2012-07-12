#!/bin/bash

find ./test -maxdepth 3 -name '*.k' | while read utest; do
	#echo "testing $utest .."
	konoha2 --test-with $utest || echo "[FAILED] $utest"
done

