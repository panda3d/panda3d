// Filename: eggMakeFont.cxx
// Created by:  drose (16Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "eggMakeFont.h"
#include "charBitmap.h"
#include "charPlacement.h"

#include <string_utils.h>
#include <eggGroup.h>
#include <eggTexture.h>
#include <eggVertexPool.h>
#include <eggPolygon.h>
#include <eggPoint.h>
#include <pointerTo.h>

#include <math.h>

/********************************************************************

I found the following in a source file for pftype called pftype.web.
It's some nutty TeX-based Pascal program, and the documentation is a
little hard to read because of the embedded TeX formatting controls.
But it describes quite thoroughly the format of the pk file.

*********************************************************************

@* Packed file format.
The packed file format is a compact representation of the data contained in a
\.{GF} file.  The information content is the same, but packed (\.{PK}) files
are almost always less than half the size of their \.{GF} counterparts.  They
are also easier to convert into a raster representation because they do not
have a profusion of \\{paint}, \\{skip}, and \\{new\_row} commands to be
separately interpreted.  In addition, the \.{PK} format expressedly forbids
\&{special} commands within a character.  The minimum bounding box for each
character is explicit in the format, and does not need to be scanned for as in
the \.{GF} format.  Finally, the width and escapement values are combined with
the raster information into character ``packets'', making it simpler in many
cases to process a character.

A \.{PK} file is organized as a stream of 8-bit bytes.  At times, these bytes
might be split into 4-bit nybbles or single bits, or combined into multiple
byte parameters.  When bytes are split into smaller pieces, the `first' piece
is always the most significant of the byte.  For instance, the first bit of
a byte is the bit with value 128; the first nybble can be found by dividing
a byte by 16.  Similarly, when bytes are combined into multiple byte
parameters, the first byte is the most significant of the parameter.  If the
parameter is signed, it is represented by two's-complement notation.

The set of possible eight-bit values is separated into two sets, those that
introduce a character definition, and those that do not.  The values that
introduce a character definition range from 0 to 239; byte values
above 239 are interpreted as commands.  Bytes that introduce character
definitions are called flag bytes, and various fields within the byte indicate
various things about how the character definition is encoded.  Command bytes
have zero or more parameters, and can never appear within a character
definition or between parameters of another command, where they would be
interpeted as data.

A \.{PK} file consists of a preamble, followed by a sequence of one or more
character definitions, followed by a postamble.  The preamble command must
be the first byte in the file, followed immediately by its parameters.
Any number of character definitions may follow, and any command but the
preamble command and the postamble command may occur between character
definitions.  The very last command in the file must be the postamble.

@ The packed file format is intended to be easy to read and interpret by
device drivers.  The small size of the file reduces the input/output overhead
each time a font is loaded.  For those drivers that load and save each font
file into memory, the small size also helps reduce the memory requirements.
The length of each character packet is specified, allowing the character raster
data to be loaded into memory by simply counting bytes, rather than
interpreting each command; then, each character can be interpreted on a demand
basis.  This also makes it possible for a driver to skip a particular
character quickly if it knows that the character is unused.

@ First, the command bytes will be presented; then the format of the
character definitions will be defined.  Eight of the possible sixteen
commands (values 240 through 255) are currently defined; the others are
reserved for future extensions.  The commands are listed below.  Each command
is specified by its symbolic name (e.g., \\{pk\_no\_op}), its opcode byte,
and any parameters.  The parameters are followed by a bracketed number
telling how many bytes they occupy, with the number preceded by a plus sign if
it is a signed quantity.  (Four byte quantities are always signed, however.)

\yskip\hang|pk_xxx1| 240 |k[1]| |x[k]|.  This command is undefined in general;
it functions as a $(k+2)$-byte \\{no\_op} unless special \.{PK}-reading
programs are being used.  \MF\ generates \\{xxx} commands when encountering
a \&{special} string.  It is recommended that |x| be a string having the form
of a keyword followed by possible parameters relevant to that keyword.

\yskip\hang\\{pk\_xxx2} 241 |k[2]| |x[k]|.  Like |pk_xxx1|, but |0<=k<65536|.

\yskip\hang\\{pk\_xxx3} 242 |k[3]| |x[k]|.  Like |pk_xxx1|, but
|0<=k<@t$2^{24}$@>|.  \MF\ uses this when sending a \&{special} string whose
length exceeds~255.

\yskip\hang\\{pk\_xxx4} 243 |k[4]| |x[k]|.  Like |pk_xxx1|, but |k| can be
ridiculously large; |k| musn't be negative.

\yskip\hang|pk_yyy| 244 |y[4]|.  This command is undefined in general; it
functions as a five-byte \\{no\_op} unless special \.{PK} reading programs
are being used.  \MF\ puts |scaled| numbers into |yyy|'s, as a result of
\&{numspecial} commands; the intent is to provide numeric parameters to
\\{xxx} commands that immediately precede.

\yskip\hang|pk_post| 245.  Beginning of the postamble.  This command is
followed by enough |pk_no_op| commands to make the file a multiple
of four bytes long.  Zero through three bytes are usual, but any number
is allowed.
This should make the file easy to read on machines that pack four bytes to
a word.

\yskip\hang|pk_no_op| 246.  No operation, do nothing.  Any number of
|pk_no_op|'s may appear between \.{PK} commands, but a |pk_no_op| cannot be
inserted between a command and its parameters, between two parameters, or
inside a character definition.

\yskip\hang|pk_pre| 247 |i[1]| |k[1]| |x[k]| |ds[4]| |cs[4]| |hppp[4]|
|vppp[4]|.  Preamble command.  Here, |i| is the identification byte of the
file, currently equal to 89.  The string |x| is merely a comment, usually
indicating the source of the \.{PK} file.  The parameters |ds| and |cs| are
the design size of the file in $1/2^{20}$ points, and the checksum of the
file, respectively.  The checksum should match the \.{TFM} file and the
\.{GF} files for this font.  Parameters |hppp| and |vppp| are the ratios
of pixels per point, horizontally and vertically, multiplied by $2^{16}$; they
can be used to correlate the font with specific device resolutions,
magnifications, and ``at sizes''.  Usually, the name of the \.{PK} file is
formed by concatenating the font name (e.g., cmr10) with the resolution at
which the font is prepared in pixels per inch multiplied by the magnification
factor, and the letters \.{pk}. For instance, cmr10 at 300 dots per inch
should be named \.{cmr10.300pk}; at one thousand dots per inch and magstephalf,
it should be named \.{cmr10.1095pk}.

@ We put a few of the above opcodes into definitions for symbolic use by
this program.

@d pk_id = 89 {the version of \.{PK} file described}
@d pk_xxx1 = 240 {\&{special} commands}
@d pk_yyy = 244 {\&{numspecial} commands}
@d pk_post = 245 {postamble}
@d pk_no_op = 246 {no operation}
@d pk_pre = 247 {preamble}
@d pk_undefined == 248, 249, 250, 251, 252, 253, 254, 255

@ The \.{PK} format has two conflicting goals: to pack character raster and
size information as compactly as possible, while retaining ease of translation
into raster and other forms.  A suitable compromise was found in the use of
run-encoding of the raster information.  Instead of packing the individual
bits of the character, we instead count the number of consecutive `black' or
`white' pixels in a horizontal raster row, and then encode this number.  Run
counts are found for each row from left to right, traversing rows from the
top to bottom. This is essentially the way the \.{GF} format works.
Instead of presenting each row individually, however, we concatenate all
of the horizontal raster rows into one long string of pixels, and encode this
row.  With knowledge of the width of the bit-map, the original character glyph
can easily be reconstructed.  In addition, we do not need special commands to
mark the end of one row and the beginning of the next.

Next, we place the burden of finding the minimum bounding box on the part
of the font generator, since the characters will usually be used much more
often than they are generated.  The minimum bounding box is the smallest
rectangle that encloses all `black' pixels of a character.  We also
eliminate the need for a special end of character marker, by supplying
exactly as many bits as are required to fill the minimum bounding box, from
which the end of the character is implicit.

Let us next consider the distribution of the run counts.  Analysis of several
dozen pixel files at 300 dots per inch yields a distribution peaking at four,
falling off slowly until ten, then a bit more steeply until twenty, and then
asymptotically approaching the horizontal.  Thus, the great majority of our
run counts will fit in a four-bit nybble.  The eight-bit byte is attractive for
our run-counts, as it is the standard on many systems; however, the wasted four
bits in the majority of cases seem a high price to pay.  Another possibility
is to use a Huffman-type encoding scheme with a variable number of bits for
each run-count; this was rejected because of the overhead in fetching and
examining individual bits in the file.  Thus, the character raster definitions
in the \.{PK} file format are based on the four-bit nybble.

@ An analysis of typical pixel files yielded another interesting statistic:
Fully 37\char`\%\
of the raster rows were duplicates of the previous row.  Thus, the \.{PK}
format allows the specification of repeat counts, which indicate how many times
a horizontal raster row is to be repeated.  These repeated rows are taken out
of the character glyph before individual rows are concatenated into the long
string of pixels.

For elegance, we disallow a run count of zero.  The case of a null raster
description should be gleaned from the character width and height being equal
to zero, and no raster data should be read.  No other zero counts are ever
necessary.  Also, in the absence of repeat counts, the repeat value is set to
be zero (only the original row is sent.)  If a repeat count is seen, it takes
effect on the current row.  The current row is defined as the row on which the
first pixel of the next run count will lie.  The repeat count is set back to
zero when the last pixel in the current row is seen, and the row is sent out.

This poses a problem for entirely black and entirely white rows, however.  Let
us say that the current row ends with four white pixels, and then we have five
entirely empty rows, followed by a black pixel at the beginning of the next
row, and the character width is ten pixels.  We would like to use a repeat
count, but there is no legal place to put it.  If we put it before the white
run count, it will apply to the current row.  If we put it after, it applies
to the row with the black pixel at the beginning.  Thus, entirely white or
entirely black repeated rows are always packed as large run counts (in this
case, a white run count of 54) rather than repeat counts.

@ Now we turn our attention to the actual packing of the run counts and
repeat counts into nybbles.  There are only sixteen possible nybble values.
We need to indicate run counts and repeat counts.  Since the run counts are
much more common, we will devote the majority of the nybble values to them.
We therefore indicate a repeat count by a nybble of 14 followed by a packed
number, where a packed number will be explained later.  Since the repeat
count value of one is so common, we indicate a repeat one command by a single
nybble of 15.  A 14 followed by the packed number 1 is still legal for a
repeat one count.  The run counts are coded directly as packed
numbers.

For packed numbers, therefore, we have the nybble values 0 through 13.  We
need to represent the positive integers up to, say, $2^{31}-1$.  We would
like the more common smaller numbers to take only one or two nybbles, and
the infrequent large numbers to take three or more.  We could therefore
allocate one nybble value to indicate a large run count taking three or more
nybbles.  We do this with the value 0.

@ We are left with the values 1 through 13.  We can allocate some of these, say
|dyn_f|, to be one-nybble run counts.
These will work for the run counts |1..dyn_f|.  For subsequent run
counts, we will use a nybble greater than |dyn_f|, followed by a second nybble,
whose value can run from 0 through 15.  Thus, the two-nybble values will
run from |dyn_f+1..(13-dyn_f)*16+dyn_f|.  We have our definition of large run
count values now, being all counts greater than |(13-dyn_f)*16+dyn_f|.

We can analyze our several dozen pixel files and determine an optimal value of
|dyn_f|, and use this value for all of the characters.  Unfortunately, values
of |dyn_f| that pack small characters well tend to pack the large characters
poorly, and values that pack large characters well are not efficient for the
smaller characters.  Thus, we choose the optimal |dyn_f| on a character basis,
picking the value that will pack each individual character in the smallest
number of nybbles.  Legal values of |dyn_f| run from 0 (with no one-nybble run
counts) to 13 (with no two-nybble run counts).

@ Our only remaining task in the coding of packed numbers is the large run
counts.  We use a scheme suggested by D.~E.~Knuth
@^Knuth, Donald Ervin@>
that simply and elegantly represents arbitrarily large values.  The
general scheme to represent an integer |i| is to write its hexadecimal
representation, with leading zeros removed.  Then we count the number of
digits, and prepend one less than that many zeros before the hexadecimal
representation.  Thus, the values from one to fifteen occupy one nybble;
the values sixteen through 255 occupy three, the values 256 through 4095
require five, etc.

For our purposes, however, we have already represented the numbers one
through |(13-dyn_f)*16+dyn_f|.  In addition, the one-nybble values have
already been taken by our other commands, which means that only the values
from sixteen up are available to us for long run counts.  Thus, we simply
normalize our long run counts, by subtracting |(13-dyn_f)*16+dyn_f+1| and
adding 16, and then we represent the result according to the scheme above.

@ The final algorithm for decoding the run counts based on the above scheme
looks like this, assuming that a procedure called \\{pk\_nyb} is available
to get the next nybble from the file, and assuming that the global
|repeat_count| indicates whether a row needs to be repeated.  Note that this
routine is recursive, but since a repeat count can never directly follow
another repeat count, it can only be recursive to one level.

@<Packed number procedure@>=
function pk_packed_num : integer ;
var i, @!j : integer ;
begin
   i := get_nyb ;
   if i = 0 then begin
      repeat j := get_nyb ; incr(i) ; until j <> 0 ;
      while i > 0 do begin j := j * 16 + get_nyb ; decr(i) ; end ;
      pk_packed_num := j - 15 + (13-dyn_f)*16 + dyn_f ;
   end else if i <= dyn_f then
      pk_packed_num := i
   else if i < 14 then
      pk_packed_num := (i-dyn_f-1)*16+get_nyb+dyn_f+1
   else begin
      if repeat_count <> 0 then abort('Second repeat count for this row!') ;
@.Second repeat count...@>
      repeat_count := 1; {prevent recursion more than one level}
      if i = 14 then repeat_count := pk_packed_num;
      send_out(true, repeat_count) ;
      pk_packed_num := pk_packed_num ;
   end ;
end ;

@ For low resolution fonts, or characters with `gray' areas, run encoding can
often make the character many times larger.  Therefore, for those characters
that cannot be encoded efficiently with run counts, the \.{PK} format allows
bit-mapping of the characters.  This is indicated by a |dyn_f| value of
14.  The bits are packed tightly, by concatenating all of the horizontal raster
rows into one long string, and then packing this string eight bits to a byte.
The number of bytes required can be calculated by |(width*height+7) div 8|.
This format should only be used when packing the character by run counts takes
more bytes than this, although, of course, it is legal for any character.
Any extra bits in the last byte should be set to zero.

@ At this point, we are ready to introduce the format for a character
descriptor.  It consists of three parts: a flag byte, a character preamble,
and the raster data.  The most significant four bits of the flag byte
yield the |dyn_f| value for that character.  (Notice that only values of
0 through 14 are legal for |dyn_f|, with 14 indicating a bit mapped character;
thus, the flag bytes do not conflict with the command bytes, whose upper nybble
is always 15.)  The next bit (with weight 8) indicates whether the first run
count is a black count or a white count, with a one indicating a black count.
For bit-mapped characters, this bit should be set to a zero.  The next bit
(with weight 4) indicates whether certain later parameters (referred to as size
parameters) are given in one-byte or two-byte quantities, with a one indicating
that they are in two-byte quantities.  The last two bits are concatenated on to
the beginning of the packet-length parameter in the character preamble,
which will be explained below.

However, if the last three bits of the flag byte are all set (normally
indicating that the size parameters are two-byte values and that a 3 should be
prepended to the length parameter), then a long format of the character
preamble should be used instead of one of the short forms.

Therefore, there are three formats for the character preamble; the one that
is used depends on the least significant three bits of the flag byte.  If the
least significant three bits are in the range zero through three, the short
format is used.  If they are in the range four through six, the extended short
format is used.  Otherwise, if the least significant bits are all set, then
the long form of the character preamble is used.  The preamble formats are
explained below.

\yskip\hang Short form: |flag[1]| |pl[1]| |cc[1]| |tfm[3]| |dm[1]| |w[1]|
|h[1]| |hoff[+1]| |voff[+1]|.
If this format of the character preamble is used, the above
parameters must all fit in the indicated number of bytes, signed or unsigned
as indicated.  Almost all of the standard \TeX\ font characters fit; the few
exceptions are fonts such as \.{cminch}.

\yskip\hang Extended short form: |flag[1]| |pl[2]| |cc[1]| |tfm[3]| |dm[2]|
|w[2]| |h[2]| |hoff[+2]| |voff[+2]|.  Larger characters use this extended
format.

\yskip\hang Long form: |flag[1]| |pl[4]| |cc[4]| |tfm[4]| |dx[4]| |dy[4]|
|w[4]| |h[4]| |hoff[4]| |voff[4]|.  This is the general format that
allows all of the
parameters of the \.{GF} file format, including vertical escapement.
\vskip\baselineskip
The |flag| parameter is the flag byte.  The parameter |pl| (packet length)
contains the offset
of the byte following this character descriptor, with respect to the beginning
of the |tfm| width parameter.  This is given so a \.{PK} reading program can,
once it has read the flag byte, packet length, and character code (|cc|), skip
over the character by simply reading this many more bytes.  For the two short
forms of the character preamble, the last two bits of the flag byte should be
considered the two most-significant bits of the packet length.  For the short
format, the true packet length might be calculated as |(flag mod 4)*256+pl|;
for the short extended format, it might be calculated as
|(flag mod 4)*65536+pl|.

The |w| parameter is the width and the |h| parameter is the height in pixels
of the minimum bounding box.  The |dx| and |dy| parameters are the horizontal
and vertical escapements, respectively.  In the short formats, |dy| is assumed
to be zero and |dm| is |dx| but in pixels;
in the long format, |dx| and |dy| are both
in pixels multiplied by $2^{16}$.  The |hoff| is the horizontal offset from the
upper left pixel to the reference pixel; the |voff| is the vertical offset.
They are both given in pixels, with right and down being positive.  The
reference pixel is the pixel that occupies the unit square in \MF; the
\MF\ reference point is the lower left hand corner of this pixel.  (See the
example below.)

@ \TeX\ requires all characters that have the same character codes
modulo 256 to have also the same |tfm| widths and escapement values.  The \.{PK}
format does not itself make this a requirement, but in order for the font to
work correctly with the \TeX\ software, this constraint should be observed.
(The standard version of \TeX\ cannot output character codes greater
than 255, but extended versions do exist.)

Following the character preamble is the raster information for the
character, packed by run counts or by bits, as indicated by the flag byte.
If the character is packed by run counts and the required number of nybbles
is odd, then the last byte of the raster description should have a zero
for its least significant nybble.

@ As an illustration of the \.{PK} format, the character \char4\ from the font
amr10 at 300 dots per inch will be encoded.  This character was chosen
because it illustrates some
of the borderline cases.  The raster for the character looks like this (the
row numbers are chosen for convenience, and are not \MF's row numbers.)

\vskip\baselineskip
{\def\smbox{\vrule height 7pt width 7pt depth 0pt \hskip 3pt}%
\catcode`\*=\active \let*=\smbox
\centerline{\vbox{\baselineskip=10pt
\halign{\hfil#\quad&&\hfil#\hfil\cr
0& & &*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*\cr
1& & &*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*\cr
2& & &*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*\cr
3& & &*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*\cr
4& & &*&*& & & & & & & & & & & & & & & & &*&*\cr
5& & &*&*& & & & & & & & & & & & & & & & &*&*\cr
6& & &*&*& & & & & & & & & & & & & & & & &*&*\cr
7\cr
8\cr
9& & & & &*&*& & & & & & & & & & & & &*&*& & \cr
10& & & & &*&*& & & & & & & & & & & & &*&*& & \cr
11& & & & &*&*& & & & & & & & & & & & &*&*& & \cr
12& & & & &*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*& & \cr
13& & & & &*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*& & \cr
14& & & & &*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*& & \cr
15& & & & &*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*& & \cr
16& & & & &*&*& & & & & & & & & & & & &*&*& & \cr
17& & & & &*&*& & & & & & & & & & & & &*&*& & \cr
18& & & & &*&*& & & & & & & & & & & & &*&*& & \cr
19\cr
20\cr
21\cr
22& & &*&*& & & & & & & & & & & & & & & & &*&*\cr
23& & &*&*& & & & & & & & & & & & & & & & &*&*\cr
24& & &*&*& & & & & & & & & & & & & & & & &*&*\cr
25& & &*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*\cr
26& & &*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*\cr
27& & &*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*\cr
28&+& &*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*\cr
&\hphantom{*}&\hphantom{*}\cr
}}}}
The width of the minimum bounding box for this character is 20; its height
is 29.  The `+' represents the reference pixel; notice how it lies outside the
minimum bounding box.  The |hoff| value is $-2$, and the |voff| is~28.

The first task is to calculate the run counts and repeat counts.  The repeat
counts are placed at the first transition (black to white or white to black)
in a row, and are enclosed in brackets.  White counts are enclosed in
parentheses.  It is relatively easy to generate the counts list:
\vskip\baselineskip
\centerline{82 [2] (16) 2 (42) [2] 2 (12) 2 (4) [3]}
\centerline{16 (4) [2] 2 (12) 2 (62) [2] 2 (16) 82}
\vskip\baselineskip
Note that any duplicated rows that are not all white or all black are removed
before the run counts are calculated.  The rows thus removed are rows 5, 6,
10, 11, 13, 14, 15, 17, 18, 23, and 24.

@ The next step in the encoding of this character is to calculate the optimal
value of |dyn_f|.  The details of how this calculation is done are not
important here; suffice it to say that there is a simple algorithm that can
determine the best value of |dyn_f| in one pass over the count list.  For this
character, the optimal value turns out to be 8 (atypically low).  Thus, all
count values less than or equal to 8 are packed in one nybble; those from
nine to $(13-8)*16+8$ or 88 are packed in two nybbles.  The run encoded values
now become (in hex, separated according to the above list):
\vskip\baselineskip
\centerline{\tt D9 E2 97 2 B1 E2 2 93 2 4 E3}
\centerline{\tt 97 4 E2 2 93 2 C5 E2 2 97 D9}
\vskip\baselineskip\noindent
which comes to 36 nybbles, or 18 bytes.  This is shorter than the 73 bytes
required for the bit map, so we use the run count packing.

@ The short form of the character preamble is used because all of the
parameters fit in their respective lengths.  The packet length is therefore
18 bytes for the raster, plus
eight bytes for the character preamble parameters following the character
code, or 26.  The |tfm| width for this character is 640796, or {\tt 9C71C} in
hexadecimal.  The horizontal escapement is 25 pixels.  The flag byte is
88 hex, indicating the short preamble, the black first count, and the
|dyn_f| value of 8.  The final total character packet, in hexadecimal, is:
\vskip\baselineskip
$$\vbox{\halign{\hfil #\quad&&{\tt #\ }\cr
Flag byte&88\cr
Packet length&1A\cr
Character code&04\cr
|tfm| width&09&C7&1C\cr
Horizontal escapement (pixels)&19\cr
Width of bit map&14\cr
Height of bit map&1D\cr
Horizontal offset (signed)&FE\cr
Vertical offset&1C\cr
Raster data&D9&E2&97\cr
&2B&1E&22\cr
&93&24&E3\cr
&97&4E&22\cr
&93&2C&5E\cr
&22&97&D9\cr}}$$
********************************************************************/


