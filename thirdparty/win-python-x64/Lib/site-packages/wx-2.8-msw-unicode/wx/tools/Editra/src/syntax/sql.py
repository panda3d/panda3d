###############################################################################
# Name: sql.py                                                                #
# Purpose: Define SQL syntax for highlighting and other features              #
# Author: Thomas Keul <tgkeul@web.de>                                         #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: sql.py
AUTHOR: Thomas Keul

SUMMARY:
Lexer configuration module for Oracle 10 PL/SQL (and SQL, sqlplus, pldoc).
This work is based on sql.properties of SciTE 1.74 and other documents
which are linked (these links where current in October 2007)

The keywords are divided in several sections which unfortunally overlap.
As a fulltime PL/SQL programmer I decided which section I prefer for a
keyword which belongs to multiple sections (like 'in', 'to' or 'else').
It may be a challange to create an editor which colour the multiple
used keywords depending on their context.
/*+ HINT use same color for PL/SQL and SQL */
Another thing the SQL-Lexer is not able to handle is a token consisting
of several keywords. Although zone is only part in type definitions as
"timestamp with local time zone" it will be coloured if used as identifier
too.

If you're unhappy feel free to change it or build a new SQL-Lexer :-)

The sections I choose
  - SQL keywords
  - Exception names and transaction control (to hilite in a special color)
  - SQL functions
  - sqlplus keywords
  - PL/SQL keywords
  - data types
  - standard packages
  - pseudo variables  and constants

There are more section than supported by the lexer. Standard packages and
sqlplus keywords share a slot. You may change it.

In contrast to Gnome gtk-sourceview sql.lang file I did not divide the
function section into subsections like "analytical functions".
For my purpose distinct colours or fonts for different function types don't
help me reading programs.
... and the Scintilla SQL lexer does not support it.

Some useful internals of the Lexer.
It gets an array with 8 lists of keywords. These lists are defined below.

  - WordList &keywords1  = *keywordlists[0];  : SQL_KW   (SQL Keywords)
  - WordList &keywords2  = *keywordlists[1];  : SQL_DBO  (Data Types)
  - WordList &kw_pldoc   = *keywordlists[2];  : SQL_PLD  (epydoc field tags)
  - WordList &kw_sqlplus = *keywordlists[3];  : SQL_PLUS or SQL_PKG
  - WordList &kw_user1   = *keywordlists[4];  : SQL_UKW1 (standard functions)
  - WordList &kw_user2   = *keywordlists[5];  : SQL_UKW2 (exceptions)
  - WordList &kw_user3   = *keywordlists[6];  : SQL_UKW3 (special variables)
  - WordList &kw_user4   = *keywordlists[7];  : SQL_UKW4 (PL/SQL Keywords)

As some keywords are used multiple the colour depends on the process in the
lexer. I prefer the same colour for SQL and PL/SQL Keywords.

if a token is realized as identifier, the keyword lists are scanned:
   1. keywords1  / SQL_KW
   2. keywords2  / SQL_DBO
   3. kw_sqlplus / SQL_PLUS
   4. kw_user1   / SQL_UKW1
   5. kw_user2   / SQL_UKW2
   6. kw_user3   / SQL_UKW3
   7. kw_user4   / SQL_UKW4

What ever comes first wins. For this reason PL/SQL keyword are last in the
list so no prior used keyword must be removed.
Because of the preceeding @ pldoc is processed seperate.

The Scintilla SQL Lexer 1.75 has a restriction when '#' and '$' are not
identified as valid characters of an (Oracle-)identifier.
wxPython 2.8.7.1 does not use not use lexer version 1.75 yet - so SQL_PKG
wil not work.

@summary: Lexer configuration module for Oracle 10 PL/SQL

