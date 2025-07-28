from panda3d.core import VirtualFileSystem, VirtualFileMountRamdisk
import sys


def test_VFSImporter():
    from direct.showbase import VFSImporter

    VFSImporter.register()

    vfs = VirtualFileSystem.get_global_ptr()
    mount = VirtualFileMountRamdisk()
    success = vfs.mount(mount, "/ram", 0)
    assert success
    try:
        sys.path.insert(0, "/ram")
        vfs.write_file("/ram/testmod.py", b"var = 1\n", False)

        vfs.make_directory("/ram/testpkg")
        vfs.write_file("/ram/testpkg/__init__.py", b"var = 2\n", False)
        vfs.write_file("/ram/testpkg/test.py", b"var = 3\n", False)

        vfs.make_directory("/ram/testnspkg")
        vfs.write_file("/ram/testnspkg/test.py", b"var = 4\n", False)

        import testmod
        assert testmod.var == 1
        assert testmod.__spec__.name == 'testmod'
        assert testmod.__spec__.origin == '/ram/testmod.py'
        assert testmod.__file__ == '/ram/testmod.py'

        import testpkg
        assert testpkg.var == 2
        assert testpkg.__package__ == 'testpkg'
        assert testpkg.__path__ == ['/ram/testpkg']
        assert testpkg.__spec__.name == 'testpkg'
        assert testpkg.__spec__.origin == '/ram/testpkg/__init__.py'
        assert testpkg.__file__ == '/ram/testpkg/__init__.py'

        from testpkg import test
        assert test.var == 3
        assert test.__spec__.name == 'testpkg.test'
        assert test.__spec__.origin == '/ram/testpkg/test.py'
        assert test.__file__ == '/ram/testpkg/test.py'

        from testnspkg import test
        assert test.var == 4
        assert test.__spec__.name == 'testnspkg.test'
        assert test.__spec__.origin == '/ram/testnspkg/test.py'
        assert test.__file__ == '/ram/testnspkg/test.py'

    finally:
        vfs.unmount(mount)
        try:
            del sys.path[sys.path.index("/ram")]
        except ValueError:
            pass
