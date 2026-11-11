#!/bin/bash

BUILD_TGT=${BUILD_TGT:-"../build/moonlight"}
test -e $BUILD_TGT || ( echo "Need moonlight to be built"; exit 1 )

echo "We expect this command to FAIL."
echo ""
echo "=================================="
echo ""
$BUILD_TGT list -config moonlight-non-passing.conf

echo ""
echo "=================================="
echo ""
echo "We expect this command to SUCCEED."
echo ""
echo "=================================="
echo ""
$BUILD_TGT list -config moonlight-passing.conf
