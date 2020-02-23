#!/usr/bin/perl -w

while(<>) {
  if (/^(0x[0-9a-f]+) (0x[0-9a-f]+) (\w[\w\.]+)\s*/) {
    my $address = $1;
    my $opcode = $2;
    my $mnemonic = $3;
    my $parms = $';
    chomp $parms;
    print "$address $opcode\n";
  }
}
