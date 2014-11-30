###############################################################################
# Name: php.py                                                                #
# Purpose: Define PHP syntax for highlighting and other features              #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: php.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for PHP.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: php.py 59106 2009-02-23 15:35:30Z CJP $"
__revision__ = "$Revision: 59106 $"

#-----------------------------------------------------------------------------#
# Local Imports
import synglob
import html
from cpp import AutoIndenter

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# PHP Keywords
PHP_KEYWORDS = ("__LINE__ __FILE__ __FUNCTION__ __CLASS__ __METHOD__ declare "
                "else enddeclare endswitch elseif endif if switch as do endfor "
                "endforeach endwhile for foreach while case default switch "
                "return break continue exit var bool boolean int integer real "
                "double float string array NULL extends global static include "
                "require include_once require_once new true false function "
                "class object self final public private protected try catch "
                "throw print abstract parent interface implements ")

# PHP Standard Functions/Methods
# (roughly based off of PHP Pocket Reference by O'Reilly)
PHP_FUNC = ("__construct __autoload __destruct __get __set __isset __unset "
            "__call __sleep __wakeup __toString __set_state __clone "
            "apache_child_terminate apache_lookup_uri apache_note "
            "apache_request_headers apache_response_headers apache_setenv "
            "ascii2ebcdic ebcdic2ascii getallheaders virtual jewishtojd "
            "array_change_key_case array_chunk array_count_values "
            "array_diff_assoc array_diff array_fill array_filter array_flip "
            "array_intersect_assoc array_intersect array_key_exists array_keys "
            "array_map array_merge_recursive array_merge array_multisort "
            "array_pad array_pop array_push array_rand array_reduce array "
            "array_reverse array_search array_shift array_slice array_splice "
            "array_sum array_unique array_unshift array_values array_walk "
            "arsort asort compact count current each end extract in_array key "
            "krsort ksort list natcasesort natsort next pos prev range reset "
            "rsort shuffle sizeof sort uasort uksort usort aspell_check "
            "aspell_new aspell_suggest bcadd bccomp bcdiv bcmod bcmul bcpow "
            "bcpowmod bcscale bcsqrt bcsub bzclose bzcompress bzdecompress "
            "bzerrno bzerror bzerrstr bzflush bzopen bzread bzwrite "
            "cal_days_in_month cal_from_jd cal_info cal_to_jd easter_date "
            "easter_days frenchtojd gregoriantojd jddayofweek jdmonthname "
            "jdtofrench jdtogregorian jdtojewish jdtojulian jdtounix "
            "juliantojd unixtojd ccvs_add ccvs_auth ccvs_command ccvs_count "
            "ccvs_delete ccvs_done ccvs_init ccvs_lookup ccvs_new ccvs_report "
            "ccvs_return ccvs_reverse ccvs_sale ccvs_status ccvs_textvalue "
            "ccvs_void call_user_method_array call_user_method class_exists "
            "get_class_methods get_class_vars get_class get_declared_classes "
            "get_object_vars get_parent_class is_a is_subclass_of com_load "
            "com_addref com_get com_invoke com_isenum com_load_typelib "
            "com_propget com_propput com_propset com_release com_set "
            "cpdf_add_annotation cpdf_add_outline cpdf_arc cpdf_begin_text "
            "cpdf_circle cpdf_clip cpdf_close cpdf_closepath_fill_stroke "
            "cpdf_closepath_stroke cpdf_closepath cpdf_continue_text "
            "cpdf_end_text cpdf_fill_stroke cpdf_fill cpdf_finalize_page "
            "cpdf_finalize cpdf_global_set_document_limits cpdf_import_jpeg "
            "cpdf_lineto cpdf_moveto cpdf_newpath cpdf_open cpdf_output_buffer "
            "cpdf_page_init cpdf_place_inline_image cpdf_rect cpdf_restore "
            "cpdf_rlineto cpdf_rmoveto cpdf_rotate_text cpdf_rotate "
            "cpdf_save_to_file cpdf_save cpdf_scale cpdf_set_action_url "
            "cpdf_set_char_spacing cpdf_set_creator cpdf_set_current_page "
            "cpdf_set_font_directories cpdf_set_font_map_file cpdf_set_font "
            "cpdf_set_horiz_scaling cpdf_set_keywords cpdf_set_leading "
            "cpdf_set_page_animation cpdf_set_subject cpdf_set_text_matrix "
            "cpdf_set_text_pos cpdf_set_text_rendering cpdf_set_text_rise "
            "cpdf_set_title cpdf_set_viewer_preferences cpdf_set_word_spacing "
            "cpdf_setdash cpdf_setflat cpdf_setgray_fill cpdf_setgray_stroke "
            "cpdf_setgray cpdf_setlinecap cpdf_setlinejoin cpdf_setlinewidth "
            "cpdf_setmiterlimit cpdf_setrgbcolor_fill cpdf_setrgbcolor_stroke "
            "cpdf_setrgbcolor cpdf_show_xy cpdf_show cpdf_stringwidth "
            "cpdf_text cpdf_translate crack_check crack_closedict cpdf_curveto "
            "crack_getlastmessage crack_opendict ctype_alnum ctype_alpha "
            "ctype_cntrl ctype_digit ctype_graph ctype_lower ctype_print "
            "ctype_punct ctype_space ctype_upper ctype_xdigit curl_close "
            "curl_errno curl_error curl_exec curl_getinfo curl_init chgrp "
            "curl_version checkdate date getdate gettimeofday gmdate gmmktime "
            "gmstrftime localtime microtime mktime strftime strtotime time "
            "dba_close dba_delete dba_exists dba_fetch dba_firstkey filetype "
            "dba_insert dba_list dba_nextkey dba_open dba_optimize dba_popen "
            "dba_replace dba_sync dbase_add_record dbase_close dbase_create "
            "dbase_delete_record dbase_get_record_with_names dbase_get_record "
            "dbase_numfields dbase_numrecords dbase_open dbase_pack filectime "
            "dbase_replace_record dblist dbmclose dbmdelete dbmexists dbmfetch "
            "dbmfirstkey dbminsert dbmnextkey dbmopen dbmreplace basename "
            "chmod chown clearstatcache copy delete dirname disk_free_space "
            "disk_total_space diskfreespace fclose feof fflush fgetc fgetcsv "
            "fgets fgetss file_exists file_get_contents file fileatime ftell "
            "filegroup fileinode filemtime fileowner fileperms filesize popen "
            "flock fnmatch fopen fpassthru fputs fread fscanf fseek fstat stat "
            "ftruncate fwrite glob is_dir is_executable is_file is_link "
            "is_readable is_uploaded_file is_writable is_writeable link "
            "lstat mkdir move_uploaded_file parse_ini_file pathinfo pclose "
            "readfile readlink realpath rename rewind rmdir set_file_buffer "
            "symlink tempnam tmpfile touch umask unlink ftp_cdup ftp_chdir "
            "ftp_close ftp_connect ftp_delete ftp_exec ftp_fget ftp_fput "
            "ftp_get_option ftp_get ftp_login ftp_mdtm ftp_mkdir textdomain "
            "ftp_nb_fget ftp_nb_fput ftp_nb_get ftp_nb_put ftp_nlist ftp_pasv "
            "ftp_put ftp_pwd ftp_quit ftp_rawlist ftp_rename ftp_rmdir checkin "
            "ftp_set_option ftp_site ftp_size ftp_ssl_connect ftp_systype "
            "call_user_func_array call_user_func create_function func_get_arg "
            "func_get_args func_num_args function_exists get_defined_functions "
            "register_shutdown_function register_tick_function method_exists "
            "unregister_tick_function bind_textdomain_codeset bindtextdomain "
            "dcgettext dcngettext dgettext dngettext gettext ngettext "
            "gmp_abs gmp_add gmp_and gmp_clrbit gmp_cmp gmp_com gmp_div_q "
            "gmp_div_qr gmp_div_r gmp_div gmp_divexact gmp_fact gmp_gcd "
            "gmp_hamdist gmp_init gmp_intval gmp_invert gmp_jacobi gmp_gcdext "
            "gmp_mod gmp_mul gmp_neg gmp_or gmp_perfect_square gmp_popcount "
            "gmp_pow gmp_powm gmp_prob_prime gmp_random gmp_scan0 gmp_scan1 "
            "gmp_setbit gmp_sign gmp_sqrt gmp_sqrtrm gmp_strval gmp_sub "
            "header headers_sent setcookie hw_api_attribute hwapi_hgcsp "
            "hw_api_content hw_api_object key langdepvalue value values insert "
            "checkout children mimetype read content copy dbstat dcstat "
            "dstofsrcanchors count reason find ftstat hwstat identify info "
            "insertanchor insertcollection insertdocument link lock move "
            "attreditable count insert remove title value object dstanchors "
            "parents description type remove replace setcommitedversion assign "
            "srcanchors srcsofdst unlock user userlist iconv_get_encoding "
            "iconv_set_encoding iconv ob_iconv_handler exif_imagetype gmp_xor "
            "exif_read_data exif_thumbnail gd_info getimagesize cpdf_stroke "
            "image_type_to_mime_type image2wbmp imagealphablending imagearc "
            "imagechar imagecharup imagecolorallocate imagecolorallocatealpha "
            "imagecolorat imagecolorclosest imagecolorclosestalpha curl_setopt "
            "imagecolorclosesthwb imagecolordeallocate imagecolorexact "
            "imagecolorexactalpha imagecolorresolve imagecolorresolvealpha "
            "imagecolorset imagecolorsforindex imagecolorstotal dba_handlers "
            "imagecolortransparent imagecopy imagecopymerge imagecopymergegray "
            "imagecopyresampled imagecopyresized imagecreate objectbyanchor "
            "imagecreatefromgd2part imagecreatefromgd imagecreatefromgif "
            "imagecreatefromjpeg imagecreatefrompng imagecreatefromstring "
            "imagecreatefromwbmp imagecreatefromxbm imagecreatefromxpm "
            "imagecreatetruecolor imagedashedline imagedestroy imageellipse "
            "imagefill imagefilledarc imagefilledellipse imagefilledpolygon "
            "imagefilledrectangle imagefilltoborder imagefontheight "
            "imageftbbox imagefttext imagegammacorrect imagegd2 imagegd "
            "imageinterlace imagejpeg imageline imageloadfont imagepalettecopy "
            "imagepng imagepolygon imagepsbbox imagepscopyfont imagefontwidth "
            "imagepsextendfont imagepsfreefont imagepsloadfont gmp_legendre "
            "imagepstext imagerectangle imagerotate imagesetbrush imagegif "
            "imagesetstyle imagesetthickness imagesettile imagestring "
            "imagestringup imagesx imagesy imagetruecolortopalette "
            "imagettftext imagetypes imagewbmp iptcembed iptcparse jpeg2wbmp "
            "png2wbmp read_exif_data imap_8bit imap_alerts imap_append "
            "imap_binary imap_body imap_bodystruct imap_check imap_base64 "
            "imap_close imap_createmailbox imap_delete imap_deletemailbox "
            "imap_errors imap_expunge imap_fetch_overview imap_fetchbody "
            "imap_fetchheader imap_fetchstructure imap_get_quota imagettfbbox "
            "imap_getmailboxes imap_getsubscribed imap_header imap_headerinfo "
            "imap_headers imap_last_error imap_list imap_listmailbox "
            "imap_listsubscribed imap_lsub imap_mail_compose imap_mail_copy "
            "imap_mail_move imap_mail imap_mailboxmsginfo imap_listscan "
            "imap_msgno imap_num_msg imap_num_recent imap_open imap_ping "
            "imap_renamemailbox imap_reopen imap_rfc822_parse_adrlist linkinfo "
            "imap_rfc822_parse_headers imap_rfc822_write_address imap_qprint "
            "imap_search imap_set_quota imap_setacl imap_setflag_full "
            "imap_status imap_subscribe imap_thread imap_uid imap_undelete "
            "imap_unsubscribe imap_utf7_decode imap_utf7_encode imap_utf8 "
            "assert_options assert dl extension_loaded get_cfg_var imap_sort "
            "get_defined_constants get_extension_funcs get_included_files "
            "get_loaded_extensions get_magic_quotes_gpc get_current_user "
            "get_required_files getenv getlastmod getmygid getmyinode getmypid "
            "getmyuid getopt getrusage ini_alter ini_get_all ini_get "
            "ini_set php_ini_scanned_files php_logo_guid php_sapi_name "
            "phpcredits phpinfo phpversion putenv set_magic_quotes_runtime "
            "set_time_limit version_compare zend_logo_guid zend_version "
            "ldap_8859_to_t61 ldap_add ldap_bind ldap_close ldap_compare "
            "ldap_connect ldap_count_entries ldap_delete ldap_dn2ufn php_uname "
            "ldap_errno ldap_error ldap_explode_dn ldap_first_attribute "
            "ldap_first_entry ldap_first_reference ldap_free_result "
            "ldap_get_attributes ldap_get_dn ldap_get_entries ldap_get_option "
            "ldap_get_values_len ldap_get_values ldap_list ldap_mod_add "
            "ldap_mod_del ldap_mod_replace ldap_modify ldap_next_attribute "
            "ldap_next_entry ldap_next_reference ldap_parse_reference hypot "
            "ldap_parse_result ldap_read ldap_rename ldap_search ldap_err2str "
            "ldap_set_option ldap_set_rebind_proc ldap_sort ldap_start_tls "
            "ldap_t61_to_8859 ldap_unbind ezmlm_hash mail abs acos acosh asin "
            "asinh atan2 atan atanh base_convert bindec ceil cos cosh decbin "
            "dechex decoct deg2rad exp expm1 floor fmod getrandmax hexdec "
            "is_finite is_infinite is_nan lcg_value log10 log1p log max min "
            "mt_getrandmax mt_rand mt_srand octdec pi pow rad2deg rand round "
            "sin sinh sqrt srand tan tanh mb_convert_case mb_convert_encoding "
            "mb_convert_kana mb_convert_variables mb_decode_mimeheader "
            "mb_decode_numericentity mb_detect_encoding mb_detect_order "
            "mb_encode_mimeheader mb_encode_numericentity mb_ereg_match "
            "mb_ereg_replace mb_ereg_search_getpos mb_ereg_search_getregs "
            "mb_ereg_search_init mb_ereg_search_pos mb_ereg_search_regs "
            "mb_ereg_search_setpos mb_ereg_search mb_ereg mb_eregi_replace "
            "mb_eregi mb_get_info mb_http_input mb_http_output ini_restore "
            "mb_internal_encoding mb_language mb_output_handler mb_parse_str "
            "mb_preferred_mime_name mb_regex_encoding mb_regex_set_options "
            "mb_send_mail mb_split mb_strcut mb_strimwidth mb_strlen mb_strpos "
            "mb_strrpos mb_strtolower mb_strtoupper mb_strwidth imagesetpixel "
            "mb_substitute_character mb_substr_count mb_substr mcrypt_cbc "
            "mcrypt_cfb mcrypt_create_iv mcrypt_decrypt mcrypt_ecb "
            "mcrypt_enc_get_algorithms_name mcrypt_enc_get_block_size "
            "mcrypt_enc_get_iv_size mcrypt_enc_get_key_size ftp_nb_continue "
            "mcrypt_enc_get_modes_name mcrypt_enc_get_supported_key_sizes "
            "mcrypt_enc_is_block_algorithm_mode mcrypt_enc_is_block_algorithm "
            "mcrypt_enc_is_block_mode mcrypt_enc_self_test mcrypt_encrypt "
            "mcrypt_generic_deinit mcrypt_generic_end mcrypt_generic_init "
            "mcrypt_generic mcrypt_get_block_size mcrypt_get_cipher_name "
            "mcrypt_get_iv_size mcrypt_get_key_size mcrypt_list_algorithms "
            "mcrypt_list_modes mcrypt_module_close imap_scanmailbox "
            "mcrypt_module_get_algo_key_size imap_get_quotaroot "
            "mcrypt_module_is_block_algorithm_mode imap_mime_header_decode "
            "mcrypt_module_is_block_mode mcrypt_module_open imagecreatefromgd2 "
            "mcrypt_ofb mdecrypt_generic mhash_count mhash_get_block_size "
            "mhash_get_hash_name mhash_keygen_s2k mhash mime_content_type "
            "connection_aborted connection_status connection_timeout constant "
            "defined die eval exit get_browser highlight_file highlight_string "
            "ignore_user_abort pack show_source sleep uniqid unpack usleep "
            "msql_affected_rows msql_close msql_connect msql_create_db define "
            "msql_data_seek msql_dbname msql_drop_db msql_dropdb msql_error "
            "msql_fetch_array msql_fetch_field msql_fetch_object msql_createdb "
            "msql_field_seek msql_fieldflags msql_fieldlen msql_fieldname "
            "msql_fieldtable msql_fieldtype msql_free_result msql_freeresult "
            "msql_list_dbs msql_list_fields msql_list_tables msql_listdbs "
            "msql_listfields msql_listtables msql_num_fields msql_num_rows "
            "msql_numfields msql_numrows msql_pconnect msql_query msql_regcase "
            "msql_result msql_select_db msql_selectdb msql_tablename msql "
            "checkdnsrr closelog debugger_off debugger_on gethostbyaddr "
            "dns_check_record dns_get_mx dns_get_record fsockopen "
            "gethostbyname gethostbynamel getmxrr getprotobyname "
            "getservbyname getservbyport ip2long long2ip openlog pfsockopen "
            "socket_get_status socket_set_blocking socket_set_timeout syslog "
            "ocibindbyname ocicancel OCICollAppend ocicollassign "
            "ocicollgetelem ocicollmax ocicollsize ocicolltrim ocicolumnisnull "
            "ocicolumnname ocicolumnprecision ocicolumnscale ocicolumnsize "
            "ocicolumntype ocicolumntyperaw ocicommit ocidefinebyname ocierror "
            "ociexecute ocifetch ocifetchinto ocifetchstatement msql_fetch_row "
            "ocifreecursor OCIFreeDesc ocifreestatement ociinternaldebug "
            "ocilogoff ocilogon ocinewcollection ocinewcursor ocinewdescriptor "
            "ocinlogon ocinumcols ociparse ociplogon ociresult ocirollback "
            "ocirowcount ocisavelob ocisavelobfile ociserverversion ociloadlob "
            "ocisetprefetch ocistatementtype ociwritelobtofile flush ob_clean "
            "ob_end_clean ob_end_flush ob_flush ob_get_contents ob_get_length "
            "ob_get_level ob_get_status ob_gzhandler ob_implicit_flush "
            "overload pcntl_exec pcntl_fork pcntl_signal pcntl_waitpid "
            "pcntl_wexitstatus pcntl_wifexited pcntl_wifsignaled ob_start "
            "pcntl_wstopsig pcntl_wtermsig preg_grep preg_match_all preg_match "
            "preg_quote preg_replace_callback preg_replace preg_split "
            "pdf_add_annotation pdf_add_bookmark pdf_add_launchlink "
            "pdf_add_note pdf_add_outline pdf_add_pdflink pdf_add_thumbnail "
            "pdf_add_weblink pdf_arc pdf_arcn pdf_attach_file pdf_begin_page "
            "pdf_begin_pattern pdf_begin_template pdf_circle pdf_add_locallink "
            "pdf_close_pdi_page pdf_close_pdi pdf_close pcntl_wifstopped "
            "pdf_closepath_stroke pdf_closepath pdf_concat pdf_continue_text "
            "pdf_curveto pdf_delete pdf_end_page pdf_end_pattern "
            "pdf_endpath pdf_fill_stroke pdf_fill pdf_findfont pdf_get_buffer "
            "pdf_get_font pdf_get_fontname pdf_get_fontsize pdf_open_pdi_page "
            "pdf_get_image_width pdf_get_majorversion pdf_get_minorversion "
            "pdf_get_parameter pdf_get_pdi_parameter pdf_get_pdi_value "
            "pdf_initgraphics pdf_lineto pdf_makespotcolor pdf_moveto pdf_new "
            "pdf_open_CCITT pdf_open_file pdf_open_gif pdf_open_image_file "
            "pdf_open_image pdf_open_jpeg pdf_open_memory_image "
            "pdf_open_pdi pdf_open_png pdf_open_tiff pdf_open pdf_place_image "
            "pdf_place_pdi_page pdf_rect pdf_restore pdf_rotate pdf_get_value "
            "pdf_set_border_color pdf_set_border_dash pdf_set_border_style "
            "pdf_set_char_spacing pdf_set_duration pdf_set_font "
            "pdf_set_info_author pdf_set_info_creator pdf_set_info_keywords "
            "pdf_set_info_subject pdf_set_info_title pdf_set_info "
            "pdf_set_parameter pdf_set_text_matrix pdf_set_text_pos "
            "pdf_set_text_rendering pdf_set_text_rise pdf_set_value "
            "pdf_set_word_spacing pdf_setcolor pdf_setdash pdf_setflat "
            "pdf_setgray_fill pdf_setgray_stroke pdf_setgray pdf_setlinecap "
            "pdf_setlinejoin pdf_setlinewidth pdf_setmatrix pdf_setmiterlimit "
            "pdf_setpolydash pdf_setrgbcolor_fill pdf_setrgbcolor_stroke "
            "pdf_setrgbcolor pdf_show_boxed pdf_show_xy pdf_show pdf_skew "
            "pdf_stringwidth pdf_stroke pdf_translate pg_affected_rows "
            "pg_cancel_query pg_client_encoding pg_close pg_connect "
            "pg_connection_busy pg_connection_reset pg_connection_status "
            "pg_copy_from pg_copy_to pg_dbname pg_delete pg_end_copy "
            "pg_escape_string pg_fetch_all pg_fetch_array pg_fetch_assoc "
            "pg_fetch_object pg_fetch_result pg_fetch_row pg_field_is_null "
            "pg_field_name pg_field_num pg_field_prtlen pg_field_size "
            "pg_free_result pg_get_notify pg_get_pid pg_get_result pg_host "
            "pg_last_error pg_last_notice pg_last_oid pg_lo_close pg_lo_create "
            "pg_lo_export pg_lo_import pg_lo_open pg_lo_read_all pg_lo_read "
            "pg_lo_seek pg_lo_tell pg_lo_unlink pg_lo_write pg_meta_data "
            "pg_num_fields pg_num_rows pg_options pg_pconnect pg_ping pg_port "
            "pg_put_line pg_query pg_result_error pg_result_seek pg_field_type "
            "pg_select pg_send_query pg_set_client_encoding pg_trace pg_tty "
            "pg_unescape_bytea pg_untrace pg_update posix_ctermid posix_getcwd "
            "posix_getegid posix_geteuid posix_getgid posix_getgrgid pg_insert "
            "posix_getgroups posix_getlogin posix_getpgid posix_getpgrp "
            "posix_getppid posix_getpwnam posix_getpwuid posix_getrlimit "
            "posix_getuid posix_isatty posix_kill posix_mkfifo posix_setegid "
            "posix_seteuid posix_setgid posix_setpgid posix_setsid pdf_setfont "
            "posix_times posix_ttyname posix_uname pspell_add_to_personal "
            "pspell_add_to_session pspell_check pspell_clear_session "
            "pspell_config_create pspell_config_ignore pspell_config_mode "
            "pspell_config_personal pspell_config_repl posix_setuid "
            "pspell_config_save_repl pspell_new_config pspell_new_personal "
            "pspell_new pspell_save_wordlist pspell_store_replacement "
            "recode_file recode_string recode ereg_replace ereg eregi_replace "
            "split spliti sql_regcase ftok msg_get_queue msg_receive "
            "msg_send msg_set_queue msg_stat_queue sem_acquire sem_get "
            "sem_remove shm_attach shm_detach shm_get_var shm_put_var "
            "shm_remove session_cache_expire session_cache_limiter sem_release "
            "session_destroy session_encode session_get_cookie_params eregi "
            "session_is_registered session_module_name session_name session_id "
            "session_register session_save_path session_set_cookie_params "
            "session_set_save_handler session_start session_unregister "
            "session_write_close snmp_get_quick_print snmp_set_quick_print "
            "snmprealwalk snmpset snmpwalk snmpwalkoid socket_accept snmpget "
            "socket_clear_error socket_close socket_connect session_unset "
            "socket_create_pair socket_create socket_get_option socket_bind "
            "socket_getsockname socket_iovec_add socket_iovec_alloc "
            "socket_iovec_delete socket_iovec_fetch socket_iovec_free "
            "socket_iovec_set socket_last_error socket_listen socket_read "
            "socket_readv socket_recv socket_recvfrom socket_recvmsg "
            "socket_send socket_sendmsg socket_sendto socket_set_nonblock "
            "socket_set_option socket_shutdown socket_strerror socket_write "
            "socket_writev stream_context_create stream_context_get_options "
            "stream_context_set_option stream_context_set_params socket_select "
            "stream_filter_prepend stream_get_filters stream_get_meta_data "
            "stream_get_wrappers stream_register_filter stream_filter_append "
            "stream_select stream_set_blocking stream_set_timeout posix_getpid "
            "stream_set_write_buffer addcslashes addslashes bin2hex chop chr "
            "chunk_split convert_cyr_string count_chars crc32 crypt echo "
            "fprintf get_html_translation_table hebrev hebrevc explode "
            "htmlentities htmlspecialchars implode join levenshtein localeconv "
            "md5_file md5 metaphone money_format nl_langinfo nl2br pg_convert "
            "parse_str print printf quoted_printable_decode quotemeta rtrim "
            "setlocale sha1_file sha1 similar_text soundex sprintf sscanf ord "
            "str_repeat str_replace str_rot13 str_shuffle str_word_count ltrim "
            "strchr strcmp strcoll strcspn strip_tags stripcslashes strcasecmp "
            "stristr strlen strnatcasecmp strnatcmp strncasecmp strncmp strpos "
            "strrchr strrev strrpos strspn strstr strtok strtolower strtoupper "
            "substr_count substr_replace substr trim ucfirst ucwords vprintf "
            "wordwrap base64_decode base64_encode get_meta_tags parse_url "
            "rawurldecode rawurlencode urldecode urlencode doubleval empty "
            "get_defined_vars get_resource_type gettype stripslashes str_pad "
            "intval is_array is_bool is_callable is_double is_float is_int "
            "is_long is_null is_numeric is_object is_real is_resource floatval "
            "is_string isset print_r serialize settype strval unserialize "
            "var_dump var_export utf8_decode utf8_encode xml_error_string "
            "xml_get_current_byte_index xml_get_current_column_number unset "
            "xml_get_current_line_number xml_get_error_code is_scalar vsprintf "
            "xml_parse xml_parser_create_ns xml_parser_create xml_parser_free "
            "xml_parser_get_option xml_parser_set_option number_format "
            "xml_set_default_handler xml_set_element_handler is_integer "
            "xml_set_end_namespace_decl_handler xml_parse_into_struct strtr "
            "xml_set_notation_decl_handler xml_set_object html_entity_decode "
            "xml_set_processing_instruction_handler stream_register_wrapper "
            "xml_set_unparsed_entity_decl_handler xslt_create xslt_errno "
            "xslt_free xslt_output_process xslt_set_base xslt_set_encoding "
            "xslt_set_error_handler xslt_set_log xslt_set_sax_handler "
            "xslt_set_sax_handlers xslt_set_scheme_handler socket_getpeername "
            "zip_close zip_entry_close zip_entry_compressedsize xslt_error "
            "zip_entry_compressionmethod zip_entry_filesize zip_entry_name "
            "zip_entry_open zip_entry_read zip_open zip_read session_decode "
            "get_magic_quotes_runtime xslt_set_scheme_handlers pspell_suggest "
            "xml_set_start_namespace_decl_handler import_request_variables "
            "xml_set_external_entity_ref_handler socket_create_listen "
            "xml_set_character_data_handler session_readonly shm_remove_var "
            "msg_remove_queue pspell_config_runtogether posix_getsid "
            "posix_getgrnam pg_result_status pg_escape_bytea pdf_set_leading "
            "pdf_set_horiz_scaling pdf_save pdf_scale pdf_get_image_height "
            "pdf_end_template pdf_closepath_fill_stroke ocicollassignelem "
            "pdf_clip pdf_close_image ocifreecollection getprotobynumber "
            "mcrypt_module_self_test define_syslog_variables "
            "mcrypt_module_get_supported_key_sizes imap_clearflag_full "
            "mcrypt_module_is_block_algorithm imagepsencodefont "
            "mcrypt_module_get_algo_block_size imagepsslantfont count ")

