; Syntax Highlighting Test File for newLISP
; Comments are like this

(context 'EditraTest)

(define (hello)
  "Says hello."
  (println "Hello world"))

(define (EditraTest:EditraTest substance (times 10))
  (let ((n times))
    (while (> n 0)
      (println (format "%d bottles of %s on the wall" n (string substance)))
      (dec 'n)))
  (println "Time to get to the " substance " store!"))

(context 'MAIN)

(dolist (substance '("beer" "apple juice" "spam"))
  (EditraTest substance)
  (println))

