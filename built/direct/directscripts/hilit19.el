;;; hilit19.el --- customizable highlighting for Emacs 19

;; Copyright (C) 1993, 1994, 2001, 2002, 2003, 2004,
;;   2005, 2006, 2007, 2008 Free Software Foundation, Inc.

;; Author:   Jonathan Stigelman <stig@hackvan.com>
;; Maintainer: FSF
;;	(actually no longer maintained)
;; Keywords: faces

;; This file is part of GNU Emacs.

;; GNU Emacs is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 3, or (at your option)
;; any later version.

;; GNU Emacs is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GNU Emacs; see the file COPYING.  If not, write to the
;; Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
;; Boston, MA 02110-1301, USA.

;;; Commentary:

;; Hilit19.el is a customizable highlighting package for Emacs 19.  It supports
;; not only source code highlighting, but also Info, RMAIL, VM, gnus...
;; Hilit19 knows (or thinks it knows) how to highlight emacs buffers in
;; about 25 different modes.
;;
;; WHERE TO GET THE LATEST VERSIONS OF HILIT19.EL (beta and release):
;;
;;      http://hackvan.com/pub/stig/src/elisp/
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; TO SUBMIT BUG REPORTS (or feedback of any sort)...
;;
;;    M-x hilit-submit-feedback RET
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; hilit19.el,v 2.19 1993/09/08 18:44:10 stig Release
;;
;; LCD Archive Entry:
;; hilit19|Jonathan Stigelman|stig@hackvan.com|
;; Comprehensive (and comparatively fast) regex-based highlighting for Emacs 19|
;; 1993/09/08 18:44:10|Release 2.19|~/packages/hilit19.el.Z|
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; GENERAL OVERVIEW
;;
;;      This package installs numerous hooks to colorfully highlight your
;;      source code buffers as well as mail and news buffers.  Most
;;      programming languages have predefined highlighting patterns.
;;	Just load hilit19 and files will be automatically highlighted as
;;      they're loaded.
;;
;;      Rehighlight a buffer by typing C-S-l (control-shift-lowercase-L).
;;
;;      If, when you edit the buffer, the coloring gets messed up, just
;;      redraw and the coloring will be adjusted.  If automatic highlighting
;;      in the current buffer has been turned off, then typing C-u C-S-l will
;;	force a rehighlight of the entire buffer.
;;
;;      Hilit19 can build faces by examining the names that you give to them
;;	For example, green/black-bold-italic-underline would be created as
;;	a face with a green foreground, and a black background, using a
;;	bold-italic font...with underlining for good measure.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; SETUP -- In your .emacs:
;;
;;
;; (cond (window-system
;;        (setq hilit-mode-enable-list  '(not text-mode)
;;              hilit-background-mode   'light
;;              hilit-inhibit-hooks     nil
;;              hilit-inhibit-rebinding nil)
;;
;;        (require 'hilit19)
;;        ))
;;
;; If you like font-lock-mode and want to use both packages, then you can
;; disable hilit for the modes in which you want to use font-lock by listing
;; said modes in hilit-mode-enable-list.
;;
;;      (hilit-translate type     'RoyalBlue   ; enable highlighting in C/C++
;;			 string	  nil)         ; disable string highlighting
;;
;; To get 100% of the utility of hilit19, you may also have to apply the
;; patches below for info.el and vm5.33L_19/vm-summary.el
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; SETUP -- Are you using the right font for Emacs?
;;
;; Emacs cannot properly find bold and italic fonts unless you specify a
;; verbose X11 font name.  If you specify a font for emacs in your
;; .Xdefaults, it *MUST* be specified using the long form of the font name.
;; Here's a good font menu:
;;
;; (setq
;;  x-fixed-font-alist
;;  '("Font Menu"
;;    ("Misc"
;;     ("6x12" "-misc-fixed-medium-r-semicondensed--12-110-75-75-c-60-*-1")
;;     ("6x13" "-misc-fixed-medium-r-semicondensed--13-120-75-75-c-60-*-1")
;;     ("lucida 13"
;;      "-b&h-lucidatypewriter-medium-r-normal-sans-0-0-0-0-m-0-*-1")
;;     ("7x13" "-misc-fixed-medium-r-normal--13-120-75-75-c-70-*-1")
;;     ("7x14" "-misc-fixed-medium-r-normal--14-130-75-75-c-70-*-1")
;;     ("9x15" "-misc-fixed-medium-r-normal--15-140-*-*-c-*-*-1")
;;     ("")
;;     ("clean 8x8" "-schumacher-clean-medium-r-normal--*-80-*-*-c-*-*-1")
;;     ("clean 8x14" "-schumacher-clean-medium-r-normal--*-140-*-*-c-*-*-1")
;;     ("clean 8x10" "-schumacher-clean-medium-r-normal--*-100-*-*-c-*-*-1")
;;     ("clean 8x16" "-schumacher-clean-medium-r-normal--*-160-*-*-c-*-*-1")
;;     ("")
;;     ("sony 8x16" "-sony-fixed-medium-r-normal--16-120-100-100-c-80-*-1")
;;     ("")
;;     ("-- Courier --")
;;     ("Courier 10" "-adobe-courier-medium-r-normal--*-100-*-*-m-*-*-1")
;;     ("Courier 12" "-adobe-courier-medium-r-normal--*-120-*-*-m-*-*-1")
;;     ("Courier 14" "-adobe-courier-medium-r-normal--*-140-*-*-m-*-*-1")
;;     ("Courier 18" "-adobe-courier-medium-r-normal--*-180-*-*-m-*-*-1")
;;     ("Courier 18-b" "-adobe-courier-bold-r-normal--*-180-*-*-m-*-*-1")
;;     )))
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; KNOWN BUGS/TO DO LIST/HELP WANTED/APPLY WITHIN
;;
;; * unbalanced, unescaped double quote characters can confuse hilit19.
;;   This will be fixed someday, so don't bug me about it.
;;
;; * ALTHOUGH HILIT19 IS FASTER THAN FONT-LOCK-MODE...
;;   For various reasons, the speed of the package could still stand to be
;;   improved.  If you care to do a little profiling and make things tighter...
;;
;; * hilit-toggle-highlight is flaky when auto-rehighlight is neither t nor nil.
;;   Does anyone actually USE this?  I think I might just remove it.
;;
;; PROJECTS THAT YOU CAN TAKE OVER BECAUSE I DON'T MUCH CARE ABOUT THEM...
;;
;; * Moved hilit-wysiwyg-replace here from my version of man.el, this is not
;;   a bug.  The bug is that I don't have a reverse operation yet...just a
;;   stub Wysiwyg-anything really belongs in a package of its own.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Thanks to the following people for their input:
;;    ebert@enpc.enpc.fr (Rolf EBERT), ada, LaTeX & bibtex highlights
;;    Vivek Khera <khera@cs.duke.edu>, gnus hooks + random advice & patches
;;    brian@athe.WUstl.EDU (Brian Dunford-Shore), prolog highlights
;;    John Ladwig <jladwig@soils.umn.edu>, 1st pass nroff highlights
;;    campo@sunthpi3.difi.unipi.it (Massimo Campostrini), fortran highlights
;;    jayb@laplace.MATH.ColoState.EDU (Jay Bourland), 1st pass dired
;;    Yoshio Turner <yoshio@CS.UCLA.EDU>, modula 2 highlights
;;    Fritz Knabe <knabe@ecrc.de>, advice & patches
;;    Alon Albert <alon@milcse.rtsg.mot.com>, advice & patches
;;    dana@thumper.bellcore.com (Dana A. Chee), working on the multi-frame bug
;;    derway@ndc.com (Don Erway), for breaking it...
;;    moss_r@summer.chem.su.oz.au (Richard Moss), first pass at add-pattern
;;    Olivier Lecarme <ol@aiguemarine.unice.fr>, Pascal & Icon patterns
;;
;; With suggestions and minor regex patches from numerous others...
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; hilit19.el,v
;; Revision 2.19  1993/09/08  18:44:10  stig
;; installed patch for elusive bug in hilit-rehighlight-region that caused
;; hilit-unhighlight-region to hang in an infinite loop.
;;
;; Revision 2.18  1993/08/27  03:51:00  stig
;; minor mods to lisp-mode and c/c++ mode patterns
;;
;; Revision 2.17  1993/08/25  02:19:17  stig
;; work-around for bug in next-overlay-change that caused dired and jargon-mode
;; to hang in an endless loop.  Perhaps other modes were doing this too.
;;
;; Revision 2.16  1993/08/22  19:46:00  stig
;; bug fix for next-overlay-change and accompanying change to
;; hilit-unhighlight-region
;;
;; Revision 2.15  1993/08/20  12:16:22  stig
;; minor change to fortran patterns
;;
;; Revision 2.14  1993/08/17  14:12:10  stig
;; added default face mapping for 'formula' which is needed for new latex
;; patterns.
;;
;; twiddled the calendar-mode patterns a bit.
;;
;; Revision 2.13  1993/08/16  04:33:54  stig
;; hilit-set-mode-patterns was screwing up two part patterns.  it doesn't now.
;;
;; Revision 2.12  1993/08/16  00:16:41  stig
;; changed references to default-bold-italic to just bold-italic because the
;;   font for that face is maintained by emacs.
;;
;; the pattern matcher now starts its searches from the end of the most
;;   recently highlighted region (which is not necessarily the end of the most
;;   recently matched regex).
;;
;; multiple errors in pattern matcher now just give an error instead of lots of
;;   annoying messages and dings.
;;
;; no longer use vm-summary-mode-hooks.
;;
;; some code moved from hilit-highlight-region to hilit-set-mode-patterns.
;;   This will affect you if you pass your patterns directly to
;;   hilit-highlight-region....use a pseudo-mode instead.
;;
;; pattern changes to C/C++, latex, texinfo, fortran, nroff, etc.
;;
;; Revision 2.11  1993/08/13  12:12:37  stig
;; removed some crufty commented-out code
;;
;; diverged lisp-mode and emacs-lisp-mode...also added lisp keywords.
;;
;; Revision 2.10  1993/08/13  09:47:06  stig
;; added calendar-mode, icon-mode and pascal-mode patterns
;;
;; commented out hilit-toggle-highlight because I want to phase it out entirely
;;
;; Revision 2.9  1993/08/13  08:44:22  stig
;; added optional case-fold argument to hilit-set-mode-patterns, this case-fold
;; parameter is now stored in hilit-patterns-alist.
;;
;; Revision 2.8  1993/08/12  22:05:03  stig
;; fixed some typos in documentation
;;
;; twiddled some of the color defaults for dark backgrounds
;;
;; always get 'mono color defaults if (not (x-display-color-p))
;;
;; added hilit-rehighlight-buffer-quietly to dired-after-readin-hook
;;
;; fixed bug in hilit-string-find that mishandled strings of the form: "\\"
;;
;; NEW FUNCTION: hilit-add-mode-pattern...  kinda like add-hook for patterns
;;
;; fixed minor pattern bugs for latex-mode and emacs-lisp-mode
;;
;; Revision 2.7  1993/07/30  02:43:01  stig
;; added const to the list of modifiers for C/C++ types
;;
;; Revision 2.6  1993/07/30  00:30:54  stig
;; now permit selection of arbitrary subexpressions for highlighting...
;; fixed keyword patterns for C/C++ using this technique.
;;
;; Revision 2.5  1993/07/28  05:02:56  stig
;; improvements to makefile regular expressions
;; removed about 130 lines just by compacting the big defconst for
;;   hilit-face-translation-table into a mapcar and defining a separate table
;;   of default faces.
;;
;; Revision 2.4  1993/07/27  14:09:05  stig
;; documented another "known problem" to "head off gripe mail at the pass."
;;
;; Revision 2.3  1993/07/27  02:15:49  stig
;; (hilit-lookup-face-create) incorporated patch which improves its behavior
;; with more than one frame...  Still can't have bold on the same face in two
;; different fonts sizes at the same time...
;;
;; Revision 2.2  1993/07/27  02:02:59  stig
;; vastly improved the makefile patterns
;; added hook for mh-show-mode
;;
;; Revision 2.1  1993/07/24  17:46:21  stig
;; Phasing out Info-select-hook...  Version 19.18 will use Info-selection-hook.
;;
;; Revision 2.0  1993/07/24  13:50:10  stig
;; better documentation and added the function hilit-submit-feedback.
;; C-S-l (control shift l) repaints the buffer.  Other bindings are optional.
;; multi-line highlights no longer cause problems when
;;   hilit-auto-rehighlight is 'visible
;; added hilit-predefined-face-list...
;; changed name of hilit-mode-alist to hilit-patterns-alist
;; added hilit-message-quietly to mail-setup-hook
;; added hilit-parser-alist which can be used to apply different patterns to
;;   different parts of a buffer.  This could be integrated in a far more
;;   elegant manner, but it presently serves the purpose of not applying
;;   message header patterns to message bodies in mail-mode and its kin.
;; hilit-set-mode-patterns now takes a list of modes and an optional parse-fn
;;

