#!/usr/bin/perl -w

my $DIMEBOX_HOME = $ENV{HOME} . '/Desktop/dimebox';

my $isa_testsdir = $ENV{HOME} . '/Desktop/riscv/riscv-tests/isa';

my @all_files = split(' ',`cd $isa_testsdir; ls rv32ui-p-* rv32um-p-* rv32mi-p-*`);

my %known_fails = ( 'rv32mi-p-breakpoint' => 1,  # certain debug registers read as zero on some platforms
                    'rv32mi-p-illegal' => 1,     # dimebox not supporting S mode causes mstatus mismatch
                    'rv32mi-p-ma_addr' => 1,     # dimebox does enforce instruction alignment, but not data alignment
                    'rv32mi-p-sbreak' => 1,      # dimebox
                    'rv32mi-p-scall' => 1,       #   does not support S mode
                    'rv32mi-p-shamt' => 1        # unsupported dimebox instruction
                  );

my @tests = ();
my @pass_addresses = ();

foreach my $tf (@all_files) {
    if ($tf =~ /dump/) {
	# ignore test 'dump'...
    } elsif (defined $known_fails{$tf}) {
	# ignore tests known to fail...
    } else {
	push(@tests,$tf)
    }
}

my $pwd = `/bin/pwd`;
chomp $pwd;

my @fails = ();

for (my $i = 0; $i < $#tests; $i++) {
    my $tf = $tests[$i];
    
    system("rm -rf $tf");
    system("mkdir $tf");
    chdir $tf;
    
    open(FH,">","./run_test.sh");
    print FH "#!/bin/sh -f\n\n";
    print FH "$DIMEBOX_HOME/utils/VERIFY_TEST.sh $tf 1>${tf}.log 2>\&1\n";
    close FH;
    
    system("chmod ugo+rx ./run_test.sh");
    
    if (system("./run_test.sh")) {
      print 'x';
      push(@my_fails,$tf);
    } else {
      print '!';
    }
    chdir $pwd;
}

print "\n" . join("\n",@my_fails) . "\n";








