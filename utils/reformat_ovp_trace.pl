#!/usr/bin/perl -w

my %rvals = ();

while(<>) {
  if (/^Info \'riscvOVPsim\/cpu\', 0x00000000([0-9a-f]+)\([\+\w]+\)\: ([0-9a-f]+)\s+(\w[\w\.]*)\s+/) {
    my $address = '0x' . $1;
    my $opcode = '0x' . $2;
    my $mnemonic = $3;
    my $parms = $';
    chomp $parms;
    print "$address $opcode\n";
  }
  next;
  if (/^Info\s+(\w+)\s+([0-9a-f]+)\s\-\>\s([0-9a-f]+)/) {
      my $rname = $1;
      my $old_val = hex $2;
      my $new_val = hex $3;
      $rvals{$rname} = $old_val if not defined $rvals{$rname};
      if (not defined $rvals{$rname} or $rvals{$rname} != $new_val) {
	  $rvals{$rname} = $new_val;
          printf "  %s = 0x%08x\n",$rname,$new_val;
      }
  }
}
