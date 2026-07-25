from panda3d.core import Shader, VirtualFileSystem, Filename
from panda3d.core import BamCache, CompilerOptions, ShaderPool
from panda3d.core import load_prc_file_data, unload_prc_file
import time
import pytest
import sys

VERT_BODY = "#version 330\nvoid main() {}\n"
FRAG_BODY = "#version 330\nout vec4 o;\nvoid main() { o = vec4(1); }\n"


@pytest.fixture
def cache_generated_shaders():
    """Ensures caching of generated shaders is enabled, regardless of any
    config settings that may be in effect."""
    page = load_prc_file_data("cache_generated_shaders",
                              "cache-generated-shaders true")
    yield
    unload_prc_file(page)


@pytest.fixture
def shader_disk_cache(tmp_path):
    """Points the global BamCache at a fresh directory with compiled shader
    caching enabled, restoring the original settings afterwards."""
    cache = BamCache.get_global_ptr()
    old_active = cache.get_active()
    old_ccs = cache.get_cache_compiled_shaders()
    old_root = cache.get_root()

    cache.set_root(Filename.from_os_specific(str(tmp_path)))
    cache.set_active(True)
    cache.set_cache_compiled_shaders(True)
    yield cache

    if not old_root.empty():
        cache.set_root(old_root)
    cache.set_active(old_active)
    cache.set_cache_compiled_shaders(old_ccs)


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
    vfs.write_file(comp_file, b"#version 420\n#extension GL_ARB_compute_shader : require\nvoid main() {}\n", False)

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
    vfs.write_file(comp_file, b"#version 420\n#extension GL_ARB_compute_shader : require\nuniform int a;void main() {}\n", False)

    shad2 = Shader.load_compute(Shader.SL_GLSL, comp_file)

    assert shad2.this != shad1.this


def test_shader_make_caching(cache_generated_shaders):
    shad1 = Shader.make(Shader.SL_GLSL, vertex=VERT_BODY, fragment=FRAG_BODY)
    assert shad1 is not None

    # Making a shader from the same source strings returns the same object.
    shad2 = Shader.make(Shader.SL_GLSL, vertex=VERT_BODY, fragment=FRAG_BODY)
    assert shad2 is not None
    assert shad1.this == shad2.this

    # A different source string produces a different shader.
    shad3 = Shader.make(Shader.SL_GLSL,
                        vertex=VERT_BODY + "// this is a different shader\n",
                        fragment=FRAG_BODY)
    assert shad3 is not None
    assert shad3.this != shad1.this


def test_shader_make_caching_options(cache_generated_shaders):
    shad1 = Shader.make(Shader.SL_GLSL, vertex=VERT_BODY, fragment=FRAG_BODY)
    assert shad1 is not None

    # Different compiler options produce a different shader...
    options = CompilerOptions()
    options.define("FOO", "1")
    shad2 = Shader.make(Shader.SL_GLSL, vertex=VERT_BODY, fragment=FRAG_BODY,
                        options=options)
    assert shad2 is not None
    assert shad2.this != shad1.this

    # ...but an equivalent options object maps to the same shader.
    options2 = CompilerOptions()
    options2.define("FOO", "1")
    shad3 = Shader.make(Shader.SL_GLSL, vertex=VERT_BODY, fragment=FRAG_BODY,
                        options=options2)
    assert shad3 is not None
    assert shad3.this == shad2.this


def test_shader_make_no_ram_cache(cache_generated_shaders):
    shad1 = Shader.make(Shader.SL_GLSL, vertex=VERT_BODY, fragment=FRAG_BODY)
    assert shad1 is not None

    # The F_no_ram_cache flag bypasses the shader pool entirely.
    options = CompilerOptions()
    options.flags = CompilerOptions.F_no_ram_cache
    shad2 = Shader.make(Shader.SL_GLSL, vertex=VERT_BODY, fragment=FRAG_BODY,
                        options=options)
    assert shad2 is not None
    assert shad2.this != shad1.this

    shad3 = Shader.make(Shader.SL_GLSL, vertex=VERT_BODY, fragment=FRAG_BODY,
                        options=options)
    assert shad3 is not None
    assert shad3.this != shad2.this


def test_shader_make_compute_caching(cache_generated_shaders):
    body = "#version 430\nlayout(local_size_x = 1) in;\nvoid main() {}\n"
    shad1 = Shader.make_compute(Shader.SL_GLSL, body)
    assert shad1 is not None

    shad2 = Shader.make_compute(Shader.SL_GLSL, body)
    assert shad2 is not None
    assert shad1.this == shad2.this


