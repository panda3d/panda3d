"""
You can use :func:`verify()` just like assert, with these small differences:

- you may need to ``import Verify``, if someone hasn't done it for you.

- unlike assert where using parentheses are optional, :func:`verify()`
  requires them, e.g.::

    assert foo  # OK
    verify foo  # Error
    assert foo  # Not Recomended (may be interpreted as a tuple)
    verify(foo) # OK

- :func:`verify()` will print something like this before raising an exception::

    verify failed:
        File "direct/src/showbase/ShowBase.py", line 60

- :func:`verify()` will optionally start pdb for you (this is currently false
  by default).  You can either edit Verify.py to set ``wantVerifyPdb = 1`` or
  if you are using ShowBase you can set ``want-verify-pdb 1`` in your
  Config.prc file to start pdb automatically.

- :func:`verify()` will still function in the release build.  It will not be
  removed by -O like assert will.

:func:`verify()` will also throw an AssertionError, but you can ignore that if
you like (I don't suggest trying to catch it, it's just doing it so that it can
replace assert more fully).

Please do not use assert for things that you want run on release builds.
That is a bad thing to do.  One of the main reasons that assert exists
is to stip out debug code from a release build.  The fact that it throws
an exception can get it mistaken for an error handler.  If your code
needs to handle an error or throw an exception, you should do that
(and not just assert for it).

If you want to be a super keen software engineer then avoid using
:func:`verify()`.  If you want to be, or already are, a super keen software
engineer, but you don't always have the time to write proper error handling,
go ahead and use :func:`verify()` -- that's what it's for.

Please use assert (properly) and do proper error handling; and use
:func:`verify()` only when debugging (i.e. when it won't be checked-in) or
where it helps you resist using assert for error handling.
"""

from panda3d.core import ConfigVariableBool

# Set to true to load pdb on failure.
wantVerifyPdb = ConfigVariableBool('want-verify-pdb', False)


def verify(assertion):
    """
    verify() is intended to be used in place of assert where you
    wish to have the assertion checked, even in release (-O) code.
    """
    if not assertion:
        print("\n\nverify failed:")
        import sys
        print("    File \"%s\", line %d" % (
                sys._getframe(1).f_code.co_filename,
                sys._getframe(1).f_lineno))
        if wantVerifyPdb:
            import pdb
            pdb.set_trace()
        raise AssertionError


if not hasattr(__builtins__, "verify"):
    __builtins__["verify"] = verify
