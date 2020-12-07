"""

Class CommonFilters implements certain common image
postprocessing filters.  See the :ref:`common-image-filters` page for
more information about how to use these filters.

It is not ideal that these filters are all included in a single
monolithic module.  Unfortunately, when you want to apply two filters
at the same time, you have to compose them into a single shader, and
the composition process isn't simply a question of concatenating them:
you have to somehow make them work together.  I suspect that there
exists some fairly simple framework that would make this automatable.
However, until I write some more filters myself, I won't know what
that framework is.  Until then, I'll settle for this
clunky approach.  - Josh

"""

from .FilterManager import FilterManager
from .filterBloomI import BLOOM_I
from .filterBloomX import BLOOM_X
from .filterBloomY import BLOOM_Y
from .filterBlurX import BLUR_X
from .filterBlurY import BLUR_Y
from .filterCopy import COPY
from .filterDown4 import DOWN_4
from panda3d.core import LVecBase4, LPoint2
from panda3d.core import Filename
from panda3d.core import AuxBitplaneAttrib
from panda3d.core import Texture, Shader, ATSNone
from panda3d.core import FrameBufferProperties
import os

CARTOON_BODY="""
float4 cartoondelta = k_cartoonseparation * texpix_txaux.xwyw;
float4 cartoon_c0 = tex2D(k_txaux, %(texcoord)s + cartoondelta.xy);
float4 cartoon_c1 = tex2D(k_txaux, %(texcoord)s - cartoondelta.xy);
float4 cartoon_c2 = tex2D(k_txaux, %(texcoord)s + cartoondelta.wz);
float4 cartoon_c3 = tex2D(k_txaux, %(texcoord)s - cartoondelta.wz);
float4 cartoon_mx = max(cartoon_c0, max(cartoon_c1, max(cartoon_c2, cartoon_c3)));
float4 cartoon_mn = min(cartoon_c0, min(cartoon_c1, min(cartoon_c2, cartoon_c3)));
float cartoon_thresh = saturate(dot(cartoon_mx - cartoon_mn, float4(3,3,0,0)) - 0.5);
o_color = lerp(o_color, k_cartooncolor, cartoon_thresh);
"""

