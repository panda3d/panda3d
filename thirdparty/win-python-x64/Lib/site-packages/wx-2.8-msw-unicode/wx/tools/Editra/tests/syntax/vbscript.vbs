' Syntax Highlighting test file for VBScript
' Comments look like this

function hello(name) 'comment for function

    on error resume next

    dim x,y

    if name<>"World" then
        for x = 1 to 10
            wscript.echo "Hello " & name
        next
    else
        x=0
        do while x<10
            wscript.echo "Hello World"
            x=x+1
        loop
    end if

    hello=x+y

end function

Public Function testPublicFunct()
    dim c,d
end Sub

sub testStub()
    dim a,b
end sub

Public Sub testPublicSub()
    dim c,d
end Sub

'---------------------------

on error resume next

dim didIt
dim userid

set WshShell = WScript.CreateObject("WScript.Shell")
'Finds the user name from an environment variable
userid=wshshell.expandenvironmentstrings("%username%")

didIt=hello(userid)

wscript.exit(0)
