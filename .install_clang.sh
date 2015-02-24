#!/bin/bash

wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-add-repository 'deb http://llvm.org/apt/precise/ llvm-toolchain-precise-3.5 main'
sudo apt-get -qq update
sudo apt-get -qq --force-yes install clang-3.5 libc++-dev
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-3.5 1

export CXXFLAGS="-std=gnu++14 -stdlib=libc++"
sudo rm /usr/local/clang-3.4/bin/clang++

svn co --quiet http://llvm.org/svn/llvm-project/libcxx/trunk libcxx

cd libcxx/lib && bash buildit
sudo cp ./libc++.so.1.0 /usr/lib/
sudo mkdir /usr/include/c++/v1
cd .. && sudo cp -r include/* /usr/include/c++/v1/
cd /usr/lib && sudo ln -sf libc++.so.1.0 libc++.so
sudo ln -sf libc++.so.1.0 libc++.so.1 && cd $cwd

CXX="clang++-3.5"
