#!/usr/bin/perl -w


my %reg_vals = ();

while(<>) {
  if (/^(0x[0-9a-f]+) (0x[0-9a-f]+) (\w[\w\.]+)\s*/) {
    my $address = $1;
    my $opcode = $2;
    my $mnemonic = $3;
    my $parms = $';
    chomp $parms;
    print "$address $opcode\n";
    next;
  }
  next;
  if (/^  # R (\w+) \(csr 0x[0-9a-f]+\) \= (0x[0-9a-f]+)/) {
      my $rname = $1;
      my $rval  = hex $2;
      $reg_vals{$rname} = $rval if not defined $reg_vals{$rname};
      next;
  }
  
  if (/^  # W (\w+) \(csr 0x[0-9a-f]+\) \= (0x[0-9a-f]+)/) {
      my $rname = $1;
      my $rval = hex $2;
      if (not defined $reg_vals{$rname} or $reg_vals{$rname} != $rval) {
        $reg_vals{$rname} = $rval;
        printf "  %s = 0x%08x\n",$rname,$rval;
      }
      next;
  }

  if (/^  # R (\w+) \([^\)]+\) \= (0x[0-9a-f]+)/) {
      my $rname = $1;
      my $rval = hex $2;
      $reg_vals{$rname} = $rval if not defined $reg_vals{$rname};
      next;
  }
  
  if (/^  # W (\w+) \([^\)]+\) \= (0x[0-9a-f]+)/) {
      my $rname = $1;
      my $rval = hex $2;
      if (not defined $reg_vals{$rname} or $reg_vals{$rname} != $rval) {
        $reg_vals{$rname} = $rval;
        printf "  %s = 0x%08x\n",$rname,$rval;
      }
      next;
  }
}