#define PK_XXX1 240
#define PK_XXX2 241
#define PK_XXX3 242
#define PK_XXX4 243
#define PK_YYY 244
#define PK_POST 245
#define PK_NO_OP 246
#define PK_PRE 247
  

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggMakeFont::
EggMakeFont() : EggWriter(true, false) {
  set_program_description
    ("egg-mkfont reads a rasterized font stored in a "
     "Metafont/TeX pk file format, and generates an egg "
     "file and corresponding texture map that can be used with "
     "Panda's TextNode object to render text in the font.\n\n"

     "The input pk file can come from any of a number of sources.  "
     "It may be a Metafont font that ships with TeX, or it may "
     "have been converted from a PostScript font, or you can use "
     "freetype (see www.freetype.org) to convert a TTF font to pk.\n\n");


  clear_runlines();
  add_runline("[opts] -o output.egg file.pk");
  add_runline("[opts] file.pk output.egg");

  add_option
    ("i", "filename", 0, 
     "Name of the texture image to write.  The default if this is omitted "
     "is based on the name of the egg file.",
     &EggMakeFont::dispatch_filename, NULL, &_output_image_filename);

  add_option
    ("c", "num", 0, 
     "Specifies the number of channels of the output image.  This should "
     "either 1, 2, 3, or 4.  If the number is 1 or 3 a grayscale image is "
     "generated, with the text in white on black.  If the number is 2 or 4 "
     "a completely white image is generated, with the text in the alpha "
     "channel.  This parameter may also be specified as the third number "
     "on -d, below.  The default is 1.",
     &EggMakeFont::dispatch_int, NULL, &_output_zsize);

  add_option
    ("d", "x,y[,c]", 0, 
     "Dimensions in pixels of the texture image, with an optional number of "
     "channels.  Normally, you should not specify this parameter, as "
     "egg-mkfont will choose an image size that yields a scale factor "
     "between 2.5 and 4, which leads to good antialiased letters.  If you "
     "want a larger or smaller image, you could force the image size "
     "with this parameter, but it would probably yield better results if "
     "you re-rasterized the font at a different DPI instead.",
     &EggMakeFont::dispatch_dimensions, &_got_output_size, (void *)this);

  add_option
    ("g", "radius", 0, 
     "The radius of the Gaussian filter used to antialias the letters. [1.2]",
     &EggMakeFont::dispatch_double, NULL, &_gaussian_radius);

  add_option
    ("b", "n", 0, 
     "The number of buffer pixels between two adjacent characters in "
     "the palette image. [4.0]",
     &EggMakeFont::dispatch_double, NULL, &_buffer_pixels);

  add_option
    ("B", "n", 0, 
     "The number of extra pixels around a single character in the "
     "generated polygon. [1.0]",
     &EggMakeFont::dispatch_double, NULL, &_poly_pixels);

  add_option
    ("p", "n", 0, 
     "Points per unit: the size of the egg characters relative to the "
     "point size of the font.  This is the number of points per each "
     "unit in the egg file.  Set it to zero to normalize the "
     "font height to 1 unit. [12.0]",
     &EggMakeFont::dispatch_double, NULL, &_ppu);

  add_option
    ("all", "", 0, 
     "Extract all the characters in the font.  Normally, only the "
     "ASCII characters in the range 33 .. 127 are extracted.",
     &EggMakeFont::dispatch_none, &_get_all);

  add_option
    ("only", "'chars'", 0, 
     "Extract *only* the indicated characters from the font.  The parameter "
     "should be a quoted string of letters and symbols that are to be "
     "extracted.  If the hyphen appears, it indicates a range of characters, "
     "e.g. A-Z to extract all the capital letters.  If the hyphen appears "
     "as the first or last character it loses its special meaning.",
     &EggMakeFont::dispatch_string, NULL, &_only_chars);

  _output_xsize = 256;
  _output_ysize = 256;
  _output_zsize = 1;
  _buffer_pixels = 4.0;
  _poly_pixels = 1.0;
  _scale_factor = 3.0;
  _gaussian_radius = 1.2;
  _ppu = 12.0;
}


