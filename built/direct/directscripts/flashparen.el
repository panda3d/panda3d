;;; flashparen.el --- flash matching parens a la Zmacs

;; Copyright (C) 1995 Noah S. Friedman

;; Author: Noah Friedman <friedman@prep.ai.mit.edu>
;; Maintainer: friedman@prep.ai.mit.edu
;; Keywords: extensions
;; Status: Works in Emacs 19
;; Created: 1995-03-03

;; LCD Archive Entry:
;; flashparen|Noah Friedman|friedman@prep.ai.mit.edu|
;; flash matching parens a la Zmacs|
;; 12-Nov-1995|1.8|~/misc/flashparen.el.gz|

;; $Id$

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program; if not, you can either send email to this
;; program's maintainer or write to: The Free Software Foundation,
;; Inc.; 675 Massachusetts Avenue; Cambridge, MA 02139, USA.

;;; Commentary:

;; Loading this makes emacs's paren blinking behavior more closely
;; approximate the behavior of Zmacs.  It should work under X or on ascii
;; terminals.

;; Note that in XEmacs, blink-paren.el implements this functionality in a
;; more reliable manner, so use that instead of this program.

;; To use this program, load this file and do
;;
;;    (flash-matching-mode 1)
;;
;; It is vitally important that flash-matching-char be the *last* hook on
;; post-command-hook.  If anything comes after it, it won't get run until
;; flash-matching-char is interrupted by user input, which is almost
;; certainly undesirable.  As a consequence, the function
;; flash-matching-mode will make sure this is the case whenever it is run.
;; The real solution is to get the flashing function off the command hook
;; entirely, but since emacs has no builtin timers there seems to be no
;; readily apparent way to accomplish this efficiently.

;;; Code:

