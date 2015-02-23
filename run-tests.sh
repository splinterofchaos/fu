#!/bin/sh

if [ "$CXX" = "clang++" ]; then export EXTRA="-stdlib=libc++ -I/usr/include/c++/v1"; fi

for file in test/*
do
  echo "compiling ${file}..."
  $CXX $file -std=c++14 -Iinclude -Wall -Wextra -Werror $EXTRA || exit 1
  ./a.out || exit 1
done