;;;;;; AND THIS CAN BE APPLIED TO VM 5.33L_19
;;
;; *** ../site/vm5.33L_19/vm-summary.el    Fri Jun  4 22:17:11 1993
;; --- ./vm-summary.el     Tue Jun 22 16:39:30 1993
;; ***************
;; *** 152,158 ****
;;                   (insert "->")
;;                   (delete-char 2)
;;                   (forward-char -2)
;; !                 (and w vm-auto-center-summary (vm-auto-center-summary))))
;;             (and old-window (select-window old-window)))))))
;;
;;   (defun vm-mark-for-display-update (message)
;; --- 152,159 ----
;;                   (insert "->")
;;                   (delete-char 2)
;;                   (forward-char -2)
;; !                 (and w vm-auto-center-summary (vm-auto-center-summary))
;; !                 (run-hooks 'vm-summary-pointer-hook)))
;;             (and old-window (select-window old-window)))))))
;;
;;   (defun vm-mark-for-display-update (message)
;;
;;;;;;

;;; Code:

;; User Options:

(defvar hilit-quietly nil
  "* If non-nil, this inhibits progress indicators during highlighting")

(defvar hilit-auto-highlight t
  "* t if we should highlight all buffers as we find 'em, nil to disable
  automatic highlighting by the find-file hook.")

(defvar hilit-auto-highlight-maxout 60000 ; hilit19 keeps getting bigger...
  "* auto-highlight is disabled in buffers larger than this")

(defvar hilit-auto-rehighlight t
  "* If this is non-nil, then hilit-redraw and hilit-recenter will also
  rehighlight part or all of the current buffer.  t will rehighlight the
  whole buffer, a NUMBER will rehighlight that many lines before and after
  the cursor, and the symbol 'visible' will rehighlight only the visible
  portion of the current buffer.  This variable is buffer-local.")

(make-variable-buffer-local 'hilit-auto-rehighlight)

(defvar hilit-auto-rehighlight-fallback '(20000 . 100)
  "* Cons of the form (THRESHOLD . FALLBACK), where FALLBACK is assigned to
  hilit-auto-rehighlight if the size of a newly opened buffer is larger than
  THRESHOLD.")

(defvar hilit-face-check t
  "* t slows down highlighting but permits the user to change fonts without
  losing bold and italic faces...  t causes hilit-lookup-face-create to dig
  through the frame parameters for the current window every time it's called.
  If you never change fonts in emacs, set this to nil.")

;; Variables which must be set before loading hilit19.

(defvar hilit-inhibit-rebinding nil
  "If non-nil, this inhibits replacement of recenter, yank, and yank-pop.")

(defvar hilit-inhibit-hooks nil
  "If non-nil, this inhibits installation of hooks for Info, gnus, & vm.")

(defvar hilit-background-mode 'light
  "'mono inhibits color, 'dark or 'light indicate the background brightness.")

(defvar hilit-mode-enable-list nil
  "If a list of modes to exclusively enable or specifically disable.
The sense of the list is negated if it begins with the symbol 'not'.
Set this variable before you load hilit19.

Ex:  (perl-mode jargon-mode c-mode)	; just perl, C, and jargon modes
     (not text-mode)			; all modes except text mode")

;; Variables that are not generally modified directly

(defvar hilit-parser-alist nil
  "alist of major-mode values and parsers called by hilit-rehighlight-buffer.

Parsers for a given mode are IGNORED for partial rehighlights...maybe you'd
like to make this more universal?")

(defvar hilit-patterns-alist nil
  "alist of major-mode values and default highlighting patterns

A highlighting pattern is a list of the form (start end face), where
start is a regex, end is either a regex or a match number for start, and face
is the name of an entry in hilit-face-translation-table, the name of a face,
or nil (which disables the pattern).

Each entry in the alist is of the form:
	(mode . (case-fold pattern [pattern ...]))

See the hilit-lookup-face-create documentation for valid face names.")

(defvar hilit-predefined-face-list (face-list)
  "List of faces with which hilit-lookup-face-create will NOT tamper.

If hilit19 is dumped into emacs at your site, you may have to set this in
your init file.")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Use this to report bugs:

(eval-when-compile (require 'reporter))	; no compilation gripes

(defun hilit-submit-feedback ()
  "Submit feedback on hilit19 to hilit@hackvan.com"
  (interactive)
  (require 'reporter)
  (and (y-or-n-p "Do you really want to submit a report on hilit19? ")
       (reporter-submit-bug-report
	"Jonathan Stigelman <hilit@hackvan.com>"
	"hilit19.el (Release 2.19)"
	(and (y-or-n-p "Do you need to include a dump hilit variables? ")
	     (append
	      '(
		hilit-quietly    		hilit-inhibit-hooks
		hilit-background-mode		hilit-mode-enable-list
		hilit-auto-highlight		hilit-auto-highlight-maxout
		hilit-auto-rehighlight		hilit-auto-rehighlight-fallback
		hilit-face-check
		)
	      (and (y-or-n-p "Have you modified the standard patterns? ")
		   (yes-or-no-p "Are your patterns *REALLY* relevant? ")
		   '(hilit-parser-alist
		     hilit-patterns-alist
		     hilit-predefined-face-list
		     ))))
	 (function
	  (lambda ()
	    (and (y-or-n-p "Is this a problem with font display? ")
		 (insert "\nFrame Configuration:\n====================\n"
			 (prin1-to-string (frame-configuration-to-register ?F))
			 "\n"
			 ))))
	 nil
	 (concat
	  "This is (check all that apply, and delete what's irrelevant):\n"
	  "  [ ] a _MASSIVE_THANK_YOU_ for writing hilit19.el\n"
	  "  [ ] An invitation to attend the next Hackers Conference\n"
	  "  [ ] You're a RIGHTEOUS HACKER, what are your rates?\n"
	  "  [ ] I've used the force and read the source, but I'M CONFUSED\n"
	  "  [ ] a PATCH. (output of 'diff -uw old.el new.el' or 'diff -cw')\n"
	  "  [ ] a SERIOUS AND REPRODUCIBLE BUG that is not an EMACS bug\n"
	  "     - I *swear* that it's not already mentioned in the KNOWN BUGS\n"
	  "     - I HAVE CHECKED ftp.hackvan.com:/pub/stig/src/elisp/hilit19.el.gz\n"
	  "       for a newer release that fixes the problem.\n"
	  "    >> I HAVE ALSO CHECKED ftp.hackvan.com:/pub/stig/src/elisp/hl319.el.gz\n"
	  "       This is the alpha version...what will become hilit19 (Beta 3.0).\n"
	  "\n"
	  "Hey Stig, I *know* you're busy but...\n"))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; These faces are either a valid face name, or nil
;; if you want to change them, you must do so AFTER hilit19 is loaded

(defconst hilit-default-face-table
  '(
    ;; used for C/C++ and Emacs Lisp and perl
    (comment	firebrick-italic    moccasin           italic)
    (include	purple		    Plum1	       bold-italic)
    (define	ForestGreen-bold    green	       bold)
    (defun	blue-bold	    cyan-bold	       bold-italic)
    (decl	RoyalBlue	    cyan	       bold)
    (type	nil		    yellow	       nil)
    (keyword	RoyalBlue	    cyan	       bold-italic)
    (label	red-underline	    orange-underlined  underline)
    (string	grey40		    orange	       underline)

    ;; some further faces for Ada
    (struct	  black-bold        white-bold	       bold)
    (glob-struct  magenta	    Plum1	       default-bold-underline)
    (named-param  DarkGoldenrod	    Goldenrod	       underline)

    ;; and another one for LaTeX
    (crossref	  DarkGoldenrod	    Goldenrod	       underline)
    (formula	  Goldenrod	    DarkGoldenrod      underline)

    ;; compilation buffers
    (active-error default/pink-bold  default/DeepPink-bold  default-underline)
    (error	  red-bold           yellow	       bold)
    (warning	  blue-italic	     green	       italic)

    ;; Makefiles (some faces borrowed from C/C++ too)
    (rule	  blue-bold-underline cyan-underline   default-bold-underline)

    ;; VM, GNUS and Text mode
    (msg-subject    blue-bold       yellow             bold)
    (msg-from	    purple-bold	    green	       bold)
    (msg-header	    firebrick-bold  cyan	       italic)
    (msg-separator  black/tan-bold  black/lightblue    nil)
    (msg-quote	    ForestGreen	    pink	       italic)

    (summary-seen     grey40	    white	       nil)
    (summary-killed   grey50	    white	       nil)
    (summary-Xed      OliveDrab2    green	       nil)
    (summary-deleted  firebrick	    white	       italic)
    (summary-unread   RoyalBlue	    yellow	       bold)
    (summary-new      blue-bold	    yellow-bold	       bold-italic)
    (summary-current  default/skyblue-bold green/dimgrey-bold reverse-default)

    (gnus-group-unsubscribed grey50		white	    nil)
    (gnus-group-empty	     nil		nil	    nil)
    (gnus-group-full	     ForestGreen	green	    italic)
    (gnus-group-overflowing  firebrick		red	    bold-italic)

    ;; dired mode
    (dired-directory blue-bold         cyan            bold)
    (dired-link	     firebrick-italic  green	       italic)
    (dired-ignored   ForestGreen       moccasin	       nil)
    (dired-deleted   red-bold-italic   orange	       bold-italic)
    (dired-marked    purple	       Plum1	       nil)

    ;; Info-mode, and jargon-mode.el and prep.ai.mit.edu:/pub/gnu/jargon*
    (jargon-entry    blue-bold	       cyan            bold)
    (jargon-xref     purple-bold       Plum1	       italic)
    (jargon-keyword  firebrick-underline yellow	       underline)
    )
  "alist of default faces (face . (light-default dark-default mono-default))

There is no way for the user to modify this table such that it will have any
effect upon the translations used by hilit19.  Instead, use the function
hilit-translate AFTER hilit19 has been loaded.

See also the documentation for hilit-lookup-face-create.")

(defconst hilit-face-translation-table
  (let ((index (or (and (x-display-color-p)
			(cdr (assq hilit-background-mode
				   '((light . 1) (dark . 2)))))
		   3)))
    (mapcar (function (lambda (x) (cons (car x) (nth index x))))
	    hilit-default-face-table))
  "alist that maps symbolic face-names to real face names")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; To translate one face to another...
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defmacro hilit-translate (&rest args)
  "(hilit-translate FROM TO FROM TO ...): translate each face FROM to the
value of its TO face.  This is like setq for faces.

The function hilit-lookup-face-create will repeatedly translate until no more
translations for the face exist in the translation table.

See the documentation for hilit-lookup-face-create for names of valid faces."
  (or (zerop (% (length args) 2))
      (error "wrong number of args"))
  (let (cmdl from to)
    (while args
      (setq from (car args) to (nth 1 args) args (nthcdr 2 args)
	    cmdl (cons (list 'hilit-associate ''hilit-face-translation-table
			     (list 'quote from) to)
		       cmdl)))
    (cons 'progn cmdl)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; This function actually translates and then creates the faces...
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defun hilit-lookup-face-create (face &optional force)
  "Get a FACE, or create it if it doesn't exist.  In order for it to
properly create the face, the following naming convention must be used:
    [reverse-](fgcolor[/bgcolor])[-bold][-italic][-underline]
Example: (hilit-lookup-face-create 'comment-face) might create and return 'red

Each color is either the name of an X color (see .../X11/lib/X11/rgb.txt),
a hexadecimal specification of the form \"hex-[0-9A-Fa-f]+\", or \"default\".

An optional argument, FORCE, will cause the face to be recopied from the
default...which is probably of use only if you've changed fonts.

See the documentation for hilit-translate and hilit-face-translation-table."

;; translate the face ...
  (let ((trec t) visited)
    (while trec
      (cond ((memq face visited) (error "face translation loop: %S" visited))
	    (t (setq visited (cons face visited)
		     trec (assq face hilit-face-translation-table))
	       (and trec (setq face (cdr trec)))))))

  ;; make the face if we need to...
  (let* ((fn (symbol-name face))
	 (frame (selected-frame))
	 (basefont (cdr (assq 'font (frame-parameters frame))))
	 error fgcolor bgcolor)
    (cond
     ((or (null face)
	  (memq face hilit-predefined-face-list))
      ;; do nothing if the face is nil or if it's predefined.
      )
     ((or force
	  (not (memq face (face-list)))
	  (and hilit-face-check
	       (not (string= (get face 'basefont) basefont))))
      (copy-face 'default 'scratch-face)
      (if (string-match "^reverse-?" fn)
	  (progn (invert-face 'scratch-face)
		 (setq fn (substring fn (match-end 0)))))

      ;; parse foreground color
      (if (string-match "^\\(hex-\\)?\\([A-Za-z0-9]+\\)" fn)
	  (setq fgcolor (concat
			 (if (match-beginning 1) "#")
			 (substring fn (match-beginning 2) (match-end 2)))
		fn (substring fn (match-end 0)))
	(error "bad face name %S" face))

      ;; parse background color
      (if (string-match "^/\\(hex-\\)?\\([A-Za-z0-9]+\\)" fn)
	  (setq bgcolor (concat
			 (and (match-beginning 1) "#")
			 (substring fn (match-beginning 2) (match-end 2)))
		fn (substring fn (match-end 0))))

      (and (string= "default" fgcolor) (setq fgcolor nil))
      (and (string= "default" bgcolor) (setq bgcolor nil))

      ;; catch errors if we can't allocate the color(s)
      (condition-case nil
	  (progn (and fgcolor (set-face-foreground 'scratch-face fgcolor))
		 (and bgcolor (set-face-background 'scratch-face bgcolor))
		 (copy-face 'scratch-face face)
		 (put face 'basefont basefont))
	(error (message "couldn't allocate color for '%s'"
			(symbol-name face))
	       (setq face 'default)
	       (setq error t)))
      (or error
	  ;; don't bother w/ bold or italic if we didn't get the color
	  ;; we wanted, but ignore errors making the face bold or italic
	  ;; if the font isn't available, there's nothing to do about it...
	  (progn
	    (when (display-graphic-p frame)
	      (set-face-font face basefont frame))
	    (set-face-underline-p face (string-match "underline" fn))
	    (if (string-match ".*bold" fn)
		;; make face bold in all frames
		(make-face-bold face nil 'noerr))
	    (if (string-match ".*italic" fn)
		;; make face italic in all frames
		(make-face-italic face nil 'noerr))
	    ))
      )))
  face)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Region Highlight/Unhighlight code (Both overlay and text-property versions)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defsubst hilit-region-set-face (start end face-name &optional prio prop)
  "Highlight region from START to END using FACE and, optionally, PRIO.
The optional 5th arg, PROP is a property to set instead of 'hilit."
  (let ((overlay (make-overlay start end)))
    (overlay-put overlay 'face face-name)
    (overlay-put overlay (or prop 'hilit) t)
    (and prio (overlay-put overlay 'priority prio))))

(defun hilit-unhighlight-region (start end &optional quietly)
  "Unhighlights the region from START to END, optionally in a QUIET way"
  (interactive "r")
  (or quietly hilit-quietly (message "Unhighlighting"))
  (let ((lstart 0))
    (while (and start (> start lstart) (< start end))
      (mapcar (function (lambda (ovr)
			  (and (overlay-get ovr 'hilit) (delete-overlay ovr))))
	      (overlays-at start))
      (setq lstart start start (next-overlay-change start))))
  (or quietly hilit-quietly (message "Done unhighlighting")))

;;;; These functions use text properties instead of overlays.  Text properties
;;;; are copied through kill and yank...which might be convenient, but is not
;;;; terribly efficient as of 19.12, ERGO it's been disabled
;;
;;(defsubst hilit-region-set-face (start end face-name &optional prio prop)
;;  "Highlight region from START to END using FACE and, optionally, PRIO.
;;The optional 5th arg, PROP is a property to set instead of 'hilit."
;;    (put-text-property start end 'face face-name)
;;    )
;;
;;(defun hilit-unhighlight-region (start end &optional quietly)
;;  "Unhighlights the region from START to END, optionally in a QUIET way"
;;  (interactive "r")
;;  (let ((buffer-read-only nil)
;;	(bm (buffer-modified-p)))
;;    (remove-text-properties start end '(face))
;;    (set-buffer-modified-p bm)))
;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Pattern Application code and user functions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defun hilit-highlight-region (start end &optional patterns quietly)
  "Highlights the area of the buffer between START and END (the region when
interactive).  Without the optional PATTERNS argument, the pattern for
major-mode is used.  If PATTERNS is a symbol, then the patterns associated
with that symbol are used.  QUIETLY suppresses progress messages if
non-nil."
  (interactive "r")
  (cond ((null patterns)
	 (setq patterns (cdr (assq major-mode hilit-patterns-alist))))
	((symbolp patterns)
	 (setq patterns (cdr (assq patterns hilit-patterns-alist)))))
  ;; txt prop: (setq patterns (reverse patterns))
  (let ((case-fold-search (car patterns))
	(prio (1- (length patterns)))
	;; txt prop: (buffer-read-only nil)
	;; txt prop: (bm (buffer-modified-p))
	p pstart pend face mstart (puke-count 0))
    ;; txt prop: (unwind-protect
    (setq patterns (cdr patterns))	; remove case-fold from head of pattern
    (save-excursion
      (save-restriction
	(narrow-to-region start end)
	(while patterns
	  (setq p (car patterns))
	  (setq pstart (car p)
		pend (nth 1 p)
		face (hilit-lookup-face-create (nth 2 p)))
	  (if (not face)		; skipped if nil
	      nil
	    (or quietly hilit-quietly
		(message "highlighting %d: %s%s" prio pstart
			 (if (stringp pend) (concat " ... " pend) "")))
	    (goto-char (point-min))
	    (condition-case msg
		(cond
		      ((symbolp pstart)
		       ;; inner loop -- special function to find pattern
		       (let (region)
			 (while (setq region (funcall pstart pend))
			   (hilit-region-set-face (car region) (cdr region)
						  face prio))))
		      ((stringp pend)
		       ;; inner loop -- regex-start ... regex-end
		       (while (re-search-forward pstart nil t nil)
			 (goto-char (setq mstart (match-beginning 0)))
			 (if (re-search-forward pend nil t nil)
			     (hilit-region-set-face mstart (match-end 0)
						    face prio)
			   (forward-char 1))))
		      ((numberp pend)
		       ;; inner loop -- just one regex to match whole pattern
		       (while (re-search-forward pstart nil t nil)
			 (goto-char (match-end pend))
			 (hilit-region-set-face  (match-beginning pend)
						 (match-end pend) face prio)))
		      (t (error "malformed pattern")))
	      (error (if (> (setq puke-count (1+ puke-count)) 1)
			 (error msg)
		       (message "Error: '%s'" msg)
		       (ding) (sit-for 4)))))
	  (setq prio (1- prio)
		patterns (cdr patterns)))
	))
    (or quietly hilit-quietly (message "")) ; "Done highlighting"
    ;; txt prop: (set-buffer-modified-p bm)) ; unwind protection
    ))

(defun hilit-rehighlight-region (start end &optional quietly)
  "Re-highlights the region, optionally in a QUIET way"
  (interactive "r")
  (save-restriction
    (widen)
    (setq start (apply 'min start (mapcar 'overlay-start (overlays-at start)))
	  end (apply 'max end (mapcar 'overlay-end (overlays-at end))))
    (hilit-unhighlight-region start end quietly)
    (hilit-highlight-region   start end nil quietly)))

(defun hilit-rehighlight-buffer (&optional quietly)
  "Re-highlights the buffer, optionally in a QUIET way"
  (interactive "")
  (let ((parse-fn (cdr (assq major-mode hilit-parser-alist))))
    (if parse-fn
	(funcall parse-fn quietly)
      (hilit-rehighlight-region (point-min) (point-max) quietly)))
  nil)

(defun hilit-rehighlight-buffer-quietly ()
  (hilit-rehighlight-buffer t))

(defun hilit-rehighlight-message (quietly)
  "Highlight a buffer containing a news article or mail message."
  (save-excursion
    (goto-char (point-min))
    ;; find separation between headers and body (either a blank line or
    ;; the message separator line in mail-mode)
    (re-search-forward "^\\(\\|--text follows this line--\\)$" nil 'noerr)
    (hilit-unhighlight-region (point-min) (point-max) quietly)
    (hilit-highlight-region (point-min) (point) 'msg-header quietly)
    (hilit-highlight-region (point) (point-max) 'msg-body quietly)))

(defalias 'hilit-highlight-buffer 'hilit-rehighlight-buffer)

;; Well, I want to remove this function...there's one sure way to find out if
;; anyone uses it or not...and that's to comment it out.
;;
;; (defun hilit-toggle-highlight (arg)
;;   "Locally toggle highlighting.  With arg, forces highlighting off."
;;   (interactive "P")
;;   ;; FIXME -- this loses numeric information in hilit-auto-rehighlight
;;   (setq hilit-auto-rehighlight
;;         (and (not arg) (not hilit-auto-rehighlight)))
;;   (if hilit-auto-rehighlight
;;       (hilit-rehighlight-buffer)
;;     (hilit-unhighlight-region (point-min) (point-max)))
;;   (message "Rehighlighting is set to %s" hilit-auto-rehighlight))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; HOOKS
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defun hilit-find-file-hook ()
  "Find-file hook for hilit package.  See the variable hilit-auto-highlight."
  (cond ((and hilit-auto-highlight
	      (assq major-mode hilit-patterns-alist))
	 (if (> buffer-saved-size (car hilit-auto-rehighlight-fallback))
	     (setq hilit-auto-rehighlight
		   (cdr hilit-auto-rehighlight-fallback)))
	 (if (> buffer-saved-size hilit-auto-highlight-maxout)
	     nil
	   (let ((bm (buffer-modified-p)))
	     (hilit-rehighlight-buffer)
	     (set-buffer-modified-p bm))))))

(defun hilit-repaint-command (arg)
  "Rehighlights according to the value of hilit-auto-rehighlight, or the
prefix argument if that is specified.
\t\\[hilit-repaint-command]\t\trepaint according to hilit-auto-rehighlight
\t^U \\[hilit-repaint-command]\trepaint entire buffer
\t^U - \\[hilit-repaint-command]\trepaint visible portion of buffer
\t^U n \\[hilit-repaint-command]\trepaint n lines to either side of point"
  (interactive "P")
  (let (st en quietly)
    (or arg (setq arg hilit-auto-rehighlight))
    (cond ((or (eq  arg 'visible) (eq arg '-))
	   (setq st (window-start) en (window-end) quietly t))
	  ((numberp arg)
	   (setq st (save-excursion (forward-line (- arg)) (point))
		 en (save-excursion (forward-line arg) (point))))
	  (arg
	   (hilit-rehighlight-buffer)))
    (if st
	  (hilit-rehighlight-region st en quietly))))

(defun hilit-recenter (arg)
  "Recenter, then rehighlight according to hilit-auto-rehighlight.  If called
with an unspecified prefix argument (^U but no number), then a rehighlight of
the entire buffer is forced."
  (interactive "P")
  (recenter arg)
  ;; force display update
  (sit-for 0)
  (hilit-repaint-command (consp arg)))

(defun hilit-yank (arg)
  "Yank with rehighlighting"
  (interactive "*P")
  (let ((transient-mark-mode nil))
    (yank arg)
    (and hilit-auto-rehighlight
	 (hilit-rehighlight-region (region-beginning) (region-end) t))
    (setq this-command 'yank)))

(defun hilit-yank-pop (arg)
  "Yank-pop with rehighlighting"
  (interactive "*p")
  (let ((transient-mark-mode nil))
    (yank-pop arg)
    (and hilit-auto-rehighlight
	 (hilit-rehighlight-region (region-beginning) (region-end) t))
    (setq this-command 'yank)))

;;; this line highlighting stuff is untested.  play with it only if you feel
;;; adventurous...don't ask me to fix it...though you're welcome to.  -- Stig
;;
;; (defun hilit-rehighlight-line-quietly (&rest args)
;;   "Quietly rehighlight just this line.
;; Useful as an after change hook in VM/gnus summary buffers and dired buffers.
;; If only there were an after-change-function, that is..."
;;   (save-excursion
;;     (push-mark nil t)
;;     (hilit-rehighlight-yank-region)
;;     (and orig-achange-function (apply orig-achange-function args))))
;;
;; (defun hilit-install-line-hooks ()
;;   (make-variable-buffer-local 'after-change-function)
;;   (make-local-variable 'orig-achange-function)
;;   (setq orig-achange-function after-change-function)
;;   (setq after-change-function 'hilit-rehighlight-line-quietly))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Wysiwyg Stuff...  take it away and build a whole package around it!
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; ; For the Jargon-impaired, WYSIWYG === What You See Is What You Get
;; ; Sure, it sucks to type.  Oh, well.
;; (defun hilit-wysiwyg-replace ()
;;   "Replace overstruck text with normal text that's been overlaid with the
;; appropriate text attribute.  Suitable for a find-file hook."
;;   (save-excursion
;;     (goto-char (point-min))
;;     (let ((wysb (hilit-lookup-face-create 'wysiwyg-bold))
;; 	  (wysu (hilit-lookup-face-create 'wysiwyg-underline))
;; 	  (bmod (buffer-modified-p)))
;;       (while (re-search-forward "\\(.\b.\\)+" nil t)
;; 	(let ((st (match-beginning 0)) (en (match-end 0)))
;; 	  (goto-char st)
;; 	  (if (looking-at "_")
;; 	      (hilit-region-set-face st en wysu 100 'wysiwyg)
;; 	    (hilit-region-set-face st en wysb 100 'wysiwyg))
;; 	  (while (and (< (point) en) (looking-at ".\b"))
;; 	    (replace-match "") (forward-char))
;; 	  ))
;;       (set-buffer-modified-p bmod))))
;;
;; ; is this more appropriate as a write-file-hook or a write-contents-hook?
;; (defun hilit-wysiwyg-write-repair ()
;;   "Replace wysiwyg overlays with overstrike text."
;;   (message "*sigh* hilit-wysiwyg-write-repair not implemented yet")
;;
;; For efficiency, this hook should copy the current buffer to a scratch
;; buffer and do its overstriking there.  Overlays are not copied, so it'll
;; be necessary to hop back and forth.  This is OK since you're not fiddling
;; with--making or deleting--any overlays.  THEN write the new buffer,
;; delete it, and RETURN T. << important
;;
;; Just so you know...there is already an emacs function called
;; underline-region that does underlining.  I think that the thing to do is
;; extend that to do overstriking as well.
;;
;;  (while (< start end)
;;    (mapcar (function (lambda (ovr)
;;			  (and (overlay-get ovr 'hilit) (delete-overlay ovr))))
;;	    (overlays-at start))
;;    (setq start (next-overlay-change start)))
;;  nil)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Initialization.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-minor-mode hilit-mode
  "Obsolete minor mode.  Use `global-font-lock-mode' instead."
  :global t

  (unless (and hilit-inhibit-rebinding hilit-mode)
    (substitute-key-definition
     (if hilit-mode 'yank 'hilit-yank)
     (if hilit-mode 'hilit-yank 'yank)
     (current-global-map))
    (substitute-key-definition
     (if hilit-mode 'yank-pop 'hilit-yank-pop)
     (if hilit-mode 'hilit-yank-pop 'yank-pop)
     (current-global-map))
    (substitute-key-definition
     (if hilit-mode 'recenter 'hilit-recenter)
     (if hilit-mode 'hilit-recenter 'recenter)
     (current-global-map)))

  (if hilit-mode
      (global-set-key [?\C-\S-l] 'hilit-repaint-command)
    (global-unset-key [?\C-\S-l]))

  (if hilit-mode
      (add-hook 'find-file-hook 'hilit-find-file-hook t)
    (remove-hook 'find-file-hook 'hilit-find-file-hook))

  (unless (and hilit-inhibit-hooks hilit-mode)
    (condition-case c
	(progn

	  ;; BUFFER highlights...
	  (mapcar (lambda (hook)
		    (if hilit-mode
			(add-hook hook 'hilit-rehighlight-buffer-quietly)
		      (remove-hook hook 'hilit-rehighlight-buffer-quietly)))
		  '(
		    Info-selection-hook

		    ;; runs too early		     vm-summary-mode-hooks
		    vm-summary-pointer-hook
		    vm-preview-message-hook
		    vm-show-message-hook

		    rmail-show-message-hook
		    mail-setup-hook
		    mh-show-mode-hook

		    dired-after-readin-hook
		    ))
	  )
      (error (message "Error loading highlight hooks: %s" c)
	     (ding) (sit-for 1)))))

(eval-when-compile (require 'gnus))	; no compilation gripes

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Default patterns for various modes.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;; do I need this?  I changed the defconst to a defvar because defconst is
;;; inappropriate, but I don't know why I wanted hilit-patterns-alist to be
;;; reset on every reload...

(setq hilit-patterns-alist nil)

(defun hilit-associate (alist key val)
  "creates, or destructively replaces, the pair (key . val) in alist"
  (let ((oldentry (assq key (eval alist))))
    (if oldentry
	(setcdr oldentry val)
      (set alist (cons (cons key val) (eval alist))))))

(defun hilit-set-mode-patterns (modelist patterns
					 &optional parse-fn case-fold)
  "Sets the default highlighting patterns for MODE to PATTERNS.
See the variable hilit-mode-enable-list.

Takes optional arguments PARSE-FN and CASE-FOLD."
  ;; change pattern
  (mapcar (function (lambda (p)
		      (and (stringp (car p))
			   (null (nth 1 p))
			   (setcar (cdr p) 0))))
	  patterns)
  (setq patterns (cons case-fold patterns))

  (or (consp modelist) (setq modelist (list modelist)))
  (let (ok (flip (eq (car hilit-mode-enable-list) 'not)))
    (mapcar (function
	     (lambda (m)
	       (setq ok (or (null hilit-mode-enable-list)
			    (memq m hilit-mode-enable-list)))
	       (and flip (setq ok (not ok)))
	       (and ok
		    (progn
		      (and parse-fn
			   (hilit-associate 'hilit-parser-alist m parse-fn))
		      (hilit-associate 'hilit-patterns-alist m patterns)))))
	    modelist)))

(defun hilit-add-pattern (pstart pend face &optional mode first)
  "Highlight pstart with face for the current major-mode.
Optionally, place the new pattern first in the pattern list"
  (interactive "sPattern start regex: \nsPattern end regex (default none): \nxFace: ")

  (and (equal pstart "") (error "Must specify starting regex"))
  (cond ((equal pend "") (setq pend 0))
	((string-match "^[0-9]+$" pend) (setq pend (string-to-number pend))))
  (or mode (setq mode major-mode))
  (let ((old-patterns (cdr (assq mode hilit-patterns-alist)))
	(new-pat (list pstart pend face)))
    (cond ((not old-patterns)
	   (hilit-set-mode-patterns mode (list new-pat)))
	  (first
	   (setcdr old-patterns (cons new-pat (cdr old-patterns))))
	  (t
	   (nconc old-patterns (list new-pat)))))
  (and (interactive-p) (hilit-rehighlight-buffer)))

(defun hilit-string-find (qchar)
  "Looks for a string and returns (start . end) or nil.  The argument QCHAR
is the character that would precede a character constant double quote.
Finds strings delimited by double quotes.  The first double quote may not be
preceded by QCHAR and the closing double quote may not be preceded by an odd
number of backslashes."
  (let (st en)
    (while (and (search-forward "\"" nil t)
		(eq qchar (char-after (1- (setq st (match-beginning 0)))))))
    (while (and (search-forward "\"" nil t)
		(save-excursion
		  (setq en (point))
		  (forward-char -1)
		  (skip-chars-backward "\\\\")
		  (forward-char 1)
		  (not (zerop (% (- en (point)) 2))))))
    (and en (cons st en))))

;; return types on same line...
;; ("^[a-zA-z].*\\(\\w\\|[$_]\\)+\\s *\\(\\(\\w\\|[$_]\\)+\\s *((\\|(\\)[^)]*)+" nil defun)

;; On another note, a working pattern for grabbing function definitions for C is
;;
;; ("^[a-zA-Z_]+.*[;{]$" nil ForestGreen)  ; global defns ( start at col 1 )
;; ("^[a-zA-Z_]+.*(" ")" defun)
;; ; defuns assumed to start at col 1, not with # or {
;;
;; this will make external declarations/definitions green, and function
;; definitions the defun face.  Hmmm - seems to work for me anyway.

(let ((comments     '(("/\\*" "\\*/" comment)))
      (c++-comments '(("//.*$" nil comment)
		      ("^/.*$" nil comment)))
      (strings      '((hilit-string-find ?' string)))
      (preprocessor '(("^#[ \t]*\\(undef\\|define\\).*$" "[^\\]$" define)
		      ("^#.*$" nil include))))

  (hilit-set-mode-patterns
   '(c-mode c++-c-mode elec-c-mode)
   (append
    comments strings preprocessor
    '(
      ;; function decls are expected to have types on the previous line
      ("^\\(\\w\\|[$_]\\)+\\s *\\(\\(\\w\\|[$_]\\)+\\s *((\\|(\\)[^)]*)+" nil defun)
      ("^\\(typedef\\|struct\\|union\\|enum\\).*$" nil decl)
      ;; datatype -- black magic regular expression
      ("[ \n\t({]\\(\\(const\\|register\\|volatile\\|unsigned\\|extern\\|static\\)\\s +\\)*\\(\\(\\w\\|[$_]\\)+_t\\|float\\|double\\|void\\|char\\|short\\|int\\|long\\|FILE\\|\\(\\(struct\\|union\\|enum\\)\\([ \t]+\\(\\w\\|[$_]\\)*\\)\\)\\)\\(\\s +\\*+)?\\|[ \n\t;()]\\)" nil type)
      ;; key words
      ("[^_]\\<\\(return\\|goto\\|if\\|else\\|case\\|default\\|switch\\|break\\|continue\\|while\\|do\\|for\\)\\>[^_]" 1 keyword)
      )))

  (hilit-set-mode-patterns
   'c++-mode
   (append
    comments c++-comments strings preprocessor
    '(
      ;; function decls are expected to have types on the previous line
      ("^\\(\\(\\w\\|[$_]\\)+::\\)?\\(\\w\\|[$_]\\)+\\s *\\(\\(\\w\\|[$_]\\)+\\s *((\\|(\\)[^)]*)+" nil defun)
      ("^\\(\\(\\w\\|[$_]\\)+[ \t]*::[ \t]*\\)?\\(\\(\\w\\|[$_]\\)+\\|operator.*\\)\\s *\\(\\(\\w\\|[$_]\\)+\\s *((\\|(\\)[^)]*)+" nil defun)
      ("^\\(template\\|typedef\\|struct\\|union\\|class\\|enum\\|public\\|private\\|protected\\).*$" nil decl)
      ;; datatype -- black magic regular expression
      ("[ \n\t({]\\(\\(const\\|register\\|volatile\\|unsigned\\|extern\\|static\\)\\s +\\)*\\(\\(\\w\\|[$_]\\)+_t\\|float\\|double\\|void\\|char\\|short\\|int\\|long\\|FILE\\|\\(\\(struct\\|union\\|enum\\|class\\)\\([ \t]+\\(\\w\\|[$_]\\)*\\)\\)\\)\\(\\s +\\*+)?\\|[ \n\t;()]\\)" nil type)
      ;; key words
      ("[^_]\\<\\(return\\|goto\\|if\\|else\\|case\\|default\\|switch\\|break\\|continue\\|while\\|do\\|for\\|public\\|protected\\|private\\|delete\\|new\\)\\>[^_]"
       1 keyword))))

  (hilit-set-mode-patterns
   '(objc-mode objective-C-mode)
   (append
    comments c++-comments strings preprocessor
    '(
      ;; function decls are expected to have types on the previous line
      ("^\\(\\(\\w\\|[$_]\\)+::\\)?\\(\\w\\|[$_]\\)+\\s *\\(\\(\\w\\|[$_]\\)+\\s *((\\|(\\)[^)]*)+" nil defun)
      ("^\\(\\(\\w\\|[$_]\\)+[ \t]*::[ \t]*\\)?\\(\\(\\w\\|[$_]\\)+\\|operator.*\\)\\s *\\(\\(\\w\\|[$_]\\)+\\s *((\\|(\\)[^)]*)+" nil defun)

      ("^\\(template\\|typedef\\|struct\\|union\\|class\\|enum\\|public\\|private\\|protected\\).*$" nil decl)
      ;; datatype -- black magic regular expression
      ("[ \n\t({]\\(\\(const\\|register\\|volatile\\|unsigned\\|extern\\|static\\)\\s +\\)*\\(\\(\\w\\|[$_]\\)+_t\\|float\\|double\\|void\\|char\\|short\\|int\\|long\\|FILE\\|\\(\\(struct\\|union\\|enum\\|class\\)\\([ \t]+\\(\\w\\|[$_]\\)*\\)\\)\\)\\(\\s +\\*+)?\\|[ \n\t;()]\\)" nil type)
      ;; key words
      ("[^_]\\<\\(return\\|goto\\|if\\|else\\|case\\|default\\|switch\\|break\\|continue\\|while\\|do\\|for\\|public\\|protected\\|private\\|interface\\|implementation\\|end\\|super\\|self\\)\\>[^_]"
       1 keyword))))
  )

(hilit-set-mode-patterns
 'perl-mode
 '(("\\s #.*$" nil comment)
   ("^#.*$" nil comment)
   ("\"[^\\\"]*\\(\\\\\\(.\\|\n\\)[^\\\"]*\\)*\"" nil string)
   ("^\\(__....?__\\|\\s *\\sw+:\\)" nil label)
   ("^require.*$" nil include)
   ("^package.*$" nil decl)
   ("^\\s *sub\\s +\\(\\w\\|[_']\\)+" nil defun)
   ("\\b\\(do\\|if\\|unless\\|while\\|until\\|else\\|elsif\\|for\\|foreach\\|continue\\|next\\|redo\\|last\\|goto\\|return\\|die\\|exit\\)\\b" nil keyword)))

(hilit-set-mode-patterns
 'ada-mode
 '(;; comments
   ("--.*$" nil comment)
   ;; main structure
   ("[ \t\n]procedure[ \t]" "\\([ \t]\\(is\\|renames\\)\\|);\\)" glob-struct)
   ("[ \t\n]task[ \t]" "[ \t]is" glob-struct)
   ("[ \t\n]function[ \t]" "return[ \t]+[A-Za-z_0-9]+[ \t]*\\(is\\|;\\|renames\\)" glob-struct)
   ("[ \t\n]package[ \t]" "[ \t]\\(is\\|renames\\)" glob-struct)
   ;; if there is nothing before "private", it is part of the structure
   ("^[ \t]*private[ \t\n]" nil glob-struct)
   ;; if there is no indentation before the "end", then it is most
   ;; probably the end of the package
   ("^end.*$" ";" glob-struct)
   ;; program structure -- "null", "delay" and "terminate" omitted
   ("[ \n\t]\\(in\\|out\\|select\\|if\\|else\\|case\\|when\\|and\\|or\\|not\\|accept\\|loop\\|do\\|then\\|elsif\\|else\\|for\\|while\\|exit\\)[ \n\t;]" nil struct)
   ;; block structure
   ("[ \n\t]\\(begin\\|end\\|declare\\|exception\\|generic\\|raise\\|return\\|package\\|body\\)[ \n\t;]" nil struct)
   ;; type declaration
   ("^[ \t]*\\(type\\|subtype\\).*$" ";" decl)
   ("[ \t]+is record.*$" "end record;" decl)
   ;; "pragma", "with", and "use" are close to C cpp directives
   ("^[ \t]*\\(with\\|pragma\\|use\\)" ";" include)
   ;; nice for named parameters, but not so beautiful in case statements
   ("[A-Za-z_0-9.]+[ \t]*=>"   nil named-param)
   ;; string constants probably not everybody likes this one
   ("\"" ".*\"" string)))

(hilit-set-mode-patterns
 'fortran-mode
 '(("^[*Cc].*$" nil comment)
   ("'[^'\n]*'" nil string)
   ("\\(^[ \t]*[0-9]+\\|[ \t]continue[ \t\n]\\|format\\)" nil define)
   ("[ \t]\\(do\\|do[ \t]*[0-9]+\\|go[ \t]*to[ \t]*[0-9]+\\|end[ \t]*do\\|if\\|else[ \t]*if\\|then\\|else\\|end[ \t]*if\\)[ \t\n(]" nil define)
   ("[ \t]\\(call\\|program\\|subroutine\\|function\\|stop\\|return\\|end\\|include\\)[ \t\n]" nil include)
   ("[ \t]\\(parameter[\t\n ]*([^)]*)\\|data\\|save\\|common[ \t\n]*/[^/]*/\\)"
    nil decl)
   ("^     ." nil type)
   ("implicit[ \t]*none" nil decl)
   ("\\([ \t]\\|implicit[ \t]*\\)\\(dimension\\|integer\\|real\\|double[ \t]*precision\\|character\\|logical\\|complex\\|double[ \t]*complex\\)\\([*][0-9]*\\|[ \t\n]\\)" nil keyword)
   )
 nil 'case-insensitive)

(hilit-set-mode-patterns
 '(m2-mode modula-2-mode)
 '(("(\\*" "\\*)" comment)
   (hilit-string-find ?\\ string)
   ("^[ \t]*PROCEDURE[ \t]+\\w+[^ \t(;]*" nil defun)
   ("\\<\\(RECORD\\|ARRAY\\|OF\\|POINTER\\|TO\\|BEGIN\\|END\\|FOR\\|IF\\|THEN\\|ELSE\\|ELSIF\\|CASE\\|WHILE\\|DO\\|MODULE\\|FROM\\|RETURN\\|IMPORT\\|EXPORT\\|VAR\\|LOOP\\|UNTIL\\|\\DEFINITION\\|IMPLEMENTATION\\|AND\\|OR\\|NOT\\|CONST\\|TYPE\\|QUALIFIED\\)\\>" nil keyword)
   )
 nil 'case-insensitive)

(hilit-set-mode-patterns 'prolog-mode
 '(("/\\*" "\\*/" comment)
   ("%.*$" nil comment)
   (":-" nil defun)
   ("!" nil label)
   ("\"[^\\\"]*\\(\\\\\\(.\\|\n\\)[^\\\"]*\\)*\"" nil string)
   ("\\b\\(is\\|mod\\)\\b" nil keyword)
   ("\\(->\\|-->\\|;\\|==\\|\\\\==\\|=<\\|>=\\|<\\|>\\|=\\|\\\\=\\|=:=\\|=\\\.\\\.\\|\\\\\\\+\\)" nil decl)
   ("\\(\\\[\\||\\|\\\]\\)" nil include)))

(hilit-set-mode-patterns
 '(
   LaTeX-mode japanese-LaTeX-mode SliTeX-mode
   japanese-SliTeX-mode FoilTeX-mode latex-mode
   )
 '(
   ;; comments
   ("[^\\]%.*$" nil comment)

   ;; the following two match \foo[xx]{xx} or \foo*{xx} or \foo{xx}
   ("\\\\\\(sub\\)*\\(paragraph\\|section\\)\\(\*\\|\\[.*\\]\\)?{" "}"
    keyword)
   ("\\\\\\(chapter\\|part\\)\\(\*\\|\\[.*\\]\\)?{" "}" keyword)
   ("\\\\footnote\\(mark\\|text\\)?{" "}" keyword)
   ("\\\\[a-z]+box" nil keyword)
   ("\\\\\\(v\\|h\\)space\\(\*\\)?{" "}" keyword)

   ;; (re-)define new commands/environments/counters
   ("\\\\\\(re\\)?new\\(environment\\|command\\){" "}" defun)
   ("\\\\new\\(length\\|theorem\\|counter\\){" "}" defun)

   ;; various declarations/definitions
   ("\\\\\\(setlength\\|settowidth\\|addtolength\\|setcounter\\|addtocounter\\)" nil define)
   ("\\\\\\(title\\|author\\|date\\|thanks\\){" "}" define)

   ("\\\\documentstyle\\(\\[.*\\]\\)?{" "}" decl)
   ("\\\\\\(begin\\|end\\|nofiles\\|includeonly\\){" "}" decl)
   ("\\\\\\(raggedright\\|makeindex\\|makeglossary\\|maketitle\\)\\b" nil
    decl)
   ("\\\\\\(pagestyle\\|thispagestyle\\|pagenumbering\\){" "}" decl)
   ("\\\\\\(normalsize\\|small\\|footnotesize\\|scriptsize\\|tiny\\|large\\|Large\\|LARGE\\|huge\\|Huge\\)\\b" nil decl)
   ("\\\\\\(appendix\\|tableofcontents\\|listoffigures\\|listoftables\\)\\b"
    nil decl)
   ("\\\\\\(bf\\|em\\|it\\|rm\\|sf\\|sl\\|ss\\|tt\\)\\b" nil decl)

   ;; label-like things
   ("\\\\item\\(\\[[^]]*\\]\\)?" nil label)
   ("\\\\caption\\(\\[[^]]*\\]\\)?{" "}" label)

   ;; formulas
   ("[^\\]\\\\("  "\\\\)" formula)                   ; \( \)
   ("[^\\]\\\\\\[" "\\\\\\]" formula)                ; \[ \]
   ("[^\\$]\\(\\$\\(\\$[^$]*\\$\\|[^$]*\\)\\$\\)" 1 formula) ; '$...$' or '$$...$$'

   ;; things that bring in external files
   ("\\\\\\(include\\|input\\|bibliography\\){" "}" include)

   ;; "wysiwyg" emphasis -- these don't work with nested expressions
   ;; ("{\\\\\\(em\\|it\\|sl\\)" "}" italic)
   ;; ("{\\\\bf" "}" bold)

   ("``" "''" string)

   ;; things that do some sort of cross-reference
   ("\\\\\\(\\(no\\)?cite\\|\\(page\\)?ref\\|label\\|index\\|glossary\\){" "}" crossref)
   ))

(hilit-set-mode-patterns
 'bibtex-mode
 '(;;(";.*$"			nil	comment)
   ("%.*$"			nil	comment)
   ("@[a-zA-Z]+"		nil	keyword)
   ("{[ \t]*[-a-z:_A-Z0-9]+,"	nil	label) ; is wrong sometimes
   ("^[ \t]*[a-zA-Z]+[ \t]*="	nil	define)))

(hilit-set-mode-patterns
 'compilation-mode
 '(
   ("^[-_.\"A-Za-z0-9]+\\(:\\|, line \\)[0-9]+: warning:.*$" nil warning)
   ("^[-_.\"A-Za-z0-9]+\\(:\\|, line \\)[0-9]+:.*$" nil error)
   ))

(hilit-set-mode-patterns
 'makefile-mode
 '(("^#.*$" nil comment)
   ("[^$]#.*$" nil comment)
   ;; rules
   ("^[^ \t\n]*%[^ \t\n]*[ \t]*::?[ \t]*[^ \t\n]*[ \t]*\\(#.*\\)?$" nil rule)
   ("^[.][A-Za-z][A-Za-z]?\..*$" nil rule)
   ;; variable definition
   ("^[_A-Za-z0-9]+[ \t]*\+?=" nil define)
   ("\\( \\|:=\\)[_A-Za-z0-9]+[ \t]*\\+=" nil define)
   ;; variable references
   ("\\$\\([^ \t\n{(]\\|[{(]@?[_A-Za-z0-9:.,%/=]+[)}]\\)" nil keyword)
   ("^[A-Za-z0-9.,/_-]+[ \t]*:.*$" nil defun)
   ("^include " nil include)))

(let* ((header-patterns '(("^Subject:.*$" nil msg-subject)
			  ("^From:.*$" nil msg-from)
			  ("^--text follows this line--$" nil msg-separator)
			  ("^[A-Za-z][A-Za-z0-9-]+:" nil msg-header)))
       (body-patterns '(("^\\(In article\\|[ \t]*\\w*[]<>}|]\\).*$"
			 nil msg-quote)))
       (message-patterns (append header-patterns body-patterns)))
  (hilit-set-mode-patterns 'msg-header header-patterns)
  (hilit-set-mode-patterns 'msg-body body-patterns)
  (hilit-set-mode-patterns '(vm-mode text-mode mail-mode rmail-mode
			     gnus-article-mode news-reply-mode mh-show-mode)
			   message-patterns
			   'hilit-rehighlight-message))

(hilit-set-mode-patterns
 'gnus-group-mode
 '(("^ U.*$" nil gnus-group-unsubscribed)
   ("^\\*? +[01]?[0-9]:.*$" nil gnus-group-empty)
   ("^ +[2-9][0-9]:.*$" nil gnus-group-full)
   ("^ +[0-9][0-9][0-9]+:.*$" nil gnus-group-overflowing)))

(hilit-set-mode-patterns
 'vm-summary-mode
 '(("^   .*$" nil summary-seen)
   ("^->.*$" nil  summary-current)
   ("^  D.*$" nil summary-deleted)
   ("^  U.*$" nil summary-unread)
   ("^  N.*$" nil summary-new)))


;;; this will match only comments w/ an even (zero is even) number of quotes...
;;; which is still inadequate because it matches comments in multi-line strings
;;; how anal do you want to get about never highlighting comments in strings?
;;; I could twiddle with this forever and still it wouldn't be perfect.
;;;   (";\\([^\"\n]*\"[^\"\n]*\"\\)*[^\"\n]*$" nil comment)

(hilit-set-mode-patterns
 '(emacs-lisp-mode lisp-interaction-mode)
 '(
   (";.*" nil comment)

;;; This almost works...but I think I'll stick with the parser function
;;;("[^?]\\(\"\\(\"\\||\\([^\"]+\\|[\\]\\([\\][\\]\\)*\"\\)*\"\\)\\)" 1 string)
   (hilit-string-find ?\\ string)

   ("^\\s *(def\\(un\\|macro\\|advice\\|alias\\|subst\\)[ \t\n]"
    "\\()\\|nil\\)" defun)
   ("^\\s *(defvar\\s +\\S +" nil decl)
   ("^\\s *(defconst\\s +\\S +" nil define)
   ("^\\s *(\\(provide\\|require\\|\\(auto\\)?load\\).*$" nil include)
   ("\\s *\\&\\(rest\\|optional\\)\\s *" nil keyword)
   ("(\\(let\\*?\\|cond\\|if\\|or\\|and\\|map\\(car\\|concat\\)\\|prog[n1*]?\\|while\\|lambda\\|function\\|set\\([qf]\\|car\\|cdr\\)?\\|nconc\\|eval-when-compile\\|condition-case\\|unwind-protect\\|catch\\|throw\\|error\\)[ \t\n]" 1 keyword)
   ))

(hilit-set-mode-patterns
 '(lisp-mode ilisp-mode)
 '(
   (";.*" nil comment)
   ("#|" "|#" comment)
;;; This almost works...but I think I'll stick with the parser function
;;;("[^?]\\(\"\\(\"\\||\\([^\"]+\\|[\\]\\([\\][\\]\\)*\"\\)*\"\\)\\)" 1 string)
   (hilit-string-find ?\\ string)

   ;; this is waaaaaaaay too slow
   ;;   ("^\\s *(def\\(un\\|macro\\|advice\\|alias\\|method\\|subst\\)\\s \\S +[ \t\n]+\\(nil\\|(\\(([^()]*)\\|[^()]+\\)*)\\)" nil defun)
   ("^\\s *(def\\(un\\|macro\\|advice\\|subst\\|method\\)\\s " "\\()\\|nil\\)" defun)

   ("^\\s *(\\(def\\(var\\|type\\|parameter\\)\\|declare\\)\\s +\\S +" nil decl)
   ("^\\s *(def\\(const\\(ant\\)?\\|class\\|struct\\)\\s \\S +[ \t\n]+" nil define)
   ("^\\s *(\\(provide\\|require\\|\\(auto\\)?load\\).*$" nil include)
   ("[ \t]\\&\\(key\\|rest\\|optional\\|aux\\)\\s *" nil keyword)
   ("(\\(let\\*?\\|locally\\|cond\\|if\\*?\\|or\\|and\\|map\\(car\\|c[ao]n\\)?\\|prog[nv1*]?\\|while\\|when\\|unless\\|do\\(\\*\\|list\\|times\\)\\|list\\|lambda\\|function\\|values\\|set\\([qf]\\|car\\|cdr\\)?\\|rplac[ad]\\|nconc\\|block\\|go\\|return\\(-from\\)?\\|[ec]?\\(type\\)?case\\|multiple-value-\\(bind\\|setq\\|list\\|call\\|prog1\\)\\|unwind-protect\\|handler-case\\|catch\\|throw\\|eval-when\\(-compile\\)?\\)[ \t\n]" 1 keyword)
   ))


(hilit-set-mode-patterns
 'plain-tex-mode
 '(("^%%.*$" nil comment)
   ("{\\\\em\\([^}]+\\)}" nil comment)
   ("\\(\\\\\\w+\\)" nil keyword)
   ("{\\\\bf\\([^}]+\\)}" nil keyword)
   ("^[ \t\n]*\\\\def[\\\\@]\\(\\w+\\)" nil defun)
   ("\\\\\\(begin\\|end\\){\\([A-Za-z0-9\\*]+\\)}" nil defun)
   ;; ("[^\\\\]\\$\\([^$]*\\)\\$" nil string)
   ("\\$\\([^$]*\\)\\$" nil string)
   ))

;; Reasonable extensions would include smarter parameter handling for such
;; things as the .IX and .I macros, which alternate the handling of following
;; arguments.

(hilit-set-mode-patterns
 'nroff-mode
 '(("^\\.[\\\][\\\"].*$" nil comment)
   ("^\\.so .*$" nil include)
   ("^\\.[ST]H.*$" nil defun)
;;   ("^[^\\.].*\"[^\\\"]*\\(\\\\\\(.\\)[^\\\"]*\\)*\"" nil string)
   ("\"" "[^\\]\"" string)
   ("^\\.[A-Z12\\\\].*$" nil define)
   ("\\([\\\][^ ]*\\)" nil keyword)
   ("^\\.[A-Z].*$" nil keyword))
 nil 'case-insensitive)

(hilit-set-mode-patterns
 'texinfo-mode
 '(("^\\(@c\\|@comment\\)\\>.*$" nil comment)
   ("@\\(emph\\|strong\\|b\\|i\\){[^}]+}" nil comment)
;; seems broken
;; ("\\$[^$]*\\$" nil string)
   ("@\\(file\\|kbd\\|key\\){[^}]+}" nil string)
   ("^\\*.*$" nil defun)
   ("@\\(if\\w+\\|format\\|item\\)\\b.*$" nil defun)
   ("@end +[A-Za-z0-9]+[ \t]*$" nil defun)
   ("@\\(samp\\|code\\|var\\){[^}]+}" nil defun)
   ("@\\w+\\({[^}]+}\\)?" nil keyword)
   ))

(hilit-set-mode-patterns
 'dired-mode
 (append
  '(("^D.*$"  nil dired-deleted)
   ("^\\*.*$" nil dired-marked)
   ("^  d.*$" nil dired-directory)
   ("^  l.*$" nil dired-link)
   ("^  -.*#.*#$" nil dired-ignored))
  (list (cons
	 (concat "^  .*\\("
		 (mapconcat 'regexp-quote completion-ignored-extensions "\\|")
		 "\\)$")
	 '(nil dired-ignored)))))

(hilit-set-mode-patterns
 'jargon-mode
 '(("^:[^:]*:" nil jargon-entry)
   ("{[^}]*}+" nil jargon-xref)))

(hilit-set-mode-patterns
 'Info-mode
 '(("^\\* [^:]+:+" nil jargon-entry)
   ("\\*[Nn]ote\\b[^:]+:+" nil jargon-xref)
   ("  \\(Next\\|Prev\\|Up\\):" nil jargon-xref)
   ("- \\(Variable\\|Function\\|Macro\\|Command\\|Special Form\\|User Option\\):.*$"
    nil jargon-keyword)))	; lisp manual

(hilit-set-mode-patterns
 'calendar-mode
 '(("[A-Z][a-z]+ [0-9]+" nil define)	; month and year
   ("S  M Tu  W Th  F  S" nil label)))	; week days

(hilit-set-mode-patterns
 'asm-mode
 '(("/\\*" "\\*/" comment)
   ("^#[ \t]*\\(undef\\|define\\).*$" "[^\\]$" define)
   ("^#.*$" nil include)
   ;; labels
   ("^.+:" nil defun)
   ;; assembler directives
   ("^[ \t]*\\..*$" nil decl)
   ;; register names
   ("\\$[a-z0-9]+" nil string)
   ;; mnemonics
   ("^[ \t]*[a-z]+" nil struct)))

(hilit-set-mode-patterns
 'pascal-mode
 '(("(\\*" "\\*)" comment)
   ("{" "}" comment)
   ;; Doesn't work when there are strings in comments....
   ;; ("'[^']*'" nil string)
   ("^#.*$" nil include)
   ("^[ \t]*\\(procedure\\|function\\)[ \t]+\\w+[^ \t(;]*" nil defun)
   ("\\<\\(program\\|begin\\|end\\)\\>" nil defun)
   ("\\<\\(external\\|forward\\)\\>" nil include)
   ("\\<\\(label\\|const\\|type\\|var\\)\\>" nil define)
   ("\\<\\(record\\|array\\|file\\)\\>" nil type)
   ("\\<\\(of\\|to\\|for\\|if\\|then\\|else\\|case\\|while\\|do\\|until\\|and\\|or\\|not\\|with\\|repeat\\)\\>" nil keyword)
   )
 nil 'case-insensitive)

(hilit-set-mode-patterns
 'icon-mode
 '(("#.*$" nil comment)
   ("\"[^\\\"]*\\(\\\\.[^\\\"]*\\)*\"" nil string)
   ;; charsets: these do not work because of a conflict with strings
   ;; ("'[^\\']*\\(\\\\.[^\\']*\\)*'" nil string)
   ("^[ \t]*procedure[ \t]+\\w+[ \t]*(" ")" defun)
   ("^[ \t]*record.*(" ")" include)
   ("^[ \t]*\\(global\\|link\\)[ \t\n]+[A-Za-z_0-9]+\\([ \t\n]*,[ \t\n]*[A-Za-z_0-9]+\\)*" nil include)
   ("^[ \t]*\\(local\\|static\\)[ \t\n]+[A-Za-z_0-9]+\\([ \t\n]*,[ \t\n]*[A-Za-z_0-9]+\\)*" nil decl)
   ("\\<\\(initial\\|end\\)\\>" nil glob-struct)
   ("\\<\\(while\\|until\\|return\\|every\\|if\\|then\\|else\\|to\\|case\\|of\\|suspend\\|create\\|do\\|repeat\\|break\\)\\>" nil keyword)
   ))

;; as you can see, I had two similar problems for Pascal and Icon. In
;; Pascal, strings are delimited with ' and an embedded quote is doubled,
;; thus string syntax would be extremely simple. However, if a string
;; occurs within a comment, the following text is considered a string.
;;
;; In Icon, strings are similar to C ones, but there are also charsets,
;; delimited with simple quotes. I could not manage to use both regexps at
;; the same time.

;; The problem I have with my patterns for Icon is that this language has a
;; string similar constant to the C one (but a string can be cut on several
;; lines, if terminated by a dash and continued with initial blanks, like
;; this:
;;         "This is a somewhat long -
;;          string, written on three -
;;          successive lines"
;; in order to insert a double quote in a string, you have to escape it
;; with a \), bu also a character set constant (named a charset), which
;; uses single quotes instead of double ones. It would seem intuitive to
;; highlight both constants in the same way.


(provide 'hilit19)

;; arch-tag: db99739a-4837-41ee-ad02-3baced8ae71d
;;; hilit19.el ends here
