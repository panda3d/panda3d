""" This simple Python script can be run to generate
store_pixel_code.h and store_pixel_table.h, which are a poor man's
form of generated code to cover the explosion of different blending
options when storing a pixel into the framebuffer.

Each different combination of options is compiled to a different
inner-loop store function.  The code in tinyGraphicsStateGuardian.cxx
will select the appropriate function pointer at draw time. """

Operands = [
    'zero', 'one',
    'icolor', 'micolor',
    'fcolor', 'mfcolor',
    'ialpha', 'mialpha',
    'falpha', 'mfalpha',
    'ccolor', 'mccolor',
    'calpha', 'mcalpha',
]

CodeTable = {
    'zero' : '0',
    'one' : '0x10000',
    'icolor' : 'i',
    'micolor' : '0xffff - i',
    'fcolor' : 'f',
    'mfcolor' : '0xffff - f',
    'ialpha' : 'a',
    'mialpha' : '0xffff - a',
    'falpha' : 'fa',
    'mfalpha' : '0xffff - fa',
    'ccolor' : 'zb->blend_ ## i',
    'mccolor' : '0xffff - zb->blend_ ## i',
    'calpha' : 'zb->blend_a',
    'mcalpha' : '0xffff - zb->blend_a',
}    


bitnames = 'rgba'

def getFname(op_a, op_b, mask):
    maskname = ''
    for b in range(4):
        if (mask & (1 << b)):
            maskname += bitnames[b]
        else:
            maskname += '0'
    return 'store_pixel_%s_%s_%s' % (op_a, op_b, maskname)

# We write the code that actually instantiates the various
# pixel-storing functions to store_pixel_code.h.
code = open('store_pixel_code.h', 'wb')
print >> code, '/* This file is generated code--do not edit.  See store_pixel.py. */'
print >> code, ''

# The external reference for the table containing the above function
# pointers gets written here.
table = open('store_pixel_table.h', 'wb')
print >> table, '/* This file is generated code--do not edit.  See store_pixel.py. */'
print >> table, ''

for op_a in Operands:
    for op_b in Operands:
        for mask in range(0, 16):
            fname = getFname(op_a, op_b, mask)
            print >> code, '#define FNAME(name) %s' % (fname)
            if mask & (1 | 2 | 3):
                print >> code, '#define FNAME_S(name) %s_s' % (fname)

            print >> code, '#define OP_A(f, i) ((unsigned int)(%s))' % (CodeTable[op_a])
            print >> code, '#define OP_B(f, i) ((unsigned int)(%s))' % (CodeTable[op_b])
            for b in range(0, 4):
                if (mask & (1 << b)):
                    print >> code, "#define STORE_PIXEL_%s(fr, r) STORE_PIX_CLAMP(r)" % (b)
                else:
                    print >> code, "#define STORE_PIXEL_%s(fr, r) (fr)" % (b)
            print >> code, '#include "store_pixel.h"'
            print >> code, ''
            

# Now, generate the table of function pointers.
arraySize = '[%s][%s][16]' % (len(Operands), len(Operands))

print >> table, 'extern const ZB_storePixelFunc store_pixel_funcs%s;' % (arraySize)
print >> code, 'const ZB_storePixelFunc store_pixel_funcs%s = {' % (arraySize)

for op_a in Operands:
    print >> code, '  {'
    for op_b in Operands:
        print >> code, '    {'
        for mask in range(0, 16):
            fname = getFname(op_a, op_b, mask)
            print >> code, '      %s,' % (fname)
        print >> code, '    },'
    print >> code, '  },'
print >> code, '};'

print >> code

# Now do this again, but for the sRGB function pointers.
print >> table, 'extern const ZB_storePixelFunc store_pixel_funcs_sRGB%s;' % (arraySize)
print >> code, 'const ZB_storePixelFunc store_pixel_funcs_sRGB%s = {' % (arraySize)

for op_a in Operands:
    print >> code, '  {'
    for op_b in Operands:
        print >> code, '    {'
        for mask in range(0, 16):
            fname = getFname(op_a, op_b, mask)
            if mask & (1 | 2 | 3):
                print >> code, '      %s_s,' % (fname)
            else:
                print >> code, '      %s,' % (fname)
        print >> code, '    },'
    print >> code, '  },'
print >> code, '};'
