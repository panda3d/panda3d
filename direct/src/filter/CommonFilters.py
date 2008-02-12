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
from pandac.PandaModules import Point3, Vec3, Vec4
from pandac.PandaModules import NodePath, PandaNode
from pandac.PandaModules import RenderState, Texture, Shader

HEADER = """//Cg

void vshader(float4 vtx_position : POSITION,
             out float4 l_position : POSITION,
             out float4 l_texcoord : TEXCOORD0,
             uniform float4 texpad_txcolor,
             uniform float4x4 mat_modelproj)
{
  l_position=mul(mat_modelproj, vtx_position);
  l_texcoord=(vtx_position.xzxz * texpad_txcolor) + texpad_txcolor;
}
"""

CARTOON_BODY="""
float4 cartoondelta = k_cartoonseparation * texpix_txcolor.xwyw;
float4 cartoon_p0 = l_texcoord + cartoondelta.xyzw;
float4 cartoon_c0 = tex2D(k_txnormal, cartoon_p0.xy);
float4 cartoon_p1 = l_texcoord - cartoondelta.xyzw;
float4 cartoon_c1 = tex2D(k_txnormal, cartoon_p1.xy);
float4 cartoon_p2 = l_texcoord + cartoondelta.wzyx;
float4 cartoon_c2 = tex2D(k_txnormal, cartoon_p2.xy);
float4 cartoon_p3 = l_texcoord - cartoondelta.wzyx;
float4 cartoon_c3 = tex2D(k_txnormal, cartoon_p3.xy);
float4 cartoon_mx = max(cartoon_c0,max(cartoon_c1,max(cartoon_c2,cartoon_c3)));
float4 cartoon_mn = min(cartoon_c0,min(cartoon_c1,min(cartoon_c2,cartoon_c3)));
float4 cartoon_trigger = saturate(((cartoon_mx-cartoon_mn) * 3) - k_cartooncutoff.x);
float  cartoon_thresh = dot(cartoon_trigger.xyz,float3(1,1,1));
o_color = lerp(o_color, float4(0,0,0,1), cartoon_thresh);
"""

CARTOON_PARAMS="""
uniform float4 k_cartoonseparation,
uniform float4 k_cartooncutoff,
"""
class CommonFilters:

    """ Class CommonFilters implements certain common image postprocessing
    filters.  The constructor requires a filter builder as a parameter. """

    def __init__(self, win, cam):
        self.manager = FilterManager(win, cam)
        self.configuration = {}
        self.cleanup()

    def cleanup(self):
        self.manager.cleanup()
        self.textures = {}
        self.finalQuad = None

    def reconfigure(self, fullrebuild, changed):

        """ Reconfigure is called whenever any configuration change is made. """

        configuration = self.configuration

        if (fullrebuild):

            self.cleanup()

            if (len(configuration) == 0):
                return

            needtexpix = False
            needtex = {}
            needtex["color"] = True
            if (configuration.has_key("CartoonInk")):
                needtex["normal"] = True
            for tex in needtex:
                self.textures[tex] = Texture("scene-"+tex)
                needtexpix = True

            self.finalQuad = self.manager.renderSceneInto(textures = self.textures)
    
            text = HEADER
            text += "void fshader(\n"
            text += "float4 l_texcoord : TEXCOORD0,\n"
            if (needtexpix):
                text += "uniform float4 texpix_txcolor,\n"
            for key in self.textures:
                text += "uniform sampler2D k_tx" + key + ",\n"
            if (configuration.has_key("CartoonInk")):
                text += CARTOON_PARAMS
            text += "out float4 o_color : COLOR)\n"
            text += "{\n"
            text += " o_color = tex2D(k_txcolor, l_texcoord.xy);\n"
            if (configuration.has_key("CartoonInk")):
                text += CARTOON_BODY
            text += "}\n"
    
            print "Using shader: ", text
            self.finalQuad.setShader(Shader.make(text))
            for tex in self.textures:
                self.finalQuad.setShaderInput("tx"+tex, self.textures[tex])

        if (changed == "CartoonInk") or fullrebuild:
            if (configuration.has_key("CartoonInk")):
                (separation, cutoff) = configuration["CartoonInk"]
                self.finalQuad.setShaderInput("cartoonseparation", Vec4(separation,0,separation,0))
                self.finalQuad.setShaderInput("cartooncutoff", Vec4(cutoff,cutoff,cutoff,cutoff))

    def setCartoonInk(self, separation=1, cutoff=0.3):
        fullrebuild = (self.configuration.has_key("CartoonInk") == False)
        self.configuration["CartoonInk"] = (separation, cutoff)
        self.reconfigure(fullrebuild, "CartoonInk")

    def delCartoonInk(self):
        if (self.configuration.has_key("CartoonInk")):
            del self.configuration["CartoonInk"]
            self.reconfigure(True)


