; Syntax Highlighting Test File for Lisp
; Comment Line

; Lisp funciton and keyword test
(defpackage :mypackage
  (:use :common-lisp :cffi))

; hello version 1
(defun hello-word1 ()
  (print (list 'HELLO 'WORLD)))

; hello version 2
(defun hello-world ()
  (format t "hello world~%"))

; Lets do some factorials too
(defun factorial (N)
  (if (= N 1)
      1
    (* N (factorial (- N 1)))))

; Fibonacci numbers are fun too
(defun fibonacci (N)
  (if (or (zerop N) (= N 1))
      1
    (+ (fibonacci (- N 1)) (fibonacci (- N 2)))))
