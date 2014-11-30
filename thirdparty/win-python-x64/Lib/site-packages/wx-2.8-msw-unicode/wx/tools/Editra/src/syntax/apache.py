###############################################################################
# Name: apache.py                                                             #
# Purpose: Define Apache syntax for highlighting and other features           #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: apache.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Apache Configuration Files

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: apache.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#
# Imports

import synglob
#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#
DIRECTIVES = (0, 'acceptmutex acceptpathinfo accessconfig accessfilename '
                 'action addalt addaltbyencoding addaltbytype addcharset '
                 'adddefaultcharset adddescription addencoding addhandler '
                 'addicon addiconbyencoding addiconbytype addinputfilter '
                 'addlanguage addmodule addmoduleinfo addoutputfilter '
                 'addoutputfilterbytype addtype agentlog alias aliasmatch all '
                 'allow allowconnect allowencodedslashes allowoverride '
                 'anonymous anonymous_authoritative anonymous_logemail '
                 'anonymous_mustgiveemail anonymous_nouserid '
                 'anonymous_verifyemail assignuserid authauthoritative '
                 'authdbauthoritative authdbgroupfile authdbmauthoritative '
                 'authdbmgroupfile authdbmtype authdbmuserfile authdbuserfile '
                 'authdigestalgorithm authdigestdomain authdigestfile '
                 'authdigestgroupfile authdigestnccheck authdigestnonceformat '
                 'authdigestnoncelifetime authdigestqop authdigestshmemsize '
                 'authgroupfile authldapauthoritative authldapbinddn '
                 'authldapbindpassword authldapcharsetconfig '
                 'authldapcomparednonserver authldapdereferencealiases '
                 'authldapenabled authldapfrontpagehack authldapgroupattribute '
                 'authldapgroupattributeisdn authldapremoteuserisdn '
                 'authldapurl authname authtype authuserfile bindaddress '
                 'browsermatch browsermatchnocase bs2000account bufferedlogs '
                 'cachedefaultexpire cachedirlength cachedirlevels '
                 'cachedisable cacheenable cacheexpirycheck cachefile '
                 'cacheforcecompletion cachegcclean cachegcdaily '
                 'cachegcinterval cachegcmemusage cachegcunused '
                 'cacheignorecachecontrol cacheignoreheaders '
                 'cacheignorenolastmod cachelastmodifiedfactor cachemaxexpire '
                 'cachemaxfilesize cacheminfilesize cachenegotiateddocs '
                 'cacheroot cachesize cachetimemargin cgimapextension '
                 'charsetdefault charsetoptions charsetsourceenc checkspelling '
                 'childperuserid clearmodulelist contentdigest cookiedomain '
                 'cookieexpires cookielog cookiename cookiestyle '
                 'cookietracking coredumpdirectory customlog dav '
                 'davdepthinfinity davlockdb davmintimeout defaulticon '
                 'defaultlanguage defaulttype define deflatebuffersize '
                 'deflatecompressionlevel deflatefilternote deflatememlevel '
                 'deflatewindowsize deny directory directoryindex '
                 'directorymatch directoryslash documentroot dumpioinput '
                 'dumpiooutput enableexceptionhook enablemmap enablesendfile '
                 'errordocument errorlog example expiresactive expiresbytype '
                 'expiresdefault extendedstatus extfilterdefine '
                 'extfilteroptions fancyindexing fileetag files filesmatch '
                 'forcelanguagepriority forcetype forensiclog from group '
                 'header headername hostnamelookups identitycheck ifdefine '
                 'ifmodule imapbase imapdefault imapmenu include indexignore '
                 'indexoptions indexorderdefault isapiappendlogtoerrors '
                 'isapiappendlogtoquery isapicachefile isapifakeasync '
                 'isapilognotsupported isapireadaheadbuffer keepalive '
                 'keepalivetimeout languagepriority ldapcacheentries '
                 'ldapcachettl ldapconnectiontimeout ldapopcacheentries '
                 'ldapopcachettl ldapsharedcachefile ldapsharedcachesize '
                 'ldaptrustedca ldaptrustedcatype limit limitexcept '
                 'limitinternalrecursion limitrequestbody limitrequestfields '
                 'limitrequestfieldsize limitrequestline limitxmlrequestbody '
                 'listen listenbacklog loadfile loadmodule location '
                 'locationmatch lockfile logformat loglevel maxclients '
                 'maxkeepaliverequests maxmemfree maxrequestsperchild '
                 'maxrequestsperthread maxspareservers maxsparethreads '
                 'maxthreads maxthreadsperchild mcachemaxobjectcount '
                 'mcachemaxobjectsize mcachemaxstreamingbuffer '
                 'mcacheminobjectsize mcacheremovalalgorithm mcachesize '
                 'metadir metafiles metasuffix mimemagicfile minspareservers '
                 'minsparethreads mmapfile modmimeusepathinfo multiviewsmatch '
                 'namevirtualhost nocache noproxy numservers nwssltrustedcerts '
                 'nwsslupgradeable options order passenv pidfile port '
                 'protocolecho proxy proxybadheader proxyblock proxydomain '
                 'proxyerroroverride proxyiobuffersize proxymatch '
                 'proxymaxforwards proxypass proxypassreverse '
                 'proxypreservehost proxyreceivebuffersize proxyremote '
                 'proxyremotematch proxyrequests proxytimeout proxyvia qsc '
                 'readmename redirect redirectmatch redirectpermanent '
                 'redirecttemp refererignore refererlog removecharset '
                 'removeencoding removehandler removeinputfilter '
                 'removelanguage removeoutputfilter removetype requestheader '
                 'require resourceconfig rewritebase rewritecond rewriteengine '
                 'rewritelock rewritelog rewriteloglevel rewritemap '
                 'rewriteoptions rewriterule rlimitcpu rlimitmem rlimitnproc '
                 'satisfy scoreboardfile script scriptalias scriptaliasmatch '
                 'scriptinterpretersource scriptlog scriptlogbuffer '
                 'scriptloglength scriptsock securelisten sendbuffersize '
                 'serveradmin serveralias serverlimit servername serverpath '
                 'serverroot serversignature servertokens servertype setenv '
                 'setenvif setenvifnocase sethandler setinputfilter '
                 'setoutputfilter singlelisten ssiendtag ssierrormsg '
                 'ssistarttag ssitimeformat ssiundefinedecho '
                 'sslcacertificatefile sslcacertificatepath '
                 'sslcarevocationfile sslcarevocationpath '
                 'sslcertificatechainfile sslcertificatefile '
                 'sslcertificatekeyfile sslciphersuite sslengine sslmutex '
                 'ssloptions sslpassphrasedialog sslprotocol '
                 'sslproxycacertificatefile sslproxycacertificatepath '
                 'sslproxycarevocationfile sslproxycarevocationpath '
                 'sslproxyciphersuite sslproxyengine '
                 'sslproxymachinecertificatefile '
                 'sslproxymachinecertificatepath sslproxyprotocol '
                 'sslproxyverify sslproxyverifydepth sslrandomseed sslrequire '
                 'sslrequiressl sslsessioncache sslsessioncachetimeout '
                 'sslusername sslverifyclient sslverifydepth startservers '
                 'startthreads suexecusergroup threadlimit threadsperchild '
                 'threadstacksize timeout transferlog typesconfig unsetenv '
                 'usecanonicalname user userdir virtualdocumentroot '
                 'virtualdocumentrootip virtualhost virtualscriptalias '
                 'virtualscriptaliasip win32disableacceptex xbithack')

