# Syntax Highlighting test file for Perl
# Some comments about this file

# Hello world in Perl
print "Hello, world!\n";

# Numerous other style region tests

# Number
$number1 = 42;

# String Tests
$answer = "The answer is $number1";  # Variable interpolation
$h1  = "Hello World \"Perl\""; # Double quoted string
$h2  = 'Hello World "Perl"';  # Single Quoted String
$h3  = qq(Hello World "Perl"); # qq() instead of quotes
$multilined_string =<<EOF
A multilined string that is terminated with
with the word "EOF"
EOF

# Array
@greetings = ('Hello', 'Holla', 'Konichiwa');

# Hash Table
%translate = (
    Hello => 'Hola',
    Bye => 'Adios'
);
print $translate[Hello];

=item B<function1>

This is a POD doc section

=cut
sub function1 { 
  my %args = @_;
  print "Joe said '$args{Joe}'\n";
}
function1( Joe => "Hello World" );

# Some Regular Expressions
$x =~ m/abc/
$x =~ s/abc/aBc/;   # substitute lowercase b with uppercase B
