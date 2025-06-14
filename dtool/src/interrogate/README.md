A key advantage of Panda3D is that it provides developers with the
ability to use both C++ and Python simultaneously. Essentially, Panda3D
gives programmers the best of both worlds, as they are able to take
advantage of the high performance and low-level programming found in
C++ in addition to the flexibility, interactive scripting, and
rapid-prototyping capabilities of Python. This feature is made possible
due to Python’s ability to call C libraries, and ultimately make use of
Panda3D’s Interrogate System: an automated C++ Extension Module
generation utility similar to SWIG.  Although Python is the favored
scripting language of Panda3D, the engine is highly extensible in this
aspect, as any language that has a foreign function interface can make
use of the Interrogate System.

The Interrogate System works like a compiler by scanning and parsing
C++ code for the Panda3D-specific, “PUBLISHED” keyword. This keyword
marks the particular methods of a class that are to be exposed within a
C++ Extension Module for that class which is eventually generated. One
benefit of using the “PUBLISHED” keyword is that it alleviates the need
for an interface file that provides function prototypes for the class
methods that will be exposed within the extension module, as is the
case with SWIG. Interrogate turns a class into a loose collection of
Python interface wrapper functions that make up the C++ Extension
Module.

This package depends on the 'cppparser' package, which contains the
code that parses the C++ headers, and the 'interrogatedb' package,
which contains the intermediate representation of the interfaces that
can be saved to a database file for FFI generation tools to consume.
