#!/usr/bin/perl -w

# output class declarations for RV32I Base Instruction Set
# ---------------------------------------------------------

my @U = qw(LUI AUIPC FENCE FENCE.I ECALL EBREAK);
    
my @J = qw(JAL);
    
my @I = qw(JALR LB LH LW LBU LHU ADDI SLTI SLTIU XORI ORI ANDI
           SLLI SRLI SRAI CSRRW CSRRS CSRRC CSRRWI CSRRSI
           CSRRCI);
    
my @S = qw(SB SH SW);

my @B = qw(BEQ BNE BLT BGE BLTU BGEU ADD SUB SLL SLT
           SLTU XOR SRL SRA OR AND);

my $instr_template = '
class $INSTR$ : public $TYPE$typeInstruction {
  public:
    $INSTR$(RiscvState *_state,Memory *_memory,Signals *_signals,unsigned int _encoding)
    : $TYPE$typeInstruction(_state,_memory,_signals,_encoding) {};
    std::string InstrName() { return std::string("$instr$"); };
    void Step(); 
};
';

print "#ifndef __RV32I_ICLASSES__\n\n";

print "// !!! AUTO-GENERATED CODE - SEE UTILS/make_iclasses.pl !!!\n";

foreach $instr (@U) { &class_decl($instr,'U'); }
foreach $instr (@J) { &class_decl($instr,'J'); }
foreach $instr (@I) { &class_decl($instr,'I'); }
foreach $instr (@S) { &class_decl($instr,'S'); }
foreach $instr (@B) { &class_decl($instr,'B'); }

print "\n#endif\n";
print "#define __RV32I_ICLASSES__ 1\n";

sub class_decl {
    my ($instr,$type) = @_;

    my $instr_lc = lc $instr;
    $instr =~ s/\./_/;
    my $itemp = $instr_template;
    $itemp =~ s/\$INSTR\$/$instr/g;
    $itemp =~ s/\$instr\$/$instr_lc/g;
    $itemp =~ s/\$TYPE\$/$type/g;
    print $itemp;
}
