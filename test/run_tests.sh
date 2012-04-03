#!/bin/sh

if [ -z "$testBin" ]; then
    testBin="$1"
fi

if [ ! -x $testBin ] ; then
    testBin="../build/test/bzing_test"
    if [ ! -x $testBin ] ; then
        ${ECHO} "cannot execute test binary: '$testBin'"
        exit 1;
    fi
fi

$testBin
