#!/bin/sh

konoha=$1
script=$2
args="-Ipackage_not_found"

exec ${konoha} ${args} --test-with ${script}
