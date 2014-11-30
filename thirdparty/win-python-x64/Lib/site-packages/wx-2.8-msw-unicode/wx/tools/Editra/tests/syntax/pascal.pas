{ Syntax Highlighting Test File for Pascal
  Comments are like this
  Hello World in Pascal
}
program Hello;
 
uses
   crt;
 
begin
   ClrScr;
   Write('Hello world');
   Readln;
end.

program Variables;

const
   pi: Real = 3.14;

var
   Num1, Num2, Ans: Integer;
 
begin
   Ans := 1 + 1;
   Num1 := 5;
   Ans := Num1 + 3;
   Num2 := 2;
   Ans := Num1 - Num2;
   Ans := Ans * Num1;
end.
