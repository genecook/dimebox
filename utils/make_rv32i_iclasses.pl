#!/usr/bin/perl -w

# output class declarations for RV32I Base Instruction Set
# ---------------------------------------------------------

my @U = qw(LUI FENCE FENCE.I ECALL EBREAK);

my @U_AUIPC = qw(AUIPC);

my @J = qw(JAL);
    
my @I = qw(ADDI SLTI SLTIU XORI ORI ANDI
           SLLI SRLI SRAI CSRRW CSRRS CSRRC CSRRWI CSRRSI
           CSRRCI);
    
my @I_JALR = qw(JALR);
    
my @I_LOAD = qw(LB LH LW LBU LHU);

my @S = qw(SB SH SW);

my @B = qw(BEQ BNE BLT BGE BLTU BGEU);

my @R = qw(ADD SLT SLTU AND OR XOR SLL SRL SUB SRA);

my $instr_template = '
class $INSTR$ : public $TYPE$typeInstruction {
  public:
    $INSTR$(RiscvState *_state,Memory *_memory,Signals *_signals,unsigned int _encoding)
    : $TYPE$typeInstruction(_state,_memory,_signals,_encoding) { $INIT$; };
    std::string InstrName() { return std::string("$instr$"); };
    void Step(); 
};
';

print "#ifndef __RV32I_ICLASSES__\n\n";

print "// !!! AUTO-GENERATED CODE - SEE utils/make_iclasses.pl !!!\n";

foreach $instr (@U)       { &class_decl($instr,'U','');                }
foreach $instr (@U_AUIPC) { &class_decl($instr,'U','uaui=true');       }
foreach $instr (@J)       { &class_decl($instr,'J','jal=true');        }
foreach $instr (@I)       { &class_decl($instr,'I','');                }
foreach $instr (@I_JALR)  { &class_decl($instr,'I','jal=true');        }
foreach $instr (@I_LOAD)  { &class_decl($instr,'I','load_store=true'); }
foreach $instr (@S)       { &class_decl($instr,'S','load_store=true'); }
foreach $instr (@B)       { &class_decl($instr,'B','');                }
foreach $instr (@R)       { &class_decl($instr,'R','');                }

print "\n#endif\n";
print "#define __RV32I_ICLASSES__ 1\n";

sub class_decl {
    my ($instr,$type,$init) = @_;

    my $instr_lc = lc $instr;
    $instr =~ s/\./_/;
    my $itemp = $instr_template;
    $itemp =~ s/\$INSTR\$/$instr/g;
    $itemp =~ s/\$instr\$/$instr_lc/g;
    $itemp =~ s/\$TYPE\$/$type/g;
    $itemp =~ s/\$INIT\$/$init/g;
    print $itemp;
}
