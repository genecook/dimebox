#!/bin/sh -f

export RISCVtools=$HOME/Desktop/riscv

# Imperas riscvOVPsim \(https://github.com/riscv/riscv-ovpsim\), version v20191217.0 was used as \'golden model\'. Thanks Imperas!

export OVPsim=$RISCVtools/riscv-ovpsim-20191217/bin/Linux64/riscvOVPsim.exe
export RISCVisa=$RISCVtools/riscv-tests/isa
export SIGNATURE_OPT='--signaturedump --override riscvOVPsim/cpu/sigdump/SignatureAtEnd=T --override riscvOVPsim/cpu/sigdump/SignatureFile'

$OVPsim --variant RV32IM -t -traceregs --program $RISCVisa/$1 ${SIGNATURE_OPT}=$1.signature

