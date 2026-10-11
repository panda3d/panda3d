""" This simple Python script can be run to generate
store_pixel_code.h and store_pixel_table.h, which are a poor man's
form of generated code to cover the explosion of different blending
options when storing a pixel into the framebuffer.

Each different combination of options is compiled to a different
inner-loop store function.  The code in tinyGraphicsStateGuardian.cxx
will select the appropriate function pointer at draw time. """

Modes = [
    'add',
    'min',
    'max',
]

# We handle alpha write as a special "off" mode, reducing redundancy.
AlphaModes = Modes + ['off']

CodeTable = {
    'add': 'STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))',
    'min': 'std::min((unsigned int)c, (unsigned int)fc)',
    'max': 'std::max((unsigned int)c, (unsigned int)fc)',
    'off': '(unsigned int)fc',
}

bitnames = 'rgba'

def get_fname(rgb_mode, alpha_mode, mask):
    maskname = ''
    for b in range(3):
        if (mask & (1 << b)):
            maskname += bitnames[b]
        else:
            maskname += '0'
    return 'store_pixel_%s_%s_%s' % (rgb_mode, alpha_mode, maskname)

# We write the code that actually instantiates the various
# pixel-storing functions to store_pixel_code.h.
code = open('store_pixel_code.h', 'w')
print('/* This file is generated code--do not edit.  See store_pixel.py. */', file=code)
print('', file=code)

# The external reference for the table containing the above function
# pointers gets written here.
table = open('store_pixel_table.h', 'w')
print('/* This file is generated code--do not edit.  See store_pixel.py. */', file=table)
print('', file=table)

for rgb_mode in Modes:
    for alpha_mode in AlphaModes:
        for mask in range(0, 8):
            fname = get_fname(rgb_mode, alpha_mode, mask)
            print('#define FNAME(name) %s' % (fname), file=code)
            if mask != 0:
                print('#define FNAME_S(name) %s_s' % (fname), file=code)

            print('#define MODE_RGB(c, c_opa, fc, c_opb) %s' % (CodeTable[rgb_mode]), file=code)
            print('#define MODE_ALPHA(c, c_opa, fc, c_opb) %s' % (CodeTable[alpha_mode]), file=code)
            for b in range(0, 3):
                if (mask & (1 << b)):
                    print("#define HAVE_%s 1" % (bitnames[b].upper()), file=code)
            if alpha_mode != 'off':
                print("#define HAVE_A 1", file=code)
            print('#include "store_pixel.h"', file=code)
            print('', file=code)


# Now, generate the table of function pointers.
arraySize = '[%s][%s][8][2]' % (len(Modes), len(AlphaModes))

print('extern const ZB_storePixelFunc store_pixel_funcs%s;' % (arraySize), file=table)
print('const ZB_storePixelFunc store_pixel_funcs%s = {' % (arraySize), file=code)

for rgb_mode in Modes:
    print('  {', file=code)
    for alpha_mode in AlphaModes:
        print('    {', file=code)
        for mask in range(0, 8):
            fname = get_fname(rgb_mode, alpha_mode, mask)
            if mask != 0:
                fname_s = fname + '_s'
            else:
                fname_s = fname
            print('      {%s, %s},' % (fname, fname_s), file=code)
        print('    },', file=code)
    print('  },', file=code)
print('};', file=code)

print('', file=code)
