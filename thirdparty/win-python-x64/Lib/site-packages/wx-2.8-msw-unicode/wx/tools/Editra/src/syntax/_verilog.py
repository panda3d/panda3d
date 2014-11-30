###############################################################################
# Name: verilog.py                                                            #
# Purpose: Configuration module for Verilog HDL language                      #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: verilog.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Verilog Hardware Description Language
          and System Verilog programming languages. Much help in creating this
          module from Tim Corcoran.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: _verilog.py 63834 2010-04-03 06:04:33Z CJP $"
__revision__ = "$Revision: 63834 $"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Local Imports
import synglob
import syndata

#-----------------------------------------------------------------------------#
#TODO: What to do with preprocessors?
#TODO: What to do with standard methods?

#---- Keyword Definitions ----#

#==============================================================================
# IEEE 1364-1995 Verilog
#==============================================================================
# IEEE 1364-1995 Verilog Preprocessors
V_1364_1995_PREPROCESSORS = (
    # 1364-1995 Section 14
    "`celldefine `default_nettype `define `else `endcelldefine `endif `ifdef "
    "`include `nounconnected_drive `resetall `timescale `unconnected_drive "
    "`undef "
    # 1364-1995 Annex G
    "`default_decay_time `default_trireg_strength `delay_mode_distributed "
    "`delay_mode_path `delay_mode_unit `delay_mode_zero "
    #1364-1995 Verilog Preprocessor Commercial Extensions
    "`accelerate `autoexpand_vectornets `disable_portfaults `enable_portfaults "
    "`endprotect `endprotected `expand_vectornets `noaccelerate "
    "`noexpand_vectornets `noremove_gatenames `noremove_netnames "
    "`nosuppress_faults `portcoerce `protect `protected "
    "`remove_gatenames `remove_netnames `suppress_faults "
)

# IEEE 1364-1995 Verilog Keywords (NOT USED: attribute endattribute signed unsigned)
V_1364_1995_KEYWORDS = (
    "always assign begin case casex casez deassign default defparam disable "
    "edge else end endcase endfunction endmodule endprimitive endspecify "
    "endtable endtask for force forever fork function if ifnone initial "
    "inout input join macromodule module negedge output parameter posedge "
    "primitive release repeat scalared specify specparam strength table task "
    "vectored wait while"
)

# IEEE 1364-1995 Verilog Types (NOT USED: xbuf)
V_1364_1995_TYPES = (
    "and buf bufif0 bufif1 cmos event highz0 highz1 integer large medium nand "
    "nmos nor not notif0 notif1 or pmos pull0 pull1 pulldown pullup rcmos real "
    "realtime reg rnmos rpmos rtran rtranif0 rtranif1 small strong0 strong1 "
    "supply0 supply1 time tran tranif0 tranif1 tri tri0 tri1 triand trior "
    "trireg wand weak0 weak1 wire wor xnor xor"
)

# IEEE 1364-1995 Verilog System Tasks and Functions
V_1364_1995_TASKS = (
    # 1364-1995 Section 14
    "$async$and$array $async$and$plane $async$nand$array $async$nand$plane "
    "$async$nor$array $async$nor$plane $async$or$array $async$or$plane "
    "$bitstoreal $display $displayb $displayh $displayo $dist_chi_square "
    "$dist_erlang $dist_exponential $dist_normal $dist_poisson $dist_t "
    "$dist_uniform $dumpall $dumpfile $dumpflush $dumplimit $dumpoff $dumpon "
    "$dumpvars $fclose $fdisplay $fdisplayb $fdisplayh $fdisplayo $finish "
    "$fmonitor $fmonitorb $fmonitorh $fmonitoro $fopen $fstrobe $fstrobeb "
    "$fstrobeh $fstrobeo $fwrite $fwriteb $fwriteh $fwriteo $hold $itor "
    "$monitor $monitorb $monitorh $monitoro $monitoroff $monitoron $nochange "
    "$period $printtimescale $q_add $q_exam $q_full $q_initialize $q_remove "
    "$random $readmemb $readmemh $realtime $realtobits $recovery $rtoi $setup "
    "$setuphold $skew $stime $stop $strobe $strobeb $strobeh $strobeo "
    "$sync$and$array $sync$and$plane $sync$nand$array $sync$nand$plane "
    "$sync$nor$array $sync$nor$plane $sync$or$array $sync$or$plane $time "
    "$timeformat $width $write $writeb $writeh $writeo "
    # 1364-1995 Annex F
    "$countdrivers $getpattern $incsave $input $key $list $log $nokey $nolog "
    "$reset $reset_count $reset_value $restart $save $scale $scope $showscopes "
    "$showvars $sreadmemb $sreadmemh"
)

