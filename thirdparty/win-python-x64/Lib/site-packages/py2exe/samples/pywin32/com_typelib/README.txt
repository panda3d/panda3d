Some py2exe samples relating to the use of typelibs and pywin32.

pywin32's COM support takes advantage of COM typelibs by generating Python
stubs for the objects in these typelibs.  This generation is often known as
a 'makepy' process, from the name of the script that performs the generation,
but it is not always necessary to explcitly invoke makepy.py - the use of
win32com.client.gencache will often cause implicit generation of these stubs.

This directory contains samples showing how to use these techniques with 
py2exe apps.  It contains the following samples.

build_gen: contains samples that demonstrate how to build a typelib as py2exe
           is run.  This means the machine running py2exe must have the 
           typelibs installed locally, but the target machines need not.
           py2exe generates the typelib stubs as it is run.

           There is currently a single sample which assumes MSWord is 
           installed.  Please contribute samples for other common objects!

pre_gen:   contains samples that demonstrate how to package typelib stubs 
           previously generated.  Such stubs will have come from a previous
           invocation of makepy, possibly on another computer and possibly
           from a source control system.  In this case, the computer running
           py2exe does *not* need to have the relevant typelibs installed.
