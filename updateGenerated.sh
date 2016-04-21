#!/bin/sh

rm -rf gen
python ../ARSDKBuildUtils/Utils/Python/ARSDK_PrebuildActions.py --lib libARNetworkAL --root . || exit 1
