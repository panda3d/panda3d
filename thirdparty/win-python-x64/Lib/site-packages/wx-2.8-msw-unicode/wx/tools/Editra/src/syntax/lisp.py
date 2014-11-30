###############################################################################
# Name: lisp.py                                                               #
# Purpose: Define Lisp syntax for highlighting and other features             #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Author: Jeff                                                                #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: lisp.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Lisp Files.
@todo: Add Standard Variables

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: lisp.py 55074 2008-08-12 23:46:33Z CJP $"
__revision__ = "$Revision: 55074 $"

#-----------------------------------------------------------------------------#
import synglob

#-----------------------------------------------------------------------------#

#---- Keyword Definitions ----#

# Lisp Functions/Operators
LISP_FUNC = (0, "abort abs access acons acos acosh add-method adjoin "
                "adjust-array adjustable-array-p alist allocate-instance "
                "alpha-char-p alphanumericp and append apply applyhook apropos "
                "apropos-list aref arithmetic-error arithmetic-error-operands "
                "arithmetic-error-operation array array-dimension "
                "array-dimension-limit array-dimensions array-displacement "
                "array-element-type array-has-fill-pointer-p "
                "array-in-bounds-p array-rank array-rank-limit "
                "array-row-major-index array-total-size "
                "array-total-size-limit arrayp ash asin asinh assert assoc "
                "assoc-if assoc-if-not atan atanh atom backquote baktrace "
                "base-char base-string bignum bignums bit bit-and bit-andc1 "
                "bit-andc2 bit-eqv bit-ior bit-nand bit-nor bit-not bit-orc1 "
                "bit-orc2 bit-vector bit-vector-p bit-xor block boole boole-1 "
                "boole-2 boole-and boole-andc1 boole-andc2 boole-c1 boole-c2 "
                "boole-clr boole-eqv boole-ior boole-nand boole-nor boole-orc1 "
                "boole-orc2 boole-set boole-xor boolean both-case-p boundp "
                "break broadcast-stream broadcast-stream-streams "
                "built-in-class butlast byte byte-position byte-size caaaar "
                "caaadr caaar caadar caaddr caadr caar cadaar cadadr cadar "
                "caddar cadddr caddr cadr call-arguments-limit call-method "
                "call-next-method capitalize car case catch ccase cdaaar "
                "cdaadr cdaar cdadar cdaddr cdadr cdar cddaar cddadr cddar "
                "cdddar cddddr cdddr cddr cdr ceil-error ceil-error-name "
                "ceiling cerror change-class char char-bit char-bits "
                "char-bits-limit char-code char-code-limit char-control-bit "
                "char-downcase char-equal char-font char-font-limit "
                "char-greaterp char-hyper-bit char-int char-lessp "
                "char-meta-bit char-name char-not-equal char-not-greaterp "
                "char-not-lessp char-super-bit char-upcase char/= char<= char= "
                "char>= character characterp check-type cirhash cis class "
                "class-name class-of clear-input clear-output close code-char "
                "coerce commonp compilation-speed compile compile-file "
                "compile-file-pathname compiled-function compiled-function-p "
                "compiler-let compiler-macro compiler-macro-function "
                "complement complex complexp compute-applicable-methods "
                "compute-restarts concatenate concatenated-stream "
                "concatenated-stream-streams cond condition conjugate cons "
                "consp constantly constantp continue control-error copy "
                "copy-list copy-pprint-dispatch copy-readtable copy-seq "
                "copy-structure copy-symbol copy-tree cos cosh count count-if "
                "count-if-not ctypecase debug decf declaim declaration declare "
                "decode-float decode-universal-time defclass defconstant "
                "defgeneric define-compiler-macro define-condition "
                "define-method-combination define-modify-macro "
                "define-setf-expander define-setf-method define-symbol-macro "
                "defmacro defmethod defpackage defparameter defsetf defstruct "
                "deftype defun defvar delete delete-duplicates delete-file "
                "delete-if delete-if-not delete-package denominator "
                "deposite-field describe describe-object destructuring-bind "
                "digit-char digit-char-p directory directory-namestring "
                "disassemble division-by-zero do do* do-all-symbols "
                "do-external-symbols do-symbols dolist dotimes double-float "
                "double-float-epsilon double-float-negative-epsilion dpb "
                "dribble dynamic-extent ecase echo-stream "
                "echo-stream-input-stream echo-stream-output-stream ed eigth "
                "elt encode-universal-time end-of-file endp enough-namestring "
                "ensure-directories-exist ensure-generic-function eq eql equal "
                "equalp error errset etypecase eval eval-when evalhook evenp "
                "every exp export expt extend-char fboundp fceiling "
                "fdefinition fflor fifth file-author file-error "
                "file-error-pathname file-length file-namestring file-position "
                "file-stream file-string-length file-write-date fill "
                "fill-pointer find find-all-symbols find-class find-if "
                "find-if-not find-method find-package find-restart find-symbol "
                "finish-output first fixnum flet float float-digits "
                "float-precision float-radix float-sign floating-point-inexact "
                "floating-point-invalid-operation floating-point-underflow "
                "floatp floor fmakunbound force-output format formatter fourth "
                "fresh-line fround ftruncate ftype funcall function "
                "function-keywords function-lambda-expression functionp gbitp "
                "gcd generic-function gensym gentemp get get-decoded-time "
                "get-dispatched-macro-character get-internal-real-time "
                "get-internal-run-time get-macro-character "
                "get-output-stream-string get-properties get-setf-expansion "
                "get-setf-method get-universial-time getf gethash go "
                "graphic-char-p handler-bind handler-case hash hash-table "
                "hash-table-count hash-table-p hash-table-rehash-size "
                "hash-table-rehash-threshold hash-table-size hash-table-test "
                "host-namestring identity if if-exists ignorable ignore "
                "ignore-errors imagpart import in-package incf "
                "initialize-instance inline input-stream-p inspect int-char "
                "integer integer-decode-float integer-length integerp "
                "interactive-stream-p intern internal-time-units-per-second "
                "intersection invalid-method-error invoke-debugger "
                "invoke-restart invoke-restart-interactively isqrt keyword "
                "keywordp l labels lambda lambda-list-keywords "
                "lambda-parameters-limit last lcm ldb ldb-test ldiff "
                "least-negative-double-float least-negative-long-float "
                "least-negative-normalized-double-float "
                "least-negative-normalized-long-float "
                "least-negative-normalized-short-font "
                "least-negative-normalized-single-font "
                "least-negative-short-font least-negative-single-font "
                "least-positive-double-float least-positive-long-float "
                "least-positive-normalized-double-float "
                "least-positive-normalized-long-float "
                "least-positive-normalized-short-float "
                "least-positive-normalized-single-float "
                "least-positive-short-float least-positive-single-float length "
                "let let* lisp lisp-implementation-type "
                "lisp-implementation-version list list* "
                "list-all-packages list-lenght listen listp load "
                "load-logical-pathname-translation load-time-value locally "
                "log logand logandc1 logandc2 logbitp logcount logeqv "
                "logical-pathname logical-pathname-translations logior lognand "
                "lognor lognot logorc1 logorc2 logtest logxor long-float "
                "long-float-epsilon long-float-negative-epsilon long-site-name "
                "loop loop-finish lower-case-p machine-instance machine-type "
                "machine-version macro-function macroexpand macroexpand-1 "
                "macroexpand-l macrolet make make-array make-broadcast-stream "
                "make-char make-concatenated-stream make-condition "
                "make-dispatch-macro-character make-echo-stream "
                "make-hash-table make-instance make-instances-obsolete "
                "make-list make-load-form make-load-form-saving-slots "
                "make-method make-package make-pathname make-random-state "
                "make-sequence make-string make-string-input-stream "
                "make-string-output-stream make-symbol make-synonym-stream "
                "make-two-way-stream makunbound map map-into mapc mapcan "
                "mapcar mapcon maphash mapl maplist mask-field max member "
                "member-if member-if-not merge merge-pathname merge-pathnames "
                "method method-combination method-combination-error "
                "method-qualifiers min minusp mismatch mod "
                "most-negative-double-float most-negative-fixnum "
                "most-negative-long-float most-negative-short-float "
                "most-negative-single-float most-positive-fixnum "
                "most-positive-long-float most-positive-short-float "
                "most-positive-single-float muffle-warning "
                "multiple-value-bind multiple-value-call multiple-value-limit "
                "multiple-value-list multiple-value-prog1 multiple-value-seteq "
                "multiple-value-setq name name-char namestring nbutlast nconc "
                "next-method-p nil nintersection ninth no-applicable-method "
                "no-next-method not notany notevery notinline nreconc nreverse "
                "nset-difference nset-exclusive-or nstring nstring-capitalize "
                "nstring-downcase nstring-upcase nstubst-if-not nsublis nsubst "
                "nsubst-if nth nth-value nthcdr null number numberp numerator "
                "nunion oddp open open-stream-p optimize or otherwise "
                "output-stream-p package package-error package-error-package "
                "package-name package-nicknames package-shadowing-symbols "
                "package-use-list package-used-by-list packagep pairlis "
                "parse-error parse-integer parse-namestring pathname "
                "pathname-device pathname-directory pathname-host "
                "pathname-match-p pathname-name pathname-type "
                "pathname-version pathnamep peek-char phase pi plist plusp pop "
                "position position-if position-if-not pprint pprint-dispatch "
                "pprint-exit-if-list-exhausted pprint-fill pprint-indent "
                "pprint-linear pprint-logical-block pprint-newline pprint-pop "
                "pprint-tab pprint-tabular prin1 prin1-to-string princ "
                "princ-to-string print print-not-readable "
                "print-not-readable-object print-object probe-file proclaim "
                "prog prog* prog1 prog2 progn program-error progv provide "
                "psetf psetq push pushnew putprop quote random random-state "
                "random-state-p rassoc rassoc-if rassoc-if-not ration rational "
                "rationalize rationalp read read-byte read-car-no-hang "
                "read-char read-delimited-list read-eval-print "
                "read-from-string read-line read-preserving-whitespace "
                "read-squence reader-error readtable readtable-case readtablep "
                "real realp realpart reduce reinitialize-instance rem remf "
                "remhash remove remove-duplicates remove-if "
                "remove-if-not remove-method remprop rename-file "
                "rename-package replace require rest restart restart-bind "
                "restart-case restart-name return return-from revappend "
                "reverse room rotatef round row-major-aref rplaca rplacd "
                "safety satisfies sbit scale-float schar search second "
                "sequence serious-condition set set-char-bit set-difference "
                "set-dispatched-macro-character set-exclusive-or "
                "set-macro-character set-pprint-dispatch "
                "set-syntax-from-char setf setq seventh shadow "
                "shadowing-import shared-initialize shiftf short-float "
                "short-float-epsilon short-float-negative-epsilon "
                "short-site-name signal signed-byte signum simple-array "
                "simple-base-string simple-bit-vector- simple-bit-vector-p "
                "simple-condition simple-condition-format-arguments "
                "simple-condition-format-control simple-error simple-string "
                "simple-string-p simple-type-error simple-vector "
                "simple-vector-p simple-warning sin single-float "
                "single-float-epsilon single-float-negative-epsilon sinh "
                "sixth sleep slot-boundp slot-exists-p slot-makunbound "
                "slot-missing slot-unbound slot-value software-type "
                "software-version some sort space special special-form-p "
                "special-operator-p speed sqrt stable-sort standard "
                "standard-char standard-char-p standard-class "
                "standard-generic-function standard-method standard-object "
                "step storage-condition store-value stream stream-element-type "
                "stream-error stream-error-stream stream-external-format "
                "streamp streamup string string-capitalize string-char "
                "string-char-p string-downcase string-equal string-greaterp "
                "string-left-trim string-lessp string-not-equal "
                "string-not-greaterp string-not-lessp string-right-strim "
                "string-right-trim string-stream string-trim string-upcase "
                "string/= string< string<= string= string> string>= stringp "
                "structure structure-class structure-object style-warning "
                "sublim sublis subseq subsetp subst subst-if subst-if-not "
                "substitute substitute-if substitute-if-not subtypep svref "
                "sxhash symbol symbol-function symbol-macrolet symbol-name "
                "symbol-package symbol-plist symbol-value symbolp "
                "synonym-stream synonym-stream-symbol sys system t tagbody "
                "tailp tan tanh tenth terpri the third throw time trace "
                "translate-logical-pathname translate-pathname tree-equal "
                "truename truncase truncate two-way-stream "
                "two-way-stream-input-stream two-way-stream-output-stream "
                "type type-error type-error-datnum type-error-expected-type "
                "type-of typecase typep unbound-slot unbound-slot-instance "
                "unbound-variable undefined-function unexport unintern union "
                "unless unread unread-char unsigned-byte untrace unuse-package "
                "unwind-protect update-instance-for-different-class "
                "update-instance-for-redefined-class "
                "upgraded-array-element-type upgraded-complex-part-type "
                "upper-case-p use-package use-value user user-homedir-pathname "
                "value value-list values vector vector-pop vector-push "
                "vector-push-extend vectorp warn warning when "
                "wild-pathname-p with-accessors with-compilation-unit "
                "with-condition-restarts with-hash-table-iterator "
                "with-input-from-string with-open-file with-open-stream "
                "with-output-to-string with-package-iterator "
                "with-simple-restart with-slots with-standard-io-syntax write "
                "write-byte write-char write-line write-sequence" )

