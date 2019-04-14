import os

from panda3d import core

def run_cg_compile_check(gsg, shader_path, expect_fail=False):
    """Compile supplied Cg shader path and check for errors"""
    shader = core.Shader.load(shader_path, core.Shader.SL_Cg)
    # assert shader.is_prepared(gsg.prepared_objects)
    if expect_fail:
        assert shader is None
    else:
        assert shader is not None


def test_cg_compile_error(gsg):
    """Test getting compile errors from bad Cg shaders"""
    shaders_dir = os.path.dirname(__file__)
    shader_path = os.path.join(shaders_dir, 'cg_bad.sha')
    run_cg_compile_check(gsg, shader_path, expect_fail=True)


def test_cg_from_file(gsg):
    """Test compiling Cg shaders from files"""
    shaders_dir = os.path.dirname(__file__)
    shader_path = os.path.join(shaders_dir, 'cg_simple.sha')
    run_cg_compile_check(gsg, shader_path)
