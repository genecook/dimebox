#!/bin/sh -f

export WDIR=`/bin/pwd`

# download boost...

wget -c https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz -O boost_1_72_0.tar.gz 
tar xzfv boost_1_72_0.tar.gz

cd boost_1_72_0

# configure boost - program options library only...

./bootstrap.sh --prefix=$WDIR --exec-prefix=$WDIR --libdir=$WDIR/boost/lib --includedir=$WDIR/boost/include --with-libraries=program_options

# build boost. install optional...

./b2

# -I/home/genec/Desktop/dimebox/tools/boost_1_72_0
# -L /home/genec/Desktop/dimebox/tools/boost_1_72_0/stage/lib/ -l libboost_program_options.a
