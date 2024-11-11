from panda3d import core
import pytest
from _pytest.outcomes import Failed
import sys


# Instantiate all of the available graphics pipes.
ALL_PIPES = []
sel = core.GraphicsPipeSelection.get_global_ptr()

pipe = sel.make_module_pipe('p3headlessgl')
if pipe and pipe.is_valid():
    ALL_PIPES.append(pipe)
else:
    pipe = sel.make_module_pipe('pandagl')
    if pipe and pipe.is_valid():
        ALL_PIPES.append(pipe)

#pipe = sel.make_module_pipe('pandagles2')
#if pipe and pipe.is_valid():
#    ALL_PIPES.append(pipe)

if sys.platform == 'win32':
    pipe = sel.make_module_pipe('pandadx9')
    if pipe and pipe.is_valid():
        ALL_PIPES.append(pipe)


@pytest.fixture(scope='session')
def graphics_pipes():
    yield ALL_PIPES


@pytest.fixture(scope='session', params=ALL_PIPES)
def graphics_pipe(request):
    pipe = request.param
    if pipe is None or not pipe.is_valid():
        pytest.skip("GraphicsPipe is invalid")

    yield pipe


@pytest.fixture(scope='session')
def graphics_engine():
    from panda3d.core import GraphicsEngine

    engine = GraphicsEngine.get_global_ptr()
    yield engine

    # This causes GraphicsEngine to also terminate the render threads.
    engine.remove_all_windows()


@pytest.fixture
def window(graphics_pipe, graphics_engine):
    from panda3d.core import GraphicsPipe, FrameBufferProperties, WindowProperties

    fbprops = FrameBufferProperties.get_default()
    winprops = WindowProperties.get_default()

    win = graphics_engine.make_output(
        graphics_pipe,
        'window',
        0,
        fbprops,
        winprops,
        GraphicsPipe.BF_require_window
    )
    graphics_engine.open_windows()

    if win is None:
        pytest.skip("GraphicsPipe cannot make windows")

    yield win

    if win is not None:
        graphics_engine.remove_window(win)


# This is the template for the compute shader that is used by run_glsl_test.
# It defines an assert() macro that writes failures to a buffer, indexed by
# line number.
# The reset() function serves to prevent the _triggered variable from being
# optimized out in the case that the assertions are being optimized out.
GLSL_COMPUTE_TEMPLATE = """#version {version}
{extensions}

layout(local_size_x = 1, local_size_y = 1) in;

{preamble}

layout(r32ui) uniform writeonly uimageBuffer _triggered;

void _reset() {{
    imageStore(_triggered, 0, uvec4(1));
    memoryBarrier();
}}

void _assert(bool cond, int line) {{
    if (!cond) {{
        imageStore(_triggered, line, uvec4(1));
    }}
}}

#define assert(cond) _assert(cond, __LINE__ - line_offset)

void main() {{
    _reset();
    const int line_offset = __LINE__;
{body}
}}
"""

# This is a version that uses a vertex and fragment shader instead.  This is
# slower to set up, but it works even when compute shaders are not supported.
# The shader is rendered on a fullscreen triangle to a texture, where each
# pixel represents one line of the code.  The assert writes the result to the
# output color if the current fragment matches the line number of that assert.
# The first pixel is used as a control, to check that the shader has run.
GLSL_VERTEX_TEMPLATE = """#version {version}

in vec4 p3d_Vertex;

void main() {{
    gl_Position = p3d_Vertex;
}}
"""

GLSL_FRAGMENT_TEMPLATE = """#version {version}
{extensions}

{preamble}

layout(location = 0) out vec4 p3d_FragColor;

void _reset() {{
    p3d_FragColor = vec4(0, 0, 0, 0);

    if (int(gl_FragCoord.x) == 0) {{
        p3d_FragColor = vec4(1, 1, 1, 1);
    }}
}}

void _assert(bool cond, int line) {{
    if (int(gl_FragCoord.x) == line) {{
        p3d_FragColor = vec4(!cond, !cond, !cond, !cond);
    }}
}}

#define assert(cond) _assert(cond, __LINE__ - line_offset)

void main() {{
    _reset();
    const int line_offset = __LINE__;
{body}
}}
"""