# HTML Keywords
HTML_KEYWORDS = html.Keywords()

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ ('STC_HPHP_DEFAULT', 'default_style'),
                 ('STC_HPHP_COMMENT', 'comment_style'),
                 ('STC_HPHP_COMMENTLINE', 'comment_style'),
                 ('STC_HPHP_COMPLEX_VARIABLE', 'pre_style'), #STYLE ME
                 ('STC_HPHP_HSTRING', 'string_style'),
                 ('STC_HPHP_HSTRING_VARIABLE', 'scalar_style'), # STYLE ME
                 ('STC_HPHP_NUMBER', 'number_style'),
                 ('STC_HPHP_OPERATOR', 'operator_style'),
                 ('STC_HPHP_SIMPLESTRING', 'string_style'),
                 ('STC_HPHP_VARIABLE', 'pre2_style'),
                 ('STC_HPHP_WORD', 'keyword_style') ]

#---- Extra Properties ----#
# Inherited from html.py

#------------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_PHP:
        # Support Embedded HTML highlighting
        keywords = HTML_KEYWORDS
        keywords.append((4, PHP_KEYWORDS))
        return keywords
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_PHP:
        return html.SYNTAX_ITEMS + SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_PHP:
        return [html.FOLD, html.FLD_HTML]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)
    @note: assuming pure php code for comment character(s)

    """
    if lang_id == synglob.ID_LANG_PHP:
        return [u'//']
    else:
        return list()

#---- End Required Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString(option=0):
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return PHP_KEYWORDS

#---- End Syntax Modules Internal Functions ----#
