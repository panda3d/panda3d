; Syntax Highlighting Test File for Scheme
; Comments are like this

; Hello World
(define hello-world
  (lambda ()
    (begin
      (write 'Hello-World)
      (newline)
      (hello-world)))) 

; Factorial
(define (fact n)
    (if (= n 0)
        1
        (* n (fact (- n 1)))))

