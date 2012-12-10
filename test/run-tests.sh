#!/bin/bash
# QPy - Copyright (c) 2012,2013 Ugo Varetto
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the author and copyright holder nor the
#       names of contributors to the project may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL UGO VARETTO BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

TEST_DRIVER=$1
TEST_DIR=$2
PASSED=0
FAILED=0
if [ -z $TEST_DRIVER ] || [ -z $TEST_DIR ]; then
  echo "Usage: $0 <test driver> <test directory>" 
else
  for f in $( ls $TEST_DIR/*.py ); do
  	TOTAL=$[TOTAL+1]
    echo "__________________________________"
    echo
    echo $f
    echo
    diff <( $TEST_DRIVER $f ) <( cat $f-expected )
    if [ $? -eq 0 ]; then
      echo "== OK =="
      PASSED=$[PASSED+1]
    else
      echo	
      echo "!!!!!! FAILED !!!!!!"
      FAILED=$[FAILED+1]
    fi    
  done  
fi
echo
echo "=================================="
echo
TOTAL=$[PASSED + FAILED]
echo "Total tests: $TOTAL"
echo "Passed       $PASSED"
echo "Failed:      $FAILED"
if [ $FAILED -ne 0 ]; then
  echo "!!!!!!!!!! FAILED !!!!!!!!!!!"
else
  echo "=== PASSED ==="
fi  