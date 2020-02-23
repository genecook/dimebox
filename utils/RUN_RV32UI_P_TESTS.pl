#!/usr/bin/perl -w

my $dimebox = $ENV{HOME} . '/Desktop/dimebox/build/dimebox';

my $isa_testsdir = $ENV{HOME} . '/Desktop/riscv/riscv-tests/isa';

my @all_files = split(' ',`ls $isa_testsdir/rv32ui-p-*`);

my @tests = ();
my @pass_addresses = ();

foreach my $tf (@all_files) {
    if ($tf =~ /dump/) {
	if ($tf =~ 'rv32ui-p-simple.dump') {
	    push(@pass_addresses,'0x8000010c'); # ugly but this 'simple' test does not tag 'pass' region
	} else {
            push(@pass_addresses,&test_passes_address($tf));
	}
    } else {
	push(@tests,$tf)
    }
}

my @fails = ();

for(my $i = 0; $i < $#tests; $i++) {
    my $tf = $tests[$i];
    my $pa = $pass_addresses[$i];

    my @ta = split('/',$tf);
    
    my $logfile = $ta[$#ta] . '.log';
    my $cmdline = "$dimebox -L $tf --reset_address 0x80000000 --pass_address $pa -U 1>$logfile 2>&1";
    if (system($cmdline)) {
      print 'x';
      push(@my_fails,$logfile);
    } else {
      print '!';
    }
}

print "\n" . join("\n",@my_fails) . "\n";


sub test_passes_address {
    my($dump_file) = @_;

    my $pass_address = '';
    open(DF,$dump_file) or die;
    while(<DF>) {
      $pass_address = '0x' . $1 if /^([0-9a-f]+) \<pass\>\:$/;
    }

    close DF;
    return $pass_address;
}