#==============================================================================
# IEEE 1364-2001 Verilog
#==============================================================================
# IEEE 1364-2001 Verilog Preprocessors
V_1364_2001_PREPROCESSORS = (
    "`elsif `ifndef `line"
)

# IEEE 1364-2001 Verilog Keywords
V_1364_2001_KEYWORDS = (
    "automatic cell config design endconfig endgenerate generate genvar incdir "
    "include instance liblist library localparam noshowcancelled "
    "pulsestyle_ondetect pulsestyle_onevent showcancelled signed unsigned use"
)

# IEEE 1364-2001 Verilog Types
V_1364_2001_TYPES = (
    ""
)

# IEEE 1364-2001 Verilog System Tasks and Functions
V_1364_2001_TASKS = (
    "$dumpports $dumpportsall $dumpportsflush $dumpportslimit $dumpportsoff "
    "$dumpportson $ferror $fflush $fgetc $fgets $fread $fscanf $fseek $ftell "
    "$rewind $sdf_annotate $sformat $signed $sscanf $swrite $swriteb $swriteh "
    "$swriteo $test$plusargs $ungetc $unsigned $value$plusargs"
)

#==============================================================================
# IEEE 1364-2005 Verilog
#==============================================================================
# IEEE 1364-2005 Verilog Preprocessors
V_1364_2005_PREPROCESSORS = (
    "`pragma"
)

# IEEE 1364-2005 Verilog Keywords (NONE)
V_1364_2005_KEYWORDS = (
    ""
)

# IEEE 1364-2005 Verilog Types
V_1364_2005_TYPES = (
    "uwire"
)

# IEEE 1364-2005 Verilog System Tasks and Functions (NONE)
V_1364_2005_TASKS = (
    ""
)

#VERILOG_PREPROCESSORS = " ".join([V_1364_1995_PREPROCESSORS, V_1364_2001_PREPROCESSORS, V_1364_2005_PREPROCESSORS])
#VERILOG_KEYWORDS      = " ".join([V_1364_1995_KEYWORDS,      V_1364_2001_KEYWORDS,      V_1364_2005_KEYWORDS     ])
#VERILOG_TYPES         = " ".join([V_1364_1995_TYPES,         V_1364_2001_TYPES,         V_1364_2005_TYPES        ])
#VERILOG_TASKS         = " ".join([V_1364_1995_TASKS,         V_1364_2001_TASKS,         V_1364_2005_TASKS        ])

# ---\/----- Overkill maybe? -----\/---
VERILOG_PREPROCESSORS = " ".join( [V_1364_1995_PREPROCESSORS,
                                   V_1364_2001_PREPROCESSORS,
                                   V_1364_2005_PREPROCESSORS] ).split()
VERILOG_PREPROCESSORS.sort()
VERILOG_PREPROCESSORS = " ".join( VERILOG_PREPROCESSORS )

VERILOG_KEYWORDS = " ".join( [V_1364_1995_KEYWORDS,
                              V_1364_2001_KEYWORDS,
                              V_1364_2005_KEYWORDS] ).split()
VERILOG_KEYWORDS.sort()
VERILOG_KEYWORDS = " ".join( VERILOG_KEYWORDS )

VERILOG_TYPES = " ".join( [V_1364_1995_TYPES,
                           V_1364_2001_TYPES, 
                           V_1364_2005_TYPES] ).split()
VERILOG_TYPES.sort()
VERILOG_TYPES = " ".join( VERILOG_TYPES )

VERILOG_TASKS = " ".join( [V_1364_1995_TASKS, 
                           V_1364_2001_TASKS, 
                           V_1364_2005_TASKS] ).split()
VERILOG_TASKS.sort()
VERILOG_TASKS = " ".join( VERILOG_TASKS )
# ---/\----- Overkill maybe? -----/\---

#---- System Verilog Extensions ----#

#==============================================================================
# IEEE 1800-2005 SystemVerilog
#==============================================================================
# IEEE 1800-2005 SystemVerilog Preprocessors
SV_1800_2005_PREPROCESSORS = (
    # One of "1800-2009" "1800-2005" "1364-2005" "1364-2001" "1364-2001-noconfig" "1364-1995"
    "`begin_keywords `end_keywords"
)

