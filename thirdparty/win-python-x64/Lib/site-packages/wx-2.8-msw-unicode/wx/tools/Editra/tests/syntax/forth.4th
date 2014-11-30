\ Test file for Forth Syntax
\ Comments are like this
10 constant ten
: .hello ( -- )
  s" Hello World" type [char] ! emit ;
: hello ( flag -- )
  ?dup if .hello then drop ;
hello


