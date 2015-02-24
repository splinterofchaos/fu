#!/bin/sh

for file in test/*
do
  echo "compiling ${file}..."
  $CXX $file -std=c++14 -Iinclude -Wall -Wextra -Werror $CXXFLAGS || exit 1
  ./a.out || exit 1
done