def test_shader_load_twin_modified(vfs, ramdir):
    """Two files with identical contents share their compiled modules, but
    modifying one of them must still be detected when reloading it."""
    vert = b"#version 330\nvoid main() { /* twin test */ }\n"
    frag = b"#version 330\nout vec4 o;\nvoid main() { o = vec4(0.0625); }\n"

    for stem in ("twin_a", "twin_b"):
        vfs.write_file(Filename(ramdir, stem + ".vert"), vert, False)
        vfs.write_file(Filename(ramdir, stem + ".frag"), frag, False)

    shad_a = Shader.load(Shader.SL_GLSL, vertex=Filename(ramdir, "twin_a.vert"),
                         fragment=Filename(ramdir, "twin_a.frag"))
    assert shad_a is not None
    shad_b = Shader.load(Shader.SL_GLSL, vertex=Filename(ramdir, "twin_b.vert"),
                         fragment=Filename(ramdir, "twin_b.frag"))
    assert shad_b is not None

    # Having identical contents in the same directory, they share modules.
    assert shad_a.get_module(Shader.Stage.FRAGMENT).this == \
           shad_b.get_module(Shader.Stage.FRAGMENT).this

    # After waiting a second to make the timestamp different, modify only B.
    time.sleep(1.0)
    vfs.write_file(Filename(ramdir, "twin_b.frag"),
                   frag.replace(b"0.0625", b"0.375"), False)

    # Loading B must now notice the change, despite the shared modules.
    shad_b2 = Shader.load(Shader.SL_GLSL, vertex=Filename(ramdir, "twin_b.vert"),
                          fragment=Filename(ramdir, "twin_b.frag"))
    assert shad_b2 is not None
    assert shad_b2.this != shad_b.this

    # Whereas A is unchanged, so it still gets the cached shader.
    shad_a2 = Shader.load(Shader.SL_GLSL, vertex=Filename(ramdir, "twin_a.vert"),
                          fragment=Filename(ramdir, "twin_a.frag"))
    assert shad_a2.this == shad_a.this


def test_shader_include_dir_in_cache_key(vfs, ramdir):
    """Two byte-identical shaders in different directories whose relative
    includes resolve to different files must not share a cache entry."""
    vert = b"#version 120\nvoid main() { gl_Position = vec4(0); }\n"
    frag = (b"#version 120\n"
            b"#pragma include \"common.glsl\"\n"
            b"void main() { gl_FragColor = vec4(VALUE); }\n")

    dir_a = Filename(ramdir, "a")
    dir_b = Filename(ramdir, "b")
    vfs.make_directory_full(dir_a)
    vfs.make_directory_full(dir_b)

    vfs.write_file(Filename(dir_a, "main.vert"), vert, False)
    vfs.write_file(Filename(dir_a, "main.frag"), frag, False)
    vfs.write_file(Filename(dir_a, "common.glsl"), b"#define VALUE 1.0\n", False)

    vfs.write_file(Filename(dir_b, "main.vert"), vert, False)
    vfs.write_file(Filename(dir_b, "main.frag"), frag, False)
    vfs.write_file(Filename(dir_b, "common.glsl"), b"#define VALUE 0.5\n", False)

    shad_a = Shader.load(Shader.SL_GLSL, vertex=Filename(dir_a, "main.vert"),
                         fragment=Filename(dir_a, "main.frag"))
    assert shad_a is not None
    shad_b = Shader.load(Shader.SL_GLSL, vertex=Filename(dir_b, "main.vert"),
                         fragment=Filename(dir_b, "main.frag"))
    assert shad_b is not None

    mod_a = shad_a.get_module(Shader.Stage.FRAGMENT)
    mod_b = shad_b.get_module(Shader.Stage.FRAGMENT)
    assert bytes(mod_a.code) != bytes(mod_b.code)


def test_shader_module_disk_cache(tmp_path, shader_disk_cache,
                                  cache_generated_shaders):
    """Tests that a compiled module survives a round-trip through the model
    cache on disk."""
    # These bodies must be unique to this test, since a shader with the same
    # source may already have been cached in RAM by another test.
    vert = "#version 330\nvoid main() { /* disk cache test */ }\n"
    frag = "#version 330\nout vec4 o;\nvoid main() { o = vec4(0.5); }\n"

    # This compiles the modules and stores them in the fresh disk cache.
    shad1 = Shader.make(Shader.SL_GLSL, vertex=vert, fragment=frag)
    assert shad1 is not None

    entries = {p: p.read_bytes() for p in tmp_path.rglob("*.smo")}
    assert len(entries) == 2

    # Clear the RAM caches, so that the next make() must go to disk.
    ShaderPool.release_all_shaders()

    # If the bam round-trip failed, the entry would be silently discarded,
    # recompiled and re-stored.  Sleeping past a second boundary guarantees
    # that a re-store would embed a different timestamp in the entry, which
    # makes that failure mode detectable below.
    time.sleep(1.1)

    shad2 = Shader.make(Shader.SL_GLSL, vertex=vert, fragment=frag)
    assert shad2 is not None
    assert shad2.get_module(Shader.Stage.VERTEX) is not None
    assert shad2.get_module(Shader.Stage.FRAGMENT) is not None

    # The cache entries must be untouched: rewritten entries mean the round
    # trip failed, and additional entries mean the second make() missed.
    assert {p: p.read_bytes() for p in tmp_path.rglob("*.smo")} == entries


