"""
Class CommonFilters implements certain common image
postprocessing filters.  See the :ref:`common-image-filters` page for
more information about how to use these filters.

These filters are written in the Cg shading language.
"""

# It is not ideal that these filters are all included in a single
# monolithic module.  Unfortunately, when you want to apply two filters
# at the same time, you have to compose them into a single shader, and
# the composition process isn't simply a question of concatenating them:
# you have to somehow make them work together.  I suspect that there
# exists some fairly simple framework that would make this automatable.
# However, until I write some more filters myself, I won't know what
# that framework is.  Until then, I'll settle for this
# clunky approach.  - Josh
from panda3d.core import VirtualFileSystem

from panda3d.core import LVecBase4, LPoint2
from panda3d.core import AuxBitplaneAttrib, AntialiasAttrib
from panda3d.core import Texture, Shader, ATSNone
from panda3d.core import FrameBufferProperties
from panda3d.core import getDefaultCoordinateSystem, CS_zup_right, CS_zup_left

from direct.task.TaskManagerGlobal import taskMgr

from direct.filter.FilterManager import FilterManager
from direct.filter.filterBloomI import BLOOM_I
from direct.filter.filterBloomX import BLOOM_X
from direct.filter.filterBloomY import BLOOM_Y
from direct.filter.filterBlurX import BLUR_X
from direct.filter.filterBlurY import BLUR_Y
from direct.filter.filterCopy import COPY
from direct.filter.filterDown4 import DOWN_4

# Some GPUs do not support variable-length loops.
#
# We fill in the actual value of numsamples in the loop limit
# when the shader is configured.
#
SSAO_BODY = """//Cg

void vshader(float4 vtx_position : POSITION,
             float2 vtx_texcoord : TEXCOORD0,
             out float4 l_position : POSITION,
             out float2 l_texcoord : TEXCOORD0,
             out float2 l_texcoordD : TEXCOORD1,
             out float2 l_texcoordN : TEXCOORD2,
             uniform float4 texpad_depth,
             uniform float4 texpad_normal,
             uniform float4x4 mat_modelproj)
{
  l_position = mul(mat_modelproj, vtx_position);
  l_texcoord = vtx_texcoord;
  l_texcoordD = vtx_texcoord * texpad_depth.xy * 2;
  l_texcoordN = vtx_texcoord * texpad_normal.xy * 2;
}

float3 sphere[16] = float3[](float3(0.53812504, 0.18565957, -0.43192),float3(0.13790712, 0.24864247, 0.44301823),float3(0.33715037, 0.56794053, -0.005789503),float3(-0.6999805, -0.04511441, -0.0019965635),float3(0.06896307, -0.15983082, -0.85477847),float3(0.056099437, 0.006954967, -0.1843352),float3(-0.014653638, 0.14027752, 0.0762037),float3(0.010019933, -0.1924225, -0.034443386),float3(-0.35775623, -0.5301969, -0.43581226),float3(-0.3169221, 0.106360726, 0.015860917),float3(0.010350345, -0.58698344, 0.0046293875),float3(-0.08972908, -0.49408212, 0.3287904),float3(0.7119986, -0.0154690035, -0.09183723),float3(-0.053382345, 0.059675813, -0.5411899),float3(0.035267662, -0.063188605, 0.54602677),float3(-0.47761092, 0.2847911, -0.0271716));

void fshader(out float4 o_color : COLOR,
             uniform float4 k_params1,
             uniform float4 k_params2,
             float2 l_texcoord : TEXCOORD0,
             float2 l_texcoordD : TEXCOORD1,
             float2 l_texcoordN : TEXCOORD2,
             uniform sampler2D k_random : TEXUNIT0,
             uniform sampler2D k_depth : TEXUNIT1,
             uniform sampler2D k_normal : TEXUNIT2)
{
  float pixel_depth = tex2D(k_depth, l_texcoordD).a;
  float3 pixel_normal = (tex2D(k_normal, l_texcoordN).xyz * 2.0 - 1.0);
  float3 random_vector = normalize((tex2D(k_random, l_texcoord * 18.0 + pixel_depth + pixel_normal.xy).xyz * 2.0) - float3(1.0)).xyz;
  float occlusion = 0.0;
  float radius = k_params1.z / pixel_depth;
  float depth_difference;
  float3 sample_normal;
  float3 ray;
  for(int i = 0; i < %d; ++i) {
   ray = radius * reflect(sphere[i], random_vector);
   sample_normal = (tex2D(k_normal, l_texcoordN + ray.xy).xyz * 2.0 - 1.0);
   depth_difference =  (pixel_depth - tex2D(k_depth,l_texcoordD + ray.xy).r);
   occlusion += step(k_params2.y, depth_difference) * (1.0 - dot(sample_normal.xyz, pixel_normal)) * (1.0 - smoothstep(k_params2.y, k_params2.x, depth_difference));
  }
  o_color.rgb = 1.0 + (occlusion * k_params1.y);
  o_color.a = 1.0;
}
"""

