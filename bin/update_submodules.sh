#!/usr/bin/env bash

# initialize submodules recursively
git submodule update --init --force --recursive

# update wownero
cd ./external/wownero
git checkout master
git pull --ff-only origin master
cd ../../
