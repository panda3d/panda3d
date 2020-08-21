"""
This module hooks into Python's import mechanism to print out all imports to
the standard output as they happen.
"""

__all__ = []


import sys

# Save a pointer to the old import function
oldimport = __import__

# The current indent level, every time we import, this is incremented
# so we can hierarchically see who imports who by the indentation
indentLevel = 0

# The new import function
def newimport(*args, **kw):
    global indentLevel
    fPrint = 0
    name = args[0]
    # Only print the name if we have not imported this before
    if name not in sys.modules:
        print((" "*indentLevel + "import " + args[0]))
        fPrint = 1
    indentLevel += 1
    result = oldimport(*args, **kw)
    indentLevel -= 1
    if fPrint:
        print((" "*indentLevel + "DONE: import " + args[0]))
    return result

# Replace the builtin import with our new import
__builtins__["__import__"] = newimport
