(* Syntax Highlighting Test File for Caml *)
(* Some Comments about this file *)

(* Hello World *)
print_endline "Hello world!";;

(* Calculate Fibbonacci Value of N *)
let rec fib n =
  if n < 2 then 1 else fib(n-1) + fib(n-2);;
let main () =
  let arg = int_of_string Sys.argv.(1) in
  print_int(fib arg);
  print_newline();
  exit 0;;
main ();;