# This is the template for the shader that is used by run_cg_test.
# We render this to an nx1 texture, where n is the number of lines in the body.
# An assert
CG_VERTEX_TEMPLATE = """//Cg

void vshader(float4 vtx_position : POSITION, out float4 l_position : POSITION) {{
    l_position = vtx_position;
}}
"""

CG_FRAGMENT_TEMPLATE = """//Cg

{preamble}

float4 _assert(bool cond) {{
    return float4(cond.x, 1, 1, 1);
}}

float4 _assert(bool2 cond) {{
    return float4(cond.x, cond.y, 1, 1);
}}

float4 _assert(bool3 cond) {{
    return float4(cond.x, cond.y, cond.z, 1);
}}

float4 _assert(bool4 cond) {{
    return float4(cond.x, cond.y, cond.z, cond.w);
}}

#define assert(cond) {{ if ((int)l_vpos.x == __LINE__ - line_offset) o_color = _assert(cond); }}

void fshader(in float2 l_vpos : VPOS, out float4 o_color : COLOR) {{
    o_color = float4(1, 1, 1, 1);

    if ((int)l_vpos.x == 0) {{
        o_color = float4(0, 0, 0, 0);
    }}
    const int line_offset = __LINE__;
{body}
}}
"""


class ShaderEnvironment:
    def __init__(self, name, gsg, allow_compute=True):
        self.name = name
        self.gsg = gsg
        self.engine = gsg.get_engine()
        self.allow_compute = allow_compute

    def __repr__(self):
        return f'<{self.name} vendor="{self.gsg.driver_vendor}" renderer="{self.gsg.driver_renderer}" version="{self.gsg.driver_version}">'

    def extract_texture_data(self, tex):
        __tracebackhide__ = True
        result = self.engine.extract_texture_data(tex, self.gsg)
        assert result

    def run_glsl(self, body, preamble="", inputs={}, version=420, exts=set(),
                 state=core.RenderState.make_empty()):
        """ Runs a GLSL test on the given GSG.  The given body is executed in the
        main function and should call assert().  The preamble should contain all
        of the shader inputs. """

        gsg = self.gsg
        if not gsg.supports_basic_shaders:
            pytest.skip("shaders not supported")

        use_compute = self.allow_compute and gsg.supports_compute_shaders and \
                      (gsg.supported_shader_capabilities & core.Shader.C_image_load_store) != 0

        missing_exts = sorted(ext for ext in exts if not gsg.has_extension(ext))
        if missing_exts:
            pytest.skip("missing extensions: " + ' '.join(missing_exts))

        version = version or 420
        exts = exts | {'GL_ARB_compute_shader', 'GL_ARB_shader_image_load_store'}

        extensions = ''
        for ext in exts:
            extensions += '#extension {ext} : require\n'.format(ext=ext)

        __tracebackhide__ = True

        preamble = preamble.strip()
        body = body.rstrip().lstrip('\n')

        if use_compute:
            code = GLSL_COMPUTE_TEMPLATE.format(version=version, extensions=extensions, preamble=preamble, body=body)
            shader = core.Shader.make_compute(core.Shader.SL_GLSL, code)
        else:
            vertex_code = GLSL_VERTEX_TEMPLATE.format(version=version, extensions=extensions, preamble=preamble, body=body)
            code = GLSL_FRAGMENT_TEMPLATE.format(version=version, extensions=extensions, preamble=preamble, body=body)
            shader = core.Shader.make(core.Shader.SL_GLSL, vertex_code, code)

        if not shader:
            pytest.fail("error compiling shader:\n" + code)

        unsupported_caps = shader.get_used_capabilities() & ~gsg.supported_shader_capabilities
        if unsupported_caps != 0:
            stream = core.StringStream()
            core.ShaderEnums.output_capabilities(stream, unsupported_caps)
            pytest.skip("unsupported capabilities: " + stream.data.decode('ascii'))

        num_lines = body.count('\n') + 1

        # Create a buffer to hold the results of the assertion.  We use one texel
        # per line of shader code, so we can show which lines triggered.
        engine = gsg.get_engine()
        result = core.Texture("")
        if use_compute:
            result.set_clear_color((0, 0, 0, 0))
            result.setup_buffer_texture(num_lines + 1,
                                        core.Texture.T_unsigned_int,
                                        core.Texture.F_r32i,
                                        core.GeomEnums.UH_static)
        else:
            fbprops = core.FrameBufferProperties()
            fbprops.force_hardware = True
            fbprops.set_rgba_bits(8, 8, 8, 8)
            fbprops.srgb_color = False

            buffer = engine.make_output(
                gsg.pipe,
                'buffer',
                0,
                fbprops,
                core.WindowProperties.size(core.Texture.up_to_power_2(num_lines + 1), 1),
                core.GraphicsPipe.BF_refuse_window,
                gsg
            )
            buffer.add_render_texture(result, core.GraphicsOutput.RTM_copy_ram, core.GraphicsOutput.RTP_color)
            buffer.set_clear_color_active(True)
            buffer.set_clear_color((0, 0, 0, 0))
            engine.open_windows()

        # Build up the shader inputs
        attrib = core.ShaderAttrib.make(shader)
        for name, value in inputs.items():
            attrib = attrib.set_shader_input(name, value)
        if use_compute:
            attrib = attrib.set_shader_input('_triggered', result)
        state = state.set_attrib(attrib)

        # Run the shader.
        if use_compute:
            try:
                engine.dispatch_compute((1, 1, 1), state, gsg)
            except AssertionError as exc:
                assert False, "Error executing compute shader:\n" + code
        else:
            scene = core.NodePath("root")
            scene.set_attrib(core.DepthTestAttrib.make(core.RenderAttrib.M_always))

            format = core.GeomVertexFormat.get_v3()
            vdata = core.GeomVertexData("tri", format, core.Geom.UH_static)
            vdata.unclean_set_num_rows(3)

            vertex = core.GeomVertexWriter(vdata, "vertex")
            vertex.set_data3(-1, -1, 0)
            vertex.set_data3(3, -1, 0)
            vertex.set_data3(-1, 3, 0)

            tris = core.GeomTriangles(core.Geom.UH_static)
            tris.add_next_vertices(3)

            geom = core.Geom(vdata)
            geom.add_primitive(tris)

            gnode = core.GeomNode("tri")
            gnode.add_geom(geom, state)
            scene.attach_new_node(gnode)
            scene.set_two_sided(True)

            camera = scene.attach_new_node(core.Camera("camera"))
            camera.node().get_lens(0).set_near_far(-10, 10)
            camera.node().set_cull_bounds(core.OmniBoundingVolume())

            region = buffer.make_display_region()
            region.active = True
            region.camera = camera

            try:
                engine.render_frame()
            except AssertionError as exc:
                assert False, "Error executing shader:\n" + code
            finally:
                engine.remove_window(buffer)

        # Download the texture to check whether the assertion triggered.
        if use_compute:
            success = engine.extract_texture_data(result, gsg)
            assert success

        triggered = result.get_ram_image()
        triggered = tuple(memoryview(triggered).cast('I'))

        if not triggered[0]:
            pytest.fail("control check failed")

        if any(triggered[1:]):
            count = len(triggered) - triggered.count(0) - 1
            lines = body.split('\n')
            formatted = ''
            for i, line in enumerate(lines):
                if triggered[i + 1]:
                    formatted += '=>  ' + line + '\n'
                else:
                    formatted += '    ' + line + '\n'
            pytest.fail("{0} GLSL assertions triggered:\n{1}".format(count, formatted))

    def run_cg(self, body, preamble="", inputs={}, state=core.RenderState.make_empty()):
        """ Runs a Cg test on the given GSG.  The given body is executed in the
        main function and should call assert().  The preamble should contain all
        of the shader inputs. """

        if self.name.endswith("-legacy"):
            pytest.skip("no Cg support in legacy pipeline")

        gsg = self.gsg
        if not gsg.supports_basic_shaders:
            pytest.skip("basic shaders not supported")

        __tracebackhide__ = True

        preamble = preamble.strip()
        body = body.rstrip().lstrip('\n')
        num_lines = body.count('\n') + 1

        vertex_code = CG_VERTEX_TEMPLATE.format(preamble=preamble, body=body)
        code = CG_FRAGMENT_TEMPLATE.format(preamble=preamble, body=body)
        shader = core.Shader.make(core.Shader.SL_Cg, vertex_code, code)
        if not shader:
            pytest.fail("error compiling shader:\n" + code)

        result = core.Texture("")
        fbprops = core.FrameBufferProperties()
        fbprops.force_hardware = True
        fbprops.set_rgba_bits(8, 8, 8, 8)
        fbprops.srgb_color = False

        engine = gsg.get_engine()
        buffer = engine.make_output(
            gsg.pipe,
            'buffer',
            0,
            fbprops,
            core.WindowProperties.size(core.Texture.up_to_power_2(num_lines + 1), 1),
            core.GraphicsPipe.BF_refuse_window,
            gsg
        )
        buffer.add_render_texture(result, core.GraphicsOutput.RTM_copy_ram, core.GraphicsOutput.RTP_color)
        buffer.set_clear_color_active(True)
        buffer.set_clear_color((0, 0, 0, 0))
        engine.open_windows()

        # Build up the shader inputs
        attrib = core.ShaderAttrib.make(shader)
        for name, value in inputs.items():
            attrib = attrib.set_shader_input(name, value)
        state = state.set_attrib(attrib)

        scene = core.NodePath("root")
        scene.set_attrib(core.DepthTestAttrib.make(core.RenderAttrib.M_always))

        format = core.GeomVertexFormat.get_v3()
        vdata = core.GeomVertexData("tri", format, core.Geom.UH_static)
        vdata.unclean_set_num_rows(3)

        vertex = core.GeomVertexWriter(vdata, "vertex")
        vertex.set_data3(-1, -1, 0)
        vertex.set_data3(3, -1, 0)
        vertex.set_data3(-1, 3, 0)

        tris = core.GeomTriangles(core.Geom.UH_static)
        tris.add_next_vertices(3)

        geom = core.Geom(vdata)
        geom.add_primitive(tris)

        gnode = core.GeomNode("tri")
        gnode.add_geom(geom, state)
        scene.attach_new_node(gnode)
        scene.set_two_sided(True)

        camera = scene.attach_new_node(core.Camera("camera"))
        camera.node().get_lens(0).set_near_far(-10, 10)
        camera.node().set_cull_bounds(core.OmniBoundingVolume())

        region = buffer.make_display_region()
        region.active = True
        region.camera = camera

        try:
            engine.render_frame()
        except AssertionError as exc:
            assert False, "Error executing shader:\n" + code
        finally:
            engine.remove_window(buffer)

        # Download the texture to check whether the assertion triggered.
        triggered = tuple(result.get_ram_image())
        if triggered[0]:
            pytest.fail("control check failed")

        if not all(triggered[4:]):
            count = 0
            lines = body.split('\n')
            formatted = ''
            for i, line in enumerate(lines):
                offset = (i + 1) * 4
                x = triggered[offset + 2] == 0
                y = triggered[offset + 1] == 0
                z = triggered[offset] == 0
                w = triggered[offset + 3] == 0
                if x or y or z or w:
                    count += 1
                else:
                    continue
                formatted += '=>  ' + line
                components = ''
                if x:
                    components += 'x'
                if y:
                    components += 'y'
                if z:
                    components += 'z'
                if w:
                    components += 'w'
                formatted += f'      <= {components} components don\'t match'
                formatted += '\n'
            pytest.fail("{0} Cg assertions triggered:\n{1}".format(count, formatted))