(defvar flash-matching-mode nil
  "*If non-nil, then flash corresponding matching character on display.
It's best to call the function of the same name, since there are other
things to be done by side effect when enabling this feature.")

(defvar flash-matching-delay
  (cond (window-system 0.2)
        ((> baud-rate 19200) 0.2)
        ((>= baud-rate 9600) 0.5)
        (t 1))
  "Interval (in seconds) for flash delay.
This number may be a floating-point number in instances of emacs that
support floating point arguments to `sit-for'.")


;;;###autoload
(defun flash-matching-mode (&optional prefix)
  "*If non-nil, then flash corresponding matching character on display."
  (interactive "P")

  ;; Make sure flash-matching-char is last on post-command-hook or
  ;; post-command-idle-hook.  The latter is defined in Emacs 19.30 and later.
  (let* ((hook (if (boundp 'post-command-idle-hook)
                   'post-command-idle-hook
                 'post-command-hook))
         (h (memq 'flash-matching-char (symbol-value hook))))
    (cond ((null h)
           (add-hook hook 'flash-matching-char 'append))
          ((cdr h)
           (remove-hook hook 'flash-matching-char)
           (add-hook hook 'flash-matching-char 'append))))

  (setq flash-matching-mode
        (>= (prefix-numeric-value prefix) 0))
  (and (interactive-p)
       (if flash-matching-mode
           (message "flash-matching-mode is enabled")
         (message "flash-matching-mode is disabled")))
  flash-matching-mode)

;; Verify that an even number of quoting characters precede char at point.
(defsubst flash-matching-even-quoting-p (point)
  (let ((p (point)))
    (if (= point (point-min))
        t
      (= 1 (logand 1 (- point
                        (progn
                          (goto-char point)
                          (forward-char -1)
                          (skip-syntax-backward "/\\" (point-min))
                          (prog1
                              (point)
                            (goto-char p)))))))))

(defun flash-matching-char ()
  (and flash-matching-mode
       ;; prefix args do strange things with commands; it seems that
       ;; running post-command-hook after invoking one of these is delayed
       ;; until the command is finished, then the hook is run twice.
       ;; It's undesirable to wait for user input twice before returning to
       ;; the top command loop, so skip this the first time.
       (not (memq this-command '(digit-argument universal-argument)))
       ;; keyboard macros run a sequence of interactive commands, each one
       ;; of which will cause a call to post-command-hook; so as long as
       ;; the keyboard macro is still executing, do nothing.
       (null executing-macro)
       (let* ((saved-point (point))
              (cho (char-after saved-point))
              (chc (char-after (1- saved-point)))
              ch)
         (cond
          ((or (and (numberp cho)
                    (= (char-syntax cho) ?\()
                    (< saved-point (window-end))
                    (flash-matching-even-quoting-p saved-point)
                    (setq ch cho))
               (and (numberp chc)
                    (= (char-syntax chc) ?\))
                    (> saved-point (window-start))
                    (flash-matching-even-quoting-p saved-point)
                    (setq ch chc)))

           (let ((parse-sexp-ignore-comments t)
                 ;; this beginning of line is not necessarily the same as
                 ;; the one of the matching char `line-beg', below.
                 (bol-point (progn
                              (beginning-of-line)
                              (point)))
                 match-point)

             ;; should be at bol now
             ;; If we're inside a comment already, turn off ignoring comments.
             (and comment-start
                  (looking-at (concat "^[ \t]*" (regexp-quote comment-start)))
                  (setq parse-sexp-ignore-comments nil))

             ;; Find matching paren position, but don't search any further
             ;; than the visible window.
             (save-restriction
               (condition-case ()
                   (progn
                     (narrow-to-region (window-start) (window-end))
                     (cond
                      ((= (char-syntax ch) ?\()
                       (setq match-point (1- (scan-sexps saved-point 1))))
                      (t
                       (setq match-point (scan-sexps saved-point -1)))))
                 (error nil)))

             ;; Matched char must be the corresponding character for the
             ;; char at the saved point, not just another paired delimiter.
             ;; This can happen when parens and brackets are mismatched,
             ;; for example.  Also don't be fooled by things in an
             ;; open/close syntax class but with no defined matching
             ;; character.
             (and match-point
                  (flashparen-matching-paren ch)
                  (not (= (char-after match-point)
                          (flashparen-matching-paren ch)))
                  (setq match-point nil))

             ;; match char must be horizontally visible on display.
             ;; Unfortunately we cannot just use pos-visible-in-window-p
             ;; since that returns t for things that are actually off the
             ;; display horizontally.
             (and truncate-lines
                  match-point
                  (let ((window-hstart (window-hscroll))
                        (match-column (progn
                                        (goto-char match-point)
                                        (current-column))))
                    (if (or (< match-column window-hstart)
                            (> match-column (+ window-hstart (window-width))))
                        (setq match-point nil))))

             (cond (match-point
                    ;; I added this to remove messages left over from
                    ;; blink-matching-open, but it also causes messages
                    ;; returned by eval-expression, etc. not to appear if
                    ;; point is right after a sexp, which is too annoying.
                    ;;(message nil)
                    (flash-matching-do-flash saved-point match-point))
                   (t
                    (goto-char saved-point)
                    (and chc
                         (= (char-syntax chc) ?\))
                         ;; blink-matching-open can sometimes signal an
                         ;; error if the function name is outside of a
                         ;; narrowed region---this can happen in C, perl,
                         ;; and other languages where the function label is
                         ;; outside the starting block character, depending
                         ;; on how one's narrow-to-defun function is defined.
                         (condition-case ()
                             (blink-matching-open)
                           (error nil)))))))))))

