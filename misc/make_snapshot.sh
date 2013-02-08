#!/bin/sh
stable="$1"
version="0.3-alpha"

if [ "x$stable" == "x" ]; then
    echo "usage $0 stable_revision"
    exit 1
fi

date=`date +"%Y-%m-%d-%H-%M-%S"`
git checkout $stable
git tag "$version-$date"
git checkout master
