#!/usr/bin/perl -w

while(<>) {
  if (/^Info \'riscvOVPsim\/cpu\', 0x00000000([0-9a-f]+)\([\+\w]+\)\: ([0-9a-f]+) (\w[\w\.]*)\s+/) {
    my $address = '0x' . $1;
    my $opcode = '0x' . $2;
    my $mnemonic = $3;
    my $parms = $';
    chomp $parms;
    print "$address $opcode\n";
  }
}
