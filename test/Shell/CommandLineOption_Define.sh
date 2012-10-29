#!/bin/sh

konoha=$1
script=$2
args="-DIVAL=123 -DSVAL=abc"

exec ${konoha} ${args} --test-with ${script}