# Which environments should we test shaders in?
ENVS = set()
for pipe in ALL_PIPES:
    if pipe.interface_name == 'OpenGL':
        ENVS |= frozenset(("gl-legacy", "gl-spirv", "gl-cross-120", "gl-cross-130", "gl-cross-140", "gl-cross-150", "gl-cross-330", "gl-cross-400", "gl-cross-410", "gl-cross-420", "gl-cross-430"))
    elif pipe.interface_name == 'OpenGL ES':
        ENVS |= frozenset(("gles-cross-100", "gles-cross-300", "gles-cross-310", "gles-cross-320"))
    elif pipe.interface_name == 'DirectX9':
        ENVS |= frozenset(("dx9-cross", ))


@pytest.fixture(scope="session", params=sorted(ENVS))
def env(request):
    config = {}

    if request.param.startswith("gl-"):
        for pipe in ALL_PIPES:
            if pipe.interface_name == 'OpenGL':
                break
        else:
            pytest.skip("no OpenGL pipe found")

    elif request.param.startswith("gles-"):
        for pipe in ALL_PIPES:
            if pipe.interface_name == 'OpenGL ES':
                break
        else:
            pytest.skip("no OpenGL ES pipe found")

    elif request.param.startswith("dx9-"):
        for pipe in ALL_PIPES:
            if pipe.interface_name == 'DirectX9':
                break
        else:
            pytest.skip("no DirectX 9 pipe found")

    words = request.param.split("-")
    if words[0] == "gl" or words[0] == "gles":
        if words[0] == "gles":
            # Necessary for image load/store support
            config["gl-immutable-texture-storage"] = "true"

        if words[1] == "legacy":
            config["glsl-force-legacy-pipeline"] = "true"
            allow_compute = True

        elif words[1] == "spirv":
            config["glsl-force-legacy-pipeline"] = "false"
            config["gl-support-spirv"] = "true"
            allow_compute = True

        elif words[1] == "cross":
            config["glsl-force-legacy-pipeline"] = "false"
            config["gl-support-spirv"] = "false"

            version = int(words[2])
            allow_compute = (version >= 330)

            config["gl-force-glsl-version"] = str(version)

    else:
        allow_compute = False

    prc = '\n'.join(f"{key} {value}" for key, value in config.items())
    page = core.load_prc_file_data("", prc)

    engine = core.GraphicsEngine()

    fbprops = core.FrameBufferProperties()
    fbprops.set_rgba_bits(8, 8, 8, 8)
    fbprops.force_hardware = True

    props = core.WindowProperties.size(32, 32)

    buffer = engine.make_output(
        pipe,
        'buffer',
        0,
        fbprops,
        props,
        core.GraphicsPipe.BF_refuse_window
    )
    engine.open_windows()

    if buffer is None:
        # Try making a window instead, putting it in the background so it
        # disrupts the desktop as little as possible
        props.minimized = True
        props.foreground = False
        props.z_order = core.WindowProperties.Z_bottom
        buffer = engine.make_output(
            pipe,
            'buffer',
            0,
            fbprops,
            props,
            core.GraphicsPipe.BF_require_window
        )

    if buffer is None:
        pytest.skip("GraphicsPipe cannot make offscreen buffers or windows")

    gsg = buffer.gsg

    # Check if the environment is actually supported.
    if words[0] == "gl" or words[0] == "gles":
        if words[1] == "legacy":
            if not gsg.supports_glsl:
                pytest.skip("legacy GLSL shaders not supported")

        elif words[1] == "spirv":
            if (gsg.driver_version_major, gsg.driver_version_minor) < (4, 6) and \
               not gsg.has_extension("GL_ARB_gl_spirv"):
                pytest.skip("SPIR-V shaders not supported")

        elif words[1] == "cross":
            version = int(words[2])
            if version > gsg.driver_shader_version_major * 100 + gsg.driver_shader_version_minor:
                pytest.skip(f"GLSL {version} shaders not supported")

    env = ShaderEnvironment(request.param, gsg, allow_compute)

    try:
        yield env
    finally:
        core.unload_prc_file(page)

        if buffer is not None:
            engine.remove_window(buffer)