def test_shader_module_disk_cache_content_keyed(vfs, ramdir, tmp_path,
                                                shader_disk_cache,
                                                cache_generated_shaders):
    """Tests that a disk cache entry compiled from a file remains valid for a
    file with identical contents in the same directory, even after the
    original file is gone."""
    vert = "#version 330\nvoid main() { /* content keyed test */ }\n"
    frag = "#version 330\nout vec4 o;\nvoid main() { o = vec4(0.75); }\n"

    for stem in ("ck1", "ck2"):
        vfs.write_file(Filename(ramdir, stem + ".vert"), vert.encode('ascii'), False)
        vfs.write_file(Filename(ramdir, stem + ".frag"), frag.encode('ascii'), False)

    # This compiles the modules and stores them in the fresh disk cache.
    shad1 = Shader.load(Shader.SL_GLSL, vertex=Filename(ramdir, "ck1.vert"),
                        fragment=Filename(ramdir, "ck1.frag"))
    assert shad1 is not None

    entries = {p: p.read_bytes() for p in tmp_path.rglob("*.smo")}
    assert len(entries) == 2

    # The entries are keyed by content, so they must remain valid even after
    # the files they were compiled from are deleted.
    vfs.delete_file(Filename(ramdir, "ck1.vert"))
    vfs.delete_file(Filename(ramdir, "ck1.frag"))
    ShaderPool.release_all_shaders()

    # See test_shader_module_disk_cache for why this sleep is needed.
    time.sleep(1.1)

    shad2 = Shader.load(Shader.SL_GLSL, vertex=Filename(ramdir, "ck2.vert"),
                        fragment=Filename(ramdir, "ck2.frag"))
    assert shad2 is not None

    # A cache hit leaves the entries untouched; a recompile of ck2 would have
    # rewritten them (or added new ones, had the content keying regressed).
    assert {p: p.read_bytes() for p in tmp_path.rglob("*.smo")} == entries


def test_shader_pool_garbage_collect(cache_generated_shaders):
    """Tests that a single garbage_collect() pass also collects the modules
    of the shaders that it released."""
    vert = "#version 330\nvoid main() { /* gc test */ }\n"
    frag = "#version 330\nout vec4 o;\nvoid main() { o = vec4(0.125); }\n"

    options = CompilerOptions()
    options.flags = CompilerOptions.F_no_disk_cache

    # Start from a clean slate, so that the pool holds exactly this shader.
    ShaderPool.release_all_shaders()

    shad = Shader.make(Shader.SL_GLSL, vertex=vert, fragment=frag,
                       options=options)
    assert shad is not None

    # The pool tracks the modules under the synthetic source filename.
    assert ShaderPool.has_shader("created-shader")

    shad = None

    # Nothing references the shader now, so this releases it, and with it,
    # its modules, leaving nothing for a second pass to find.
    assert ShaderPool.garbage_collect() == 1
    assert not ShaderPool.has_shader("created-shader")
    assert ShaderPool.garbage_collect() == 0


def test_shader_load_unifies_twins(vfs, ramdir):
    """Two files with identical contents in the same directory don't just
    share their compiled modules; they resolve to the same Shader object."""
    vert = b"#version 330\nvoid main() { /* unify test */ }\n"
    frag = b"#version 330\nout vec4 o;\nvoid main() { o = vec4(0.875); }\n"

    for stem in ("uni_a", "uni_b"):
        vfs.write_file(Filename(ramdir, stem + ".vert"), vert, False)
        vfs.write_file(Filename(ramdir, stem + ".frag"), frag, False)

    shad_a = Shader.load(Shader.SL_GLSL, vertex=Filename(ramdir, "uni_a.vert"),
                         fragment=Filename(ramdir, "uni_a.frag"))
    assert shad_a is not None
    shad_b = Shader.load(Shader.SL_GLSL, vertex=Filename(ramdir, "uni_b.vert"),
                         fragment=Filename(ramdir, "uni_b.frag"))
    assert shad_b is not None
    assert shad_a.this == shad_b.this


