from panda3d.core import Shader, VirtualFileSystem, Filename
import time
import pytest


@pytest.fixture(scope="session")
def vfs():
    return VirtualFileSystem.get_global_ptr()


@pytest.fixture
def ramdir():
    """Fixture yielding a fresh ramdisk directory."""
    from panda3d.core import VirtualFileMountRamdisk, Filename

    vfs = VirtualFileSystem.get_global_ptr()
    mount = VirtualFileMountRamdisk()
    dir = Filename.temporary("/virtual", "ram.")
    assert vfs.mount(mount, dir, 0)

    yield dir
    vfs.unmount(mount)


def test_shader_load_multi(vfs, ramdir):
    # Try non-existent first.
    shad0 = Shader.load(Shader.SL_GLSL,
                        vertex="/nonexistent.glsl",
                        fragment="/nonexistent.glsl")
    assert shad0 is None

    vert_file = Filename(ramdir, "shader.glsl")
    frag_file = Filename(ramdir, "shader.glsl")

    # Now write some actual content to the shader files.
    vfs.write_file(vert_file, b"#version 100\nvoid main() {}\n", False)

    shad1 = Shader.load(Shader.SL_GLSL, vertex=vert_file, fragment=frag_file)
    assert shad1 is not None
    assert shad1.this

    # Load the same shader, it should return the cached result.
    shad2 = Shader.load(Shader.SL_GLSL, vertex=vert_file, fragment=frag_file)
    assert shad2 is not None
    assert shad1.this == shad2.this

    # After waiting a second to make the timestamp different, modify the
    # shader and load again, it should result in a different object now
    time.sleep(1.0)
    vfs.write_file(vert_file, b"#version 110\nvoid main() {}\n", False)

    shad2 = Shader.load(Shader.SL_GLSL, vertex=vert_file, fragment=frag_file)

    assert shad2.this != shad1.this


def test_shader_load_compute(vfs, ramdir):
    # Try non-existent first.
    shad0 = Shader.load_compute(Shader.SL_GLSL, "/nonexistent.glsl")
    assert shad0 is None

    comp_file = Filename(ramdir, "shader.glsl")

    # Now write some actual content to the shader file.
    vfs.write_file(comp_file, b"#version 100\nvoid main() {}\n", False)

    shad1 = Shader.load_compute(Shader.SL_GLSL, comp_file)
    assert shad1 is not None
    assert shad1.this

    # Load the same shader, it should return the cached result.
    shad2 = Shader.load_compute(Shader.SL_GLSL, comp_file)
    assert shad2 is not None
    assert shad1.this == shad2.this

    # After waiting a second to make the timestamp different, modify the
    # shader and load again, it should result in a different object now
    time.sleep(1.0)
    vfs.write_file(comp_file, b"#version 110\nvoid main() {}\n", False)

    shad2 = Shader.load_compute(Shader.SL_GLSL, comp_file)

    assert shad2.this != shad1.this
