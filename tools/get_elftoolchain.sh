#!/bin/sh -f

# whereever you downloaded this script is where the elf tool chain will also be downloaded and compiled...

export TOOLS_DIR=`/bin/pwd`

export ELFTOOLCHAIN=elftoolchain-0.7.1

echo $TOOLS_DIR
echo $ELFTOOLCHAIN


echo Get,unpack elf tool chain...

wget -c https://sourceforge.net/projects/elftoolchain/files/Sources/${ELFTOOLCHAIN}/${ELFTOOLCHAIN}.tgz/download -O ${ELFTOOLCHAIN}.tgz
tar xzfv ${ELFTOOLCHAIN}.tgz


echo Get,unpack test software used by elf tool chain...

wget -c http://tetworks.opengroup.org/downloads/38/software/Sources/3.8/tet3.8-src.tar.gz -O tet3.8-src.tar.gz
cd ${ELFTOOLCHAIN}/test/tet; tar xzfv ${TOOLS_DIR}/tet3.8-src.tar.gz


echo build elf tool chain to yield libelf.a...

cd ${TOOLS_DIR}/${ELFTOOLCHAIN}
export CFLAGS='-O2 -Wno-format-truncation -Wno-stringop-truncation -Wno-stringop-overflow'
pmake



