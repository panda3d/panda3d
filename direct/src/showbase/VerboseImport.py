
import sys

# Save a pointer to the old import function
oldimport = __import__

# The current indent level, every time we import, this is incremented
# so we can hierarchically see who imports who by the indentation
indentLevel = 0

# The new import function
def newimport(*args, **kw):
    global indentLevel
    name = args[0]
    # Only print the name if we have not imported this before
    if not sys.modules.has_key(name):
        print (" "*indentLevel + "import " + args[0])
    indentLevel += 1
    result = oldimport(*args, **kw)
    indentLevel -= 1
    return result

# Replace the builtin import with our new import
__builtins__["__import__"] = newimport
