import os
import string

def _font_initialise(root, size=None, fontScheme = None):
    global _fontSize
    if size is not None:
        _fontSize = size

    if fontScheme in ('pmw1', 'pmw2'):
        if os.name == 'posix':
            defaultFont = logicalfont('Helvetica')
            menuFont = logicalfont('Helvetica', weight='bold', slant='italic')
            scaleFont = logicalfont('Helvetica', slant='italic')
            root.option_add('*Font',            defaultFont,  'userDefault')
            root.option_add('*Menu*Font',       menuFont,     'userDefault')
            root.option_add('*Menubutton*Font', menuFont,     'userDefault')
            root.option_add('*Scale.*Font',     scaleFont,    'userDefault')

            if fontScheme == 'pmw1':
                balloonFont = logicalfont('Helvetica', -6, pixel = '12')
            else: # fontScheme == 'pmw2'
                balloonFont = logicalfont('Helvetica', -2)
            root.option_add('*Balloon.*Font', balloonFont, 'userDefault')
        else:
            defaultFont = logicalfont('Helvetica')
            root.option_add('*Font', defaultFont,  'userDefault')
    elif fontScheme == 'default':
        defaultFont = ('Helvetica', '-%d' % (_fontSize,), 'bold')
        entryFont = ('Helvetica', '-%d' % (_fontSize,))
        textFont = ('Courier', '-%d' % (_fontSize,))
        root.option_add('*Font',            defaultFont,  'userDefault')
        root.option_add('*Entry*Font',      entryFont,    'userDefault')
        root.option_add('*Text*Font',       textFont,     'userDefault')

def logicalfont(name='Helvetica', sizeIncr = 0, **kw):
    if name not in _fontInfo:
        raise ValueError('font %s does not exist' % name)

    rtn = []
    for field in _fontFields:
        if field in kw:
            logicalValue = kw[field]
        elif field in _fontInfo[name]:
            logicalValue = _fontInfo[name][field]
        else:
            logicalValue = '*'

        if (field, logicalValue) in _propertyAliases[name]:
            realValue = _propertyAliases[name][(field, logicalValue)]
        elif (field, None) in _propertyAliases[name]:
            realValue = _propertyAliases[name][(field, None)]
        elif (field, logicalValue) in _propertyAliases[None]:
            realValue = _propertyAliases[None][(field, logicalValue)]
        elif (field, None) in _propertyAliases[None]:
            realValue = _propertyAliases[None][(field, None)]
        else:
            realValue = logicalValue

        if field == 'size':
            if realValue == '*':
                realValue = _fontSize
            realValue = str((realValue + sizeIncr) * 10)

        rtn.append(realValue)
    return '-'.join(rtn)
    #return str.join(rtn, '-')

def logicalfontnames():
    return list(_fontInfo.keys())

if os.name == 'nt':
    _fontSize = 16
else:
    _fontSize = 14

_fontFields = (
  'registry', 'foundry', 'family', 'weight', 'slant', 'width', 'style',
  'pixel', 'size', 'xres', 'yres', 'spacing', 'avgwidth', 'charset', 'encoding')

# <_propertyAliases> defines other names for which property values may
# be known by.  This is required because italics in adobe-helvetica
# are specified by 'o', while other fonts use 'i'.

_propertyAliases = {}

_propertyAliases[None] = {
  ('slant', 'italic') : 'i',
  ('slant', 'normal') : 'r',
  ('weight', 'light') : 'normal',
  ('width', 'wide') : 'normal',
  ('width', 'condensed') : 'normal',
}

# <_fontInfo> describes a 'logical' font, giving the default values of
# some of its properties.

_fontInfo = {}

_fontInfo['Helvetica'] = {
  'foundry' : 'adobe',
  'family' : 'helvetica',
  'registry' : '',
  'charset' : 'iso8859',
  'encoding' : '1',
  'spacing' : 'p',
  'slant' : 'normal',
  'width' : 'normal',
  'weight' : 'normal',
}

_propertyAliases['Helvetica'] = {
  ('slant', 'italic') : 'o',
  ('weight', 'normal') : 'medium',
  ('weight', 'light') : 'medium',
}

_fontInfo['Times'] = {
  'foundry' : 'adobe',
  'family' : 'times',
  'registry' : '',
  'charset' : 'iso8859',
  'encoding' : '1',
  'spacing' : 'p',
  'slant' : 'normal',
  'width' : 'normal',
  'weight' : 'normal',
}

_propertyAliases['Times'] = {
  ('weight', 'normal') : 'medium',
  ('weight', 'light') : 'medium',
}

_fontInfo['Fixed'] = {
  'foundry' : 'misc',
  'family' : 'fixed',
  'registry' : '',
  'charset' : 'iso8859',
  'encoding' : '1',
  'spacing' : 'c',
  'slant' : 'normal',
  'width' : 'normal',
  'weight' : 'normal',
}

_propertyAliases['Fixed'] = {
  ('weight', 'normal') : 'medium',
  ('weight', 'light') : 'medium',
  ('style', None) : '',
  ('width', 'condensed') : 'semicondensed',
}

_fontInfo['Courier'] = {
  'foundry' : 'adobe',
  'family' : 'courier',
  'registry' : '',
  'charset' : 'iso8859',
  'encoding' : '1',
  'spacing' : 'm',
  'slant' : 'normal',
  'width' : 'normal',
  'weight' : 'normal',
}

_propertyAliases['Courier'] = {
  ('weight', 'normal') : 'medium',
  ('weight', 'light') : 'medium',
  ('style', None) : '',
}

_fontInfo['Typewriter'] = {
  'foundry' : 'b&h',
  'family' : 'lucidatypewriter',
  'registry' : '',
  'charset' : 'iso8859',
  'encoding' : '1',
  'spacing' : 'm',
  'slant' : 'normal',
  'width' : 'normal',
  'weight' : 'normal',
}

_propertyAliases['Typewriter'] = {
  ('weight', 'normal') : 'medium',
  ('weight', 'light') : 'medium',
}

if os.name == 'nt':
    # For some reason 'fixed' fonts on NT aren't.
    _fontInfo['Fixed'] = _fontInfo['Courier']
    _propertyAliases['Fixed'] = _propertyAliases['Courier']
