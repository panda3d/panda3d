# Functions for converting colors and modifying the color scheme of
# an application.

import math
import string
import sys
import Tkinter

_PI = math.pi
_TWO_PI = _PI * 2
_THIRD_PI = _PI / 3
_SIXTH_PI = _PI / 6
_MAX_RGB = float(256 * 256 - 1) # max size of rgb values returned from Tk

def setscheme(root, background=None, **kw):
    root = root._root()
    palette = apply(_calcPalette, (root, background,), kw)
    for option, value in palette.items():
	root.option_add('*' + option, value, 'widgetDefault')

def getdefaultpalette(root):
    # Return the default values of all options, using the defaults
    # from a few widgets.

    ckbtn = Tkinter.Checkbutton(root)
    entry = Tkinter.Entry(root)
    scbar = Tkinter.Scrollbar(root)

    orig = {}
    orig['activeBackground'] = str(ckbtn.configure('activebackground')[4])
    orig['activeForeground'] = str(ckbtn.configure('activeforeground')[4])
    orig['background'] = str(ckbtn.configure('background')[4])
    orig['disabledForeground'] = str(ckbtn.configure('disabledforeground')[4])
    orig['foreground'] = str(ckbtn.configure('foreground')[4])
    orig['highlightBackground'] = str(ckbtn.configure('highlightbackground')[4])
    orig['highlightColor'] = str(ckbtn.configure('highlightcolor')[4])
    orig['insertBackground'] = str(entry.configure('insertbackground')[4])
    orig['selectColor'] = str(ckbtn.configure('selectcolor')[4])
    orig['selectBackground'] = str(entry.configure('selectbackground')[4])
    orig['selectForeground'] = str(entry.configure('selectforeground')[4])
    orig['troughColor'] = str(scbar.configure('troughcolor')[4])

    ckbtn.destroy()
    entry.destroy()
    scbar.destroy()

    return orig

#======================================================================

# Functions dealing with brightness, hue, saturation and intensity of colors.

def changebrightness(root, colorName, brightness):
    # Convert the color name into its hue and back into a color of the
    # required brightness.

    rgb = name2rgb(root, colorName)
    hue, saturation, intensity = rgb2hsi(rgb)
    if saturation == 0.0:
        hue = None
    return hue2name(hue, brightness)

def hue2name(hue, brightness = None):
    # Convert the requested hue and brightness into a color name.  If
    # hue is None, return a grey of the requested brightness.

    if hue is None:
	rgb = hsi2rgb(0.0, 0.0, brightness)
    else:
	while hue < 0:
	    hue = hue + _TWO_PI
	while hue >= _TWO_PI:
	    hue = hue - _TWO_PI

	rgb = hsi2rgb(hue, 1.0, 1.0)
	if brightness is not None:
	    b = rgb2brightness(rgb)
	    i = 1.0 - (1.0 - brightness) * b
	    s = bhi2saturation(brightness, hue, i)
	    rgb = hsi2rgb(hue, s, i)

    return rgb2name(rgb)

def bhi2saturation(brightness, hue, intensity):
    while hue < 0:
        hue = hue + _TWO_PI
    while hue >= _TWO_PI:
        hue = hue - _TWO_PI
    hue = hue / _THIRD_PI
    f = hue - math.floor(hue)

    pp = intensity
    pq = intensity * f
    pt = intensity - intensity * f
    pv = 0

    hue = int(hue)
    if   hue == 0: rgb = (pv, pt, pp)
    elif hue == 1: rgb = (pq, pv, pp)
    elif hue == 2: rgb = (pp, pv, pt)
    elif hue == 3: rgb = (pp, pq, pv)
    elif hue == 4: rgb = (pt, pp, pv)
    elif hue == 5: rgb = (pv, pp, pq)

    return (intensity - brightness) / rgb2brightness(rgb)