SCHEME_KW = (0, "* + - / < <= = => > >= abs acos and angle append apply asin "
                "assoc assq assv atan begin boolean? caaaar caaadr caaar "
                "caadar caaddr caadr caar cadaar cadadr cadar caddar cadddr "
                "caddr cadr call-with-current-continuation "
                "call-with-input-file call-with-output-file call-with-values "
                "call/cc car case cdaaar cdaadr cdaar cdadar cdaddr cdadr cdar "
                "cddaar cddadr cddar cdddar cddddr cdddr cddr cdr ceiling "
                "char->integer char-alphabetic? char-ci<=? char-ci<? char-ci=? "
                "char-ci>=? char-ci>? char-downcase char-lower-case? "
                "char-numeric? char-ready? char-upcase char-upper-case? "
                "char-whitespace? char<=? char<? char=? char>=? char>? char? "
                "close-input-port close-output-port complex? cond cons cos "
                "current-input-port current-output-port define define-syntax "
                "delay denominator display do dynamic-wind else eof-object? "
                "eq? equal? eqv? eval even? exact->inexact exact? exp expt "
                "floor for-each force gcd if imag-part inexact->exact inexact? "
                "input-port? integer->char integer? interaction-environment "
                "lambda lcm length let let* let-syntax letrec letrec-syntax "
                "list list->string list->vector list-ref list-tail list? load "
                "log magnitude make-polar make-rectangular make-string "
                "make-vector map max member memq memv min modulo negative? "
                "newline not null-environment null? number->string number? "
                "numerator odd? open-input-file open-output-file or "
                "output-port? pair? peek-char positive? procedure? quasiquote "
                "quote quotient rational? rationalize read read-char "
                "real-part real? remainder reverse round "
                "scheme-report-environment set! set-car! set-cdr! sin sqrt "
                "string string->list string->number string->symbol "
                "string-append string-ci<=? string-ci<? string-ci=? "
                "string-ci>=? string-ci>? string-copy string-fill! "
                "string-length string-ref string-set! string<=? string<? "
                "string=? string>=? string>? string? substring symbol->string "
                "symbol? syntax-rules transcript-off transcript-on truncate "
                "unquote unquote-splicing values vector vector->list "
                "vector-fill! vector-length vector-ref vector-set! vector? "
                "with-input-from-file with-output-to-file write write-char "
                "zero?" )

