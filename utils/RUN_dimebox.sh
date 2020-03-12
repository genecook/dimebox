#!/bin/sh -f

if [ -z $DIMEBOX_HOME ]; then
  export DIMEBOX_HOME=$HOME/Desktop/dimebox;
fi

export RISCVtools=$HOME/Desktop/riscv

export DIMEBOX=$DIMEBOX_HOME/build/dimebox

export RISCVisa=$RISCVtools/riscv-tests/isa

export ENTRY_POINT=`/usr/bin/readelf -a $RISCVisa/$1 | grep 'Entry point address' | awk '{print $4;}'`

export SIG_START=`/usr/bin/readelf -s $RISCVisa/$1 | grep 'begin_signature' | awk '{print $2;}'`
export SIG_END=`/usr/bin/readelf -s $RISCVisa/$1 | grep 'end_signature' | awk '{print $2;}'`

$DIMEBOX --sim_load_file $RISCVisa/$1 --reset_address $ENTRY_POINT --show_updates --signature_address_range 0x${SIG_START}..0x$SIG_END --sim_dump_file ${1}.elf

echo signature file: test_signature
echo test image: ${1}.elf