def hsi2rgb(hue, saturation, intensity):
    i = intensity
    if saturation == 0:
	rgb = [i, i, i]
    else:
	while hue < 0:
	    hue = hue + _TWO_PI
	while hue >= _TWO_PI:
	    hue = hue - _TWO_PI
	hue = hue / _THIRD_PI
	f = hue - math.floor(hue)
	p = i * (1.0 - saturation)
	q = i * (1.0 - saturation * f)
	t = i * (1.0 - saturation * (1.0 - f))

	hue = int(hue)
	if   hue == 0: rgb = [i, t, p]
	elif hue == 1: rgb = [q, i, p]
	elif hue == 2: rgb = [p, i, t]
	elif hue == 3: rgb = [p, q, i]
	elif hue == 4: rgb = [t, p, i]
	elif hue == 5: rgb = [i, p, q]

    for index in range(3):
	val = rgb[index]
	if val < 0.0:
	    val = 0.0
	if val > 1.0:
	    val = 1.0
	rgb[index] = val

    return rgb

def average(rgb1, rgb2, fraction):
    return (
	rgb2[0] * fraction + rgb1[0] * (1.0 - fraction),
	rgb2[1] * fraction + rgb1[1] * (1.0 - fraction),
	rgb2[2] * fraction + rgb1[2] * (1.0 - fraction)
    )

def rgb2name(rgb):
    return '#%02x%02x%02x' % \
        (int(rgb[0] * 255), int(rgb[1] * 255), int(rgb[2] * 255))

def rgb2brightness(rgb):
    # Return the perceived grey level of the color
    # (0.0 == black, 1.0 == white).

    rf = 0.299
    gf = 0.587
    bf = 0.114
    return rf * rgb[0] + gf * rgb[1] + bf * rgb[2]

def rgb2hsi(rgb):
    maxc = max(rgb[0], rgb[1], rgb[2])
    minc = min(rgb[0], rgb[1], rgb[2])

    intensity = maxc
    if maxc != 0:
      saturation  = (maxc - minc) / maxc
    else:
      saturation = 0.0

    hue = 0.0
    if saturation != 0.0:
	c = []
	for index in range(3):
	    c.append((maxc - rgb[index]) / (maxc - minc))

	if rgb[0] == maxc:
	    hue = c[2] - c[1]
	elif rgb[1] == maxc:
	    hue = 2 + c[0] - c[2]
	elif rgb[2] == maxc:
	    hue = 4 + c[1] - c[0]

	hue = hue * _THIRD_PI
	if hue < 0.0:
	    hue = hue + _TWO_PI

    return (hue, saturation, intensity)

def name2rgb(root, colorName, asInt = 0):
    if colorName[0] == '#':
	# Extract rgb information from the color name itself, assuming
	# it is either #rgb, #rrggbb, #rrrgggbbb, or #rrrrggggbbbb
	# This is useful, since tk may return incorrect rgb values if
	# the colormap is full - it will return the rbg values of the
	# closest color available.
        colorName = colorName[1:]
        digits = len(colorName) / 3
        factor = 16 ** (4 - digits)
        rgb = (
            string.atoi(colorName[0:digits], 16) * factor,
            string.atoi(colorName[digits:digits * 2], 16) * factor,
            string.atoi(colorName[digits * 2:digits * 3], 16) * factor,
        )
    else:
	# We have no choice but to ask Tk what the rgb values are.
	rgb = root.winfo_rgb(colorName)

    if not asInt:
        rgb = (rgb[0] / _MAX_RGB, rgb[1] / _MAX_RGB, rgb[2] / _MAX_RGB)
    return rgb