# Lisp Keywords
LISP_KEYWORDS = (1, ":abort :adjustable :append :array :base :case :circle "
                    ":conc-name :constructor :copier :count :create :default "
                    ":device :directory :displaced-index-offset :displaced-to "
                    ":element-type :end :end1 :end2 :error :escape :external "
                    ":from-end :gensym :host :include :if-does-not-exist "
                    ":if-exists :index :inherited :internal :initial-contents "
                    ":initial-element :initial-offset :initial-value :input "
                    ":io :junk-allowed :key :length :level :name :named "
                    ":new-version :nicknames :output :ouput=file :overwrite "
                    ":predicate :preserve-whitespace :pretty :print "
                    ":print-function :probe :radix :read-only :rehash-size "
                    ":rehash-threshold :rename :size :rename-and-delete :start "
                    ":start1 :start2 :stream :supersede :test :test-not :use "
                    ":verbose :version")

NEWLISP_FUNC = (0, "! != % & * + - / : < << <= = > >= >> ? @ NaN? abort abs "
                   "acos acosh add address " "amb and append append-file apply "
                   "args array array-list array? asin asinh assoc assoc-set "
                   "atan atan2 atanh atom? base64-dec base64-enc bayes-query "
                   "bayes-train begin beta betai bind binomial callback case "
                   "catch ceil change-dir char chop clean close command-event "
                   "cond cons constant context context? copy-file cos cosh "
                   "count cpymem crc32 crit-chi2 crit-z current-line curry "
                   "date date-value debug dec def-new default define "
                   "define-macro delete delete-file delete-url destroy det "
                   "device difference directory directory? div do-until "
                   "do-while doargs dolist dostring dotimes dotree dump dup "
                   "empty? encrypt ends-with env erf error-event error-number "
                   "error-text eval eval-string exec exists exit exp expand "
                   "explode factor fft file-info file? filter find find-all "
                   "first flat " "float float? floor flt for for-all fork "
                   "format fv gammai gammaln gcd get-char get-float get-int "
                   "get-long get-string get-url global global? if if-not ifft "
                   "import inc index int integer integer? intersect invert irr "
                   "join lambda? last legal? length let letex letn list list? "
                   "load local log lookup lower-case macro? main-args make-dir "
                   "map mat match max member min mod mul multiply name "
                   "net-accept net-close net-connect net-error net-eval "
                   "net-listen net-local net-lookup net-peek net-peer net-ping"
                   "-receive " "net-receive-from net-receive-udp net-select "
                   "net-send net-send-to net-send-udp net-service net-sessions "
                   "new nil nil? normal not now nper npv nth nth-set null? "
                   "number? open or ostype pack parse parse-date peek pipe pmt "
                   "pop pop-assoc post-url pow pretty-print primitive? print "
                   "println prob-chi2 prob-z process prompt-event protected? "
                   "push put-url pv quote quote? rand random randomize "
                   "read-buffer read-char read-expr read-file read-key "
                   "read-line real-path ref ref-all ref-set regex regex-comp "
                   "remove-dir rename-file replace reset rest reverse rotate "
                   "round save search seed seek select semaphore sequence "
                   "series set set-assoc set-locale set-nth set-ref "
                   "set-ref-all setq sgn share signal silent sin sinh sleep "
                   "slice sort source spawn sqrt starts-with string string? "
                   "sub swap sym symbol? symbols sync sys-error sys-info tan "
                   "tanh throw throw-error time time-of-day timer title-case "
                   "trace trace-highlight transpose trim true true? unicode "
                   "unify unique unless unpack until upper-case utf8 utf8len "
                   "uuid wait-pid when while write-buffer write-char "
                   "write-file write-line xml-error xml-parse xml-type-tags "
                   "zero? | ~ lambda")
 
