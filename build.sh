#!/bin/bash

bold=$(tput bold)
normal=$(tput sgr0)

mkdir -p build \
&& cd build \
&& make -j4 install

if [ $? -eq 0 ]; then
  echo -e "\nBuild complete! Launch with ${bold}./run.sh${normal} or debug with ${bold}./run --debug${normal}"
else
  echo -e "\nBuild failed! Tip: you can debug successful builds with ${bold}./run --debug${normal}."
fi