vfs = VirtualFileSystem.get_global_ptr()


class FilterConfig:
    pass


class CommonFilters:
    """ Class CommonFilters implements certain common image postprocessing
    filters.  The constructor requires a filter builder as a parameter. """

    def __init__(self, win, cam):
        self.clamping = None
        self.finalQuad = None
        self.texcoords = {}
        self.texcoordPadding = {}
        self.textures = {}

        self.current_text = None
        self.manager = FilterManager(win, cam)
        self.configuration = {}
        self.task = None
        self.cleanup()
        self.filters = {}
        self.uniforms = {}
        self.render_textures = {}
        self.fbprops = FrameBufferProperties()

        self.setFilter("base_color", "o_color = tex2D(k_txcolor, l_texcoord_color);")

    def cleanup(self, reconfiguring=False):
        self.manager.cleanup()
        self.finalQuad = None
        self.render_textures = {}
        if not reconfiguring:
            self.textures = {}
            self.uniforms = {}

        if self.task is not None:
            taskMgr.remove(self.task)
            self.task = None

    def loadFilter(self, name, filepath, uniforms=None, consts=None, needed_textures=None, needed_coords=None,
                   render_into=None, auxbits=None, sort=None):
        self.set_filter(name, vfs.get_file(filepath), uniforms, consts, needed_textures, needed_coords, render_into,
                        auxbits, sort)

    def setFilter(self, name, shader_string, uniforms=None, consts=None, needed_textures=None, needed_coords=None,
                  render_into=None, auxbits=None, sort=None):

        # Defualt filter structure
        self.filters[name] = {
            "shader": None,
            "uniforms": uniforms,
            "needed_textures": needed_textures,
            "needed_coords": needed_coords,
            "render_into": render_into,
            "auxbits": auxbits,
            "sort": None,
            "consts": consts}

        self.filters[name]["shader"] = shader_string

        # Add filter to end if no sort is present
        if sort:
            self.filters[name]["sort"] = sort
        else:
            self.filters[name]["sort"] = len(self.filters)

        # The uniforms dictionary controls both the passing in of the uniform and its value
        if uniforms:
            self.add_uniforms(uniforms, reconfigure=False)
            for name, uniform in uniforms.items():
                self.set_shader_input(name, uniform["value"])

        # Apply new filter
        self.reconfigure()

    def addUniforms(self, uniforms, reconfigure=True):
        self.uniforms.update(uniforms)
        if reconfigure:
            self.reconfigure()

    def delUniform(self, uniform, reconfigure=True):
        if uniform in self.uniforms:
            del self.uniforms[uniform]
        if reconfigure:
            self.reconfigure()

    def delUniforms(self, uniforms, reconfigure=True):
        for uniform in uniforms:
            if uniform in self.uniforms:
                del self.uniforms[uniform]
        if reconfigure:
            self.reconfigure()

    def delFilter(self, name):
        if name in self.filters:
            filter = self.filters[name]
            if filter["uniforms"]:
                self.del_uniforms(filter["uniforms"], reconfigure=False)
            del self.filters[name]
            self.reconfigure()

    def setShaderInputs(self, inputs):
        for key, value in inputs.items():
            self.uniforms[key]["value"] = value
        self.finalQuad.setShaderInputs(**inputs)

    def setShaderInput(self, name, value):
        self.uniforms[name]["value"] = value
        self.finalQuad.setShaderInput(name, value)

    def reconfigure(self):
        """ Reconfigure is called whenever any configuration change is made. """
        configuration = self.configuration

        self.cleanup(reconfiguring=True)

        if not self.manager.win.gsg.getSupportsBasicShaders():
            return False

        auxbits = 0

        needtex = {"color"}
        needtexcoord = {"color"}

        for settings in self.filters.values():
            if settings["auxbits"]:
                for auxbit in settings["auxbits"]:
                    auxbits |= auxbit
            if settings["needed_textures"]:
                for texture in settings["needed_textures"]:
                    needtex.add(texture)
            if settings["needed_coords"]:
                for coords in settings["needed_coords"]:
                    needtexcoord.add(coords)

        for tex in needtex:
            self.textures[tex] = Texture("scene-" + tex)
            self.textures[tex].setWrapU(Texture.WMClamp)
            self.textures[tex].setWrapV(Texture.WMClamp)

        self.finalQuad = self.manager.renderSceneInto(textures=self.textures, auxbits=auxbits, fbprops=self.fbprops,
                                                      clamping=self.clamping)

        if self.finalQuad is None:
            self.cleanup()
            return False

        for filter_settings in self.filters.values():
            if filter_settings["render_into"]:
                for render_name, settings in filter_settings["render_into"].items():

                    mul = settings.get("mul")
                    if not mul:
                        mul = 1

                    div = settings.get("div")
                    if not div:
                        div = 1

                    align = settings.get("align")
                    if not align:
                        align = 1

                    depthtex = settings.get("depthtex")
                    if type(depthtex) is str:
                        depthtex = self.textures[depthtex]

                    colortex = settings.get("colortex")
                    if type(colortex) is str:
                        colortex = self.textures[colortex]

                    auxtex0 = settings.get("auxtex0")
                    if type(auxtex0) is str:
                        auxtex0 = self.textures[auxtex0]

                    auxtex1 = settings.get("auxtex1")
                    if type(auxtex1) is str:
                        auxtex1 = self.textures[auxtex1]

                    fbprops = settings.get("fbprops")

                    quad = self.manager.renderQuadInto(render_name,
                                                       mul=mul,
                                                       align=align,
                                                       div=div,
                                                       depthtex=depthtex,
                                                       colortex=colortex,
                                                       auxtex0=auxtex0,
                                                       auxtex1=auxtex1,
                                                       fbprops=fbprops)

                    self.render_textures[render_name] = quad

                    shader = settings.get("shader")
                    if shader:
                        if type(shader) is str:
                            shader = Shader.make(shader, Shader.SL_Cg)
                        quad.set_shader(shader)

                    shader_inputs = settings.get("shader_inputs")
                    if shader_inputs:
                        for name, value in shader_inputs.items():
                            if type(value) is str:
                                value = self.textures[value]
                            quad.set_shader_input(name, value)

        for tex in needtexcoord:
            if self.textures[tex].getAutoTextureScale() != ATSNone or "HalfPixelShift" in configuration:
                self.texcoords[tex] = "l_texcoord_" + tex
                self.texcoordPadding["l_texcoord_" + tex] = tex
            else:
                # Share unpadded texture coordinates.
                self.texcoords[tex] = "l_texcoord"
                self.texcoordPadding["l_texcoord"] = None

        texcoordSets = list(enumerate(self.texcoordPadding.keys()))

        text = "//Cg\n"

        # Add any global constants
        for settings in self.filters.values():
            if settings["consts"]:
                for const in settings["consts"]:
                    text += const

        text += "void vshader(float4 vtx_position : POSITION,\n"
        text += "  out float4 l_position : POSITION,\n"
        text += "  out float4 l_fragpos ,\n"

        for texcoord, padTex in self.texcoordPadding.items():
            if padTex is not None:
                text += "  uniform float4 texpad_tx%s,\n" % (padTex)
                if "HalfPixelShift" in self.configuration:
                    text += "  uniform float4 texpix_tx%s,\n" % (padTex)

        for i, name in texcoordSets:
            text += "  out float2 %s : TEXCOORD%d,\n" % (name, i)

        text += "  uniform float4x4 mat_modelproj)\n"
        text += "{\n"
        text += "  l_position = mul(mat_modelproj, vtx_position);\n"

        # The card is oriented differently depending on our chosen
        # coordinate system.  We could just use vtx_texcoord, but this
        # saves on an additional variable.
        if getDefaultCoordinateSystem() in (CS_zup_right, CS_zup_left):
            pos = "vtx_position.xz"
        else:
            pos = "vtx_position.xy"

        for texcoord, padTex in self.texcoordPadding.items():
            if padTex is None:
                text += "  %s = %s * float2(0.5, 0.5) + float2(0.5, 0.5);\n" % (texcoord, pos)
            else:
                text += "  %s = (%s * texpad_tx%s.xy) + texpad_tx%s.xy;\n" % (texcoord, pos, padTex, padTex)

                if "HalfPixelShift" in self.configuration:
                    text += "  %s += texpix_tx%s.xy * 0.5;\n" % (texcoord, padTex)

        text += "l_fragpos = l_position;"
        text += "}\n"

        text += "void fshader(\n"
        text += "float4 l_fragpos,\n"

        for i, name in texcoordSets:
            text += "  float2 %s : TEXCOORD%d,\n" % (name, i)

        for key in self.textures:
            text += "  uniform sampler2D k_tx" + key + ",\n"

        # Add all uniforms and create shader inputs variable
        shader_inputs = {}
        for name, uniform in self.uniforms.items():
            text += f"uniform {uniform['type']} {name},\n"
            shader_inputs[name] = uniform["value"]

        text += "  out float4 o_color : COLOR)\n"
        text += "{\n"
        text += """        l_fragpos /= l_fragpos.w; l_fragpos.xy = (l_fragpos.xy + 1) / 2;"""

        user_filters = sorted(self.filters.values(), key=lambda l: l["sort"])
        for user_filter in user_filters:
            text += "\n" + user_filter["shader"]

        text += "}\n"

        self.current_text = text

        shader = Shader.make(text, Shader.SL_Cg)
        if not shader:
            return False

        self.finalQuad.setShader(shader)

        # Apply all inputs and set up task for internal inputs
        for tex in self.textures:
            self.finalQuad.setShaderInput("tx" + tex, self.textures[tex])

        self.task = taskMgr.add(self.update, "common-filters-update")

        self.finalQuad.setShaderInputs(**shader_inputs)

        self.update()
        return True

    def update(self, task=None):
        """Updates the shader inputs that need to be updated every frame.
        Normally, you shouldn't call this, it's being called in a task."""

        if "VolumetricLighting" in self.configuration:
            caster = self.configuration["VolumetricLighting"].caster
            casterpos = LPoint2()
            self.manager.camera.node().getLens().project(caster.getPos(self.manager.camera), casterpos)
            self.finalQuad.setShaderInput("casterpos",
                                          LVecBase4(casterpos.getX() * 0.5 + 0.5, (casterpos.getY() * 0.5 + 0.5), 0, 0))
        if task is not None:
            return task.cont

    def setMSAA(self, samples):
        """Enables multisample anti-aliasing on the render-to-texture buffer.
        If you enable this, it is recommended to leave any multisample request
        on the main framebuffer OFF (ie. don't set framebuffer-multisample true
        in Config.prc), since it would be a waste of resources otherwise.

        .. versionadded:: 1.10.13
        """

        self.fbprops.setMultisamples(samples)

        camNode = self.manager.camera.node()
        state = camNode.getInitialState()
        state.setAttrib(AntialiasAttrib.make(AntialiasAttrib.M_multisample))
        camNode.setInitialState(state)

    def delMSAA(self):
        self.fbprops.setMultisamples(0)

    def setCartoonInk(self, separation=1, color=(0, 0, 0, 1), sort=1):
        self.setFilter(
            "CartoonInk",
            """float4 cartoondelta = cartoonseparation * texpix_txaux.xwyw;
float4 cartoon_c0 = tex2D(k_txaux, l_texcoord_aux + cartoondelta.xy);
float4 cartoon_c1 = tex2D(k_txaux, l_texcoord_aux - cartoondelta.xy);
float4 cartoon_c2 = tex2D(k_txaux, l_texcoord_aux + cartoondelta.wz);
float4 cartoon_c3 = tex2D(k_txaux, l_texcoord_aux - cartoondelta.wz);
float4 cartoon_mx = max(cartoon_c0, max(cartoon_c1, max(cartoon_c2, cartoon_c3)));
float4 cartoon_mn = min(cartoon_c0, min(cartoon_c1, min(cartoon_c2, cartoon_c3)));
float cartoon_thresh = saturate(dot(cartoon_mx - cartoon_mn, float4(3,3,0,0)) - 0.5);
o_color = lerp(o_color, cartooncolor, cartoon_thresh);""",
            uniforms={
                "cartoonseparation": {"type": "float4", "value": (separation, 0, separation, 0)},
                "cartooncolor": {"type": "float4", "value": color},
                "texpix_txaux": {"type": "float4", "value": (0, 0, 0, 0)}
            },
            auxbits=[AuxBitplaneAttrib.ABOAuxNormal],
            needed_textures=["aux"],
            needed_coords=["aux"],
            sort=sort
        )

    def delCartoonInk(self):
        self.del_filter("CartoonInk")

    def setBloom(self, blend=(0.3, 0.4, 0.3, 0.0), mintrigger=0.6, maxtrigger=1.0, desat=0.6, intensity=1.0,
                 size="medium", sort=4):
        """
        Applies the Bloom filter to the output.
        size can either be "off", "small", "medium", or "large".
        Setting size to "off" will remove the Bloom filter.
        """
        if size == 0 or size == "off":
            self.delBloom()
            return
        elif size == 1:
            size = "small"
        elif size == 2:
            size = "medium"
        elif size == 3:
            size = "large"

        if maxtrigger is None:
            maxtrigger = mintrigger + 0.8

        intensity *= 3.0
        if size == "large":
            scale = 8
            downsamplerName = "filter-down4"
            downsampler = DOWN_4
        elif size == "medium":
            scale = 4
            downsamplerName = "filter-copy"
            downsampler = COPY
        else:
            scale = 2
            downsamplerName = "filter-copy"
            downsampler = COPY

        self.setFilter("Bloom",
                       """
        o_color = saturate(o_color);
        float4 bloom = 0.5 * tex2D(k_txbloom3, l_texcoord_bloom3);
        o_color = 1-((1-bloom)*(1-o_color));
        """,
                       auxbits=[AuxBitplaneAttrib.ABOGlow],
                       needed_coords=["bloom3"],
                       needed_textures=["bloom0", "bloom1", "bloom2", "bloom3"],
                       render_into={
                           "filter-bloomi":
                               {
                                   "shader_inputs":
                                       {
                                           "blend": (blend[0], blend[1], blend[2], blend[3] * 2.0),
                                           "trigger": (mintrigger, 1.0 / (maxtrigger - mintrigger), 0.0, 0.0),
                                           "desat": desat,
                                           "src": "color"
                                       },
                                   "shader": Shader.make(BLOOM_I, Shader.SL_Cg),
                                   "colortex": "bloom0",
                                   "div": 2,
                                   "align": scale

                               },
                           downsamplerName:
                               {
                                   "colortex": "bloom1",
                                   "div": scale,
                                   "align": scale,
                                   "shader": Shader.make(downsampler, Shader.SL_Cg),
                                   "shader_inputs":
                                       {
                                           "src": "bloom0"
                                       }

                               },
                           "filter-bloomx":
                               {
                                   "colortex": "bloom2",
                                   "div": scale,
                                   "align": scale,
                                   "shader": Shader.make(BLOOM_X, Shader.SL_Cg),
                                   "shader_inputs":
                                       {
                                           "src": "bloom1"
                                       }

                               },
                           "filter-bloomy":
                               {
                                   "colortex": "bloom3",
                                   "div": scale,
                                   "align": scale,
                                   "shader": Shader.make(BLOOM_Y, Shader.SL_Cg),

                                   "shader_inputs":
                                       {
                                           "intensity": (intensity, intensity, intensity, intensity),
                                           "src": "bloom2"
                                       }
                               }
                       },
                       sort=sort
                       )

    def delBloom(self):
        self.del_filter("Bloom")

    def setHalfPixelShift(self):
        self.configuration["HalfPixelShift"] = 1
        return self.reconfigure()

    def delHalfPixelShift(self):
        if "HalfPixelShift" in self.configuration:
            del self.configuration["HalfPixelShift"]
            return self.reconfigure()
        return True

    def setViewGlow(self, sort=5):

        self.setFilter("ViewGlow", shader_string="o_color.r = o_color.a;",
                       auxbits=[AuxBitplaneAttrib.ABOAuxGlow], sort=sort)

    def delViewGlow(self):
        self.del_filter("ViewGlow")

    def setInverted(self, sort=10):
        self.setFilter("Inverted", "  o_color = float4(1, 1, 1, 1) - o_color;\n", sort=sort)

    def delInverted(self):
        self.del_filter("Inverted")

    def setVolumetricLighting(self, caster, numsamples=32, density=5.0, decay=0.1, exposure=0.1, source="color",
                              sort=6):
        config = FilterConfig()
        config.caster = caster
        self.configuration["VolumetricLighting"] = config

        self.setFilter("VolumetricLighting",
                       uniforms={
                           "volume_light_numsamples": {"type": "float", "value": numsamples},
                           "casterpos": {"type": "float4", "value": (0, 0, 0, 0)},
                           "vlparams": {"type": "float4", "value":
                               (density / float(numsamples), decay, exposure)}},
                       needed_textures=[source],
                       shader_string="""float decay = 1.0f;
          float2 curcoord = l_texcoord_color; 
          float2 lightdir = curcoord - casterpos.xy;\n
          lightdir *= vlparams.x;
          half4 sample = tex2D(k_txcolor, curcoord);
          float3 vlcolor = sample.rgb * sample.a;
          for (int i = 0; i < volume_light_numsamples; i++) {
            curcoord -= lightdir;
            sample = tex2D(k_tx@color, curcoord);
            sample *= sample.a * decay;
            vlcolor += sample.rgb;
            decay *= vlparams.y;
          }
          o_color += float4(vlcolor * vlparams.z, 1);""".replace("@color", source), sort=sort)

    def delVolumetricLighting(self):
        self.del_filter("VolumetricLighting")
        del self.configuration["VolumetricLighting"]

    def setBlurSharpen(self, amount=0.0, sort=3):
        """Enables the blur/sharpen filter. If the 'amount' parameter is 1.0, it will not have any effect.
        A value of 0.0 means fully blurred, and a value higher than 1.0 sharpens the image."""

        self.setFilter(
            "BlurSharpen",
            "  o_color = lerp(tex2D(k_txblur1, l_texcoord_blur1), o_color, blurval.x);\n",
            uniforms={"blurval": {"type": "float4", "value": LVecBase4(amount, amount, amount, amount)}},
            needed_textures=["blur0", "blur1"],
            needed_coords=["blur1"],
            render_into={
                "filter-blur0": {
                    "colortex": "blur0",
                    "div": 2,
                    "shader_inputs": {
                        "src": "color"
                    },
                    "shader": Shader.make(BLUR_X, Shader.SL_Cg)
                },
                "filter-blur1": {
                    "colortex": "blur1",
                    "shader_inputs":
                        {
                            "src": "blur0"
                        },
                    "shader": Shader.make(BLUR_Y, Shader.SL_Cg)
                }
            }, sort=sort
        )

    def delBlurSharpen(self):
        self.del_filter("BlurSharpen")

    def setAmbientOcclusion(self, numsamples=16, radius=0.05, amount=2.0, strength=0.01, falloff=0.000002, sort=2):
        self.setFilter("AmbientOcclusion",
                       shader_string="o_color *= tex2D(k_txssao2, l_texcoord_ssao2).r;\n",
                       needed_textures=["ssao0", "ssao1", "ssao2", "depth", "aux"],
                       needed_coords=["ssao2"],
                       auxbits=[AuxBitplaneAttrib.ABOAuxNormal],
                       render_into={
                           "filter-ssao0": {
                               "colortex": "ssao0",
                               "shader_inputs": {
                                   "depth": "depth",
                                   "params1": (numsamples, -amount / numsamples, radius, 0),
                                   "params2": (strength, falloff, 0, 0),
                                   "normal": "aux",
                                   "random": base.loader.loadTexture("maps/random.rgb"),

                               },
                               "shader": Shader.make(SSAO_BODY % numsamples, Shader.SL_Cg)
                           },
                           "filter-ssao1": {
                               "colortex": "ssao1",
                               "shader_inputs": {
                                   "src": "ssao0",

                               },
                               "div": 2,
                               "shader": Shader.make(BLUR_X, Shader.SL_Cg)
                           },
                           "filter-ssao2": {
                               "colortex": "ssao2",
                               "shader_inputs":
                                   {
                                       "src": "ssao1",
                                   },
                               "shader": Shader.make(BLUR_Y, Shader.SL_Cg)
                           },
                       }, sort=sort)

    def delAmbientOcclusion(self):
        self.del_filter("AmbientOcclusion")

    def setGammaAdjust(self, gamma, sort=None):
        """ Applies additional gamma correction to the image.  1.0 = no correction. """

        if gamma == 0.5:
            self.setFilter(
                "GammaAdjust",
                "  o_color.rgb = sqrt(o_color.rgb);\n", sort=sort
            )

        elif gamma == 2.0:
            self.setFilter(
                "GammaAdjust",
                "  o_color.rgb *= o_color.rgb;\n", sort=sort
            )

        elif gamma != 1.0:
            self.setFilter(
                "GammaAdjust",
                "  o_color.rgb = pow(o_color.rgb, %ff);\n" % gamma, sort=sort
            )

    def delGammaAdjust(self):
        self.del_filter("GammaAdjust")

    def setSrgbEncode(self, force=False, sort=9):
        """ Applies the inverse sRGB EOTF to the output, unless the window
        already has an sRGB framebuffer, in which case this filter refuses to
        apply, to prevent accidental double-application.

        Set the force argument to True to force it to be applied in all cases.

        .. versionadded:: 1.10.7
        """
        self.setFilter("SRBGEncode", shader_string="""
        o_color.r = (o_color.r < 0.0031308) ? (o_color.r * 12.92) : (1.055 * pow(o_color.r, 0.41666) - 0.055);
o_color.g = (o_color.g < 0.0031308) ? (o_color.g * 12.92) : (1.055 * pow(o_color.g, 0.41666) - 0.055);
o_color.b = (o_color.b < 0.0031308) ? (o_color.b * 12.92) : (1.055 * pow(o_color.b, 0.41666) - 0.055);"""
                       , sort=sort)

    def delSrgbEncode(self):
        """ Reverses the effects of setSrgbEncode. """
        self.del_filter("SRBGEncode")

    def setHighDynamicRange(self, sort=8):
        """ Enables HDR rendering by using a floating-point framebuffer,
        disabling color clamping on the main scene, and applying a tone map
        operator (ACES).

        It may also be necessary to use setExposureAdjust to perform exposure
        compensation on the scene, depending on the lighting intensity.

        .. versionadded:: 1.10.7
        """
        self.fbprops.setFloatColor(True)
        self.fbprops.setSrgbColor(False)
        self.clamping = False

        # With thanks to Stephen Hill!
        self.setFilter("HighDynamicRange",
                       shader_string="""float3 aces_color = mul(aces_input_mat, o_color.rgb);
                                     o_color.rgb = saturate(mul(aces_output_mat, 
                                     (aces_color * (aces_color + 0.0245786f) - 0.000090537f) / 
                                     (aces_color * (0.983729f * aces_color + 0.4329510f) + 0.238081f)));""",
                       consts=["""static const float3x3 aces_input_mat = {
                      {0.59719, 0.35458, 0.04823},
                      {0.07600, 0.90834, 0.01566},
                      {0.02840, 0.13383, 0.83777},
                    };
                    static const float3x3 aces_output_mat = {
                      { 1.60475, -0.53108, -0.07367},
                      {-0.10208,  1.10813, -0.00605},
                      {-0.00327, -0.07276,  1.07602},
                    };"""
                               ], sort=sort)

    def delHighDynamicRange(self):
        self.fbprops.setFloatColor(False)
        self.fbprops.setSrgbColor(True)
        self.clamping = True
        self.del_filter("HighDynamicRange")

    def setExposureAdjust(self, stops, sort=7):
        """ Sets a relative exposure adjustment to multiply with the result of
        rendering the scene, in stops.  A value of 0 means no adjustment, a
        positive value will result in a brighter image.  Useful in conjunction
        with HDR, see setHighDynamicRange.

        .. versionadded:: 1.10.7
        """
        self.setFilter("ExposureAdjust", shader_string="o_color.rgb *= exposure;",
                       uniforms={
                           "exposure": {
                               "type": "float",
                               "value": 2 ** stops}
                       },
                       sort=sort)

    def delExposureAdjust(self):
        self.del_filter("ExposureAdjust")

    def printShader(self):
        for count, line in enumerate(c.current_text.split("\n")):
            print(count, line)

    #snake_case alias:
    set_msaa = setMSAA
    del_msaa = delMSAA
    del_cartoon_ink = delCartoonInk
    set_half_pixel_shift = setHalfPixelShift
    del_half_pixel_shift = delHalfPixelShift
    set_inverted = setInverted
    del_inverted = delInverted
    del_view_glow = delViewGlow
    set_volumetric_lighting = setVolumetricLighting
    set_bloom = setBloom
    set_view_glow = setViewGlow
    set_ambient_occlusion = setAmbientOcclusion
    set_cartoon_ink = setCartoonInk
    del_bloom = delBloom
    del_ambient_occlusion = delAmbientOcclusion
    set_blur_sharpen = setBlurSharpen
    del_blur_sharpen = delBlurSharpen
    del_volumetric_lighting = delVolumetricLighting
    set_gamma_adjust = setGammaAdjust
    del_gamma_adjust = delGammaAdjust
    set_srgb_encode = setSrgbEncode
    del_srgb_encode = delSrgbEncode
    set_exposure_adjust = setExposureAdjust
    del_exposure_adjust = delExposureAdjust
    set_high_dynamic_range = setHighDynamicRange
    del_high_dynamic_range = delHighDynamicRange
    set_filter = setFilter
    load_filter = loadFilter
    del_filter = delFilter
    add_uniforms = addUniforms
    del_uniform = delUniform
    del_uniforms = delUniforms
    set_shader_inputs = setShaderInputs
    set_shader_input = setShaderInput
    print_shader = printShader


