###############################################################################
# Name: masm.py                                                               #
# Purpose: Define MASM syntax for highlighting and other features             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: masm.py
AUTHOR: Cody Precord
@summary: Lexer configuration file Microsoft Assembly Code

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: masm.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#

# MASM CPU Instructions/Operators
MASM_CPU_INST = (0, "aaa aad aam aas adc and arpl bound bsf bsr bswap bt btc "
                    "btr bts call cdw cdq clc cld cli clts cmc cmp cmps cmpsb "
                    "cmpsw cmpsd cmpxchng cwd cwde daa das enter in ins insb "
                    "insw insd int into invd invlpg iret iretd ja jae jb jbe "
                    "jc jcxz jecxz je jz jg jge jl jle jna jnae jnb jnbe jnc "
                    "jne jng jnge jnl jnle jno jnp jns jnz jo jp jpe jpo js jz "
                    "jmp lahf lar lea leave lgdt lidt lgs lss lfs lods lodsb "
                    "lodsw lodsd loop loope loopz loone loopne retf retn lds "
                    "les lldt lmsw lock lsl ltr mov movs movsb movsw movsd "
                    "movsx movzx neg nop not or out outs outsb outsw outsd "
                    "pop popa popd popf popfd push pusha pushad pushf pushfd "
                    "rcl rcr rol roro rep repe repz repne repnz ret sahf sal "
                    "sar shl shr sbb scas scasb scasw scasd seta setae setb "
                    "setbe setc sete setg setge setl setle setna setnae setnb "
                    "setnbe setnc setne setng setnge setnl setnle setno setnp "
                    "setns setnz seto setp setpe setpo ses setz sgdt sidt shld "
                    "shrd sldt smsw stc std sti stos stosb stosw stosd str "
                    "test verr verw wait wbinvd xchg xlat xlatb xor add dec "
                    "idiv imul inc mul sub xadd div "
                    # MMX/SSE/SSE2 Instructions
                    "cflush cpuid emms femms cmovo cmovno cmovb cmovc cmovnae "
                    "cmovae cmovnb cmovnc cmove cmovz cmovne cmovnz cmovbe "
                    "cmovna cmova cmovnbe cmovs cmovns cmovp cmovpe cmovnp "
                    "cmovpo cmovl cmovnge cmovge cmovnl cmovle cmovng cmovg "
                    "cmovnle cmpxchg486 cmpxchg8b loadall loadall286 ibts "
                    "icebp int1 int3 int01 int03 iretw popaw popfw pushaw "
                    "pushfw rdmsr rdpmc rdshr rdtsc rsdc rsldt rsm rsts salc "
                    "smi smint smintold svdc svldt svts syscall sysenter "
                    "sysexit sysret ud0 ud1 ud2 umov xbts wrmsr wrshr")

# floating point instructions
MASM_FPU_INST = (1, "f2xm1 fabs fadd faddp fbld fbstp fchs fclex fcom fcomp "
                    "fcompp fdecstp fdisi fdiv fdivp fdivr fdivrp feni ffree "
                    "fiadd ficom ficomp fidiv fidivr fild fimul fincstp finit "
                    "fist fistp fisub fisubr fld fld1 fldcw fldenv fldenvw "
                    "fldl2e fldl2t fldlg2 fldln2 fldpi fldz fmul fmulp fnclex "
                    "fndisi fneni fninit fnop fnsave fnsavew fnstcw fnstenv "
                    "fnstenvw fnstsw fpatan fprem fptan frndint frstor frstorw "
                    "fsave fsavew fscale fsqrt fst fstcw fstenv fstenvw fstp "
                    "fstsw fsub fsubp fsubr fsubrp ftst fwait fxam fxch "
                    "fxtract fyl2x fyl2xp1 fsetpm fcos fldenvd fnsaved "
                    "fnstenvd fprem1 frstord fsaved fsin fsincos fstenvd fucom "
                    "fucomp fucompp fcomi fcomip ffreep fcmovb fcmove fcmovbe "
                    "fcmovu fcmovnb fcmovne fcmovnbe fcmovnu ")

