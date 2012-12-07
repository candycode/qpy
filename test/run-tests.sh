#!/bin/bash

TEST_DRIVER=$1
TEST_DIR=$2

if [ -z $TEST_DRIVER ] || [ -z $TEST_DIR ]; then
  echo "Usage: $0 <test driver> <test directory>" 
else
  for f in $( ls $TEST_DIR/*.py ); do
    echo "__________________________________"
    echo
    echo $f
    echo
    diff <( $TEST_DRIVER $f ) <( cat $f-expected )
  done  
fi