# Some GPUs do not support variable-length loops.
#
# We fill in the actual value of numsamples in the loop limit
# when the shader is configured.
#
SSAO_BODY="""//Cg

void vshader(float4 vtx_position : POSITION,
             out float4 l_position : POSITION,
             out float2 l_texcoord : TEXCOORD0,
             out float2 l_texcoordD : TEXCOORD1,
             out float2 l_texcoordN : TEXCOORD2,
             uniform float4 texpad_depth,
             uniform float4 texpad_normal,
             uniform float4x4 mat_modelproj)
{
  l_position = mul(mat_modelproj, vtx_position);
  l_texcoord = vtx_position.xz;
  l_texcoordD = (vtx_position.xz * texpad_depth.xy) + texpad_depth.xy;
  l_texcoordN = (vtx_position.xz * texpad_normal.xy) + texpad_normal.xy;
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


class FilterConfig:
    pass

class CommonFilters:

    """ Class CommonFilters implements certain common image postprocessing
    filters.  The constructor requires a filter builder as a parameter. """

    def __init__(self, win, cam):
        self.manager = FilterManager(win, cam)
        self.configuration = {}
        self.task = None
        self.cleanup()

    def cleanup(self):
        self.manager.cleanup()
        self.textures = {}
        self.finalQuad = None
        self.bloom = []
        self.blur = []
        self.ssao = []
        if self.task != None:
          taskMgr.remove(self.task)
          self.task = None

    def reconfigure(self, fullrebuild, changed):
        """ Reconfigure is called whenever any configuration change is made. """

        configuration = self.configuration

        if (fullrebuild):

            self.cleanup()

            if (len(configuration) == 0):
                return

            if not self.manager.win.gsg.getSupportsBasicShaders():
                return False

            auxbits = 0
            needtex = set(["color"])
            needtexcoord = set(["color"])

            if ("CartoonInk" in configuration):
                needtex.add("aux")
                auxbits |= AuxBitplaneAttrib.ABOAuxNormal
                needtexcoord.add("aux")

            if ("AmbientOcclusion" in configuration):
                needtex.add("depth")
                needtex.add("ssao0")
                needtex.add("ssao1")
                needtex.add("ssao2")
                needtex.add("aux")
                auxbits |= AuxBitplaneAttrib.ABOAuxNormal
                needtexcoord.add("ssao2")

            if ("BlurSharpen" in configuration):
                needtex.add("blur0")
                needtex.add("blur1")
                needtexcoord.add("blur1")

            if ("Bloom" in configuration):
                needtex.add("bloom0")
                needtex.add("bloom1")
                needtex.add("bloom2")
                needtex.add("bloom3")
                auxbits |= AuxBitplaneAttrib.ABOGlow
                needtexcoord.add("bloom3")

            if ("ViewGlow" in configuration):
                auxbits |= AuxBitplaneAttrib.ABOGlow

            if ("VolumetricLighting" in configuration):
                needtex.add(configuration["VolumetricLighting"].source)

            for tex in needtex:
                self.textures[tex] = Texture("scene-" + tex)
                self.textures[tex].setWrapU(Texture.WMClamp)
                self.textures[tex].setWrapV(Texture.WMClamp)

            fbprops = None
            clamping = None
            if "HighDynamicRange" in configuration:
                fbprops = FrameBufferProperties()
                fbprops.setFloatColor(True)
                fbprops.setSrgbColor(False)
                clamping = False

            self.finalQuad = self.manager.renderSceneInto(textures = self.textures, auxbits=auxbits, fbprops=fbprops, clamping=clamping)
            if (self.finalQuad == None):
                self.cleanup()
                return False

            if ("BlurSharpen" in configuration):
                blur0=self.textures["blur0"]
                blur1=self.textures["blur1"]
                self.blur.append(self.manager.renderQuadInto("filter-blur0", colortex=blur0,div=2))
                self.blur.append(self.manager.renderQuadInto("filter-blur1", colortex=blur1))
                self.blur[0].setShaderInput("src", self.textures["color"])
                self.blur[0].setShader(Shader.make(BLUR_X, Shader.SL_Cg))
                self.blur[1].setShaderInput("src", blur0)
                self.blur[1].setShader(Shader.make(BLUR_Y, Shader.SL_Cg))

            if ("AmbientOcclusion" in configuration):
                ssao0=self.textures["ssao0"]
                ssao1=self.textures["ssao1"]
                ssao2=self.textures["ssao2"]
                self.ssao.append(self.manager.renderQuadInto("filter-ssao0", colortex=ssao0))
                self.ssao.append(self.manager.renderQuadInto("filter-ssao1", colortex=ssao1,div=2))
                self.ssao.append(self.manager.renderQuadInto("filter-ssao2", colortex=ssao2))
                self.ssao[0].setShaderInput("depth", self.textures["depth"])
                self.ssao[0].setShaderInput("normal", self.textures["aux"])
                self.ssao[0].setShaderInput("random", loader.loadTexture("maps/random.rgb"))
                self.ssao[0].setShader(Shader.make(SSAO_BODY % configuration["AmbientOcclusion"].numsamples, Shader.SL_Cg))
                self.ssao[1].setShaderInput("src", ssao0)
                self.ssao[1].setShader(Shader.make(BLUR_X, Shader.SL_Cg))
                self.ssao[2].setShaderInput("src", ssao1)
                self.ssao[2].setShader(Shader.make(BLUR_Y, Shader.SL_Cg))

            if ("Bloom" in configuration):
                bloomconf = configuration["Bloom"]
                bloom0=self.textures["bloom0"]
                bloom1=self.textures["bloom1"]
                bloom2=self.textures["bloom2"]
                bloom3=self.textures["bloom3"]
                if (bloomconf.size == "large"):
                    scale=8
                    downsamplerName="filter-down4"
                    downsampler=DOWN_4
                elif (bloomconf.size == "medium"):
                    scale=4
                    downsamplerName="filter-copy"
                    downsampler=COPY
                else:
                    scale=2
                    downsamplerName="filter-copy"
                    downsampler=COPY
                self.bloom.append(self.manager.renderQuadInto("filter-bloomi", colortex=bloom0, div=2,     align=scale))
                self.bloom.append(self.manager.renderQuadInto(downsamplerName, colortex=bloom1, div=scale, align=scale))
                self.bloom.append(self.manager.renderQuadInto("filter-bloomx", colortex=bloom2, div=scale, align=scale))
                self.bloom.append(self.manager.renderQuadInto("filter-bloomy", colortex=bloom3, div=scale, align=scale))
                self.bloom[0].setShaderInput("src", self.textures["color"])
                self.bloom[0].setShader(Shader.make(BLOOM_I, Shader.SL_Cg))
                self.bloom[1].setShaderInput("src", bloom0)
                self.bloom[1].setShader(Shader.make(downsampler, Shader.SL_Cg))
                self.bloom[2].setShaderInput("src", bloom1)
                self.bloom[2].setShader(Shader.make(BLOOM_X, Shader.SL_Cg))
                self.bloom[3].setShaderInput("src", bloom2)
                self.bloom[3].setShader(Shader.make(BLOOM_Y, Shader.SL_Cg))

            texcoords = {}
            texcoordPadding = {}

            for tex in needtexcoord:
                if self.textures[tex].getAutoTextureScale() != ATSNone or \
                                           "HalfPixelShift" in configuration:
                    texcoords[tex] = "l_texcoord_" + tex
                    texcoordPadding["l_texcoord_" + tex] = tex
                else:
                    # Share unpadded texture coordinates.
                    texcoords[tex] = "l_texcoord"
                    texcoordPadding["l_texcoord"] = None

            texcoordSets = list(enumerate(texcoordPadding.keys()))

            text = "//Cg\n"
            if "HighDynamicRange" in configuration:
                text += "static const float3x3 aces_input_mat = {\n"
                text += "  {0.59719, 0.35458, 0.04823},\n"
                text += "  {0.07600, 0.90834, 0.01566},\n"
                text += "  {0.02840, 0.13383, 0.83777},\n"
                text += "};\n"
                text += "static const float3x3 aces_output_mat = {\n"
                text += "  { 1.60475, -0.53108, -0.07367},\n"
                text += "  {-0.10208,  1.10813, -0.00605},\n"
                text += "  {-0.00327, -0.07276,  1.07602},\n"
                text += "};\n"
            text += "void vshader(float4 vtx_position : POSITION,\n"
            text += "  out float4 l_position : POSITION,\n"

            for texcoord, padTex in texcoordPadding.items():
                if padTex is not None:
                    text += "  uniform float4 texpad_tx%s,\n" % (padTex)
                    if ("HalfPixelShift" in configuration):
                        text += "  uniform float4 texpix_tx%s,\n" % (padTex)

            for i, name in texcoordSets:
                text += "  out float2 %s : TEXCOORD%d,\n" % (name, i)

            text += "  uniform float4x4 mat_modelproj)\n"
            text += "{\n"
            text += "  l_position = mul(mat_modelproj, vtx_position);\n"

            for texcoord, padTex in texcoordPadding.items():
                if padTex is None:
                    text += "  %s = vtx_position.xz * float2(0.5, 0.5) + float2(0.5, 0.5);\n" % (texcoord)
                else:
                    text += "  %s = (vtx_position.xz * texpad_tx%s.xy) + texpad_tx%s.xy;\n" % (texcoord, padTex, padTex)

                    if ("HalfPixelShift" in configuration):
                        text += "  %s += texpix_tx%s.xy * 0.5;\n" % (texcoord, padTex)

            text += "}\n"

            text += "void fshader(\n"

            for i, name in texcoordSets:
                text += "  float2 %s : TEXCOORD%d,\n" % (name, i)

            for key in self.textures:
                text += "  uniform sampler2D k_tx" + key + ",\n"

            if ("CartoonInk" in configuration):
                text += "  uniform float4 k_cartoonseparation,\n"
                text += "  uniform float4 k_cartooncolor,\n"
                text += "  uniform float4 texpix_txaux,\n"

            if ("BlurSharpen" in configuration):
                text += "  uniform float4 k_blurval,\n"

            if ("VolumetricLighting" in configuration):
                text += "  uniform float4 k_casterpos,\n"
                text += "  uniform float4 k_vlparams,\n"

            if ("ExposureAdjust" in configuration):
                text += "  uniform float k_exposure,\n"

            text += "  out float4 o_color : COLOR)\n"
            text += "{\n"
            text += "  o_color = tex2D(k_txcolor, %s);\n" % (texcoords["color"])
            if ("CartoonInk" in configuration):
                text += CARTOON_BODY % {"texcoord" : texcoords["aux"]}
            if ("AmbientOcclusion" in configuration):
                text += "  o_color *= tex2D(k_txssao2, %s).r;\n" % (texcoords["ssao2"])
            if ("BlurSharpen" in configuration):
                text += "  o_color = lerp(tex2D(k_txblur1, %s), o_color, k_blurval.x);\n" % (texcoords["blur1"])
            if ("Bloom" in configuration):
                text += "  o_color = saturate(o_color);\n";
                text += "  float4 bloom = 0.5 * tex2D(k_txbloom3, %s);\n" % (texcoords["bloom3"])
                text += "  o_color = 1-((1-bloom)*(1-o_color));\n"
            if ("ViewGlow" in configuration):
                text += "  o_color.r = o_color.a;\n"
            if ("VolumetricLighting" in configuration):
                text += "  float decay = 1.0f;\n"
                text += "  float2 curcoord = %s;\n" % (texcoords["color"])
                text += "  float2 lightdir = curcoord - k_casterpos.xy;\n"
                text += "  lightdir *= k_vlparams.x;\n"
                text += "  half4 sample = tex2D(k_txcolor, curcoord);\n"
                text += "  float3 vlcolor = sample.rgb * sample.a;\n"
                text += "  for (int i = 0; i < %s; i++) {\n" % (int(configuration["VolumetricLighting"].numsamples))
                text += "    curcoord -= lightdir;\n"
                text += "    sample = tex2D(k_tx%s, curcoord);\n" % (configuration["VolumetricLighting"].source)
                text += "    sample *= sample.a * decay;//*weight\n"
                text += "    vlcolor += sample.rgb;\n"
                text += "    decay *= k_vlparams.y;\n"
                text += "  }\n"
                text += "  o_color += float4(vlcolor * k_vlparams.z, 1);\n"

            if ("ExposureAdjust" in configuration):
                text += "  o_color.rgb *= k_exposure;\n"

            # With thanks to Stephen Hill!
            if ("HighDynamicRange" in configuration):
                text += "  float3 aces_color = mul(aces_input_mat, o_color.rgb);\n"
                text += "  o_color.rgb = saturate(mul(aces_output_mat, (aces_color * (aces_color + 0.0245786f) - 0.000090537f) / (aces_color * (0.983729f * aces_color + 0.4329510f) + 0.238081f)));\n"

            if ("GammaAdjust" in configuration):
                gamma = configuration["GammaAdjust"]
                if gamma == 0.5:
                    text += "  o_color.rgb = sqrt(o_color.rgb);\n"
                elif gamma == 2.0:
                    text += "  o_color.rgb *= o_color.rgb;\n"
                elif gamma != 1.0:
                    text += "  o_color.rgb = pow(o_color.rgb, %ff);\n" % (gamma)

            if ("SrgbEncode" in configuration):
                text += "  o_color.r = (o_color.r < 0.0031308) ? (o_color.r * 12.92) : (1.055 * pow(o_color.r, 0.41666) - 0.055);\n"
                text += "  o_color.g = (o_color.g < 0.0031308) ? (o_color.g * 12.92) : (1.055 * pow(o_color.g, 0.41666) - 0.055);\n"
                text += "  o_color.b = (o_color.b < 0.0031308) ? (o_color.b * 12.92) : (1.055 * pow(o_color.b, 0.41666) - 0.055);\n"

            if ("Inverted" in configuration):
                text += "  o_color = float4(1, 1, 1, 1) - o_color;\n"
            text += "}\n"

            shader = Shader.make(text, Shader.SL_Cg)
            if not shader:
                return False
            self.finalQuad.setShader(shader)
            for tex in self.textures:
                self.finalQuad.setShaderInput("tx"+tex, self.textures[tex])

            self.task = taskMgr.add(self.update, "common-filters-update")

        if (changed == "CartoonInk") or fullrebuild:
            if ("CartoonInk" in configuration):
                c = configuration["CartoonInk"]
                self.finalQuad.setShaderInput("cartoonseparation", LVecBase4(c.separation, 0, c.separation, 0))
                self.finalQuad.setShaderInput("cartooncolor", c.color)

        if (changed == "BlurSharpen") or fullrebuild:
            if ("BlurSharpen" in configuration):
                blurval = configuration["BlurSharpen"]
                self.finalQuad.setShaderInput("blurval", LVecBase4(blurval, blurval, blurval, blurval))

        if (changed == "Bloom") or fullrebuild:
            if ("Bloom" in configuration):
                bloomconf = configuration["Bloom"]
                intensity = bloomconf.intensity * 3.0
                self.bloom[0].setShaderInput("blend", bloomconf.blendx, bloomconf.blendy, bloomconf.blendz, bloomconf.blendw * 2.0)
                self.bloom[0].setShaderInput("trigger", bloomconf.mintrigger, 1.0/(bloomconf.maxtrigger-bloomconf.mintrigger), 0.0, 0.0)
                self.bloom[0].setShaderInput("desat", bloomconf.desat)
                self.bloom[3].setShaderInput("intensity", intensity, intensity, intensity, intensity)

        if (changed == "VolumetricLighting") or fullrebuild:
            if ("VolumetricLighting" in configuration):
                config = configuration["VolumetricLighting"]
                tcparam = config.density / float(config.numsamples)
                self.finalQuad.setShaderInput("vlparams", tcparam, config.decay, config.exposure, 0.0)

        if (changed == "AmbientOcclusion") or fullrebuild:
            if ("AmbientOcclusion" in configuration):
                config = configuration["AmbientOcclusion"]
                self.ssao[0].setShaderInput("params1", config.numsamples, -float(config.amount) / config.numsamples, config.radius, 0)
                self.ssao[0].setShaderInput("params2", config.strength, config.falloff, 0, 0)

        if (changed == "ExposureAdjust") or fullrebuild:
            if ("ExposureAdjust" in configuration):
                stops = configuration["ExposureAdjust"]
                self.finalQuad.setShaderInput("exposure", 2 ** stops)

        self.update()
        return True

    def update(self, task = None):
        """Updates the shader inputs that need to be updated every frame.
        Normally, you shouldn't call this, it's being called in a task."""

        if "VolumetricLighting" in self.configuration:
            caster = self.configuration["VolumetricLighting"].caster
            casterpos = LPoint2()
            self.manager.camera.node().getLens().project(caster.getPos(self.manager.camera), casterpos)
            self.finalQuad.setShaderInput("casterpos", LVecBase4(casterpos.getX() * 0.5 + 0.5, (casterpos.getY() * 0.5 + 0.5), 0, 0))
        if task != None:
            return task.cont

    def setCartoonInk(self, separation=1, color=(0, 0, 0, 1)):
        fullrebuild = (("CartoonInk" in self.configuration) == False)
        newconfig = FilterConfig()
        newconfig.separation = separation
        newconfig.color = color
        self.configuration["CartoonInk"] = newconfig
        return self.reconfigure(fullrebuild, "CartoonInk")

    def delCartoonInk(self):
        if ("CartoonInk" in self.configuration):
            del self.configuration["CartoonInk"]
            return self.reconfigure(True, "CartoonInk")
        return True

    def setBloom(self, blend=(0.3,0.4,0.3,0.0), mintrigger=0.6, maxtrigger=1.0, desat=0.6, intensity=1.0, size="medium"):
        if   (size==0): size="off"
        elif (size==1): size="small"
        elif (size==2): size="medium"
        elif (size==3): size="large"
        if (size=="off"):
            self.delBloom()
            return
        if (maxtrigger==None): maxtrigger=mintrigger+0.8
        oldconfig = self.configuration.get("Bloom", None)
        fullrebuild = True
        if (oldconfig) and (oldconfig.size == size):
            fullrebuild = False
        newconfig = FilterConfig()
        (newconfig.blendx, newconfig.blendy, newconfig.blendz, newconfig.blendw) = blend
        newconfig.maxtrigger = maxtrigger
        newconfig.mintrigger = mintrigger
        newconfig.desat = desat
        newconfig.intensity = intensity
        newconfig.size = size
        self.configuration["Bloom"] = newconfig
        return self.reconfigure(fullrebuild, "Bloom")

    def delBloom(self):
        if ("Bloom" in self.configuration):
            del self.configuration["Bloom"]
            return self.reconfigure(True, "Bloom")
        return True

    def setHalfPixelShift(self):
        fullrebuild = (("HalfPixelShift" in self.configuration) == False)
        self.configuration["HalfPixelShift"] = 1
        return self.reconfigure(fullrebuild, "HalfPixelShift")

    def delHalfPixelShift(self):
        if ("HalfPixelShift" in self.configuration):
            del self.configuration["HalfPixelShift"]
            return self.reconfigure(True, "HalfPixelShift")
        return True

    def setViewGlow(self):
        fullrebuild = (("ViewGlow" in self.configuration) == False)
        self.configuration["ViewGlow"] = 1
        return self.reconfigure(fullrebuild, "ViewGlow")

    def delViewGlow(self):
        if ("ViewGlow" in self.configuration):
            del self.configuration["ViewGlow"]
            return self.reconfigure(True, "ViewGlow")
        return True

    def setInverted(self):
        fullrebuild = (("Inverted" in self.configuration) == False)
        self.configuration["Inverted"] = 1
        return self.reconfigure(fullrebuild, "Inverted")

    def delInverted(self):
        if ("Inverted" in self.configuration):
            del self.configuration["Inverted"]
            return self.reconfigure(True, "Inverted")
        return True

    def setVolumetricLighting(self, caster, numsamples = 32, density = 5.0, decay = 0.1, exposure = 0.1, source = "color"):
        oldconfig = self.configuration.get("VolumetricLighting", None)
        fullrebuild = True
        if (oldconfig) and (oldconfig.source == source) and (oldconfig.numsamples == int(numsamples)):
            fullrebuild = False
        newconfig = FilterConfig()
        newconfig.caster = caster
        newconfig.numsamples = int(numsamples)
        newconfig.density = density
        newconfig.decay = decay
        newconfig.exposure = exposure
        newconfig.source = source
        self.configuration["VolumetricLighting"] = newconfig
        return self.reconfigure(fullrebuild, "VolumetricLighting")

    def delVolumetricLighting(self):
        if ("VolumetricLighting" in self.configuration):
            del self.configuration["VolumetricLighting"]
            return self.reconfigure(True, "VolumetricLighting")
        return True

    def setBlurSharpen(self, amount=0.0):
        """Enables the blur/sharpen filter. If the 'amount' parameter is 1.0, it will not have effect.
        A value of 0.0 means fully blurred, and a value higher than 1.0 sharpens the image."""
        fullrebuild = (("BlurSharpen" in self.configuration) == False)
        self.configuration["BlurSharpen"] = amount
        return self.reconfigure(fullrebuild, "BlurSharpen")

    def delBlurSharpen(self):
        if ("BlurSharpen" in self.configuration):
            del self.configuration["BlurSharpen"]
            return self.reconfigure(True, "BlurSharpen")
        return True

    def setAmbientOcclusion(self, numsamples = 16, radius = 0.05, amount = 2.0, strength = 0.01, falloff = 0.000002):
        fullrebuild = (("AmbientOcclusion" in self.configuration) == False)

        if (not fullrebuild):
            fullrebuild = (numsamples != self.configuration["AmbientOcclusion"].numsamples)

        newconfig = FilterConfig()
        newconfig.numsamples = numsamples
        newconfig.radius = radius
        newconfig.amount = amount
        newconfig.strength = strength
        newconfig.falloff = falloff
        self.configuration["AmbientOcclusion"] = newconfig
        return self.reconfigure(fullrebuild, "AmbientOcclusion")

    def delAmbientOcclusion(self):
        if ("AmbientOcclusion" in self.configuration):
            del self.configuration["AmbientOcclusion"]
            return self.reconfigure(True, "AmbientOcclusion")
        return True

    def setGammaAdjust(self, gamma):
        """ Applies additional gamma correction to the image.  1.0 = no correction. """
        old_gamma = self.configuration.get("GammaAdjust", 1.0)
        if old_gamma != gamma:
            self.configuration["GammaAdjust"] = gamma
            return self.reconfigure(True, "GammaAdjust")
        return True

    def delGammaAdjust(self):
        if ("GammaAdjust" in self.configuration):
            old_gamma = self.configuration["GammaAdjust"]
            del self.configuration["GammaAdjust"]
            return self.reconfigure((old_gamma != 1.0), "GammaAdjust")
        return True

    def setSrgbEncode(self, force=False):
        """ Applies the inverse sRGB EOTF to the output, unless the window
        already has an sRGB framebuffer, in which case this filter refuses to
        apply, to prevent accidental double-application.

        Set the force argument to True to force it to be applied in all cases.

        .. versionadded:: 1.10.7
        """
        new_enable = force or not self.manager.win.getFbProperties().getSrgbColor()
        old_enable = self.configuration.get("SrgbEncode", False)
        if new_enable and not old_enable:
            self.configuration["SrgbEncode"] = True
            return self.reconfigure(True, "SrgbEncode")
        elif not new_enable and old_enable:
            del self.configuration["SrgbEncode"]
        return new_enable

    def delSrgbEncode(self):
        """ Reverses the effects of setSrgbEncode. """
        if ("SrgbEncode" in self.configuration):
            old_enable = self.configuration["SrgbEncode"]
            del self.configuration["SrgbEncode"]
            return self.reconfigure(old_enable, "SrgbEncode")
        return True

    def setHighDynamicRange(self):
        """ Enables HDR rendering by using a floating-point framebuffer,
        disabling color clamping on the main scene, and applying a tone map
        operator (ACES).

        It may also be necessary to use setExposureAdjust to perform exposure
        compensation on the scene, depending on the lighting intensity.

        .. versionadded:: 1.10.7
        """

        fullrebuild = (("HighDynamicRange" in self.configuration) is False)
        self.configuration["HighDynamicRange"] = 1
        return self.reconfigure(fullrebuild, "HighDynamicRange")

    def delHighDynamicRange(self):
        if ("HighDynamicRange" in self.configuration):
            del self.configuration["HighDynamicRange"]
            return self.reconfigure(True, "HighDynamicRange")
        return True

    def setExposureAdjust(self, stops):
        """ Sets a relative exposure adjustment to multiply with the result of
        rendering the scene, in stops.  A value of 0 means no adjustment, a
        positive value will result in a brighter image.  Useful in conjunction
        with HDR, see setHighDynamicRange.

        .. versionadded:: 1.10.7
        """
        old_stops = self.configuration.get("ExposureAdjust")
        if old_stops != stops:
            self.configuration["ExposureAdjust"] = stops
            return self.reconfigure(old_stops is None, "ExposureAdjust")
        return True

    def delExposureAdjust(self):
        if ("ExposureAdjust" in self.configuration):
            del self.configuration["ExposureAdjust"]
            return self.reconfigure(True, "ExposureAdjust")
        return True

    #snake_case alias:
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
