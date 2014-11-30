// Syntax Highlighting test file for Flagship/XBase programming languages
// Some comments about this file
/* A multline comment about some stuff in this
 * file.
 */

// Hello World in Clipper
? "Hello World!"

 
USE address ALIAS adr SHARED NEW
SET COLOR TO "W+/B,GR+/R,W/B,W/B,GR+/BG"
cls
@  1, 0 SAY "Id No. " GET adr->IdNum   PICT "999999" VALID IdNum > 0
@  3, 0 SAY "Company" GET adr->Company
@  3,35 SAY "Branch"  GET adr->Branch  WHEN  !empty(adr->Company)
@  4, 0 SAY "Name   " GET adr->Name    VALID !empty(adr->Name)
@  4,35 SAY "First "  GET adr->First
@  6, 0 SAY "Country" GET adr->Country PICTURE "@!"
@  8, 0 SAY "Zip    " GET adr->Zip     PICT "@!" VALID !empty(adr->Zip)
@  9, 0 SAY "City   " GET adr->City
@ 10, 0 SAY "Street " GET adr->Street
READ