////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool EggMakeFont::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "Must specify name of pk file on command line.\n";
    return false;
  }

  _input_pk_filename = args[0];
  args.pop_front();
  return EggWriter::handle_args(args);
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::dispatch_dimensions
//       Access: Protected, Static
//  Description: Reads the dimensions of the output image and stores
//               them in _output_[xyz]size.
////////////////////////////////////////////////////////////////////
bool EggMakeFont::
dispatch_dimensions(const string &opt, const string &arg, void *data) {
  EggMakeFont *me = (EggMakeFont *)data;
  return me->ns_dispatch_dimensions(opt, arg);
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::ns_dispatch_dimensions
//       Access: Protected
//  Description: Reads the dimensions of the output image and stores
//               them in _output_[xyz]size.
////////////////////////////////////////////////////////////////////
bool EggMakeFont::
ns_dispatch_dimensions(const string &opt, const string &arg) {
  vector_string words;
  tokenize(arg, words, ",");

  bool okflag = false;
  if (words.size() == 2) {
    okflag =
      string_to_int(words[0], _output_xsize) &&
      string_to_int(words[1], _output_ysize);

  } else if (words.size() == 3) {
    okflag =
      string_to_int(words[0], _output_xsize) &&
      string_to_int(words[1], _output_ysize) &&
      string_to_int(words[2], _output_zsize);
  }

  if (!okflag) {
    nout << "-" << opt
	 << " requires two or three integers separated by commas.\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::get_uv
//       Access: Private
//  Description: Given the X, Y coordinates of a particular pixel on
//               the image, return the corresponding UV coordinates.
////////////////////////////////////////////////////////////////////
TexCoordd EggMakeFont::
get_uv(double x, double y) {
  return TexCoordd(x / (double)_working_xsize, 
		   ((double)_working_ysize - y) / (double)_working_ysize);
}


////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::get_xy
//       Access: Private
//  Description: Given X, Y coordinates in pixels, scale to unit
//               coordinates for the character's geometry.
////////////////////////////////////////////////////////////////////
LPoint2d EggMakeFont::
get_xy(double x, double y) {
  return LPoint2d(x / (_hppp * _ppu), -y / (_vppp * _ppu));
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::copy_character
//       Access: Private
//  Description: Copy the indicated character image to its home on the
//               bitmap and generate egg structures for it.
////////////////////////////////////////////////////////////////////
void EggMakeFont::
copy_character(const CharPlacement &pl) {
  const CharBitmap *bm = pl._bm;
  int xp = pl._x;
  int yp = pl._y;

  int character = bm->_character;
  int hoff = bm->_hoff;
  int voff = bm->_voff;
  double dx = bm->_dx;
  double dy = bm->_dy;
  int width = bm->get_width();
  int height = bm->get_height();

  // First, create an egg group to hold the character.

  string group_name = format_string(character);
  PT(EggGroup) group = new EggGroup(group_name);
  _egg_defs[character] = group;

  // Now copy the character into the image.
  if (_output_image.has_alpha()) {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
	if (bm->_block[y][x]) {
	  _output_image.set_alpha(xp + x, yp + y, 1.0);
	}
      }
    }
  } else {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
	if (bm->_block[y][x]) {
	  _output_image.set_xel(xp + x, yp + y, 1.0, 1.0, 1.0);
	}
      }
    }
  }

  // Create the polygon that will have the character mapped onto it.
    
  // b is the number of pixels bigger than the character in each
  // direction the polygon will be.  It needs to be larger than zero
  // just because when we filter the image down, we end up with some
  // antialiasing blur that extends beyond the original borders of the
  // character.  But it shouldn't be too large, because we don't want
  // the neighboring polygons of a word to overlap any more than they
  // need to.

  double b = _working_poly_pixels;
    
  TexCoordd uv_ul = get_uv(xp - b, yp - b);
  TexCoordd uv_lr = get_uv(xp + width + b, yp + height + b);
  LPoint2d xy_ul = get_xy(-hoff - b, -voff - b);
  LPoint2d xy_lr = get_xy(-hoff + width + b, -voff + height + b);

  EggVertex *v1 = _vpool->make_new_vertex(LPoint3d(xy_ul[0], xy_lr[1], 0.0));
  EggVertex *v2 = _vpool->make_new_vertex(LPoint3d(xy_lr[0], xy_lr[1], 0.0));
  EggVertex *v3 = _vpool->make_new_vertex(LPoint3d(xy_lr[0], xy_ul[1], 0.0));
  EggVertex *v4 = _vpool->make_new_vertex(LPoint3d(xy_ul[0], xy_ul[1], 0.0));

  v1->set_uv(TexCoordd(uv_ul[0], uv_lr[1]));
  v2->set_uv(TexCoordd(uv_lr[0], uv_lr[1]));
  v3->set_uv(TexCoordd(uv_lr[0], uv_ul[1]));
  v4->set_uv(TexCoordd(uv_ul[0], uv_ul[1]));
    
  EggPolygon *poly = new EggPolygon();
  group->add_child(poly);
  poly->set_texture(_tref);
    
  poly->add_vertex(v1);
  poly->add_vertex(v2);
  poly->add_vertex(v3);
  poly->add_vertex(v4);

  // Now create a single point where the origin of the next character
  // will be.

  LPoint2d dp = get_xy(dx, dy);
  
  EggVertex *v0 = _vpool->make_new_vertex(LPoint3d(dp[0], dp[1], 0.0));
  EggPoint *point = new EggPoint;
  group->add_child(point);
  point->add_vertex(v0);
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::consider_scale_factor
//       Access: Private
//  Description: Attempts to place all of the characters on an image
//               of the indicated size (scaled up from the output
//               image size by scale_factor in each dimension).
//               Returns true if all the characters fit, or false if
//               the image was too small.  In either case, leaves
//               _scale_factor and _working_* set to reflect the
//               chosen scale factor.
////////////////////////////////////////////////////////////////////
bool EggMakeFont::
consider_scale_factor(double scale_factor) {
  _scale_factor = scale_factor;
  _working_xsize = (int)floor(_output_xsize * _scale_factor + 0.5);
  _working_ysize = (int)floor(_output_ysize * _scale_factor + 0.5);
  _working_buffer_pixels = (int)floor(_buffer_pixels * _scale_factor + 0.5);

  _layout.reset(_working_xsize, _working_ysize, _working_buffer_pixels);

  Chars::iterator ci;
  bool ok = true;
  for (ci = _chars.begin(); ci != _chars.end() && ok; ++ci) {
    ok = _layout.place_character(*ci);
    if (!ok) {
      // Out of room.
      return false;
    }
  }

  // They all fit!
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::choose_scale_factor
//       Access: Private
//  Description: Binary search on scale factor, given a factor that is
//               known to be too small and one that is known to be too
//               large.
////////////////////////////////////////////////////////////////////
void EggMakeFont::
choose_scale_factor(double too_small, double too_large) {
  if (too_large - too_small < 0.000001) {
    // Close enough.
    consider_scale_factor(too_large);
    return;
  }

  double mid = (too_small + too_large) / 2.0;
  if (consider_scale_factor(mid)) {
    // This midpoint is too large.
    choose_scale_factor(too_small, mid);
  } else {
    // This midpoint is too small.
    choose_scale_factor(mid, too_large);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::choose_scale_factor
//       Access: Private
//  Description: Tries several scale_factors until an optimal one
//               (that is, the smallest one that all letters fit
//               within) is found.  Returns when all characters have
//               been successfully placed on the layout.
//
//               Returns true if successful, or false if it just could
//               not be done.
////////////////////////////////////////////////////////////////////
bool EggMakeFont::
choose_scale_factor() {
  // We need to determine a scale factor that will definitely be too
  // small, and one that will definitely be too large.
  double too_small, too_large;
  int sanity_count = 0;

  double guess = 1.0;
  if (consider_scale_factor(guess)) {
    // This guess is too large.
    do {
      too_large = guess;
      guess = guess / 2.0;
      if (sanity_count++ > 20) {
	return false;
      }
    } while (consider_scale_factor(guess));
    too_small = guess;

  } else {
    // This guess is too small.
    do {
      too_small = guess;
      guess = guess * 2.0;
      if (sanity_count++ > 20) {
	return false;
      }
    } while (!consider_scale_factor(guess));
    too_large = guess;
  }

  choose_scale_factor(too_small, too_large);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::choose_image_size
//       Access: Private
//  Description: Chooses a size for the output image that should yield
//               a scale_factor in the range (2.5 .. 4], which will give
//               pretty good antialiased letters.
////////////////////////////////////////////////////////////////////
void EggMakeFont::
choose_image_size() {
  // Start with an arbitrary guess.
  _output_xsize = 256;
  _output_ysize = 256;

  bool sane = choose_scale_factor();

  if (sane && _scale_factor <= 2.5) {
    // The scale factor is too small.  The letters may appear jaggy,
    // and we don't need so much image space.
    do {
      if (_output_ysize < _output_xsize) {
	_output_xsize /= 2;
      } else {
	_output_ysize /= 2;
      }
      sane = choose_scale_factor();
    } while (sane && _scale_factor <= 2.5);

    if (!sane) {
      // Oops, better backpedal.
      _output_xsize *= 2;
    }
    choose_scale_factor();

  } else if (_scale_factor > 4.0) {
    // The scale factor is too large.  The letters will be overly
    // reduced and may be blurry.  We need a larger image.
    do {
      if (_output_ysize < _output_xsize) {
	_output_ysize *= 2;
      } else {
	_output_xsize *= 2;
      }
      sane = choose_scale_factor();
    } while (!sane || _scale_factor > 4.0);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::fetch_nibble
//       Access: Private
//  Description: Returns the next 4-bit nibble from the pk stream.
////////////////////////////////////////////////////////////////////
unsigned int EggMakeFont::
fetch_nibble() {
  assert(_p < (int)_pk.size());
  if (_high) {
    _high = false;
    return _pk[_p] >> 4;
  } else {
    _high = true;
    return _pk[_p++] & 0xf;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::fetch_packed_int
//       Access: Private
//  Description: Returns the next packed integer from the pk stream.
////////////////////////////////////////////////////////////////////
unsigned int EggMakeFont::
fetch_packed_int() {
  int i = fetch_nibble();
  if (i == 0) {
    int j;
    do {
      j = fetch_nibble();
      i++;
    } while (j == 0);
    while (i > 0) {
      j = (j << 4) | fetch_nibble();
      i--;
    }
    return j - 15 + (13 - _dyn_f)*16 + _dyn_f;

  } else if (i <= _dyn_f) {
    return i;

  } else if (i < 14) { 
    return (i - _dyn_f - 1)*16 + fetch_nibble() + _dyn_f + 1;

  } else {
    _repeat_count = 1;
    if (i == 14) {
      _repeat_count = fetch_packed_int();
    }
    //    nout << "[" << _repeat_count << "]";
    return fetch_packed_int();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::fetch_byte
//       Access: Private
//  Description: Returns the next 8-bit unsigned byte from the pk
//               stream.
////////////////////////////////////////////////////////////////////
unsigned int EggMakeFont::
fetch_byte() {
  assert(_high);
  assert(_p < (int)_pk.size());
  return _pk[_p++];
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::fetch_int
//       Access: Private
//  Description: Returns the next n-byte unsigned int from
//               the pk stream.
////////////////////////////////////////////////////////////////////
unsigned int EggMakeFont::
fetch_int(int n) {
  assert(_high);

  unsigned int result = 0;
  for (int i = 0; i < n; i++) {
    assert(_p < (int)_pk.size());
    result = (result << 8) | _pk[_p];
    _p++;
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::fetch_signed_int
//       Access: Private
//  Description: Returns the next n-byte signed int from
//               the pk stream.
////////////////////////////////////////////////////////////////////
int EggMakeFont::
fetch_signed_int(int n) {
  assert(_high);

  assert(_p < (int)_pk.size());
  int result = (signed char)_pk[_p];
  _p++;
  for (int i = 1; i < n; i++) {
    assert(_p < (int)_pk.size());
    result = (result << 8) | _pk[_p];
    _p++;
  }

  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::do_character
//       Access: Private
//  Description: Reads a single character from the pk file and
//               processes it.  Returns true if successful, false if
//               something bad happened.
////////////////////////////////////////////////////////////////////
bool EggMakeFont::
do_character(int flag_byte) {
  //  int start_p = _p - 1;
  _dyn_f = (flag_byte >> 4);
  bool first_black = ((flag_byte & 0x8) != 0);
  int bsize = (flag_byte & 0x4) ? 2 : 1;
  int prepend_length = (flag_byte & 0x3);

  bool use_long_form = ((flag_byte & 0x7) == 0x7);
  
  unsigned int pl, cc, itfm, w, h;
  int hoff, voff;
  unsigned int idx = 0;
  unsigned int idy = 0;
  int next_p;

  if (use_long_form) {
    pl = fetch_int();
    cc = fetch_int();
    next_p = _p + pl;
    itfm = fetch_int();
    idx = fetch_int();
    idy = fetch_int();
    w = fetch_int();
    h = fetch_int();
    hoff = fetch_signed_int();
    voff = fetch_signed_int();
  } else {
    pl = fetch_int(bsize) | (prepend_length << bsize*8);
    cc = fetch_byte();
    next_p = _p + pl;
    itfm = fetch_int(3);
    idx = fetch_int(bsize) << 16;
    w = fetch_int(bsize);
    h = fetch_int(bsize);
    hoff = fetch_signed_int(bsize);
    voff = fetch_signed_int(bsize);
  }

  //  double tfm = (double)itfm / (double)(1 << 24);
  double dx = (double)idx / (double)(1 << 16);
  double dy = (double)idy / (double)(1 << 16);
  //  double di_width = tfm * _ppu * _hppp / _vppp;

  if (_get_all || 
      ((cc >= 33 && cc <= 127) &&
       (_only_chars.empty() || _only_chars.find((char)cc) != string::npos))) {
    nout << " " << cc;

    CharBitmap *bm = new CharBitmap(cc, w, h, hoff, voff, dx, dy);
      
    if (_dyn_f == 14) {
      // A bitmapped character: this character has the actual w x h
      // bits stored directly in the pk file.  This kind of character
      // is quite rare, and the code is therefore untested.
      if (h > 0 && w > 0) {
	nout 
	  << "\nA rare bitmapped character encountered!  You are now running\n"
	  << "untested code.  If this works, change this line in the program\n"
	  << "to indicate that the code is actually tested!\n\n";
      }

      unsigned int bit = 0;
      unsigned int byte = 0;
      for (unsigned int y = 0; y < h; y++) {
	for (unsigned int x = 0; x < w; x++) {
	  if (bit == 0) {
	    bit = 0x80;
	    byte = fetch_byte();
	  }
	  bm->_block[y][x] = ((byte & bit)!=0);
	  bit >>= 1;
	}
      }
      
    } else {
      // A normal, rle character.  This character has sequences of
      // black and white runs stored in the pk file.  Most characters
      // will be stored this way.
      bool black = first_black;
      _repeat_count = 0;
      
      int count = fetch_packed_int();
      while (bm->paint(black, count, _repeat_count)) {
	/*
	  if (black) {
	  nout << count;
	  } else {
	  nout << "(" << count << ")";
	  }
	  */
	black = !black;
	count = fetch_packed_int();
      }
      //    nout << "\n";
    }

    _chars.push_back(bm);

    /*
    for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
	nout << (bm->_block[y][x] ? ' ' : '*');
      }
      nout << "\n";
    }
    */
    
    if (!_high) {
      _p++;
      _high = true;
    }

    if (_p != next_p) {
      nout << "Expected p == " << next_p << " got " << _p << "\n";
    }

  } else {
    nout << " (" << cc << ")";
  }

  _p = next_p;
  return true;
}
  

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::do_xxx
//       Access: Private
//  Description: The xxx1 .. xxx4 series of commands specify an
//               embedded comment or some such silliness in the pk
//               file that must be skipped.
////////////////////////////////////////////////////////////////////
void EggMakeFont::
do_xxx(int num_bytes) {
  _p += fetch_int(num_bytes);
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::do_yyy
//       Access: Private
//  Description: The yyy command is an encoded number which might have
//               meaning to a preceding xxx block, but means nothing
//               to us.
////////////////////////////////////////////////////////////////////
void EggMakeFont::
do_yyy() {
  _p += 4;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::do_post
//       Access: Private
//  Description: The beginning of the postamble.
////////////////////////////////////////////////////////////////////
void EggMakeFont::
do_post() {
  _post = true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::do_pre
//       Access: Private
//  Description: The preamble.
////////////////////////////////////////////////////////////////////
void EggMakeFont::
do_pre() {
  int id = fetch_byte();
  if (id != 89) {
    nout << "Warning: PK file had an unexpected ID, " << id << "\n";
  }

  int comment_len = fetch_byte();

  assert(_p + comment_len <= (int)_pk.size());
  nout.write(&_pk[_p], comment_len);
  nout << "\n";
  _p += comment_len;

  int ds = fetch_int();
  fetch_int();  // cs
  int hppp = fetch_int();  // hppp
  int vppp = fetch_int();  // vppp

  _ds = (double)ds / (double)(1 << 20);
  _hppp = (double)hppp / (double)(1 << 16);
  _vppp = (double)vppp / (double)(1 << 16);

  nout << "Font size is " << _ds << " points, rasterized at " 
       << floor(_vppp * 72.27 + 0.5) << " DPI.\n";
}


////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::read_pk
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void EggMakeFont::
read_pk() {
  if (_p >= (int)_pk.size()) {
    nout << "PK file is empty.\n";
    exit(1);
  }
  unsigned int cmd = fetch_byte();
  if (cmd != PK_PRE) {
    nout << "Not a PK file.\n";
    exit(1);
  }
  do_pre();

  nout << "Characters:";

  while (_p < (int)_pk.size()) {
    unsigned int cmd = fetch_byte();
    if (_post && !_post_warning && cmd != PK_NO_OP) {
      _post_warning = true;
      nout << "\nWarning: postamble was not the last command.\n";
    }
    if (cmd < 240) {
      if (!do_character(cmd)) {
	return;
      }
    } else {
      switch (cmd) {
      case PK_XXX1: 
	do_xxx(1);
	break;
	
      case PK_XXX2:
	do_xxx(2);
	break;
	
      case PK_XXX3:
	do_xxx(3);
	break;
	
      case PK_XXX4:
	do_xxx(4);
	break;
	
      case PK_YYY:
	do_yyy();
	break;
	
      case PK_POST:
	do_post();
	break;
	
      case PK_NO_OP:
	break;
	
      default:
	nout << "\nUnexpected command " << cmd << " encountered in PK file\n";
	exit(1);
      }
    }
  }
  nout << "\n";

  if (!_post) {
    nout << "Warning: did not encounter postamble.\n";
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::expand_hyphen
//       Access: Public
//  Description: If a hyphen appears in the string anywhere but in the
//               first and last position, replace it with a sequence
//               of characters.  For example, 0-9 becomes 0123456789.
//               Return the new string.
////////////////////////////////////////////////////////////////////
string EggMakeFont::
expand_hyphen(const string &str) {
  string result;
  size_t last = 0;

  size_t hyphen = str.find('-', 1);
  while (hyphen < str.length() - 1) {
    size_t ap = hyphen - 1;
    size_t zp = hyphen + 1;
    result += str.substr(last, ap - last);
    char a = str[ap];
    char z = str[zp];

    for (char i = a; i <= z; i++) {
      result += i;
    }

    last = zp;
    hyphen = str.find('-', last);
  }

  result += str.substr(last);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::run
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void EggMakeFont::
run() {
  if (_output_image_filename.empty() && has_output_filename()) {
    _output_image_filename = get_output_filename();
    _output_image_filename.set_extension("rgb");
  }

  if (_output_image_filename.empty()) {
    nout << "No output image filename given.\n";
    exit(1);
  }

  if (!_only_chars.empty()) {
    _only_chars = expand_hyphen(_only_chars);
    nout << "Extracting only characters: " << _only_chars << "\n";
  }

  _input_pk_filename.set_binary();
  ifstream pk_file;
  if (!_input_pk_filename.open_read(pk_file)) {
    nout << "Unable to read " << _input_pk_filename << "\n";
    exit(1);
  }
  
  unsigned char c = pk_file.get();
  while (pk_file && !pk_file.eof()) {
    _pk.push_back(c);
    c = pk_file.get();
  }

  _p = 0;
  _high = true;
  _post = false;
  _post_warning = false;
  read_pk();

  nout << "Placing " << _chars.size() << " letters.\n";

  // Now that we've collected all the characters, sort them in order
  // from tallest to shortest so we will hopefully get a more optimal
  // packing.
  sort(_chars.begin(), _chars.end(), SortCharBitmap());

  // Choose a suitable image size if the user didn't specify one.
  if (!_got_output_size) {
    choose_image_size();

  } else {
    // The user constrained us to use a particular image size, so
    // choose a suitable scale factor for that size.
    choose_scale_factor();
  }

  _working_poly_pixels = _poly_pixels * _scale_factor;
  _output_image.clear(_working_xsize, _working_ysize, _output_zsize);

  if (_output_image.has_alpha()) {
    // If we're generating an image with an alpha channel, the font
    // detail goes entirely into the alpha channel, and the color
    // channel(s) are pure white.
    _output_image.alpha_fill(0.0);
    _output_image.fill(1.0, 1.0, 1.0);

  } else {
    // If we're generating an image with no alpha channel, the font
    // detail goes into the color channel(s).
    _output_image.fill(0.0, 0.0, 0.0);
  }

  _group = new EggGroup();
  _data.add_child(_group);
  _tref = new EggTexture("chars", _output_image_filename);
  _group->add_child(_tref);
  _vpool = new EggVertexPool("vpool");
  _group->add_child(_vpool);

  // Make the group a sequence, as a convenience.  If we view the
  // egg file directly we can see all the characters one at a time.
  _group->set_switch_flag(true);
  _group->set_switch_fps(2.0);

  if (_ppu <= 0.0) {
    // If the user set ppu to 0, it means to normalize the character
    // height.
    _ppu = _ds;
  }

  // Now we can copy all the characters onto the actual image.
  CharLayout::Placements::const_iterator pi;
  for (pi = _layout._placements.begin();
       pi != _layout._placements.end();
       ++pi) {
    copy_character(*pi);
  }

  // And now put all the Egg structures we created into the egg file,
  // in numeric order.
  EggDefs::const_iterator edi;
  for (edi = _egg_defs.begin(); edi != _egg_defs.end(); ++edi) {
    _group->add_child((*edi).second.p());
  }

  // Also create an egg group indicating the font's design size.
  PT(EggGroup) ds_group = new EggGroup("ds");
  EggVertex *vtx = _vpool->make_new_vertex(LPoint3d(0.0, _ds / _ppu, 0.0));
  EggPoint *point = new EggPoint;
  ds_group->add_child(point);
  point->add_vertex(vtx);

  // All done!  Write everything out.
  nout << "Scale factor is " << _scale_factor << "\n";
  PNMImage small(_output_xsize, _output_ysize, _output_zsize);
  small.gaussian_filter_from(_gaussian_radius, _output_image);
  
  nout << "Generating " << _output_xsize << " by " << _output_ysize
       << " by " << _output_zsize << " image: "
       << _output_image_filename << "\n";
  small.write(_output_image_filename);

  _data.write_egg(get_output());
}


int main(int argc, char *argv[]) {
  EggMakeFont prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
