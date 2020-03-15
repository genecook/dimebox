#!/usr/bin/perl -w

#**********************************************************************************************
# Use this script to both run ISA tests from Riscv (see https://github.com/riscv/riscv-tests)
# and create/update dimebox unit tests.
#
# NOTE: Unless you have a need to review how the isa unit-tests were created, or wish to
#       include additional tests, you don't need to run this script.
#
# ~/Desktop/riscv/riscv-tests/isa
#    - riscv-tests checked out and built here.
#
# ~/Desktop/dimebox
#    - assumed dimebox dev directory. (see https://github.com/genecook/dimebox)
#
# $HOME/Desktop/riscv/riscv-ovpsim-20191217 -
#    - OVPsim. OVP riscv simulator used as 'golden model' in creating initial set of dimebox
#      tests
#
# ./test
#    - scratch directory in which riscv-tests/isa will be run.
#
# ./unit-tests
#    - unit-tests directory populated (in part) from whatever riscv-tests have been run.
#**********************************************************************************************

# -D option causes dimebox generated test image and logs for each successful test to be copied
# into dimebox/isa_test_images sub-dir...

my $install = 0;

if (defined($ARGV[0])) {
    $install = ($ARGV[0] eq '-D') ? 1 : 0;
}

# this script normally is run in the dimebox dev directory, build sub-dir...

my $DIMEBOX_HOME = $ENV{HOME} . '/Desktop/dimebox';

# heres where the riscv-test suites have been checked out and built:

my $isa_testsdir = $ENV{HOME} . '/Desktop/riscv/riscv-tests/isa';

# heres the riscv-tests/isa test suites run thusfar:

my @all_files = split(' ',`cd $isa_testsdir; ls rv32ui-p-* rv32um-p-* rv32mi-p-*`);

# these are the known fails from the above test suites:

my %known_fails = ( 'rv32mi-p-breakpoint' => 1,  # certain debug registers read as zero on some platforms
                    'rv32mi-p-illegal' => 1,     # dimebox not supporting S mode causes mstatus mismatch
                    'rv32mi-p-ma_addr' => 1,     # dimebox does enforce instruction alignment, but not data alignment
                    'rv32mi-p-sbreak' => 1,      # dimebox
                    'rv32mi-p-scall' => 1,       #   does not support S mode
                    'rv32mi-p-shamt' => 1        # unsupported dimebox instruction
                  );


my @tests = ();  # the list of tests to run...

foreach my $tf (@all_files) {
    if ($tf =~ /dump/) {
	# ignore test 'dump'...
    } elsif (defined $known_fails{$tf}) {
	# ignore tests known to fail...
    } else {
	push(@tests,$tf)
    }
}

# run the tests...

system("mkdir -p test");
chdir 'test';

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
      if ($install) {
	  # retain dimebox generated test-image, simulation log...
	  my $install_dir = $DIMEBOX_HOME . '/isa_test_images';
	  my $test_image = $tf . '.elf';
	  my $test_log = $tf . '.test_sim_log';
	  system("mkdir -p $install_dir/$tf");
	  system("cp $test_image $install_dir/$tf");
	  system("cp $test_log $install_dir/$tf");
      }
    }
    chdir $pwd;
}

print "\n" . join("\n",@my_fails) . "\n";








