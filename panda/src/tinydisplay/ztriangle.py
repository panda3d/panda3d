#!/usr/bin/env python
""" This simple Python script can be run to generate
ztriangle_code_*.h, ztriangle_table.*, and ztriangle_*.cxx, which
are a poor man's form of generated code to cover the explosion of
different rendering options while scanning out triangles.

Each different combination of options is compiled to a different
inner-loop triangle scan function.  The code in
tinyGraphicsStateGuardian.cxx will select the appropriate function
pointer at draw time. """

# This is the number of generated ztriangle_code_*.h and
# ztriangle_*.cxx files we will produce.  You may change this freely;
# you should also change the Sources.pp file accordingly.
NumSegments = 4

# We generate an #include "ztriangle_two.h" for each combination of
# these options.
Options = [
    # depth write
    [ 'zon', 'zoff' ],

    # color write
    [ 'cstore', 'cblend', 'cgeneral', 'coff', 'csstore', 'csblend' ],

    # alpha test
    [ 'anone', 'aless', 'amore' ],

    # depth test
    [ 'znone', 'zless' ],

    # texture filters
    [ 'tnearest', 'tmipmap', 'tgeneral' ],
    ]

# The total number of different combinations of the various Options, above.
OptionsCount = reduce(lambda a, b: a * b, map(lambda o: len(o), Options))

# The various combinations of these options are explicit within
# ztriangle_two.h.
ExtraOptions = [
    # shade model
    [ 'white', 'flat', 'smooth' ],

    # texturing
    [ 'untextured', 'textured', 'perspective', 'multitex2', 'multitex3' ],
    ]

# The expansion of all ExtraOptions combinations into a linear list.
ExtraOptionsMat = []
for i in range(len(ExtraOptions[0])):
    for j in range(len(ExtraOptions[1])):
        ExtraOptionsMat.append([i, j])

FullOptions = Options + ExtraOptions

CodeTable = {
    # depth write
    'zon' : '#define STORE_Z(zpix, z) (zpix) = (z)',
    'zoff' : '#define STORE_Z(zpix, z)',

    # color write
    'cstore' : '#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)',
    'cblend' : '#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)',
    'cgeneral' : '#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)',
    'coff' : '#define STORE_PIX(pix, rgb, r, g, b, a)',

    # color write, sRGB
    'csstore' : '#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)',
    'csblend' : '#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)',

    # alpha test
    'anone' : '#define ACMP(zb, a) 1',
    'aless' : '#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)',
    'amore' : '#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)',

    # depth test
    'znone' : '#define ZCMP(zpix, z) 1',
    'zless' : '#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))',

    # texture filters
    'tnearest' : '#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)\n#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)',
    'tmipmap' : '#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)\n#define INTERP_MIPMAP\n#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)',
    'tgeneral' : '#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)\n#define INTERP_MIPMAP\n#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))',
}

ZTriangleStub = """
/* This file is generated code--do not edit.  See ztriangle.py. */
#include <stdlib.h>
#include <stdio.h>
#include "pandabase.h"
#include "zbuffer.h"

/* Pick up all of the generated code references to ztriangle_two.h,
   which ultimately calls ztriangle.h, many, many times. */

#include "ztriangle_table.h"
#include "ztriangle_code_%s.h"
"""
ops = [0] * len(Options)

class DoneException:
    pass

# We write the code that actually instantiates the various
# triangle-filling functions to ztriangle_code_*.h.
code = None
codeSeg = None
fnameDict = {}
fnameList = None

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

    if keywordList[-1].startswith('multitex'):
        # We don't bother with white_multitex or flat_multitex.
        keywordList[-2] = 'smooth'

    fname = 'FB_triangle_%s' % ('_'.join(keywordList))
    return fname

def getFref(ops):
    # Returns a string that evaluates to a pointer reference to the
    # indicated function.
    fname = getFname(ops)
    codeSeg, i = fnameDict[fname]
    fref = 'ztriangle_code_%s[%s]' % (codeSeg, i)
    return fref

