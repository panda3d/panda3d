"""
You can use verify() just like assert, with these small differences:
    - you may need to "from Verify import *", if someone hasn't done it
      for you.
    - unlike assert where using parenthises are optional, verify()
      requires them.
      e.g.:
        assert foo  # OK
        assert(foo) # OK
        verify foo  # Error
        verify(foo) # OK
    - verify() will print something like the following before raising
      an exception:
      
        verify failed:
            File "direct/src/showbase/ShowBase.py", line 60
    - verify() will optionally start pdb for you (this is currently
      true by default).
    - verify() will still function in the release build.

verify() will also throw an AssertionError, but you can ignore that if you
like (I don't suggest trying to catch it, it's just doing it so that it can
replace assert more fully).

Please do not use assert() for things that you want run on release builds.  
That is a bad thing to do.  One of the main reasons that assert exists 
is to stip out debug code from a release build.  The fact that it throws 
an exception can get it mistaken for an error handler.  If your code 
needs to handle an error or throw an exception, you should do that 
(and not just assert() for it).

If you want to be a super keen software engineer then avoid using verify().
If you want to be, or already are, a super keen software engineer, but 
you don't always have the time to write proper error handling, go ahead 
and use verify() -- that's what it's for.

Please use assert (properly) and do proper error handling; and use verify()
only where it helps you resist using assert for error handling.
"""

def verify(assertion):
    """
    verify() is intended to be used in place of assert where you
    wish to have the assertion checked, even in release (-O) code.
    """
    if not assertion:
        print "\n\nverify failed:"
        import sys
        print "    File \"%s\", line %d"%(
                sys._getframe(1).f_code.co_filename,
                sys._getframe(1).f_lineno)
        if 1:
            import pdb
            pdb.set_trace()
        raise AssertionError