if __name__ == "__main__":
    from direct.showbase.ShowBase import ShowBase
    from panda3d.core import DirectionalLight

    s = ShowBase()

    # Set up models
    p = s.loader.load_model("panda")
    p.set_pos((0, 100, 0))
    p.reparent_to(s.render)
    e = s.loader.load_model("environment")
    e.reparent_to(s.render)

    # Create common filter
    c = CommonFilters(s.win, s.cam)

    # Adhoc custom shaders
    c.set_filter("raise amount",
                 "o_color += 0.5 * raise_amount;",
                 uniforms={
                     "raise_amount":
                         {
                             "value": 0.1,
                             "type": "float",
                         }
                 },
                 sort=0
                 )

    c.set_filter("increase red",
                 "o_color.r += increase_red;",
                 uniforms={
                     "increase_red": {
                         "value": 0.1,
                         "type": "float"
                     }
                 },
                 sort=13
                 )

    c.setMSAA(8)

    c.setBlurSharpen(0.5)
    c.set_gamma_adjust(1.4)

    # Volumetric Lighting
    d = DirectionalLight("A")
    dn = s.render.attach_new_node(d)
    s.render.set_light(dn)
    c.set_volumetric_lighting(dn)

    # Shaders are applied with correct sort regardless of when they're called
    c.setSrgbEncode()
    c.set_cartoon_ink()
    c.set_ambient_occlusion()
    c.set_bloom()
    c.set_view_glow()
    c.set_exposure_adjust(stops=1)
    c.set_high_dynamic_range()

    c.print_shader()

    s.accept("c", c.del_cartoon_ink)
    s.accept("p", c.print_shader)
    s.accept("m", c.del_msaa)
    s.accept("d", c.del_bloom)
    s.run()
