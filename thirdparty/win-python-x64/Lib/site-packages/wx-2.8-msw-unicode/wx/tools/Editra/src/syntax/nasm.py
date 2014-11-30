###############################################################################
# Name: nasm.py                                                               #
# Purpose: Define NASM syntax for highlighting and other features             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: nasm.py
AUTHOR: Cody Precord
@summary: Lexer configuration file Netwide Assembly Code
@todo: Add mmx, sse, 3dnow, cyrix, amd instruction sets

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: nasm.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
import synglob

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#

# NASM CPU Instructions
NASM_CPU_INST = (0, "cmps movs lcs lods stos xlat aaa aad aam adc and bound "
                    "bsf bsr bswap bt btc btr bts call cbw cdq clc cld cmc cmp "
                    "cmpsb cmpsd cmpsw cmpxchg cmpxchg8b cpuid cwd cwde daa "
                    "das enter int iret iretw jcxz jecxz jmp lahf lds lea "
                    "leave les lfs lgs lodsb lodsd lodsw loop loope loopne "
                    "loopnz loopz lss mov movsb movsd movsw movsx movzx neg "
                    "nop not or popa popad popaw popf popfd popfw push pusha "
                    "pushd pushaw pushf pushfd pushfw rcl rcr retf ret retn "
                    "rol ror sahf sal sar sbb scasb scasd scasw shl shld shrd "
                    "stc std stosb stosd stosw test xchg xlatb xor arpl lar "
                    "lsl verr verw lldt sldt lgdt sgdt ltr str clts lock wait "
                    "ins outs in insb insw insd out outsb outsw outsd cli sti "
                    "lidt sidt hlt invd lmsw prefetcht0 prefetcht1 prefetcht2 "
                    "prefetchnta rsm sfence smsw sysenter sysexit ud2 wbinvd "
                    "invlpg int1 int3 rdmsr rdtsc rdpmc wrmsr add dec div idiv "
                    "imul inc mul sub xaddf2xm1 "
                    )

# NASM FPU Instructions
NASM_FPU_INST = (1, "fchs fclex fcom fcomp fdecstp fdisi feni ffree ficom fild "
                    "finit fist fld fldcw fldenv fldl2e fldl2e fldl2t fldlg2 "
                    "fldln2 fldpi fldz fsave fscale fsetpm frndint frstor "
                    "fscale fsetpm fstcw fstenv fsts fstsw ftst fucom fucomp "
                    "fxam fxch fxtract fyl2x fyl2xp1"" fabs fadd faddp fbld "
                    "fcos fdiv fdivr fiadd fidiv fimul fisub fmul fpatan fptan "
                    "fsin fsincos fsqrt fsub fsubr fsave fbstp")

# NASM Registers
NASM_REGISTERS = (2, "ah al ax bh bl bp bx ch cl cr0 cr2 cr3 cr4 cs cx dh di "
                     "dl dr0 dr1 dr2 dr3 dr6 dr7 ds dx eax ebp ebx ecx edi edx "
                     "es esi esp fs gs si sp ss st tr3 tr4 tr5 tr6 tr7 st0 st1 "
                     "st2 st3 st4 st5 st6 st7 mm0 mm1 mm2 mm3 mm4 mm5 mm6 mm7 "
                     "xmm0 xmm1 xmm2 xmm3 xmm4 xmm5 xmm6 xmm7")

# NASM Directives
NASM_DIRECTIVES = (3, "DF EXTRN FWORD RESF TBYTE FAR NEAR SHORT BYTE WORD "
                      "QWORD DQWORD HWORD DHWORD TWORD CDECL FASTCALL NONE "
                      "PASCAL STDCALL DB DW DD DQ DDQ DT RESB RESW RESD RESQ "
                      "REST EXTERN GLOBAL COMMON __BITS__ __DATE__ __FILE__ "
                      "__FORMAT__ __LINE__ __NASM_MAJOR__ __NASM_MINOR__ "
                      "__NASM_VERSION__ __TIME__ TIMES ALIGN ALIGNB INCBIN "
                      "EQU NOSPLIT SPLIT ABSOLUTE BITS SECTION SEGMENT DWORD "
                      "ENDSECTION ENDSEGMENT __SECT__ ENDPROC EPILOGUE LOCALS "
                      "PROC PROLOGUE USES ENDIF ELSE ELIF ELSIF IF DO ENDFOR "
                      "ENDWHILE FOR REPEAT UNTIL WHILE EXIT ORG EXPORT GROUP "
                      "UPPERCASE SEG WRT LIBRARY _GLOBAL_OFFSET_TABLE_ "
                      "__GLOBAL_OFFSET_TABLE_ ..start ..got ..gotoff ..gotpc "
                      "..pit ..sym %define %idefine %xdefine %xidefine %undef "
                      "%assign %iassign %strlen %substr %macro %imacro "
                      "%endmacro %rotate .nolist %if %elif %else %endif %ifdef "
                      "%ifndef %elifdef %elifndef %ifmacro %ifnmacro "
                      "%elifnmacro %ifctk %ifnctk %elifctk %elifnctk %ifidn "
                      "%ifnidn %elifidn %elifnidn %ifidni %ifnidni %elifidni "
                      "%elifnidni %ifid %ifnid %elifid %elifnid %ifstr %ifnstr "
                      "%elifstr %elifnstr %ifnum %ifnnum %elifnum %elifnnum "
                      "%error %rep %endrep %exitrep %include %push %pop %repl "
                      "struct endstruc istruc at iend align alignb %arg "
                      "%stacksize %local %line bits use16 use32 section "
                      "absolute extern global common cpu org section group "
                      "import export %elifmacro ")

NASM_DIREC_OP = (4, "a16 a32 o16 o32 byte word dword nosplit $ $$ seq wrt flat "
                    "large small .text .data .bss near far %0 %1 %2 %3 %4 %5 "
                    "%6 %7 %8 %9")

NASM_EXT_INST = (5, "")

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
    if lang_id == synglob.ID_LANG_NASM:
        return [NASM_CPU_INST, NASM_FPU_INST, NASM_REGISTERS,
                NASM_DIRECTIVES, NASM_DIREC_OP]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_NASM:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_NASM:
        return [u';']
    else:
        return list()

#---- End Required Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