"""

__author__ = "Thomas Keul <tgkeul@web.de>"
__svnid__ = "$Id: sql.py 57444 2008-12-20 16:22:21Z CJP $"
__revision__ = "$Revision: 57444 $"

#-----------------------------------------------------------------------------#
# Dependancies
import synglob

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# SQL Keywords          (Oracle 11g SQL reserved words)
# (http://download.oracle.com/docs/cd/B28359_01/server.111/b28286/ap_keywd.htm#i690190)
SQL_KW = (0, "access add all alter and any as asc audit between by check "
             "cluster column comment compress connect create current default "
             "delete desc distinct drop else exclusive exists file for from "
             "grant group having identified immediate in increment index "
             "initial insert intersect into is like lock maxextents minus mode "
             "modify noaudit nocompress not nowait number of offline on online "
             "option or order pctfree prior privileges public rename resource "
             "revoke row rows select session set share size start successful "
             "synonym table to trigger union unique update validate values "
             "view whenever where with primary key constraint foreign "
             "references restrict no action without schema deferrable not "
             "deferrable rule do")
# dropped reserved words from the specification above:
# char date decimal float integer long mlslabel number raw smallint varchar
#   varchar2 => SQL_DBO
# level null rowid rownum sysdate uid user => SQL_UKW4
# then => SQL_UKW3

# SQL Data Object   (ORACLE 11g predefined SQL and PL/SQL data types)
# (http://download.oracle.com/docs/cd/B28359_01/appdev.111/b31088/exprn_expconcepts.htm#EXPRN007)
SQL_DBO = (1, "anydata anydataset anytype bfile binary_float binary_integer "
              "blob boolean char character clob date day dburitype dec decimal "
              "double dsinterval_unconstrained float httpuritype int integer "
              "interval local long minute mlslabel month national natural "
              "naturaln nchar nclob number numeric nvarchar2 ordaudio orddicom "
              "orddoc ordimage ordimagesignature ordvideo pls_integer positive "
              "positiven precision raw real rowid sdo_geometry sdo_georaster "
              "sdo_topo_geometry second second si_averagecolor si_color "
              "si_colorhistogram si_featurelist si_positionalcolor "
              "si_stillimage si_texture signtype simple_double simple_float "
              "simple_integer smallint time timestamp timestamp_unconstrained "
              "timestamp_ltz_unconstrained timestamp_tz_unconstrained urowid "
              "varchar varchar2 varying xdburitype xmltype year "
              "yminterval_unconstrained zone serial")
# Note: some data types consist of more than one word like "interval year to
#       month" or "timestamp with local time zone"
#       some of these words are preferred in other sections:
#       to with => SQL_KW
#       cursor => SQL_UKW4

# SQL PLDoc Keywords    (and epydoc and ... enhance as you like it)
# (http://pldoc.sourceforge.net/docs/Users_Guide/content.html)
SQL_PLD = (2, "TODO author deprecated headcom param return see since throws "
              "type ")

# SQL Plus (I avoid sqlplus, so this section may be outdated)
SQL_PLUS = (3, "acc~ept a~ppend archive attribute bre~ak bti~tle c~hange "
               "col~umn comp~ute conn~ect copy def~ine del desc~ribe e~dit "
               "exec~ute exit get help ho~st i~nput l~ist log passw~ord pau~se "
               "pri~nt pro~mpt quit recover rem~ark repf~ooter reph~eader r~un "
               "sav~e set sho~w shutdown spo~ol sta~rt startup store timi~ng "
               "undef~ine var~iable whenever oserror whenever sqlerror cl~ear "
               "disc~onnect ti~tle ")

# SQL User Keywords 1   (Oracle 11g SQL Functions)
# (http://download.oracle.com/docs/cd/B28359_01/server.111/b28286/functions001.htm#i88893)
SQL_UKW1 = (4, "abs acos add_months appendchildxml ascii asciistr asin atan "
               "atan2 avg bfilename bin_to_num bitand cardinality cast ceil "
               "chartorowid chr cluster_id cluster_probability cluster_set "
               "coalesce compose concat convert corr cos cosh count count "
               "covar_pop covar_samp cume_dist current_date current_timestamp "
               "cv dbtimezone decode decompose deletexml dense_rank depth "
               "deref dump empty_blob empty_clob existsnode exp extract "
               "extractvalue feature_id feature_set feature_value first "
               "first_value floor from_tz greatest group_id grouping "
               "grouping_id hextoraw initcap insertchildxml insertxmlbefore "
               "instr iteration_number lag last last_day last_value lead least "
               "length ln lnnvl localtimestamp log lower lpad ltrim make_ref "
               "max median min mod months_between nanvl new_time next_day "
               "nls_charset_decl_len nls_charset_id nls_charset_name "
               "nls_initcap nls_lower nls_upper nlssort ntile nullif "
               "numtodsinterval numtoyminterval nvl nvl2 ora_hash path "
               "percent_rank percentile_cont percentile_disc power "
               "powermultiset powermultiset_by_cardinality prediction "
               "prediction_bounds prediction_cost prediction_details "
               "prediction_probability prediction_set presentnnv presentv "
               "previous rank ratio_to_report rawtohex rawtonhex ref reftohex "
               "regexp_instr regexp_replace regexp_substr remainder "
               "round row_number rowidtochar rowidtonchar rpad rtrim "
               "scn_to_timestamp sessiontimezone set sign sin sinh soundex "
               "sqrt stats_binomial_test stats_crosstab stats_f_test "
               "stats_ks_test stats_mode stats_mw_test stats_one_way_anova "
               "stats_t_test_ stats_wsr_test stddev stddev_pop stddev_samp "
               "substr sum sys_connect_by_path sys_dburigen sys_extract_utc "
               "sys_xmlagg sys_xmlgen sysdate systimestamp tan tanh "
               "timestamp_to_scn to_binary_double to_binary_float to_char "
               "to_clob to_date to_dsinterval to_lob to_multi_byte to_nchar "
               "to_nclob to_number to_single_byte to_timestamp to_timestamp_tz "
               "to_yminterval translate treat trim trunc tz_offset unistr "
               "updatexml upper value var_pop var_samp variance vsize "
               "width_bucket xmlagg xmlcast xmlcdata xmlcolattval xmlcomment "
               "xmlconcat xmldiff xmlelement xmlexists xmlforest xmlparse "
               "xmlpatch xmlpi xmlquery xmlroot xmlsequence xmlserialize "
               "xmltable xmltransform")
# replace, collect => SQL_UKW4

# SQL User Keywords 2   (Oracle 11g exceptions and transaction control)
SQL_UKW2 = (5, "access_into_null case_not_found collection_is_null "
               "cursor_already_open dup_val_on_index invalid_cursor "
               "invalid_number login_denied no_data_found not_logged_on "
               "program_error rowtype_mismatch self_is_null storage_error "
               "subscript_beyond_count subscript_outside_limit "
               "sys_invalid_rowid timeout_on_resource too_many_rows "
               "value_error zero_divide commit savepoint rollback")

# SQL User KW3  (Oracle 11g pseudo variables, methods and constants)
SQL_UKW3 = (6, "binary_double_infinity binary_double_max_normal "
               "binary_double_max_subnormal binary_double_min_normal "
               "binary_double_min_subnormal binary_double_nan "
               "binary_float_infinity1 binary_float_max_normal "
               "binary_float_max_subnormal binary_float_min_normal "
               "binary_float_min_subnormal binary_float_nan1 count currval "
               "delete exists extend false first last level limit next nextval "
               "null prior rowlabel rownum sql sqlcode sqldata sqlname "
               "sqlstate sysdate trim true uid user")

# SQL User Keywords 4   (Oracle 11g PL/SQL reserved and keywords)
# (http://download.oracle.com/docs/cd/B28359_01/appdev.111/b28370/reservewords.htm#CHDBBJFE)
SQL_UKW4 = (7, "a add agent aggregate all alter and any array arrow as asc at "
               "attribute authid avg begin between bfile_base binary blob_base "
               "block body both bound bulk by byte c call calling cascade case "
               "char_base charset charsetform charsetid check clob_base close "
               "cluster clusters colauth collect columns comment commit "
               "committed compiled compress connect constant constructor "
               "context continue convert count crash create current cursor "
               "customdatum dangling date_base declare default define delete "
               "desc deterministic distinct drop duration element else elsif "
               "empty end escape except exception exceptions exclusive execute "
               "exists exit external fetch final fixed for forall force form "
               "from function general goto grant group hash having heap hidden "
               "hour identified if immediate in including index indexes "
               "indicator indices infinite insert instantiable interface "
               "intersect interval into invalidate is isolation java anguage "
               "large leading length level library like like2 like4 likec "
               "limit limited local lock long loop map max maxlen member merge "
               "min minus minute mod mode modify month multiset name nan "
               "national native nchar new nocompress nocopy not nowait null "
               "number_base object ocicoll ocidate ocidatetime ociduration "
               "ociinterval ociloblocator ocinumber ociraw ociref ocirefcursor "
               "ocirowid ocistring ocitype of on only opaque open operator "
               "option or oracle oradata order organization orlany orlvary "
               "others out overlaps overriding package parallel_enable "
               "parameter parameters partition pascal pipe pipelined pragma "
               "precision prior private procedure public raise range raw read "
               "record ref reference relies_on rem remainder rename replace "
               "resource result result_cache return returning reverse revoke "
               "rollback row sample save savepoint sb1 sb2 sb4 second segment "
               "select self separate sequence serializable set share short "
               "size size_t some sparse sql sqlcode sqldata sqlname sqlstate "
               "standard start static stddev stored string struct style "
               "submultiset subpartition substitutable subtype sum synonym "
               "tabauth table tdo the then time timestamp timezone_abbr "
               "timezone_hour timezone_minute timezone_region to trailing "
               "transaction transactional trusted type ub1 ub2 ub4 under union "
               "unique unsigned untrusted update use using valist value values "
               "variable variance varray varying view views void when where "
               "while with work wrapped write year zone")
# char character date day decimal double float int => SQL_CBO

# SQL PLUS replacement - for people not forced using acient tools
#                        (needs Scintilla 1.75 SQL Lexer to handle)
SQL_PKG = (3, "apex_custom_auth apex_application apex_item apex_util ctx_adm "
              "ctx_cls ctx_ddl ctx_doc ctx_output ctx_query ctx_report "
              "ctx_thes ctx_ulexer dbms_addm dbms_advanced_rewrite "
              "dbms_advisor dbms_alert dbms_application_info dbms_apply_adm "
              "dbms_aq dbms_aqadm dbms_aqelm dbms_aqin dbms_assert "
              "dbms_auto_task_admin dbms_aw_stats dbms_capture_adm "
              "dbms_cdc_publish dbms_cdc_subscribe dbms_comparison "
              "dbms_connection_pool dbms_cq_notification dbms_crypto "
              "dbms_csx_admin dbms_cube dbms_cube_advise dbms_data_mining "
              "dbms_data_mining_transform dbms_datapump dbms_db_version "
              "dbms_ddl dbms_debug dbms_defer dbms_defer_query dbms_defer_sys "
              "dbms_describe dbms_dg dbms_dimension "
              "dbms_distributed_trust_admin dbms_epg dbms_errlog dbms_expfil "
              "dbms_fga dbms_file_group dbms_file_transfer dbms_flashback "
              "dbms_frequent_itemset dbms_hm dbms_hprof dbms_hs_parallel "
              "dbms_hs_passthrough dbms_iot dbms_java dbms_job dbms_ldap "
              "dbms_ldap_utl dbms_libcache dbms_lob dbms_lock dbms_logmnr "
              "dbms_logmnr_d dbms_logstdby dbms_metadata dbms_mgd_id_utl "
              "dbms_mgwadm dbms_mgwmsg dbms_monitor dbms_network_acl_admin "
              "dbms_network_acl_utility dbms_obfuscation_toolkit dbms_odci "
              "dbms_offline_og dbms_outln dbms_outln_edit dbms_output "
              "dbms_pclxutil dbms_pipe dbms_predictive_analytics "
              "dbms_preprocessor dbms_profiler dbms_propagation_adm "
              "dbms_random dbms_rectifier_diff dbms_redefinition "
              "dbms_refresh dbms_repair dbms_repcat dbms_repcat_admin "
              "dbms_repcat_instantiate dbms_repcat_rgt dbms_reputil "
              "dbms_resconfig dbms_resource_manager "
              "dbms_resource_manager_privs dbms_result_cache dbms_resumable "
              "dbms_rlmgr dbms_rls dbms_rowid dbms_rule dbms_rule_adm "
              "dbms_scheduler dbms_server_alert dbms_service dbms_session "
              "dbms_shared_pool dbms_space dbms_space_admin dbms_spm dbms_sql "
              "dbms_sqldiag dbms_sqlpa dbms_sqltune dbms_stat_funcs "
              "dbms_stats dbms_storage_map dbms_streams dbms_streams_adm "
              "dbms_streams_advisor_adm dbms_streams_auth "
              "dbms_streams_messaging dbms_streams_tablespace_adm "
              "dbms_tdb dbms_trace dbms_transaction dbms_transform dbms_tts "
              "dbms_types dbms_utility dbms_warning dbms_wm "
              "dbms_workload_capture dbms_workload_replay "
              "dbms_workload_repository dbms_xa dbms_xdb dbms_xdb_admin "
              "dbms_xdbresource dbms_xdb_version dbms_xdbt dbms_xdbz "
              "dbms_xevent dbms_xmldom dbms_xmlgen dbms_xmlindex "
              "dbms_xmlparser dbms_xmlquery dbms_xmlsave dbms_xmlschema "
              "dbms_xmlstore dbms_xmltranslations dbms_xplan "
              "dbms_xslprocessor debug_extproc htf htp ord_dicom "
              "ord_dicom_admin owa_cache owa_cookie owa_custom owa_image "
              "owa_opt_lock owa_pattern owa_sec owa_text owa_util sdo_cs "
              "sdo_csw_process sdo_gcdr sdo_geom sdo_geor sdo_geor_admin "
              "sdo_geor_utl sdo_lrs sdo_migrate sdo_net sdo_net_mem sdo_ols "
              "sdo_pc_pkg sdo_sam sdo_tin_pkg sdo_topo sdo_topo_map sdo_tune "
              "sdo_util sdo_wfs_lock sdo_wfs_process sem_apis sem_perf "
              "utl_coll utl_compress utl_dbws utl_encode utl_file utl_http "
              "utl_i18n utl_inaddr utl_lms utl_mail utl_nla utl_raw " 
              "utl_recomp utl_ref utl_smtp utl_spadv utl_tcp utl_url " 
              "wpg_docload ")

#---- Syntax Style Specs ----#
# Scintilla 1.75 SQL Lexer
SYNTAX_ITEMS = [ ('STC_SQL_DEFAULT',                'default_style'),       #  0
                 ('STC_SQL_CHARACTER',              'char_style'),          #  7
                 ('STC_SQL_COMMENT',                'comment_style'),       #  1
                 ('STC_SQL_COMMENTDOC',             'comment_style'),       #  3
                 ('STC_SQL_COMMENTDOCKEYWORD',      'dockey_style'),        # 17
                 ('STC_SQL_COMMENTDOCKEYWORDERROR', 'error_style'),         # 18
                 ('STC_SQL_COMMENTLINE',            'comment_style'),       #  2
                 ('STC_SQL_COMMENTLINEDOC',         'comment_style'),       # 15
                 ('STC_SQL_IDENTIFIER',             'default_style'),       # 11
                 ('STC_SQL_NUMBER',                 'number_style'),        #  4
                 ('STC_SQL_OPERATOR',               'operator_style'),      # 10
                 ('STC_SQL_QUOTEDIDENTIFIER',       'default_style'),       # 23
                 ('STC_SQL_SQLPLUS',                'scalar_style'),        #  8
                 #('STC_SQL_SQLPLUS',                'funct_style'),        #  8
                 ('STC_SQL_SQLPLUS_COMMENT',        'comment_style'),       # 13
                 ('STC_SQL_SQLPLUS_PROMPT',         'default_style'),       #  9
                 ('STC_SQL_STRING',                 'string_style'),        #  6
                 ('STC_SQL_USER1',                  'funct_style'),         # 19
                 ('STC_SQL_USER2',                  'directive_style'),     # 20
                 ('STC_SQL_USER3',                  'keyword3_style'),      # 21
                 ('STC_SQL_USER4',                  'keyword_style'),       # 22
                 ('STC_SQL_WORD',                   'keyword_style'),       #  5
                 ('STC_SQL_WORD2',                  'keyword2_style') ]     # 16

#---- Extra Properties ----#
# found in LexSQL.cxx of Scintilla 1.74
# Folding is rather poor for PL/SQL
FOLD                    = ("fold", "1")
FOLD_COMMENT            = ("fold.comment", "1")
FOLD_COMPACT            = ("fold.compact", "0")
FOLD_SQL_ONLY_BEGIN     = ("fold.sql.only.begin", "0")

# Properties which do not match PL/SQL
SQL_BACKSLASH_ESCAPES    = ("sql.backslash.escapes", "0")
SQL_BACKTICKS_IDENTIFIER = ("lexer.sql.backticks.identifier", "0")

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """ Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords
                    (ID_LANG_SQL and ID_LANG_PLSQL supported)

    """
    if lang_id in [ synglob.ID_LANG_SQL, synglob.ID_LANG_PLSQL ]:
        common = [ SQL_KW, SQL_DBO, SQL_PLD, SQL_UKW1, SQL_UKW2, SQL_UKW4 ]
        if lang_id == synglob.ID_LANG_SQL:
            common.append(SQL_PLUS)
        else:
            common.extend([SQL_PKG, SQL_UKW3])
        return common
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """ Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs
                                   (ID_LANG_SQL and ID_LANG_PLSQL supported)

    """
    if lang_id in [ synglob.ID_LANG_SQL, synglob.ID_LANG_PLSQL ]:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """ Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties
                   (ID_LANG_SQL and ID_LANG_PLSQL supported)

    """
    if lang_id == synglob.ID_LANG_SQL:
        return [FOLD]
    elif lang_id == synglob.ID_LANG_PLSQL:
        return [FOLD, FOLD_COMMENT, FOLD_COMPACT, FOLD_SQL_ONLY_BEGIN,
                SQL_BACKSLASH_ESCAPES, SQL_BACKTICKS_IDENTIFIER]
    else:
        return list()

def CommentPattern(lang_id=0):
    """ Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)
                                   (ID_LANG_SQL and ID_LANG_PLSQL supported)

    """
    if lang_id in [synglob.ID_LANG_SQL, synglob.ID_LANG_PLSQL]:
        return [u'--']
    else:
        return list()
#---- End Required Module Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """ Returns the specified Keyword String
    @note: not used by most modules

    """
    return None