PARAMS = (1, 'on off standalone inetd force-response-1.0 downgrade-1.0 '
             'nokeepalive indexes includes followsymlinks none x-compress '
             'x-gzip warn')

#---- End Keyword Definitions ----#

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [('STC_CONF_COMMENT', 'comment_style'),
                ('STC_CONF_DEFAULT', 'default_style'),
                ('STC_CONF_DIRECTIVE', 'keyword_style'),
                ('STC_CONF_EXTENSION', 'pre_style'),
                ('STC_CONF_IDENTIFIER', 'number_style'),
                ('STC_CONF_IP', 'number2_style'),
                ('STC_CONF_NUMBER', 'number_style'),
                ('STC_CONF_OPERATOR', 'operator_style'),
                ('STC_CONF_PARAMETER', 'global_style'),
                ('STC_CONF_STRING', 'string_style')]

#---- Extra Properties ----#

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @keyword lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_APACHE:
        return [DIRECTIVES, PARAMS]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id == synglob.ID_LANG_APACHE:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @keyword lang_id: used to select a specific set of properties

    """
    if lang_id == synglob.ID_LANG_APACHE:
        return list()
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @keyword lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id == synglob.ID_LANG_APACHE:
        return [u'#']
    else:
        return list()

#---- End Required Module Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString():
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