# Lisp Keywords
NEWLISP_KEYWORDS = (1, "$ $0 $1 $10 $11 $12 $13 $14 $15 $2 $3 $4 $5 $6 $7 $8 "
                       "$9 $args $idx $main-args MAIN :")

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ ('STC_LISP_DEFAULT', 'default_style'),
                 ('STC_LISP_COMMENT', 'comment_style'),
                 ('STC_LISP_MULTI_COMMENT', 'comment_style'),
                 ('STC_LISP_IDENTIFIER', 'default_style'),
                 ('STC_LISP_KEYWORD', 'keyword_style'),
                 ('STC_LISP_KEYWORD_KW', 'keyword2_style'),
                 ('STC_LISP_NUMBER', 'number_style'),
                 ('STC_LISP_OPERATOR', 'operator_style'),
                 ('STC_LISP_SPECIAL', 'operator_style'),
                 ('STC_LISP_STRING', 'string_style'),
                 ('STC_LISP_STRINGEOL', 'stringeol_style'),
                 ('STC_LISP_SYMBOL', 'scalar_style') ]

#---- Extra Properties ----#
FOLD = ('fold', '1')

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    if lang_id == synglob.ID_LANG_LISP:
        return [LISP_FUNC, LISP_KEYWORDS]
    elif lang_id == synglob.ID_LANG_SCHEME:
        return [SCHEME_KW]
    elif lang_id == synglob.ID_LANG_NEWLISP:
        return [NEWLISP_FUNC, NEWLISP_KEYWORDS]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    if lang_id in [ synglob.ID_LANG_LISP,
                    synglob.ID_LANG_SCHEME,
                    synglob.ID_LANG_NEWLISP]:
        return SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    if lang_id in [ synglob.ID_LANG_LISP,
                    synglob.ID_LANG_SCHEME,
                    synglob.ID_LANG_NEWLISP]:
        return [FOLD]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    if lang_id in [ synglob.ID_LANG_LISP,
                    synglob.ID_LANG_SCHEME,
                    synglob.ID_LANG_NEWLISP]:
        return [u';']
    else:
        return list()

#---- End Required Module Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString(option=0):
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#

#-----------------------------------------------------------------------------#