# IEEE 1800-2005 SystemVerilog Keywords
SV_1800_2005_KEYWORDS = (
    "alias always_comb always_ff always_latch assert assume before bind "
    "bins binsof bit break constraint covergroup coverpoint class clocking const"
    "context continue cover cross dist do endclass endclocking endgroup endinterface "
    "endpackage endprogram endproperty endsequence expect export extends extern "
    "final first_match foreach forkjoin iff ignore_bins illegal_bins import "
    "inside interface intersect join_any join_none modport new null package "
    "priority process program property pure randcase randsequence ref return "
    "sequence solve super this throughout timeprecision timeunit type unique "
    "wait_order wildcard with within "
    "local packed protected static struct tagged typedef union virtual"
)

# IEEE 1800-2005 SystemVerilog Types
SV_1800_2005_TYPES = (
    "byte chandle enum int logic longint rand randc shortint shortreal std "
    "string var void"
    # Moved to keywords
#   "local packed protected static struct tagged typedef union virtual"
)

# IEEE 1800-2005 SystemVerilog System Tasks and Functions
SV_1800_2005_TASKS = (
    "$assertkill $assertoff $asserton $bits $countones $coverage_control "
    "$coverage_get $coverage_get_max $coverage_merge $coverage_save "
    "$dimensions $error $exit $fatal $fell $high $increment $info $isunbounded "
    "$isunknown $left $low $onehot $onehot0 $psprintf $right $rose $sampled "
    "$size $stable $typename $unpacked_dimensions $warning"
)

# IEEE 1800-2005 SystemVerilog Standard Methods
SV_1800_2005_METHODS = (
    "and atobin atohex atoi atooct atoreal await back bintoa clear compare data "
    "delete empty eq erase erase_range exists find find_first find_first_index "
    "find_index find_last find_last_index finish first front get getc hextoa "
    "icompare index insert insert_range itoa kill last len max min name neq new "
    "next num octtoa or peek pop_back pop_front prev product purge push_back "
    "push_front put putc rand_mode realtoa resume reverse rsort self set shuffle "
    "size sort start status stop substr sum suspend swap tolower toupper try_get "
    "try_peek try_put unique unique_index xor"
)

#==============================================================================
# IEEE 1800-2009 SystemVerilog
#==============================================================================
SV_1800_2009_PREPROCESSORS = (
    "`__FILE__ `__LINE__ `undefineall"
)

# IEEE 1800-2009 SystemVerilog Keywords
SV_1800_2009_KEYWORDS = (
    "accept_on checker endchecker eventually global implies let matches nexttime "
    "reject_on restrict s_always s_eventually s_nexttime s_until s_until_with "
    "sync_accept_on sync_reject_on unique0 until until_with untyped weak "
    # 34.4 Protect pragma directives
    "protect "
    "author author_info begin_protected comment data_block data_decrypt_key "
    "data_keyname data_keyowner data_method data_public_key decrypt_license "
    "digest_block digest_decrypt_key digest_key_method digest_keyname "
    "digest_keyowner digest_method digest_public_key encoding encrypt_agent "
    "encrypt_agent_info encrypt_license end_protected key_block key_keyname "
    "key_keyowner key_method key_public_key reset runtime_license viewport"
)

# IEEE 1800-2009 SystemVerilog Types
SV_1800_2009_TYPES = (
    # TODO: Are there new types?
    ""
)

# IEEE 1800-2009 SystemVerilog System Tasks and Functions
SV_1800_2009_TASKS = (
    # Section 20.1 General
    "$acos $acosh $asin $asinh $assertfailoff $assertfailon $assertnonvacuouson "
    "$assertpassoff $assertpasson $assertvacuousoff $atan $atan2 $atanh "
    "$bitstoshortreal $cast $ceil $changed $changed_gclk $changing_gclk $clog2 "
    "$cos $cosh $exp $falling_gclk $fell_gclk $floor $future_gclk $get_coverage "
    "$hypot $ln $load_coverage_db $log10 $past $past_gclk $pow $rising_gclk "
    "$rose_gclk $set_coverage_db_name $sformatf $shortrealtobits $sin $sinh "
    "$sqrt $stable_gclk $steady_gclk $system $tan $tanh"
)

# IEEE 1800-2009 SystemVerilog Standard Methods
SV_1800_2009_METHODS = (
    # TODO: Add new methods
    ""
)