MASM_REGISTERS = (2, "ah al ax bh bl bp bx ch cl cr0 cr2 cr3 cr4 cs cx dh di "
                     "dl dr0 dr1 dr2 dr3 dr6 dr7 ds dx eax ebp ebx ecx edi edx "
                     "es esi esp fs gs si sp ss st tr3 tr4 tr5 tr6 tr7 st0 st1 "
                     "st2 st3 st4 st5 st6 st7 mm0 mm1 mm2 mm3 mm4 mm5 mm6 mm7 "
                     "xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7")

MASM_DIRECTIVES = (3, ".186 .286 .286c .286p .287 .386 .386c .386p .387 .486 "
                      ".486p .8086 .8087 .alpha .break .code .const .continue "
                      ".cref .data .data? .dosseg .else .elseif .endif .endw "
                      ".err .err1 .err2 .errb .errdef .errdif .errdifi .erre "
                      ".erridn .erridni .errnb .errndef .errnz .exit .fardata "
                      ".fardata? .if .lall .lfcond .list .listall .listif "
                      ".listmacro .listmacroall  .model .no87 .nocref .nolist "
                      ".nolistif .nolistmacro .radix .repeat .sall .seq "
                      ".sfcond .stack .startup .tfcond .type .until .untilcxz "
                      ".while .xall .xcref .xlist alias align assume catstr "
                      "comm comment db dd df dosseg dq dt dup dw echo else "
                      "elseif elseif1 elseif2 elseifb elseifdef elseifdif "
                      "elseifdifi elseife elseifidn elseifidni elseifnb "
                      "elseifndef end endif endm endp ends eq equ even exitm "
                      "extern externdef extrn for forc ge goto group gt high "
                      "highword if if1 if2 ifb ifdef ifdif ifdifi ife ifidn "
                      "ifidni ifnb ifndef include includelib instr invoke irp "
                      "irpc label le length lengthof local low lowword "
                      "lroffset lt macro mask mod .msfloat name ne offset "
                      "opattr option org %out page popcontext proc proto ptr "
                      "public purge pushcontext record repeat rept seg segment "
                      "short size sizeof sizestr struc struct substr subtitle "
                      "subttl textequ this title type typedef union while "
                      "width")

MASM_DIREC_OP = (4, "$ ? @b @f addr basic byte c carry? dword far far16 "
                    "fortran fword near near16 overflow? parity? pascal qword "
                    "real4 real8 real10 sbyte sdword sign? stdcall sword "
                    "syscall tbyte vararg word zero? flat near32 far32 abs all "
                    "assumes at casemap common compact cpu dotname emulator "
                    "epilogue error export expr16 expr32 farstack flat "
                    "forceframe huge language large listing ljmp loadds m510 "
                    "medium memory nearstack nodotname noemulator nokeyword "
                    "noljmp nom510 none nonunique nooldmacros nooldstructs "
                    "noreadonly noscoped nosignextend nothing notpublic "
                    "oldmacros oldstructs os_dos para private prologue radix "
                    "readonly req scoped setif2 smallstack tiny use16 use32 "
                    "uses")

