;;
;; C++ customization
;;

; .h files should be in c++ mode.
(setq auto-mode-alist
      (append
       '(("\\.hh$" . c++-mode)
	 ("\\.h$"  . c++-mode)
	 ("\\.I$"  . c++-mode)
	 ("\\.T$"  . c++-mode)
	 ("\\.y$"  . c++-mode)  ; yacc source files
	 ("\\.l$"  . c++-mode)  ; lex source files
	 ("\\.stk$" . scheme-mode)
	 ("\\.ss$" . scheme-mode)
	 ("\\.sal$" . scheme-mode)
	 ("\\.emacs$". lisp-mode)
	 ) auto-mode-alist))

; There's too much disagreement between different editors
; (particularly Windows editors) over how many spaces a tab
; represents.  Better not to put them any any source files at all.
(load "custom")
(custom-set-variables
 '(indent-tabs-mode nil))

; Just to belabor the point, we'll explicitly set it to nil for the
; buffer whenever you switch to C++ mode.
(setq c++-mode-hook (lambda ()
		      (setq indent-tabs-mode nil)
                      ;; Make sure we use unix encoding
                      (setq local-write-file-hooks 'Use-Undecided-Unix-Mode)
		      ))

;;
;; Scheme customization
;;
(defun scheme-send-buffer ()
  "Send the buffer, to inferior shell running scheme."
  (interactive)
  (scheme-send-region (point-min) (point-max)))

(setq cmuscheme-load-hook
      '((lambda ()
	  (define-key inferior-scheme-mode-map "\n" 'newline-and-indent)
	  )))

(add-hook 'scheme-mode-hook
 	  (function
 	   (lambda ()
	     (define-key scheme-mode-map "\r" 'newline-and-indent)
	     (define-key scheme-mode-map "\C-cd"   'scheme-send-definition)
	     (define-key scheme-mode-map "\C-cr"   'scheme-send-region)
	     (define-key scheme-mode-map "\C-cb" 'scheme-send-buffer)
	     (define-key scheme-mode-map "\C-c\C-i" 'scheme-indent-definition)

	     ;; Scheme style
	     (put 'set! 'scheme-indent-function 1)

	     ;; Chez Scheme specific
	     (put 'extend-syntax 'scheme-indent-function 1)
	     (put 'syntax-rules 'scheme-indent-function 1)
	     (put 'with 'scheme-indent-function 1)
	     (put 'when 'scheme-indent-function 1)
	     (put 'unless 'scheme-indent-function 1)
	     (put 'syntax-case 'scheme-indent-function 2)
	     (put 'with-syntax 'scheme-indent-function 1)
	     (put 'with 'scheme-indent-function 1)
	     (put 'foreign-procedure 'scheme-indent-function 1)
	     (put 'letrec-syntax 'scheme-indent-function 1)

	     ;; Sal Specific 
	     (put 'task-while 'scheme-indent-function 1)
	     (put 'task-until 'scheme-indent-function 1)
	     (put 'while 'scheme-indent-function 1)
	     (put 'until 'scheme-indent-function 1)
	     (put 'upon-death-of 'scheme-indent-function 1)
	     (put 'compile-time-if 'scheme-indent-function 1)
	     )))

(defun scheme-indent-definition ()
  "Fix indentation of the current definition."
  (interactive)
  (save-excursion
    (beginning-of-defun 1)
    (scheme-indent-sexp)))

;; SAL/CHEZ-SCHEME HILIGHTING 
(cond (window-system
       (setq hilit-mode-enable-list  '(not text-mode)
	     hilit-inhibit-hooks     nil
	     hilit-inhibit-rebinding nil)

       (require 'hilit19)

       ;; Don't define explicit syntax colors here in deepevil.emacs,
       ;; because that forces everyone to use your color scheme.
       ;; Instead, just define a reasonable mapping for new syntax
       ;; forms to old syntax forms.

       ;; If you don't like the color scheme and want to set explicit
       ;; colors, do it in your own .emacs file.

       ;; If you simply can't read the highlited text, then try
       ;; setting hilit-background-mode in your .emacs file to either
       ;; 'light or 'dark, according to the background color you have
       ;; set.

       (hilit-translate 
	;; SAL
	sal-task       'type
	sal-event      'define
	sal-define     'decl
	sal-rtfm1	'include
	sal-rtfm2	'include
	sal-operator   'keyword
	sal-comment1   'comment
	)
       (hilit-set-mode-patterns
 '(scheme-mode)
 '(
   ;; dwd
   ("\\([Ss][Ff][Ww]\\|[Ww][Aa][Tt][Ss][Oo][Nn]\\|[Ss][Cc][Oo][Tt][Tt]\\)" nil sal-comment1)

   ;; rtfm stuff
   ; ";;;*Description", ";;;*Public", and ";;;*Private" are rtfm1
   ("\\(;;;\\*Description\\|;;;\\*Public\\|;;;\\*Private\\)" nil sal-rtfm1)
   ; ";;*", ";;.", and ";;;." are rtfm2
   ("\\(;;\\*\\|;;\\.\\|;;;\\.\\)" nil sal-rtfm2)

   ;; comments
   (";.*" nil comment)
   ("(/\\*" "\\*/)" comment)
   ("(/\\*" nil sal-comment1)
   ("\\*/)" nil sal-comment1)
   
   ;; loads, provides, requires
   ("^\\s *(\\(provide\\|require\\|load-once\\|\\(try-\\|auto\\)?load\\) " nil include)
   
   ;; strings
   (hilit-string-find ?\\ string)
   
   ;; define, define-macro, define-method
   ("(def\\(ine\\|macro\\)[^ \t]*[ \t]*[^ \t]*[ \t\n]" nil sal-define)

   ;; mutators, anything ending in !
   ("(\\(\\(\\w\\|[:-]\\)*!\\)[ \t\n]" 1 define)
   
   ;; keywords
   ("(\\(let\\w*\\|let\\*\\|case\\|let-generic\\|do\\|cond\\|else\\|sequence\\|begin\\|force\\|delay\\|if\\|lambda\\|fluid-let\\|when\\|unless\\|map\\|for-each\\|call\\(-with-current-continuation\\|/cc\\)\\)[ \t\n]" 1 keyword)
   ("\\[\\(else\\)[ \t\n]" 1 keyword)
   
   ;; operators
   ("(\\(c[ad]*r\\|list\\|list\\?\\|eq.*\\?\\|eval\\|apply\\|or\\|and\\|not\\|map\\|append\\|cons\\|null\\?\\|zero\\?\\|length\\|reverse\\|list-tail\\|list-ref\\|mem.*\\?\\|ass\\*?\\)[ \t\n]" 1 sal-operator)

   ;; events
   ("\\(event-\\(number\\|name\\)\\|add\\(-persistent\\|-value\\)\\(-hook\\|-thunk\\)\\|add-hook\\|rm-hook\\|rm-all-hooks-named\\|reset-hooks\\|put-event\\|print-hooks\\|do-\\(events\\|hooks\\|later\\|timed-thunks\\)\\|timer-event\\)[ \t\n]" 1 sal-event)

   ;; task stuff
   ("(\\(rm-task\\|spawn-task\\|reset-tasks\\|go\\|if-ever\\|upon-\\(death\\|death-of\\)\\|suspend-task\\|task-\\(while\\|pause\\|go\\|release\\|until\\)\\)[ \t\n]" 1 sal-task)

))))

;;
;; Auto NEWHEADER command
;;

(defun new-mk-elem ()
  (interactive)
  (ct-mk-elem)
  (remove-hook 'local-write-file-hooks 'new-mk-elem)
  )
				    
(defun newheader ()
  "Execute the newheader command with the current buffer name"
  (interactive)
  (add-hook 'local-write-file-hooks 'new-mk-elem)
  (save-excursion (goto-char (point-min))
		  (message (concat "Making a header for " 
				   (file-name-nondirectory (buffer-name))
				   "..."))
		  (shell-command-on-region
		   (point-min) (point-min)
		   (concat "newheader " (file-name-nondirectory (buffer-name)))
		   1)
		  )
  )

(setq no-newheader nil)

(defun auto-newheader ()
  "Automatically generate header for specfic file types"

  (let ((inslist '("\\.cxx$" "\\.c++$" "\\.c$" "\\.I$" "\\.h$" "\\.hh$" "\\.ss$" "\\.sal$" "\\.stk$"))
        (name (file-name-sans-versions buffer-file-name))
	(insert-file nil))
    
    (while (and (not insert-file) inslist)
      (if (string-match (car inslist) name)
          (setq insert-file (car inslist))
	)
      (setq inslist (cdr inslist)))
    (if insert-file
	(if no-newheader
	    (message "Newheader is disabled")
	  (newheader)))
    )
  )

; Create a new header if a file is not found.
(setq find-file-not-found-hooks
      (cons 'auto-newheader
	    find-file-not-found-hooks))

;; Find an extra open paren...
;; We need more paren matching utilities!
;; Another way to find an extra open paren is to use backward-up-list from
;; the end of the buffer.

(defun find-unbalanced-paren ()
  "Find an unbalanced parenthesis in the current buffer."
  (interactive)
  (let ((opoint (point)))
    (goto-char (point-min))
    (while (beginning-of-defun -1)
      ;; This errs if there is an extra open paren (missing close paren).
      (forward-sexp 1)
      ;; This errs if there is an extra close paren (missing open paren).
      (if (looking-at "[ \t\n\r]*)")
	  (progn
	    (goto-char (- (match-end 0) 1))
	    (error "Extra close parenthesis.")))
      (goto-char (match-beginning 0)))
    ;; Now try heuristics
    (goto-char (point-min))
    ;; Beginning-of-defun isn't supposed to err, but it does if called at
    ;; point-max, in Emacs 19.22.
    (while (and (not (eobp)) (beginning-of-defun -1))
      ;; This errs if there is an extra open paren (missing close paren).
      (forward-sexp 1)
      ;; This errs if there is an extra close paren (missing open paren).
      (if (not (looking-at "[ \t\n\r]+"))
	  (error "No whitespace at end of definition."))
      (goto-char (match-end 0))
      ;; If looking at a comment immediately following the def, skip it
      (if (looking-at ";")
	  (forward-line 1))
      ;; Skip over whitespace
      (if (looking-at "[ \t\n\r]+")
	  (goto-char (match-end 0)))
      ;; We aren't in column 0 after skipping post-def comments and whitespace.
      (if (not (= (current-column) 0))
	  (error "Possible missing open parenthesis or extra close parenthesis."))
      (goto-char (match-beginning 0)))
    (goto-char opoint)
    (message "Couldn't find unbalanced parenthesis.")))

(setq chez-last-line "")

(defun chez ()
  "Run Chez Scheme (with arguments) as an inferior Scheme process."
  (interactive)
  (setq chez-last-line
	(read-string "Run scheme (like this): scheme " chez-last-line))
  (run-scheme (concat "/fit/tool/bin/scheme " chez-last-line)))

(setq sal-last-line "")

(defun rtfm ()
  "Load the rtfm dump into a buffer."
  (interactive)
  (find-file "/usr/local/etc/rtfm/info"))

(defun sal ()
  "Run sal (with arguments) as an inferior Scheme process."
  (interactive)
  (setq sal-last-line
	(read-string "Run sal (like this): sal " sal-last-line))
  (run-scheme (concat "sal " sal-last-line)))

;; PAREN MATCHING
(load "flashparen")
(flash-matching-mode 1)

(defun goto-matching-paren ()
  "Goto the matching parenthesis of sexp after point (visually at point)."
  (interactive)
  (if (= (char-syntax (char-after (point))) ?\()
      (goto-char (1- (scan-sexps (point) 1)))
      (if (= (char-syntax (char-after (point))) ?\))
	  (goto-char (scan-sexps (1+ (point)) -1)))))

