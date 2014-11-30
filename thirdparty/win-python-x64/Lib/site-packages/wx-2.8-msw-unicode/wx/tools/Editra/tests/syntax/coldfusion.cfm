<!--- Some comments about this file --->
<!-- Support is still a work in progress -->

<cfscript>
function helloWorld() {
   var greeting = "Hello World!";
   return greeting;
}
</cfscript>

<cfoutput>#helloWorld()#</cfoutput>

<cfset Foo1 = "Hello">
<cfset Foo2 = "Hello World">

<cfoutput>
<!--- pound signs output the returned value --->
<p>The first position of #Foo1# in #Foo2# is #Find(Foo1,Foo2)#</p>
</cfoutput>
<CFSCRIPT>
/**
 * Returns a list of all factors for a given 
 * positive integer.
 * 
 * @param integer   Any non negative integer greater 
 *                  than or equal to 1. 
 * @return          Returns a comma delimited list 
 *                  of values. 
 * @author Rob Brooks-Bilson (rbils@amkor.com) 
 * @version 1.1, September 6, 2001 
 */
function factor(integer)
{
  Var i=0; 
  Var Factors = "";
  for (i=1; i LTE integer/2; i=i+1) {
    if (Int(integer/i) EQ integer/i) {
      Factors = ListAppend(Factors, i);
    }
  }
  Return ListAppend(Factors, integer);
}
</CFSCRIPT>
