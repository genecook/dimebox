#!/usr/bin/perl -w

# output class declarations for RV32I Base Instruction Set
# ---------------------------------------------------------

my @U = qw(LUI FENCE FENCE.I ECALL EBREAK);

my @U_AUIPC = qw(AUIPC);

my @J = qw(JAL);
    
my @I = qw(ADDI SLTI XORI ORI ANDI);

my @I_CSRR = qw(CSRRW CSRRC);

my @I_CSRS = qw(CSRRS);

my @I_CSRI = qw(CSRRWI CSRRSI CSRRCI);

my @I_UNS = qw(SLTIU);

my @I_SHIFT = qw(SLLI SRLI SRAI);

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

foreach $instr (@U)       { &class_decl($instr,'U','');                             }
foreach $instr (@U_AUIPC) { &class_decl($instr,'U','uaui=true');                    }
foreach $instr (@J)       { &class_decl($instr,'J','jal=true');                     }
foreach $instr (@I)       { &class_decl($instr,'I','');                             }
foreach $instr (@I_JALR)  { &class_decl($instr,'I','jal=true');                     }
foreach $instr (@I_LOAD)  { &class_decl($instr,'I','load_store=true');              }
foreach $instr (@I_UNS)   { &class_decl($instr,'I','unsigned_sign_extension=true'); }
foreach $instr (@I_SHIFT) { &class_decl($instr,'I','shift=true');                   }
foreach $instr (@I_CSRS)  { &class_decl($instr,'I','csrs=true');                    }
foreach $instr (@I_CSRR)  { &class_decl($instr,'I','csrr=true');                    }
foreach $instr (@I_CSRI)  { &class_decl($instr,'I','csri=true');                    }
foreach $instr (@S)       { &class_decl($instr,'S','load_store=true');              }
foreach $instr (@B)       { &class_decl($instr,'B','');                             }
foreach $instr (@R)       { &class_decl($instr,'R','');                             }

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
