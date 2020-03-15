#!/usr/bin/perl

#****************************************************************************************
# Resimulate each archived ISA test image using current dimebox build. The archived
# ISA test images were originally created from dimebox simulations of Riscv-V ISA
# tests...
#****************************************************************************************

my $pwd = `/bin/pwd`;
chomp $pwd;

# use latest dimebox executable, rather than installed version:

my $dimebox_exec = $pwd . '/build/dimebox';

# the 'reformat' tool simply strips all but the disassembly from a simulation log:

my $reformat_tool = $pwd . '/utils/reformat_dimebox_trace.pl';

# we expect the archived test images to be here:

my $images_dir = 'isa_test_images';

print "Simulating all archived tests";

my @isa_tests = split(/\s+/,`ls $images_dir`);

die("No archived tests in $images_dir ?\n") if $#isa_tests <= 0;

foreach my $td (@isa_tests) {
    # an 'archived' test consists of a dimebox generated test image in elf format, and the log from the
    # dimebox test generation:
    my $test_image      = "$pwd/$images_dir/$td/${td}.elf";
    my $reference_log   = "$pwd/$images_dir/$td/${td}.test_sim_log";
    # resimulate a test from the test-image, compare the (reformatted) trace against the (reformatted)
    # reference trace...
    my $reference_trace = "${td}.test_reference_trace";
    my $resim_log       = "${td}.test_resim_log";
    my $resim_trace     = "${td}.test_resim_trace";
    my $reformat_log    = "${td}.test_reformat_log";
    
    my $tbuf =`readelf -a $test_image 2>/dev/null | grep 'Entry point'`;
    $tbuf =~ /Entry point address\:\s+(0x[0-9a-f]+)/;
    my $entry_point = $1;
    my $rundir = "/tmp/dimebox/test/$td";
    system("mkdir -p $rundir");
    my $dimebox_exec = "$pwd/build/dimebox";
    chdir $rundir or die("can't cd into $rundir?");
    my $rcode = system("$dimebox_exec -L $test_image --reset_address $entry_point -U --isa_test 1> $resim_log 2>&1");
    die("Resimulation fail! run-dir: $rundir\n") if ($rcode);
    $rcode = system("cat $reference_log | $reformat_tool 1> $reference_trace");
    die("Reformat of reference log fails? run-dir: $rundir\n") if ($rcode);
    $rcode = system("cat $resim_log | $reformat_tool > $resim_trace 2> $reformat_log");
    die("Reformat of resimulation log fails? run-dir: $rundir\n") if ($rcode);
    $rcode = system("diff $reference_trace $resim_trace");
    die("Trace from test resimulation does NOT match reference trace! run-dir: $rundir\n") if ($rcode);
    #print "$td PASSES!\n";
    print '.';
}

print "\nAll archived tests appear to (re)simulate correctly.\n";





