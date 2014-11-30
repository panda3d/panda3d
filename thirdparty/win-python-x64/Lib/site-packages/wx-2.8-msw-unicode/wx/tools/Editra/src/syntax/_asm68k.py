###############################################################################
# Name: asm68k.py                                                             #
# Purpose: Define 68k/56k assembly syntax for highlighting and other features #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: asm68k.py
AUTHOR: Cody Precord
@summary: Lexer configuration file 68k/56k Assembly Code
@todo: more color configuration

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _asm68k.py 64857 2010-07-09 14:06:38Z CJP $"
__revision__ = "$Revision: 64857 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Local Imports
import synglob
import syndata

#-----------------------------------------------------------------------------#

#---- 68K Keyword Definitions ----#

ASM_CPU_INST = (0, "andi and as b beq bg b bl bne bge bpl bchg bclr bfchg "
                   "bfclr bfexts bfextu bfffo bfins bfset bftst bkpt bra bset "
                   "bsr btst callm cas2 cas chk2 chk clr cmpa cmpi cmpm cmp "
                   "dbcc dbcs dbvc dbvs dbeq dbf dbt dbge dbgt dbhi dbmi dble "
                   "dbls dblt dbne dbpl dbra eori eor exg extb ext illegaljmp "
                   "jsr lea link ls mm movea movec movem movep moveq moves "
                   "move nbcd negx neg nop not ori or pack pea reset ro rox rt "
                   "sbcd seq sne spl swap tas trap tst unlk unpk abcd")

ASM_MATH_INST = (1, "adda addi addq addx add div mul suba subi subq subx sub "
                    "tdiv")

ASM_REGISTER = (2, "a0 a1 a2 a3 a4 a5 a6 a7 d0 d1 d2 d3 d4 d5 d6 d7 pc sr "
                   "ccr sp usp ssp vbr sfc sfcr dfc dfcr msp isp zpc cacr "
                   "caar za0 za1 za2 za3 za4 za5 za6 za7 zd0 zd1 zd2 zd3 "
                   "zd4 zd5 zd6 zd7 crp srp tc ac0 ac1 acusr tt0 tt1 mmusr "
                   "dtt0 dtt1 itt0 itt1 urp cal val scc crp srp drp tc ac psr "
                   "pcsr bac0 bac1 bac2 bac3 bac4 bac5 bac6 bac7 bad0 bad1 "
                   "bad2 bad3 bad4 bad5 bad6 bad7 fp0 fp1 fp2 fp3 fp4 fp5 fp6 "
                   "fp7 control status iaddr fpcr fpsr fpiar ")

ASM_DIRECTIVES = (3, "ALIGN CHIP COMLINE COMMON DC DCB DS END EQU FEQU FAIL "
                     "FOPT IDNT LLEN MASK2 NAME NOOBJ OFFSET OPT ORG PLEN REG "
                     "RESTORE SAVE SECT SECTION SET SPC TTL XCOM XDEF XREF")

#---- 56K Keywords ----#

ASM56K_CPU_INST = (0, "adc addl addr and andi asl asr bchg bclr bra "
                      "brclr brset bsclr bset bsr bsset btst bcc bcs bec beq "
                      "bes bge bgt blc ble bls blt bmi bne bnr bpl bnn brkcc "
                      "brkcs brkec brkeq brkes brkge brkgt brklc brkle brkls "
                      "brklt brkmi brkne brknr brkpl brknn bscc bscs bsec bseq "
                      "bses bsge bsgt bslc bsle bsls bslt bsmi bsne bsnr bspl "
                      "bsnn clb clr cmp cmpm cmpu debug dmac do "
                      "forever dor enddo eor extract extractu illegal inc "
                      "insert jclr jmp jsclr jset jsr jsset jcc jcs jec jeq "
                      "jes jge jgt jlc jle jls jlt jmi jne jnr jpl jnn jscc "
                      "jscs jsec jseq jses jsge jsgt jslc jsle jsls jslt jsmi "
                      "jsne jsnr jspl jsnn lra lsl lsr lua mac maci macr macri "
                      "max maxm merge move movem movec movep mpy mpyi mpyr "
                      "mpyri nop norm normf not or ori pflush pflushun "
                      "pfree plock plockr punlock punlockr rep reset rnd rol "
                      "ror rti rts sbc stop tcc tfr trap trapcc "
                      "trapcs trapec trapeq trapes trapge trapgt traplc traple "
                      "trapls traplt trapmi trapne trapnr trappl trapnn tst "
                      "vsl wait")

ASM56K_MATH_INST = (1, "abs add dec div neg sub subl subr")

ASM56K_REGISTERS = (2, "pc mr ccr sr eom com omr sz sc vba la lc sp ssh ssl ss "
                       "a a2 a1 a0 b b2 b1 b0 x x0 x1 y y0 y1 r0 r1 r2 r3 r4 "
                       "r5 r6 r7 m0 m1 m2 m3 m4 m5 m6 m7 n0 n1 n2 n3 n4 n5 n6 "
                       "n7 ")

ASM56K_DIRECTIVES = (3, "org equ page tabs list nolist if endif else opt title "
                        "macro endm dup dupa dupc dupf baddr bsb bsc bsm dc "
                        "dcb ds dsm dsr buffer endbuf section endsec global "
                        "local xdef xref mode ")

#---- Language Styling Specs ----#
SYNTAX_ITEMS = [ (stc.STC_ASM_DEFAULT, 'default_style'),
                 (stc.STC_ASM_CHARACTER, 'char_style'),
                 (stc.STC_ASM_COMMENT, 'comment_style'),
                 (stc.STC_ASM_COMMENTBLOCK, 'comment_style'),
                 (stc.STC_ASM_CPUINSTRUCTION, 'keyword_style'),
                 (stc.STC_ASM_DIRECTIVE, 'keyword3_style'),
                 (stc.STC_ASM_DIRECTIVEOPERAND, 'keyword4_style'),
                 (stc.STC_ASM_EXTINSTRUCTION, 'funct_style'),
                 (stc.STC_ASM_IDENTIFIER, 'default_style'),
                 (stc.STC_ASM_MATHINSTRUCTION, 'keyword_style'),
                 (stc.STC_ASM_NUMBER, 'number_style'),
                 (stc.STC_ASM_OPERATOR, 'operator_style'),
                 (stc.STC_ASM_REGISTER, 'keyword2_style'),
                 (stc.STC_ASM_STRING, 'string_style'),
                 (stc.STC_ASM_STRINGEOL, 'stringeol_style') ]

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for 68k assembly files""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        # synglob.ID_LANG_68K, synglob.ID_LANG_DSP56K
        self.SetLexer(stc.STC_LEX_ASM)

    def GetKeywords(self):
        """Returns Specified Keywords List"""
        if self.LangId == synglob.ID_LANG_68K:
            return [ASM_CPU_INST, ASM_MATH_INST, ASM_REGISTER, ASM_DIRECTIVES]
        else:
            return [ASM56K_CPU_INST, ASM56K_MATH_INST,
                    ASM56K_REGISTERS, ASM56K_DIRECTIVES]

    def GetSyntaxSpec(self):
        """Syntax Specifications"""
        return SYNTAX_ITEMS

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u';']