def closeCode():
    """ Close the previously-opened code file. """
    if code:
        print >> code, ''
        print >> code, 'ZB_fillTriangleFunc ztriangle_code_%s[%s] = {' % (codeSeg, len(fnameList))
        for fname in fnameList:
            print >> code, '  %s,' % (fname)
        print >> code, '};'
        code.close()

    
def openCode(count):
    """ Open the code file appropriate to the current segment.  We
    write out the generated code into a series of smaller files,
    instead of one mammoth file, just to make it easier on the
    compiler. """
    
    global code, codeSeg, fnameList

    seg = int(NumSegments * count / OptionsCount) + 1

    if codeSeg != seg:
        closeCode()

        codeSeg = seg
        fnameList = []
        
        # Open a new file.
        code = open('ztriangle_code_%s.h' % (codeSeg), 'wb')
        print >> code, '/* This file is generated code--do not edit.  See ztriangle.py. */'
        print >> code, ''

        # Also generate ztriangle_*.cxx, to include the above file.
        zt = open('ztriangle_%s.cxx' % (codeSeg), 'wb')
        print >> zt, ZTriangleStub % (codeSeg)

# First, generate the code.
count = 0
try:
    while True:
        openCode(count)

        for i in range(len(ops)):
            keyword = Options[i][ops[i]]
            print >> code, CodeTable[keyword]

        # This reference gets just the initial fname: omitting the
        # ExtraOptions, which are implicit in ztriangle_two.h.
        fname = getFname(ops)
        print >> code, '#define FNAME(name) %s_ ## name' % (fname)
        print >> code, '#include "ztriangle_two.h"'
        print >> code, ''

        # We store the full fnames generated by the above lines
        # (including the ExtraOptions) in the fnameDict and fnameList
        # tables.
        for eops in ExtraOptionsMat:
            fops = ops + eops
            fname = getFname(fops)
            fnameDict[fname] = (codeSeg, len(fnameList))
            fnameList.append(fname)

        count += 1
        incrementOptions(ops)
        assert count < OptionsCount
except DoneException:
    pass

assert count == OptionsCount
closeCode()

# Now, generate the table of function pointers.

# The external reference for the table containing the above function
# pointers gets written here.
table_decl = open('ztriangle_table.h', 'wb')
print >> table_decl, '/* This file is generated code--do not edit.  See ztriangle.py. */'
print >> table_decl, ''

# The actual table definition gets written here.
table_def = open('ztriangle_table.cxx', 'wb')
print >> table_def, '/* This file is generated code--do not edit.  See ztriangle.py. */'
print >> table_def, ''
print >> table_def, '#include "pandabase.h"'
print >> table_def, '#include "zbuffer.h"'
print >> table_def, '#include "ztriangle_table.h"'
print >> table_def, ''

for i in range(NumSegments):
    print >> table_def, 'extern ZB_fillTriangleFunc ztriangle_code_%s[];' % (i + 1)
print >> table_def, ''

def writeTableEntry(ops):
    indent = '  ' * (len(ops) + 1)
    i = len(ops)
    numOps = len(FullOptions[i])
    
    if i + 1 == len(FullOptions):
        # The last level: write out the actual function names.
        for j in range(numOps - 1):
            print >> table_def, indent + getFref(ops + [j]) + ','
        print >> table_def, indent + getFref(ops + [numOps - 1])

    else:
        # Intermediate levels: write out a nested reference.
        for j in range(numOps - 1):
            print >> table_def, indent + '{'
            writeTableEntry(ops + [j])
            print >> table_def, indent + '},'
        print >> table_def, indent + '{'
        writeTableEntry(ops + [numOps - 1])
        print >> table_def, indent + '}'

arraySizeList = []
for opList in FullOptions:
    arraySizeList.append('[%s]' % (len(opList)))
arraySize = ''.join(arraySizeList)

print >> table_def, 'const ZB_fillTriangleFunc fill_tri_funcs%s = {' % (arraySize)
print >> table_decl, 'extern const ZB_fillTriangleFunc fill_tri_funcs%s;' % (arraySize)

writeTableEntry([])
print >> table_def, '};'

