""" This simple Python script can be run to generate ztriangle_code.h
and ztriangle_table.h, which are a poor man's form of generated code
to cover the explosion of different rendering options while scanning
out triangles.

Each different combination of options is compiled to a different
inner-loop triangle scan function.  The code in
tinyGraphicsStateGuardian.cxx will select the appropriate function
pointer at draw time. """

# We generate an #include "ztriangle_two.h" for each combination of
# these options.
Options = [
    # depth write
    [ 'zon', 'zoff' ],

    # color write
    [ 'noblend', 'blend', 'nocolor' ],

    # alpha test
    [ 'anone', 'aless', 'amore' ],

    # depth test
    [ 'znone', 'zless' ],

    # texture filters
    [ 'nearest', 'mipmap' ],
    ]

# The various combinations of these options are explicit within
# ztriangle_two.h.
ExtraOptions = [
    # shading
    [ 'white', 'flat', 'smooth' ],

    # texturing
    [ 'untextured', 'textured', 'perspective' ],
    ]

FullOptions = Options + ExtraOptions

CodeTable = {
    # depth write
    'zon' : '#define STORE_Z(zpix, z) (zpix) = (z)',
    'zoff' : '#define STORE_Z(zpix, z)',

    # color write
    'noblend' : '#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)',
    'blend' : '#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)',
    'nocolor' : '#define STORE_PIX(pix, rgb, r, g, b, a)',

    # alpha test
    'anone' : '#define ACMP(zb, a) 1',
    'aless' : '#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)',
    'amore' : '#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)',

    # depth test
    'znone' : '#define ZCMP(zpix, z) 1',
    'zless' : '#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))',

    # texture filters
    'nearest' : '#define CALC_MIPMAP_LEVEL\n#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)',
    'mipmap' : '#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL\n#define INTERP_MIPMAP\n#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)',
}

ops = [0] * len(Options)

class DoneException:
    pass

def incrementOptions(ops, i = -1):
    if i < -len(ops):
        raise DoneException

    # Increment the least-significant place if we can.
    if ops[i] + 1 < len(Options[i]):
        ops[i] += 1
        return

    # Recurse for the next-most-significant place.
    ops[i] = 0
    incrementOptions(ops, i - 1)

def getFname(ops):
    # Returns the function name corresponding to the indicated ops
    # vector.
    keywordList = []
    for i in range(len(ops)):
        keyword = FullOptions[i][ops[i]]
        keywordList.append(keyword)

    fname = 'FB_triangle_%s' % ('_'.join(keywordList))
    return fname

# We write the code that actually instantiates the various
# triangle-filling functions to ztriangle_code.h.
code = open('ztriangle_code.h', 'wb')
print >> code, '/* This file is generated code--do not edit.  See ztriangle.py. */'
print >> code, ''

# The external reference for the table containing the above function
# pointers gets written here.
table = open('ztriangle_table.h', 'wb')
print >> table, '/* This file is generated code--do not edit.  See ztriangle.py. */'
print >> table, ''

# First, generate the code.
try:
    while True:
        for i in range(len(ops)):
            keyword = Options[i][ops[i]]
            print >> code, CodeTable[keyword]
        
        fname = getFname(ops)
        print >> code, '#define FNAME(name) %s_ ## name' % (fname)
        print >> code, '#include "ztriangle_two.h"'
        print >> code, ''
            
        incrementOptions(ops)
        
except DoneException:
    pass


# Now, generate the table of function pointers.
arraySizeList = []
for opList in FullOptions:
    arraySizeList.append('[%s]' % (len(opList)))
arraySize = ''.join(arraySizeList)

print >> code, 'const ZB_fillTriangleFunc fill_tri_funcs%s = {' % (arraySize)
print >> table, 'extern const ZB_fillTriangleFunc fill_tri_funcs%s;' % (arraySize)

def writeTableEntry(ops):
    indent = '  ' * (len(ops) + 1)
    i = len(ops)
    numOps = len(FullOptions[i])
    
    if i + 1 == len(FullOptions):
        # The last level: write out the actual function names.
        for j in range(numOps - 1):
            print >> code, indent + getFname(ops + [j]) + ','
        print >> code, indent + getFname(ops + [numOps - 1])

    else:
        # Intermediate levels: write out a nested reference.
        for j in range(numOps - 1):
            print >> code, indent + '{'
            writeTableEntry(ops + [j])
            print >> code, indent + '},'
        print >> code, indent + '{'
        writeTableEntry(ops + [numOps - 1])
        print >> code, indent + '}'

writeTableEntry([])
print >> code, '};'


        
        
