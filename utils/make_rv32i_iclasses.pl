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
my @I_TRAP = qw(URET SRET HRET MRET WFI SFENCE);

my @S = qw(SB SH SW);

my @B = qw(BEQ BNE BLT BGE BLTU BGEU);

my @R = qw(ADD SLT SLTU AND OR XOR SLL SRL SUB SRA 
           MUL MULH MULHSU MULHU DIV DIVU REM REMU DIVW DIVUW REMW REMUW);

# compressed instructions:

my @CR   = qw( C.MV C.ADD C.EBREAK );
my @CR_J = qw( C.JR C.JALR );
my @CI   = qw( C.LWSP C.LI C.LUI C.ADDI C.ADDI16SP C.SLLI C.NOP );
my @CSS  = qw( C.SWSP );
my @CIW  = qw( C.ADDI4SPN );
my @CL   = qw( C.LW );
my @CS   = qw( C.SW C.AND C.OR C.XOR C.SUB );
my @CB   = qw( C.BEQZ C.BNEZ C.SRLI C.SRAI C.ANDI );
my @CJ   = qw( C.J C.JAL );

# instruction specific immediates
# (added to Decode method called prior to Disassembly):

my %decodes = (
    'ADDI'   => 'SIGN_EXTEND_IMM(12)',
    'SLTI'   => 'SIGN_EXTEND_IMM(12)',
    'XORI'   => 'SIGN_EXTEND_IMM(12)',
    'ORI'    => 'SIGN_EXTEND_IMM(12)',
    'ANDI'   => 'SIGN_EXTEND_IMM(12)',
    'SLTIU'  => 'UNSIGNED_SIGN_EXTEND_IMM(12)',
    'JAL'    => 'SIGN_EXTEND_IMM(20)',
    'JALR'   => 'SIGN_EXTEND_IMM(12)',
    'BEQ'    => 'SIGN_EXTEND_IMM(13)',
    'BNE'    => 'SIGN_EXTEND_IMM(13)',
    'BLT'    => 'SIGN_EXTEND_IMM(13)',
    'BLTU'   => 'SIGN_EXTEND_IMM(13)',
    'BGE'    => 'SIGN_EXTEND_IMM(13)',
    'BGEU'   => 'SIGN_EXTEND_IMM(13)',
    'LB'     => 'SIGN_EXTEND_IMM(12)',
    'LH'     => 'SIGN_EXTEND_IMM(12)',
    'LW'     => 'SIGN_EXTEND_IMM(12)',
    'LBU'    => 'SIGN_EXTEND_IMM(12)',
    'LHU'    => 'SIGN_EXTEND_IMM(12)',
    'SB'     => 'SIGN_EXTEND_IMM(12)',
    'SH'     => 'SIGN_EXTEND_IMM(12)',
    'SW'     => 'SIGN_EXTEND_IMM(12)',
    'C.J'    => 'SIGN_EXTEND_IMM(11)',
    'C.JAL'  => 'SIGN_EXTEND_IMM(11)',
    'C.BEQZ' => 'SIGN_EXTEND_IMM(8)',
    'C.BNEZ' => 'SIGN_EXTEND_IMM(8)',
    'C.SRAI' => 'IMM_FOR_SHIFT()',
    'C.ANDI' => 'SIGN_EXTEND_IMM(6)'
);


my $instr_template = '
class $INSTR$ : public $TYPE$typeInstruction {
  public:
    $INSTR$(RiscvState *_state,Memory *_memory,unsigned int _encoding)
    : $TYPE$typeInstruction(_state,_memory,_encoding) { $INIT$; };
    std::string InstrName() { return std::string("$instr$"); };
    void Step(); 
    void Decode() { $TYPE$typeInstruction::Decode(); $DECODE$; };
};
';

print "#ifndef __RV32IMC_ICLASSES__\n\n";

print "// !!! AUTO-GENERATED CODE - SEE utils/make_rv32i_iclasses.pl !!!\n";

foreach $instr (@U)       { &class_decl($instr,'U','',                             $decodes{$instr}); }
foreach $instr (@U_AUIPC) { &class_decl($instr,'U','uaui=true',                    $decodes{$instr}); }
foreach $instr (@J)       { &class_decl($instr,'J','jal=true',                     $decodes{$instr}); }
foreach $instr (@I)       { &class_decl($instr,'I','',                             $decodes{$instr}); }
foreach $instr (@I_JALR)  { &class_decl($instr,'I','jal=true',                     $decodes{$instr}); }
foreach $instr (@I_LOAD)  { &class_decl($instr,'I','load_store=true',              $decodes{$instr}); }
foreach $instr (@I_UNS)   { &class_decl($instr,'I','unsigned_sign_extension=true', $decodes{$instr}); }
foreach $instr (@I_SHIFT) { &class_decl($instr,'I','shift=true',                   $decodes{$instr}); }
foreach $instr (@I_CSRS)  { &class_decl($instr,'I','csrs=true',                    $decodes{$instr}); }
foreach $instr (@I_CSRR)  { &class_decl($instr,'I','csrr=true',                    $decodes{$instr}); }
foreach $instr (@I_CSRI)  { &class_decl($instr,'I','csri=true',                    $decodes{$instr}); }
foreach $instr (@I_TRAP)  { &class_decl($instr,'I','trap=true',                    $decodes{$instr}); }
foreach $instr (@S)       { &class_decl($instr,'S','load_store=true',              $decodes{$instr}); }
foreach $instr (@B)       { &class_decl($instr,'B','',                             $decodes{$instr}); }
foreach $instr (@R)       { &class_decl($instr,'R','',                             $decodes{$instr}); }

foreach $instr (@CR)      { &class_decl($instr,'CR', '',                $decodes{$instr}); }
foreach $instr (@CR_J)    { &class_decl($instr,'CR', 'jal=true',        $decodes{$instr}); }
foreach $instr (@CI)      { &class_decl($instr,'CI', '',                $decodes{$instr}); }
foreach $instr (@CSS)     { &class_decl($instr,'CSS','',                $decodes{$instr}); }
foreach $instr (@CIW)     { &class_decl($instr,'CIW','',                $decodes{$instr}); }
foreach $instr (@CL)      { &class_decl($instr,'CL', 'load_store=true', $decodes{$instr}); }
foreach $instr (@CS)      { &class_decl($instr,'CS', 'load_store=true', $decodes{$instr}); }
foreach $instr (@CB)      { &class_decl($instr,'CB', '',                $decodes{$instr}); }
foreach $instr (@CJ)      { &class_decl($instr,'CJ', 'jal=true',        $decodes{$instr}); }

print "\n#endif\n";
print "#define __RV32IMC_ICLASSES__ 1\n";

sub class_decl {
    my ($instr,$type,$init,$decode) = @_;

    $decode = '' if not defined($decode);
    
    my $instr_lc = lc $instr;
    $instr =~ s/\./_/;
    my $itemp = $instr_template;
    $itemp =~ s/\$INSTR\$/$instr/g;
    $itemp =~ s/\$instr\$/$instr_lc/g;
    $itemp =~ s/\$TYPE\$/$type/g;
    $itemp =~ s/\$INIT\$/$init/g;
    $itemp =~ s/\$DECODE\$/$decode/g;
    print $itemp;
}
