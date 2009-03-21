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
o_color = lerp(o_color, float4(0,0,0,1), cartoon_thresh);
"""

class FilterConfig:
    pass

class CommonFilters:

    """ Class CommonFilters implements certain common image postprocessing
    filters.  The constructor requires a filter builder as a parameter. """

    def __init__(self, win, cam):
        self.manager = FilterManager(win, cam)
        self.configuration = {}
        self.cleanup()
        self.task = taskMgr.add(self.update, "common-filters-update")

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
            if (configuration.has_key("CartoonInk")):
                needtex["aux"] = True
                auxbits |= AuxBitplaneAttrib.ABOAuxNormal
            if (configuration.has_key("Bloom")):
                needtex["bloom0"] = True
                needtex["bloom1"] = True
                needtex["bloom2"] = True
                needtex["bloom3"] = True
                auxbits |= AuxBitplaneAttrib.ABOGlow
            if (configuration.has_key("ViewGlow")):
                auxbits |= AuxBitplaneAttrib.ABOGlow
            for tex in needtex:
                self.textures[tex] = Texture("scene-"+tex)
                self.textures[tex].setWrapU(Texture.WMClamp)
                self.textures[tex].setWrapV(Texture.WMClamp)
                needtexpix = True

            self.finalQuad = self.manager.renderSceneInto(textures = self.textures, auxbits=auxbits)
            if (self.finalQuad == None):
                self.cleanup()
                return False

            if (configuration.has_key("Bloom")):
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
            if (configuration.has_key("CartoonInk")):
                text += " uniform float4 texpad_txaux,\n"
                text += " uniform float4 texpix_txaux,\n"
                text += " out float4 l_texcoordN : TEXCOORD1,\n"
            if (configuration.has_key("Bloom")):
                text += " uniform float4 texpad_txbloom3,\n"
                text += " out float4 l_texcoordB : TEXCOORD2,\n"
            text += " uniform float4x4 mat_modelproj)\n"
            text += "{\n"
            text += " l_position=mul(mat_modelproj, vtx_position);\n"
            text += " l_texcoordC=(vtx_position.xzxz * texpad_txcolor) + texpad_txcolor;\n"
            if (configuration.has_key("CartoonInk")):
                text += " l_texcoordN=(vtx_position.xzxz * texpad_txaux) + texpad_txaux;\n"
            if (configuration.has_key("Bloom")):
                text += " l_texcoordB=(vtx_position.xzxz * texpad_txbloom3) + texpad_txbloom3;\n"
            if (configuration.has_key("HalfPixelShift")):
                text += " l_texcoordC+=texpix_txcolor*0.5;\n"
                if (configuration.has_key("CartoonInk")):
                    text += " l_texcoordN+=texpix_txaux*0.5;\n"
            text += "}\n"

            text += "void fshader(\n"
            text += "float4 l_texcoordC : TEXCOORD0,\n"
            text += "uniform float4 texpix_txcolor,\n"
            if (configuration.has_key("CartoonInk")):
                text += "float4 l_texcoordN : TEXCOORD1,\n"
                text += "uniform float4 texpix_txaux,\n"
            if (configuration.has_key("Bloom")):
                text += "float4 l_texcoordB : TEXCOORD2,\n"
            for key in self.textures:
                text += "uniform sampler2D k_tx" + key + ",\n"
            if (configuration.has_key("CartoonInk")):
                text += "uniform float4 k_cartoonseparation,\n"
            if (configuration.has_key("VolumetricLighting")):
                text += "uniform float4 k_casterpos,\n"
                text += "uniform float4 k_vlparams,\n"
            text += "out float4 o_color : COLOR)\n"
            text += "{\n"
            text += " o_color = tex2D(k_txcolor, l_texcoordC.xy);\n"
            if (configuration.has_key("CartoonInk")):
                text += CARTOON_BODY
            if (configuration.has_key("Bloom")):
                text += "o_color = saturate(o_color);\n";
                text += "float4 bloom = 0.5*tex2D(k_txbloom3, l_texcoordB.xy);\n"
                text += "o_color = 1-((1-bloom)*(1-o_color));\n"
            if (configuration.has_key("ViewGlow")):
                text += "o_color.r = o_color.a;\n"
            if (configuration.has_key("VolumetricLighting")):
                text += "float decay = 1.0f;\n"
                text += "float2 curcoord = l_texcoordC.xy;\n"
                text += "float2 lightdir = curcoord - k_casterpos.xy;\n"
                text += "lightdir *= k_vlparams.y;\n"
                text += "half4 sample = tex2D(k_txcolor, curcoord);\n"
                text += "float3 vlcolor = sample.rgb * sample.a;\n"
                text += "for (int i = 0; i < k_vlparams.x; i++) {\n"
                text += "  curcoord -= lightdir;\n"
                text += "  sample = tex2D(k_txcolor, curcoord);\n"
                text += "  sample *= sample.a * decay;//*weight\n"
                text += "  vlcolor += sample.rgb;\n"
                text += "  decay *= k_vlparams.z;\n"
                text += "}\n"
                text += "o_color += float4(vlcolor * k_vlparams.w, 1);\n"
            if (configuration.has_key("Inverted")):
                text += "o_color = float4(1, 1, 1, 1) - o_color;\n"
            text += "}\n"
            
            self.finalQuad.setShader(Shader.make(text))
            for tex in self.textures:
                self.finalQuad.setShaderInput("tx"+tex, self.textures[tex])
        
        if (changed == "CartoonInk") or fullrebuild:
            if (configuration.has_key("CartoonInk")):
                separation = configuration["CartoonInk"]
                self.finalQuad.setShaderInput("cartoonseparation", Vec4(separation,0,separation,0))
        
        if (changed == "Bloom") or fullrebuild:
            if (configuration.has_key("Bloom")):
                bloomconf = configuration["Bloom"]
                intensity = bloomconf.intensity * 3.0
                self.bloom[0].setShaderInput("blend", bloomconf.blendx, bloomconf.blendy, bloomconf.blendz, bloomconf.blendw * 2.0)
                self.bloom[0].setShaderInput("trigger", bloomconf.mintrigger, 1.0/(bloomconf.maxtrigger-bloomconf.mintrigger), 0.0, 0.0)
                self.bloom[0].setShaderInput("desat", bloomconf.desat)
                self.bloom[3].setShaderInput("intensity", intensity, intensity, intensity, intensity)
        
        if (changed == "VolumetricLighting") or fullrebuild:
            if (configuration.has_key("VolumetricLighting")):
                config = configuration["VolumetricLighting"]
                tcparam = config.density / float(config.numsamples)
                self.finalQuad.setShaderInput("vlparams", config.numsamples, tcparam, config.decay, config.exposure)
        
        self.update()
        return True

    def update(self, task = None):
        """Updates the shader inputs that need to be updated every frame.
        Normally, you shouldn't call this, it's being called in a task."""
        if self.configuration.has_key("VolumetricLighting"):
            caster = self.configuration["VolumetricLighting"].caster
            casterpos = Point2()
            self.manager.camera.node().getLens().project(caster.getPos(self.manager.camera), casterpos)
            self.finalQuad.setShaderInput("casterpos", Vec4(casterpos.getX() * 0.5 + 0.5, (casterpos.getY() * 0.5 + 0.5), 0, 0))
        if task != None:
            return task.cont

    def setCartoonInk(self, separation=1):
        fullrebuild = (self.configuration.has_key("CartoonInk") == False)
        self.configuration["CartoonInk"] = separation
        return self.reconfigure(fullrebuild, "CartoonInk")

    def delCartoonInk(self):
        if (self.configuration.has_key("CartoonInk")):
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
        if (self.configuration.has_key("Bloom")):
            del self.configuration["Bloom"]
            return self.reconfigure(True, "Bloom")
        return True

    def setHalfPixelShift(self):
        fullrebuild = (self.configuration.has_key("HalfPixelShift") == False)
        self.configuration["HalfPixelShift"] = 1
        return self.reconfigure(fullrebuild, "HalfPixelShift")

    def delHalfPixelShift(self):
        if (self.configuration.has_key("HalfPixelShift")):
            del self.configuration["HalfPixelShift"]
            return self.reconfigure(True, "HalfPixelShift")
        return True

    def setViewGlow(self):
        fullrebuild = (self.configuration.has_key("ViewGlow") == False)
        self.configuration["ViewGlow"] = 1
        return self.reconfigure(fullrebuild, "ViewGlow")

    def delViewGlow(self):
        if (self.configuration.has_key("ViewGlow")):
            del self.configuration["ViewGlow"]
            return self.reconfigure(True, "ViewGlow")
        return True

    def setInverted(self):
        fullrebuild = (self.configuration.has_key("Inverted") == False)
        self.configuration["Inverted"] = 1
        return self.reconfigure(fullrebuild, "Inverted")

    def delInverted(self):
        if (self.configuration.has_key("Inverted")):
            del self.configuration["Inverted"]
            return self.reconfigure(True, "Inverted")
        return True

    def setVolumetricLighting(self, caster, numsamples = 32, density = 5.0, decay = 0.1, exposure = 0.1):
        fullrebuild = (self.configuration.has_key("VolumetricLighting") == False)
        newconfig = FilterConfig()
        newconfig.caster = caster
        newconfig.numsamples = numsamples
        newconfig.density = density
        newconfig.decay = decay
        newconfig.exposure = exposure
        self.configuration["VolumetricLighting"] = newconfig
        return self.reconfigure(fullrebuild, "VolumetricLighting")

    def delVolumetricLighting(self):
        if (self.configuration.has_key("VolumetricLighting")):
            del self.configuration["VolumetricLighting"]
            return self.reconfigure(True, "VolumetricLighting")
        return True

