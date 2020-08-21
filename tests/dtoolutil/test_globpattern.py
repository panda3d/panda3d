from panda3d.core import GlobPattern


def test_globpattern_matches_file():
    patt = GlobPattern('/a/b/c')
    assert patt.matches_file('/a/b/c')
    assert patt.matches_file('///a////b//c')
    assert patt.matches_file('/a/b/././c')
    assert not patt.matches_file('')
    assert not patt.matches_file('/')
    assert not patt.matches_file('/a/b/d')
    assert not patt.matches_file('/A/b/c')
    assert not patt.matches_file('/a/b/c/')
    assert not patt.matches_file('/a/b/c/.')
    assert not patt.matches_file('a/b/c')
    assert not patt.matches_file('./a/b/c')

    # Test regular pattern
    patt = GlobPattern('*a')
    assert patt.matches_file('a')
    assert patt.matches_file('aa')
    assert patt.matches_file('xa')
    assert not patt.matches_file('A')
    assert not patt.matches_file('ax')
    assert not patt.matches_file('xax')

    # Test path ending in directory
    for patt in GlobPattern('/a/b/c/'), \
                GlobPattern('/a/b/c/.'), \
                GlobPattern('/a/b//c//'), \
                GlobPattern('/a/b/./c/./'):
        assert patt.matches_file('/a/b/c/')
        assert patt.matches_file('///a////b//c//')
        assert patt.matches_file('/a/b/././c/')
        assert patt.matches_file('/a/b/c/.')
        assert not patt.matches_file('/a/b/c')
        assert not patt.matches_file('/a/b/c/./d')
        assert not patt.matches_file('a/b/c/')
        assert not patt.matches_file('./a/b/c/')

    # Test globstar in middle
    for patt in GlobPattern('/a/**/c'), GlobPattern('/a/**/**/c'):
        assert patt.matches_file('/a/c')
        assert patt.matches_file('/a/b/c')
        assert patt.matches_file('/a/b/d/c')
        assert not patt.matches_file('/a/b/c/d')
        assert not patt.matches_file('/d/b/c')
        assert not patt.matches_file('/a/b/d')

    # Test globstar in beginning
    for patt in GlobPattern('/**/b/c'), GlobPattern('/**/**/**/b/c'):
        assert patt.matches_file('/a/b/c')
        assert patt.matches_file('/a/d/b/c')
        assert patt.matches_file('/a/b/c')
        assert patt.matches_file('/a/b/c/./b//c')
        assert not patt.matches_file('/a/b/c/d')
        assert not patt.matches_file('/a/c')
        assert not patt.matches_file('/a/b/d')

    # Test globstar at end
    for patt in GlobPattern('/a/b/**'), \
                GlobPattern('/a/b/**/**'), \
                GlobPattern('/a/b//**//**/**'):
        assert patt.matches_file('/a/b/')
        assert patt.matches_file('/a/b/.')
        assert patt.matches_file('/a/b//')
        assert patt.matches_file('/a/b/c')
        assert patt.matches_file('/a/b/c/d/e/f/g/h')
        assert patt.matches_file('/a/b/d/c')
        assert not patt.matches_file('/a/')
        assert not patt.matches_file('/a/c/b')

    # Test multiple globstars at multiple locations
    patt = GlobPattern('/a/**/b/**/c')
    assert patt.matches_file('/a/b/c')
    assert patt.matches_file('/a/./b/./c')
    assert patt.matches_file('/a//b//c')
    assert patt.matches_file('/a/x/y/b/c')
    assert patt.matches_file('/a/b/x/y/c')
    assert patt.matches_file('/a/b/c/a/b/c')
    assert patt.matches_file('/a/x/y/b/x/y/c')
    assert not patt.matches_file('/a/b/x')
    assert not patt.matches_file('/a/b/c/x')
    assert not patt.matches_file('/a/b/c/')
    assert not patt.matches_file('/a/b/c/.')
