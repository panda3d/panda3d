

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