MASM_EXT_INST = (5, "addpd addps addsd addss andpd andps andnpd andnps cmpeqpd "
                    "cmpltpd cmplepd cmpunordpd cmpnepd cmpnltpd cmpnlepd "
                    "cmpordpd cmpeqps cmpltps cmpleps cmpunordps cmpneps "
                    "cmpnltps cmpnleps cmpordps cmpeqsd cmpltsd cmplesd "
                    "cmpunordsd cmpnesd cmpnltsd cmpnlesd cmpordsd cmpeqss "
                    "cmpltss cmpless cmpunordss cmpness cmpnltss cmpnless "
                    "cmpordss comisd comiss cvtdq2pd cvtdq2ps cvtpd2dq "
                    "cvtpd2pi cvtpd2ps cvtpi2pd cvtpi2ps cvtps2dq cvtps2pd "
                    "cvtps2pi cvtss2sd cvtss2si cvtsd2si cvtsd2ss cvtsi2sd "
                    "cvtsi2ss cvttpd2dq cvttpd2pi cvttps2dq cvttps2pi "
                    "cvttsd2si cvttss2si divpd divps divsd divss fxrstor "
                    "fxsave ldmxscr lfence mfence maskmovdqu maskmovdq maxpd "
                    "maxps paxsd maxss minpd minps minsd minss movapd movaps "
                    "movdq2q movdqa movdqu movhlps movhpd movhps movd movq "
                    "movlhps movlpd movlps movmskpd movmskps movntdq movnti "
                    "movntpd movntps movntq movq2dq movsd movss movupd movups "
                    "mulpd mulps mulsd mulss orpd orps packssdw packsswb "
                    "packuswb paddb paddsb paddw paddsw paddd paddsiw paddq "
                    "paddusb paddusw pand pandn pause paveb pavgb pavgw "
                    "pavgusb pdistib pextrw pcmpeqb pcmpeqw pcmpeqd pcmpgtb "
                    "pcmpgtw pcmpgtd pf2id pf2iw pfacc pfadd pfcmpeq pfcmpge "
                    "pfcmpgt pfmax pfmin pfmul pmachriw pmaddwd pmagw pmaxsw "
                    "pmaxub pminsw pminub pmovmskb pmulhrwc pmulhriw "
                    "pmulhrwa pmulhuw pmulhw pmullw pmuludq pmvzb pmvnzb "
                    "pmvlzb pmvgezb pfnacc pfpnacc por prefetch prefetchw "
                    "prefetchnta prefetcht0 prefetcht1 prefetcht2 pfrcp "
                    "pfrcpit1 pfrcpit2 pfrsqit1 pfrsqrt pfsub pfsubr pi2fd "
                    "pf2iw pinsrw psadbw pshufd pshufhw pshuflw pshufw psllw "
                    "pslld psllq pslldq psraw psrad psrlw psrld psrlq psrldq "
                    "psubb psubw psubd psubq psubsb psubsw psubusb psubusw "
                    "psubsiw pswapd punpckhbw punpckhwd punpckhdq punpckhqdq "
                    "punpcklbw punpcklwd punpckldq punpcklqdq pxor rcpps "
                    "rcpss rsqrtps rsqrtss sfence shufpd shufps sqrtpd sqrtps "
                    "sqrtsd sqrtss stmxcsr subpd subps subsd subss ucomisd "
                    "ucomiss unpckhpd unpckhps unpcklpd unpcklps xorpd xorps")

#---- Language Styling Specs ----#
SYNTAX_ITEMS = [ ('STC_ASM_DEFAULT', 'default_style'),
                 ('STC_ASM_CHARACTER', 'char_style'),
                 ('STC_ASM_COMMENT', 'comment_style'),
                 ('STC_ASM_COMMENTBLOCK', 'comment_style'),
                 ('STC_ASM_CPUINSTRUCTION', 'keyword_style'),
                 ('STC_ASM_DIRECTIVE', 'keyword3_style'),
                 ('STC_ASM_DIRECTIVEOPERAND', 'keyword4_style'),
                 ('STC_ASM_EXTINSTRUCTION', 'funct_style'),
                 ('STC_ASM_IDENTIFIER', 'default_style'),
                 ('STC_ASM_MATHINSTRUCTION', 'keyword_style'),
                 ('STC_ASM_NUMBER', 'number_style'),
                 ('STC_ASM_OPERATOR', 'operator_style'),
                 ('STC_ASM_REGISTER', 'keyword2_style'),
                 ('STC_ASM_STRING', 'string_style'),
                 ('STC_ASM_STRINGEOL', 'stringeol_style') ]
#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    return [MASM_CPU_INST, MASM_FPU_INST, MASM_REGISTERS, MASM_DIRECTIVES,
            MASM_DIREC_OP, MASM_EXT_INST]

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    return SYNTAX_ITEMS

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    return [u';']
#---- End Required Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