#SYSTEMVERILOG_PREPROCESSORS = " ".join( [VERILOG_PREPROCESSORS, SV_1800_2005_PREPROCESSORS, SV_1800_2009_PREPROCESSORS] )
#SYSTEMVERILOG_KEYWORDS      = " ".join( [VERILOG_KEYWORDS,      SV_1800_2005_KEYWORDS,      SV_1800_2009_KEYWORDS     ] )
#SYSTEMVERILOG_TYPES         = " ".join( [VERILOG_TYPES,         SV_1800_2005_TYPES,         SV_1800_2009_TYPES        ] )
#SYSTEMVERILOG_TASKS         = " ".join( [VERILOG_TASKS,         SV_1800_2005_TASKS,         SV_1800_2009_TASKS        ] )

# ---\/----- Overkill maybe? -----\/---
SYSTEMVERILOG_PREPROCESSORS = " ".join( [VERILOG_PREPROCESSORS, 
                                         SV_1800_2005_PREPROCESSORS, 
                                         SV_1800_2009_PREPROCESSORS] ).split()
SYSTEMVERILOG_PREPROCESSORS.sort()
SYSTEMVERILOG_PREPROCESSORS = " ".join( SYSTEMVERILOG_PREPROCESSORS )

SYSTEMVERILOG_KEYWORDS = " ".join( [VERILOG_KEYWORDS, 
                                    SV_1800_2005_KEYWORDS, 
                                    SV_1800_2009_KEYWORDS] ).split()
SYSTEMVERILOG_KEYWORDS.sort()
SYSTEMVERILOG_KEYWORDS = " ".join( SYSTEMVERILOG_KEYWORDS )

SYSTEMVERILOG_TYPES = " ".join( [VERILOG_TYPES, 
                                 SV_1800_2005_TYPES, 
                                 SV_1800_2009_TYPES] ).split()
SYSTEMVERILOG_TYPES.sort()
SYSTEMVERILOG_TYPES = " ".join( SYSTEMVERILOG_TYPES )

SYSTEMVERILOG_TASKS = " ".join( [VERILOG_TASKS, 
                                 SV_1800_2005_TASKS, 
                                 SV_1800_2009_TASKS] ).split()
SYSTEMVERILOG_TASKS.sort()
SYSTEMVERILOG_TASKS = " ".join( SYSTEMVERILOG_TASKS )
# ---/\----- Overkill maybe? -----/\---

#---- End System Verilog Extensions ----#

USER_KW = ( 3, "" )

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [
    (stc.STC_V_COMMENT,         'comment_style'  ),
    (stc.STC_V_COMMENTLINE,     'comment_style'  ),
    (stc.STC_V_COMMENTLINEBANG, 'comment_style'  ),
    (stc.STC_V_DEFAULT,         'default_style'  ),
    (stc.STC_V_IDENTIFIER,      'default_style'  ),
    (stc.STC_V_NUMBER,          'number_style'   ),
    (stc.STC_V_OPERATOR,        'operator_style' ),
    (stc.STC_V_PREPROCESSOR,    'pre_style'      ),
    (stc.STC_V_STRING,          'string_style'   ),
    (stc.STC_V_STRINGEOL,       'stringeol_style'),
    (stc.STC_V_USER,            'default_style'  ),
    (stc.STC_V_WORD,            'keyword_style'  ),
    (stc.STC_V_WORD2,           'keyword2_style' ),
    (stc.STC_V_WORD3,           'scalar_style'   )
]

#---- Extra Properties ----#
FOLD      = ("fold",               "1")
FOLD_CMT  = ("fold.comment",       "1")
FOLD_PRE  = ("fold.preprocessor",  "1")
FOLD_COMP = ("fold.compact",       "1")
FOLD_ELSE = ("fold.at.else",       "0")
FOLD_MOD  = ("fold.verilog.flags", "0")

#-----------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for Verilog and SysVerilog"""
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_VERILOG)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        if self.LangId == synglob.ID_LANG_VERILOG:
            return [(0, VERILOG_KEYWORDS),
                    (1, VERILOG_TYPES),
                    (2, VERILOG_TASKS)]
        else:
            return [(0, SYSTEMVERILOG_KEYWORDS),
                    (1, SYSTEMVERILOG_TYPES),
                    (2, SYSTEMVERILOG_TASKS)]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD]

    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'//']