def test_shader_make_copy(cache_generated_shaders):
    frag = FRAG_BODY.replace("vec4(1)", "vec4(0.4375)")
    shad1 = Shader.make(Shader.SL_GLSL, vertex=VERT_BODY, fragment=frag)
    assert shad1 is not None

    # The copy is a different object sharing the same modules.
    copy = shad1.make_copy()
    assert copy.this != shad1.this
    assert copy.get_module(Shader.Stage.VERTEX).this == \
           shad1.get_module(Shader.Stage.VERTEX).this

    # Modifying the copy affects neither the original nor the pool.
    assert copy.modify_module(Shader.Stage.VERTEX) is not None
    assert copy.get_module(Shader.Stage.VERTEX).this != \
           shad1.get_module(Shader.Stage.VERTEX).this

    shad2 = Shader.make(Shader.SL_GLSL, vertex=VERT_BODY, fragment=frag)
    assert shad2 is not None
    assert shad2.this == shad1.this


def test_shader_pool_release(vfs, ramdir):
    """release_shader() releases everything loaded from the given file."""
    vert = b"#version 330\nvoid main() { /* release test */ }\n"
    frag = b"#version 330\nout vec4 o;\nvoid main() { o = vec4(0.5625); }\n"
    vert_file = Filename(ramdir, "release.vert")
    frag_file = Filename(ramdir, "release.frag")
    vfs.write_file(vert_file, vert, False)
    vfs.write_file(frag_file, frag, False)

    shad1 = Shader.load(Shader.SL_GLSL, vertex=vert_file, fragment=frag_file)
    assert shad1 is not None
    assert ShaderPool.has_shader(vert_file)
    assert ShaderPool.has_shader(frag_file)

    ShaderPool.release_shader(vert_file)
    assert not ShaderPool.has_shader(vert_file)

    # The next load must produce a fresh shader.
    shad2 = Shader.load(Shader.SL_GLSL, vertex=vert_file, fragment=frag_file)
    assert shad2 is not None
    assert shad2.this != shad1.this


def test_shader_pool_release_all(cache_generated_shaders):
    # A unique body, so this test controls the lifetime of its pool entry.
    frag = FRAG_BODY.replace("vec4(1)", "vec4(0.25)")
    shad1 = Shader.make(Shader.SL_GLSL, vertex=VERT_BODY, fragment=frag)
    assert shad1 is not None

    ShaderPool.release_all_shaders()

    shad2 = Shader.make(Shader.SL_GLSL, vertex=VERT_BODY, fragment=frag)
    assert shad2 is not None
    assert shad2.this != shad1.this


def test_shader_pool_add_shader(cache_generated_shaders):
    """A shader registered with add_shader() is returned by a subsequent
    load_shader(), even under a filename that does not exist on disk."""
    frag = FRAG_BODY.replace("vec4(1)", "vec4(0.3125)")
    shad1 = Shader.make(Shader.SL_GLSL, vertex=VERT_BODY, fragment=frag)
    assert shad1 is not None

    filename = Filename("added-shader.glsl")
    ShaderPool.add_shader(filename, shad1)
    assert ShaderPool.has_shader(filename)

    shad2 = ShaderPool.load_shader(filename)
    assert shad2 is not None
    assert shad2.this == shad1.this

    ShaderPool.release_shader(filename)


def test_shader_pool_add_shader_unknown_extension(cache_generated_shaders):
    """Like the above, but with a filename whose extension gives no hint as
    to the shader language."""
    frag = FRAG_BODY.replace("vec4(1)", "vec4(0.6875)")
    shad1 = Shader.make(Shader.SL_GLSL, vertex=VERT_BODY, fragment=frag)
    assert shad1 is not None

    filename = Filename("added-shader.myext")
    ShaderPool.add_shader(filename, shad1)

    shad2 = ShaderPool.load_shader(filename)
    assert shad2 is not None
    assert shad2.this == shad1.this

    ShaderPool.release_shader(filename)


def test_shader_pool_add_shader_replaces(cache_generated_shaders):
    """add_shader() replaces a shader previously added under the same
    filename, even one that shares the exact same modules."""
    frag = FRAG_BODY.replace("vec4(1)", "vec4(0.15625)")
    shad1 = Shader.make(Shader.SL_GLSL, vertex=VERT_BODY, fragment=frag)
    assert shad1 is not None
    copy = shad1.make_copy()

    filename = Filename("replaced-shader.glsl")
    ShaderPool.add_shader(filename, shad1)
    ShaderPool.add_shader(filename, copy)

    shad2 = ShaderPool.load_shader(filename)
    assert shad2 is not None
    assert shad2.this == copy.this

    ShaderPool.release_shader(filename)
