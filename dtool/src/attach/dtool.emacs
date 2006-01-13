;; make the mouse pointer avoid the text point
;; Actually, everyone really hates this.
;(cond (window-system
;       (require 'avoid)
;       (mouse-avoidance-mode 'cat-and-mouse)))

;; Make sure utf-8 is at the top of the coding-system list; Panda's
;; TextNode uses utf-8 encoding natively, so we may have some
;; documents and code written in utf-8.

(prefer-coding-system 'utf-8)


;; make sure we have the compile library available to us
(load-library "compile")
;; Comment given for last checkout command
(setq last-co-comment "")
;; Comment given for last checkin command
(setq last-ci-comment "")
;; Target given for the last local make
(setq last-lm-target "realinstall")
;; Target given for the last global make
(setq last-gm-target "install")
;; Host given for the last ctrelease
(setq last-rel-host "")
;; Host given for the last ctship
(setq last-ship-host "")

;; check the environment
(setq ct-tool (getenv "DTOOL"))
(setq have-atria (let ((h-a (getenv "HAVE_ATRIA")))
                   (if (string= h-a "yes") t '())))
;; (setq have-neartool (let ((h-n (getenv "HAVE_NEARTOOL")))
;;                          (if (string= h-n "yes") t '())))
(setq is-cygwin (or (string= (getenv "OS") "CYGWIN_NT-4.0")
                    (string= (getenv "OS") "CYGWIN_NT-5.0")
                    (string= (getenv "OS") "CYGWIN_NT-5.1")))  

;; (setq ct-command (cond
;;                   (is-cygwin "bash /install/tool/bin/neartool")
;;                   (have-atria "cleartool")
;;                   (have-neartool "neartool")
;;                   t nil))

;; Load the Hightlight coloring scheme
(if is-cygwin
;;    (let ((filename (concat (getenv "CYGWIN_ROOT") "install\\tool\\etc\\color.emacs")))
    (let ((filename (concat (getenv "CYGWIN_ROOT") ct-tool "\\etc\\color.emacs")))
      (if (file-readable-p filename) (load filename))))

;; Checkout element in the current buffer
(defun ct-checkout-curr (comment)
  "Checkout version in current buffer with COMMENT."
  (interactive (list (read-string "Comment: " last-co-comment)))
  (setq last-co-comment comment)
  (setq pname (file-name-nondirectory (buffer-file-name)))
  (ct-shell-command-verbose
   (concat "ctco -c " (ct-quote-string comment) " " pname))
  (ct-find-curr-file-again nil)
)

;; Uncheckout element in the current buffer
(defun ct-uncheckout-curr ()
  "Uncheckout version in current buffer and remove private data."
  (interactive)
  (if (y-or-n-p "Ok to un-checkout? ")
      (progn
        (setq pname (file-name-nondirectory (buffer-file-name)))
        (ct-shell-command-verbose (concat "ctunco " pname))
        (ct-find-curr-file-again t)
        )
      (progn
        (message "Uncheckout canceled.")
        )
    )
)

;; Checkin element in the current buffer
(defun ct-checkin-curr ()
  "Checkin version in current buffer."
  (interactive)
  (setq pname (file-name-nondirectory (buffer-file-name)))
  (setq option nil)
  (while (not option)
    (setq choice (read-string "Comment: s (same), n (new), l (list): " "s"))
    (cond
      ((equal choice "s")
         (setq option "-nc"))
      ((equal choice "n")
         (setq comment (read-string "Comment: " last-ci-comment))
         (setq last-ci-comment comment)
         (setq option (concat "-c " (ct-quote-string comment))))
      ((equal choice "l")
       (ct-shell-command-verbose (concat "ctihave " pname)))
      (t
       (message (concat "Unrecognized choice: " choice "."))
       (sleep-for 2))))

  (ct-shell-command-verbose (concat "ctci " option " " pname))
  (ct-find-curr-file-again t)
)

;; Delta element in the current buffer
(defun ct-delta-curr ()
  "Delta element in current buffer."
  (interactive)
  (if (y-or-n-p "Ok to delta? ")
      (progn
        (setq pname (file-name-nondirectory (buffer-file-name)))
        (ct-shell-command-verbose (concat "ctdelta " pname))
        (ct-find-curr-file-again t)
        )
      (progn
        (message "Delta canceled.")
        )
      )
)

;; List element checkout data for the current buffer
(defun ct-lscheckout-curr ()
  "List checkout for the current buffer."
  (interactive)
  (setq pname (file-name-nondirectory (buffer-file-name)))
  (ct-shell-command-verbose (concat "ctihave &"))
)

;; List elements in the current directory that are checked out
(defun ct-lscheckout-curr-dir ()
  "List checkouts for the current directory."
  (ct-shell-command-verbose (concat "ctihave &"))
)

;; call clearmake in the local directory
(defun ct-local-make ()
  "Build TARGET from the current directory."
  (interactive)
  (setq target (read-string "Local build target: " last-lm-target))
  (setq last-lm-target target)
  (if have-atria
      (compile-internal
        (concat "clearmake -C gnu " target
                " |& grep -v \"clearmake: Warning: Config\"")
        "No more errors.")
      (compile-internal
        (concat "make " target) "No more errors."))
)

;; call clearmake in the project root directory
(defun ct-global-make ()
  "Build TARGET from the project root."
  (interactive)
  (setq target (read-string "Global build target: " last-gm-target))
  (setq last-gm-target target)
  (cond
   (have-atria
    (compile-internal
     (concat "cd `ctproj -r` ; clearmake -C gnu " target
             " |& grep -v \"clearmake: Warning: Config\"")
     "No more errors."))
   (is-cygwin
    (compile-internal
     (concat "bash -f 'cd `ctproj -r` ; make " target "'") "No more errors."))
   (t
    (compile-internal
     (concat "cd `ctproj -r` ; make " target) "No more errors."))
   )
  )

;; Do an xdiff on the current buffer to see what is different about this
;; file from the previous version.
(defun ct-xdiff-curr ()
  "Show changes to element in current buffer from the previous version."
  (interactive)
  (setq pname (file-name-nondirectory (buffer-file-name)))
  (if is-cygwin
      (ct-shell-command-verbose (concat ct-command " xdiff -pre " pname))
  
    ; The is a hack to deal with the fact that diff returns 1 if the
    ; two files do not match.
    (ct-shell-command-verbose (concat ct-command " xdiff -pre " pname ";:&")))
  )


;; Make a new element for the current buffer
(defun ct-mk-elem ()
  (interactive)
  (if (y-or-n-p (format "Make new element for %s? "
                        (file-name-nondirectory (buffer-name))))
      (progn
        (write-file (buffer-file-name))
        (ct-shell-command-verbose
         (concat "ctmkelem -eltype text_file -c '' "
                 (file-name-nondirectory (buffer-name))))
        )
    (progn
      (message "Make element canceled.")
      )
    )
  )

;; utility functions
(defun ct-shell-command-verbose (command)
  "Execute COMMAND in shell with message."
  (interactive "Shell command: \n")
  (message (concat "Executing: " command " ..."))
  (shell-command command)
  (message "Done.")
)

(defun ct-find-curr-file-again (read-only)
  "Read in the currect file again, READONLY (t) or not (nil)."
  (setq pname (buffer-file-name))
  (setq linenum (1+ (count-lines 1 (point))))
  (kill-buffer (buffer-name))
  (if read-only
      (find-file-read-only pname)
      (find-file pname))
  (goto-line linenum)
)

(defun ct-quote-string (string)
  "Enclose STRING in single or double quotes."
  (setq has-double (string-match "\"" string))
  (setq has-single (string-match "'" string))
  (cond
    ((or (and (not has-single) (not has-double))
         (and has-double (not has-single)))
       (concat "'" string "'"))
    ((and has-single (not has-double))
       (concat "\"" string "\""))
    (t
       (message (concat "Can't quote string correctly: " string))
       (sleep-for 3)
       (concat "\"" string "\"")))
)

;; default key bindings
(global-set-key "\C-xco" 'ct-checkout-curr)
(global-set-key "\C-xcu" 'ct-uncheckout-curr)
(global-set-key "\C-xci" 'ct-checkin-curr)
(global-set-key "\C-xcd" 'ct-delta-curr)
(global-set-key "\C-xcl" 'ct-lscheckout-curr)
(global-set-key "\C-xcL" 'ct-lscheckout-curr-dir)
(global-set-key "\C-xcm" 'ct-local-make)
(global-set-key "\C-xcM" 'ct-global-make)
(global-set-key "\C-xcx" 'ct-xdiff-curr)
(global-set-key "\C-xce" 'ct-mk-elem)

;; ok, lets make sure we load all other .emacs files we might need.  This is
;; attach related code.

(defun ct-load-project-emacs-file (proj-name)
  (if (string= proj-name "DTOOL") nil
    (let ((pre-name (getenv proj-name)))
      (if pre-name
          (let ((filename (concat pre-name "/built/etc/" 
                                  (downcase proj-name) ".emacs")))
            (if (file-readable-p filename)
                (load filename))
            )))
    )
  )

(defun ct-break-space-colon-str (string)
  (if (string= string "")
      '()
    (let ((substr-end (string-match ":" string 0)))
      (cons (substring string 0 substr-end)
            (let ((new-string-start (string-match " " string 0)))
              (if (eq nil new-string-start)
                  '()
                (ct-break-space-colon-str 
                 (strip-spaces (substring string (match-end 0)))))
              )))
    )
  )

(defun strip-spaces (string)
  (if (= (string-to-char string) 32)
      (if (= (length string) 1) ""
        (strip-spaces (substring string 1)))
    string)
  )

(defun ct-load-project-emacs-files ()
  "Load project specific .emacs files"
  (let ((ctprojs (getenv "CTPROJS")))
    (if ctprojs
      (mapcar 'ct-load-project-emacs-file
              (reverse (ct-break-space-colon-str ctprojs)))
      ))
  )

;; get all of the project specific .emacs files
(ct-load-project-emacs-files)