(defun flash-matching-do-flash (flash-matching-opoint flash-matching-mpoint)
  ;; Deactivate the mark now if deactivate-mark is set in transient mark
  ;; mode.  Normally the command loop does this itself, but because this
  ;; function is on post-command-hook, deactivation is delayed and causes
  ;; noticable, undesirable effects on the display.
  ;; The only time I've noticed this to be of consequence is when point is
  ;; right before a sexp and you insert a character.  Otherwise, this
  ;; function doesn't get called again because after modifying the buffer,
  ;; point is no longer at the beginning or end of a sexp.
  (and transient-mark-mode
       deactivate-mark
       (deactivate-mark))

  (let ((modp (buffer-modified-p))
        (buffer-file-name buffer-file-name)
        (buffer-auto-save-file-name buffer-auto-save-file-name)
        (auto-save-hook (and (boundp 'auto-save-hook)
                             auto-save-hook))

        ;; Don't make any undo records while flashing.
        ;; If this is nil, new undo records are appended.
        ;; Setting it to t avoids consing any records at all.
        (buffer-undo-list t)

        (before-change-function nil)
        (after-change-function nil)
        ;; buffer modification messes with transient mark mode.
        (deactivate-mark nil)

        ;; These variables have long names because they may be referenced
        ;; by a function in the auto-save-hook even if the current buffer
        ;; isn't this one (e.g. because a process filter was running at the
        ;; time).
        (flash-matching-buffer (current-buffer))
        (flash-matching-char (char-after flash-matching-mpoint))
        (flash-matching-visible-p t))

    (cond
     ((null buffer-file-name))
     (modp
      ;; If buffer is already modified, do not try to disable locking or
      ;; autosaving, but make sure flashed char is in the buffer exactly
      ;; when autosaving occurs.
      (add-hook 'auto-save-hook
                (function
                 (lambda ()
                   (or flash-matching-visible-p
                       (save-excursion
                         (set-buffer flash-matching-buffer)
                         (let ((buffer-read-only nil))
                           (goto-char flash-matching-mpoint)
                           (insert-before-markers-and-inherit
                            flash-matching-char)
                           (goto-char flash-matching-mpoint)
                           (delete-char -1)
                           (setq flash-matching-visible-p t)
                           (goto-char flash-matching-opoint))))))))
     (t
      ;; Defeat file locking.  Don't try this at home, kids!
      (setq buffer-file-name nil)
      (setq buffer-auto-save-file-name nil)))

    ;; We insert-before-markers-and-inherit one char after the one to
    ;; delete, just in case things like window-start, process-mark,
    ;; etc. are at the point of interest.
    (setq flash-matching-mpoint (1+ flash-matching-mpoint))
    (goto-char flash-matching-opoint)
    (unwind-protect
        (while (sit-for flash-matching-delay)
          (let ((buffer-read-only nil))
            (goto-char flash-matching-mpoint)
            ;; Insert char before deleting existing one, to avoid
            ;; complications having to do with overlays and text
            ;; properties on a region.
            (if flash-matching-visible-p
                (insert-before-markers-and-inherit 32)
              (insert-before-markers-and-inherit flash-matching-char))
            (goto-char flash-matching-mpoint)
            (delete-char -1)
            (setq flash-matching-visible-p
                  (not flash-matching-visible-p))

            ;; Hide fact of temporary modification during redisplay, if
            ;; buffer was unmodified originally.
            (or modp
                (set-buffer-modified-p modp))

            (goto-char flash-matching-opoint)))
      (or flash-matching-visible-p
          (let ((buffer-read-only nil))
            (goto-char flash-matching-mpoint)
            (insert-before-markers-and-inherit flash-matching-char)
            (goto-char flash-matching-mpoint)
            (delete-char -1)
            (or modp
                (set-buffer-modified-p modp))
            (goto-char flash-matching-opoint))))))

;; matching-paren wasn't defined in emacs until version 19.26.
(if (fboundp 'matching-paren)
    (defalias 'flashparen-matching-paren 'matching-paren)
  (defun flashparen-matching-paren (c)
    (and (memq (char-syntax c) '(?\( ?\)))
         (lsh (aref (syntax-table) c) -8))))

(provide 'flashparen)

;; local variables:
;; vc-make-backup-files: t
;; end:

;;; flashparen.el ends here
