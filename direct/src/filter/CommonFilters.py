"""

Class CommonFilters implements certain common image
postprocessing filters.

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

from FilterManager import FilterManager
from pandac.PandaModules import Point3, Vec3, Vec4, Point2
from pandac.PandaModules import NodePath, PandaNode
from pandac.PandaModules import Filename
from pandac.PandaModules import AuxBitplaneAttrib
from pandac.PandaModules import RenderState, Texture, Shader
import sys,os

CARTOON_BODY="""
float4 cartoondelta = k_cartoonseparation * texpix_txaux.xwyw;
float4 cartoon_p0 = l_texcoordN + cartoondelta.xyzw;
float4 cartoon_c0 = tex2D(k_txaux, cartoon_p0.xy);
float4 cartoon_p1 = l_texcoordN - cartoondelta.xyzw;
float4 cartoon_c1 = tex2D(k_txaux, cartoon_p1.xy);
float4 cartoon_p2 = l_texcoordN + cartoondelta.wzyx;
float4 cartoon_c2 = tex2D(k_txaux, cartoon_p2.xy);
float4 cartoon_p3 = l_texcoordN - cartoondelta.wzyx;
float4 cartoon_c3 = tex2D(k_txaux, cartoon_p3.xy);
float4 cartoon_mx = max(cartoon_c0,max(cartoon_c1,max(cartoon_c2,cartoon_c3)));
float4 cartoon_mn = min(cartoon_c0,min(cartoon_c1,min(cartoon_c2,cartoon_c3)));
float cartoon_thresh = saturate(dot(cartoon_mx - cartoon_mn, float4(3,3,0,0)) - 0.5);
o_color = lerp(o_color, k_cartooncolor, cartoon_thresh);
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

    def loadShader(self, name):
        fn = os.path.join(os.path.abspath(os.path.dirname(__file__)), name)
        fn = Filename.fromOsSpecific(fn)
        fn.makeTrueCase()
        return Shader.load(fn)

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

            auxbits = 0
            needtex = {}
            needtex["color"] = True
            if ("CartoonInk" in configuration):
                needtex["aux"] = True
                auxbits |= AuxBitplaneAttrib.ABOAuxNormal
            if ("AmbientOcclusion" in configuration):
                needtex["depth"] = True
                needtex["ssao0"] = True
                needtex["ssao1"] = True
                needtex["ssao2"] = True
                needtex["aux"] = True
                auxbits |= AuxBitplaneAttrib.ABOAuxNormal
            if ("BlurSharpen" in configuration):
                needtex["blur0"] = True
                needtex["blur1"] = True
            if ("Bloom" in configuration):
                needtex["bloom0"] = True
                needtex["bloom1"] = True
                needtex["bloom2"] = True
                needtex["bloom3"] = True
                auxbits |= AuxBitplaneAttrib.ABOGlow
            if ("ViewGlow" in configuration):
                auxbits |= AuxBitplaneAttrib.ABOGlow
            if ("VolumetricLighting" in configuration):
                needtex[configuration["VolumetricLighting"].source] = True
            for tex in needtex:
                self.textures[tex] = Texture("scene-"+tex)
                self.textures[tex].setWrapU(Texture.WMClamp)
                self.textures[tex].setWrapV(Texture.WMClamp)
                needtexpix = True

            self.finalQuad = self.manager.renderSceneInto(textures = self.textures, auxbits=auxbits)
            if (self.finalQuad == None):
                self.cleanup()
                return False

            if ("BlurSharpen" in configuration):
                blur0=self.textures["blur0"]
                blur1=self.textures["blur1"]
                self.blur.append(self.manager.renderQuadInto(colortex=blur0,div=2))
                self.blur.append(self.manager.renderQuadInto(colortex=blur1))
                self.blur[0].setShaderInput("src", self.textures["color"])
                self.blur[0].setShader(self.loadShader("filter-blurx.sha"))
                self.blur[1].setShaderInput("src", blur0)
                self.blur[1].setShader(self.loadShader("filter-blury.sha"))

            if ("AmbientOcclusion" in configuration):
                ssao0=self.textures["ssao0"]
                ssao1=self.textures["ssao1"]
                ssao2=self.textures["ssao2"]
                self.ssao.append(self.manager.renderQuadInto(colortex=ssao0))
                self.ssao.append(self.manager.renderQuadInto(colortex=ssao1,div=2))
                self.ssao.append(self.manager.renderQuadInto(colortex=ssao2))
                self.ssao[0].setShaderInput("depth", self.textures["depth"])
                self.ssao[0].setShaderInput("normal", self.textures["aux"])
                self.ssao[0].setShaderInput("random", loader.loadTexture("maps/random.rgb"))
                self.ssao[0].setShader(self.loadShader("filter-ssao.sha"))
                self.ssao[1].setShaderInput("src", ssao0)
                self.ssao[1].setShader(self.loadShader("filter-blurx.sha"))
                self.ssao[2].setShaderInput("src", ssao1)
                self.ssao[2].setShader(self.loadShader("filter-blury.sha"))

            if ("Bloom" in configuration):
                bloomconf = configuration["Bloom"]
                bloom0=self.textures["bloom0"]
                bloom1=self.textures["bloom1"]
                bloom2=self.textures["bloom2"]
                bloom3=self.textures["bloom3"]
                if (bloomconf.size == "large"):
                    scale=8
                    downsampler="filter-down4.sha"
                elif (bloomconf.size == "medium"):
                    scale=4
                    downsampler="filter-copy.sha"
                else:
                    scale=2
                    downsampler="filter-copy.sha"
                self.bloom.append(self.manager.renderQuadInto(colortex=bloom0, div=2,     align=scale))
                self.bloom.append(self.manager.renderQuadInto(colortex=bloom1, div=scale, align=scale))
                self.bloom.append(self.manager.renderQuadInto(colortex=bloom2, div=scale, align=scale))
                self.bloom.append(self.manager.renderQuadInto(colortex=bloom3, div=scale, align=scale))
                self.bloom[0].setShaderInput("src", self.textures["color"])
                self.bloom[0].setShader(self.loadShader("filter-bloomi.sha"))
                self.bloom[1].setShaderInput("src", bloom0)
                self.bloom[1].setShader(self.loadShader(downsampler))
                self.bloom[2].setShaderInput("src", bloom1)
                self.bloom[2].setShader(self.loadShader("filter-bloomx.sha"))
                self.bloom[3].setShaderInput("src", bloom2)
                self.bloom[3].setShader(self.loadShader("filter-bloomy.sha"))

            text = "//Cg\n"
            text += "void vshader(float4 vtx_position : POSITION,\n"
            text += " out float4 l_position : POSITION,\n"
            text += " uniform float4 texpad_txcolor,\n"
            text += " uniform float4 texpix_txcolor,\n"
            text += " out float4 l_texcoordC : TEXCOORD0,\n"
            if ("CartoonInk" in configuration):
                text += " uniform float4 texpad_txaux,\n"
                text += " uniform float4 texpix_txaux,\n"
                text += " out float4 l_texcoordN : TEXCOORD1,\n"
            if ("Bloom" in configuration):
                text += " uniform float4 texpad_txbloom3,\n"
                text += " out float4 l_texcoordB : TEXCOORD2,\n"
            if ("BlurSharpen" in configuration):
                text += " uniform float4 texpad_txblur1,\n"
                text += " out float4 l_texcoordBS : TEXCOORD3,\n"
            if ("AmbientOcclusion" in configuration):
                text += " uniform float4 texpad_txssao2,\n"
                text += " out float4 l_texcoordAO : TEXCOORD4,\n"
            text += " uniform float4x4 mat_modelproj)\n"
            text += "{\n"
            text += " l_position=mul(mat_modelproj, vtx_position);\n"
            text += " l_texcoordC=(vtx_position.xzxz * texpad_txcolor) + texpad_txcolor;\n"
            if ("CartoonInk" in configuration):
                text += " l_texcoordN=(vtx_position.xzxz * texpad_txaux) + texpad_txaux;\n"
            if ("Bloom" in configuration):
                text += " l_texcoordB=(vtx_position.xzxz * texpad_txbloom3) + texpad_txbloom3;\n"
            if ("BlurSharpen" in configuration):
                text += " l_texcoordBS=(vtx_position.xzxz * texpad_txblur1) + texpad_txblur1;\n"
            if ("AmbientOcclusion" in configuration):
                text += " l_texcoordAO=(vtx_position.xzxz * texpad_txssao2) + texpad_txssao2;\n"
            if ("HalfPixelShift" in configuration):
                text += " l_texcoordC+=texpix_txcolor*0.5;\n"
                if ("CartoonInk" in configuration):
                    text += " l_texcoordN+=texpix_txaux*0.5;\n"
            text += "}\n"

            text += "void fshader(\n"
            text += "float4 l_texcoordC : TEXCOORD0,\n"
            text += "uniform float4 texpix_txcolor,\n"
            if ("CartoonInk" in configuration):
                text += "float4 l_texcoordN : TEXCOORD1,\n"
                text += "uniform float4 texpix_txaux,\n"
            if ("Bloom" in configuration):
                text += "float4 l_texcoordB : TEXCOORD2,\n"
            if ("BlurSharpen" in configuration):
                text += "float4 l_texcoordBS : TEXCOORD3,\n"
                text += "uniform float4 k_blurval,\n"
            if ("AmbientOcclusion" in configuration):
                text += "float4 l_texcoordAO : TEXCOORD4,\n"
            for key in self.textures:
                text += "uniform sampler2D k_tx" + key + ",\n"
            if ("CartoonInk" in configuration):
                text += "uniform float4 k_cartoonseparation,\n"
                text += "uniform float4 k_cartooncolor,\n"
            if ("VolumetricLighting" in configuration):
                text += "uniform float4 k_casterpos,\n"
                text += "uniform float4 k_vlparams,\n"
            text += "out float4 o_color : COLOR)\n"
            text += "{\n"
            text += " o_color = tex2D(k_txcolor, l_texcoordC.xy);\n"
            if ("CartoonInk" in configuration):
                text += CARTOON_BODY
            if ("AmbientOcclusion" in configuration):
                text += "o_color *= tex2D(k_txssao2, l_texcoordAO.xy).r;\n"
            if ("BlurSharpen" in configuration):
                text += " o_color = lerp(tex2D(k_txblur1, l_texcoordBS.xy), o_color, k_blurval.x);\n"
            if ("Bloom" in configuration):
                text += "o_color = saturate(o_color);\n";
                text += "float4 bloom = 0.5*tex2D(k_txbloom3, l_texcoordB.xy);\n"
                text += "o_color = 1-((1-bloom)*(1-o_color));\n"
            if ("ViewGlow" in configuration):
                text += "o_color.r = o_color.a;\n"
            if ("VolumetricLighting" in configuration):
                text += "float decay = 1.0f;\n"
                text += "float2 curcoord = l_texcoordC.xy;\n"
                text += "float2 lightdir = curcoord - k_casterpos.xy;\n"
                text += "lightdir *= k_vlparams.x;\n"
                text += "half4 sample = tex2D(k_txcolor, curcoord);\n"
                text += "float3 vlcolor = sample.rgb * sample.a;\n"
                text += "for (int i = 0; i < %s; i++) {\n" % (int(configuration["VolumetricLighting"].numsamples))
                text += "  curcoord -= lightdir;\n"
                text += "  sample = tex2D(k_tx%s, curcoord);\n" % (configuration["VolumetricLighting"].source)
                text += "  sample *= sample.a * decay;//*weight\n"
                text += "  vlcolor += sample.rgb;\n"
                text += "  decay *= k_vlparams.y;\n"
                text += "}\n"
                text += "o_color += float4(vlcolor * k_vlparams.z, 1);\n"
            if ("Inverted" in configuration):
                text += "o_color = float4(1, 1, 1, 1) - o_color;\n"
            text += "}\n"
            
            self.finalQuad.setShader(Shader.make(text))
            for tex in self.textures:
                self.finalQuad.setShaderInput("tx"+tex, self.textures[tex])
            
            self.task = taskMgr.add(self.update, "common-filters-update")
        
        if (changed == "CartoonInk") or fullrebuild:
            if ("CartoonInk" in configuration):
                c = configuration["CartoonInk"]
                self.finalQuad.setShaderInput("cartoonseparation", Vec4(c.separation, 0, c.separation, 0))
                self.finalQuad.setShaderInput("cartooncolor", c.color)

        if (changed == "BlurSharpen") or fullrebuild:
            if ("BlurSharpen" in configuration):
                blurval = configuration["BlurSharpen"]
                self.finalQuad.setShaderInput("blurval", Vec4(blurval, blurval, blurval, blurval))

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

        self.update()
        return True

    def update(self, task = None):
        """Updates the shader inputs that need to be updated every frame.
        Normally, you shouldn't call this, it's being called in a task."""

        if "VolumetricLighting" in self.configuration:
            caster = self.configuration["VolumetricLighting"].caster
            casterpos = Point2()
            self.manager.camera.node().getLens().project(caster.getPos(self.manager.camera), casterpos)
            self.finalQuad.setShaderInput("casterpos", Vec4(casterpos.getX() * 0.5 + 0.5, (casterpos.getY() * 0.5 + 0.5), 0, 0))
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