def _calcPalette(root, background=None, **kw):
    # Create a map that has the complete new palette.  If some colors
    # aren't specified, compute them from other colors that are specified.
    new = {}
    for key, value in kw.items():
	new[key] = value
    if background is not None:
	new['background'] = background
    if not new.has_key('background'):
	raise ValueError, 'must specify a background color'

    if not new.has_key('foreground'):
	new['foreground'] = 'black'

    bg = name2rgb(root, new['background'])
    fg = name2rgb(root, new['foreground'])

    for i in ('activeForeground', 'insertBackground', 'selectForeground',
	    'highlightColor'):
	if not new.has_key(i):
	    new[i] = new['foreground']

    if not new.has_key('disabledForeground'):
	newCol = average(bg, fg, 0.3)
	new['disabledForeground'] = rgb2name(newCol)

    if not new.has_key('highlightBackground'):
	new['highlightBackground'] = new['background']

    # Set <lighterBg> to a color that is a little lighter that the
    # normal background.  To do this, round each color component up by
    # 9% or 1/3 of the way to full white, whichever is greater.
    lighterBg = []
    for i in range(3):
	lighterBg.append(bg[i])
	inc1 = lighterBg[i] * 0.09
	inc2 = (1.0 - lighterBg[i]) / 3
	if inc1 > inc2:
	    lighterBg[i] = lighterBg[i] + inc1
	else:
	    lighterBg[i] = lighterBg[i] + inc2
	if lighterBg[i] > 1.0:
	    lighterBg[i] = 1.0

    # Set <darkerBg> to a color that is a little darker that the
    # normal background.
    darkerBg = (bg[0] * 0.9, bg[1] * 0.9, bg[2] * 0.9)

    if not new.has_key('activeBackground'):
	# If the foreground is dark, pick a light active background.
	# If the foreground is light, pick a dark active background.
	# XXX This has been disabled, since it does not look very
	# good with dark backgrounds. If this is ever fixed, the
	# selectBackground and troughColor options should also be fixed.

	if rgb2brightness(fg) < 0.5:
	    new['activeBackground'] = rgb2name(lighterBg)
	else:
	    new['activeBackground'] = rgb2name(lighterBg)

    if not new.has_key('selectBackground'):
	new['selectBackground'] = rgb2name(darkerBg)
    if not new.has_key('troughColor'):
	new['troughColor'] = rgb2name(darkerBg)
    if not new.has_key('selectColor'):
	new['selectColor'] = 'yellow'

    return new

def spectrum(numColors, correction = 1.0, saturation = 1.0, intensity = 1.0,
	extraOrange = 1, returnHues = 0):
    colorList = []
    division = numColors / 7.0
    for index in range(numColors):
	if extraOrange:
	    if index < 2 * division:
		hue = index / division
	    else:
		hue = 2 + 2 * (index - 2 * division) / division
	    hue = hue * _SIXTH_PI
	else:
	    hue = index * _TWO_PI / numColors
	if returnHues:
	    colorList.append(hue)
	else:
	    rgb = hsi2rgb(hue, saturation, intensity)
	    if correction != 1.0:
		rgb = correct(rgb, correction)
	    name = rgb2name(rgb)
	    colorList.append(name)
    return colorList

def correct(rgb, correction):
    correction = float(correction)
    rtn = []
    for index in range(3):
	rtn.append((1 - (1 - rgb[index]) ** correction) ** (1 / correction))
    return rtn

#==============================================================================

def _recolorTree(widget, oldpalette, newcolors):
    # Change the colors in a widget and its descendants.

    # Change the colors in <widget> and all of its descendants,
    # according to the <newcolors> dictionary.  It only modifies
    # colors that have their default values as specified by the
    # <oldpalette> variable.  The keys of the <newcolors> dictionary
    # are named after widget configuration options and the values are
    # the new value for that option.

    for dbOption in newcolors.keys():
        option = string.lower(dbOption)
        try:
            value = str(widget.cget(option))
        except:
            continue
        if oldpalette is None or value == oldpalette[dbOption]:
            apply(widget.configure, (), {option : newcolors[dbOption]})

    for child in widget.winfo_children():
       _recolorTree(child, oldpalette, newcolors)

def changecolor(widget, background=None, **kw):
     root = widget._root()
     if not hasattr(widget, '_Pmw_oldpalette'):
	 widget._Pmw_oldpalette = getdefaultpalette(root)
     newpalette = apply(_calcPalette, (root, background,), kw)
     _recolorTree(widget, widget._Pmw_oldpalette, newpalette)
     widget._Pmw_oldpalette = newpalette

def bordercolors(root, colorName):
    # This is the same method that Tk uses for shadows, in TkpGetShadows.

    lightRGB = []
    darkRGB = []
    for value in name2rgb(root, colorName, 1):
        value40pc = (14 * value) / 10
        if value40pc > _MAX_RGB:
            value40pc = _MAX_RGB
        valueHalfWhite = (_MAX_RGB + value) / 2;
        lightRGB.append(max(value40pc, valueHalfWhite))

        darkValue = (60 * value) / 100
        darkRGB.append(darkValue)

    return (
        '#%04x%04x%04x' % (lightRGB[0], lightRGB[1], lightRGB[2]),
        '#%04x%04x%04x' % (darkRGB[0], darkRGB[1], darkRGB[2])
    )
