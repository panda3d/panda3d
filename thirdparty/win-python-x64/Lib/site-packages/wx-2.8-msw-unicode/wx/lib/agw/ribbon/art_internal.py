"""
This module contains methods used throughout the L{RibbonBar} library.
"""

import wx
import math


def RibbonInterpolateColour(start_colour, end_colour, position, start_position, end_position):

    if position <= start_position:    
        return start_colour
    
    if position >= end_position:    
        return end_colour
    
    position -= start_position
    end_position -= start_position
    r = end_colour.Red() - start_colour.Red()
    g = end_colour.Green() - start_colour.Green()
    b = end_colour.Blue() - start_colour.Blue()
    r = start_colour.Red()   + (((r * position * 100) / end_position) / 100)
    g = start_colour.Green() + (((g * position * 100) / end_position) / 100)
    b = start_colour.Blue()  + (((b * position * 100) / end_position) / 100)

    return wx.Colour(r, g, b)


def RibbonShiftLuminance(colour, amount):

    if amount <= 1.0:
        return colour.Darker(colour.luminance * (1.0 - amount))
    else:
        return colour.Lighter((1.0 - colour.luminance) * (amount - 1.0))



def RibbonCanLabelBreakAtPosition(label, pos):

    return label[pos] == ' '


def RibbonDrawParallelGradientLines(dc, nlines, line_origins, stepx, stepy, numsteps, offset_x,
                                    offset_y, start_colour, end_colour):

    rd = end_colour.Red() - start_colour.Red()
    gd = end_colour.Green() - start_colour.Green()
    bd = end_colour.Blue() - start_colour.Blue()

    for step in xrange(numsteps):    
        r = start_colour.Red() + (((step*rd*100)/numsteps)/100)
        g = start_colour.Green() + (((step*gd*100)/numsteps)/100)
        b = start_colour.Blue() + (((step*bd*100)/numsteps)/100)

        p = wx.Pen(wx.Colour(r, g, b))
        dc.SetPen(p)

        for n in xrange(nlines):        
            dc.DrawLine(offset_x + line_origins[n].x, offset_y + line_origins[n].y,
                        offset_x + line_origins[n].x + stepx, offset_y + line_origins[n].y + stepy)
        
        offset_x += stepx
        offset_y += stepy


def RibbonLoadPixmap(bits, fore):

    xpm = wx.BitmapFromXPMData(bits).ConvertToImage()
    xpm.Replace(255, 0, 255, fore.Red(), fore.Green(), fore.Blue())
    return wx.BitmapFromImage(xpm)


class RibbonHSLColour(object):

    def __init__(self, h=0.0, s=0.0, l=0.0):

        if isinstance(h, wx.Colour):
            
            red, green, blue = h.Red()/255.0, h.Green()/255.0, h.Blue()/255.0
            Min = min(red, min(green, blue))
            Max = max(red, max(green, blue))
            luminance = 0.5 * (Max + Min)
            
            if Min == Max:            
                # colour is a shade of grey
                hue = 0.0
                saturation = 0.0
            
            else:            
                if luminance <= 0.5:
                    saturation = (Max - Min) / (Max + Min)
                else:
                    saturation = (Max - Min) / (2.0 - (Max + Min))

                if Max == red:                
                    hue = 60.0 * (green - blue) / (Max - Min)
                    if hue < 0.0:
                        hue += 360.0
                
                elif Max == green:                
                    hue = 60.0 * (blue - red) / (Max - Min)
                    hue += 120.0
                
                else: # Max == blue
                 
                    hue = 60.0 * (red - green) / (Max - Min)
                    hue += 240.0

            self.hue = hue
            self.saturation = saturation
            self.luminance = luminance

        else:

            self.hue = h
            self.saturation = s
            self.luminance = l
        

    def ToRGB(self):

        _hue = (self.hue - math.floor(self.hue / 360.0) * 360.0)
        _saturation = self.saturation
        _luminance = self.luminance
        
        if _saturation > 1.0:
            _saturation = 1.0
        if _saturation < 0.0:
            _saturation = 0.0
        if _luminance > 1.0:
            _luminance = 1.0
        if _luminance < 0.0:
            _luminance = 0.0

        if _saturation == 0.0:        
            # colour is a shade of grey
            red = blue = green = _luminance
        
        else:
        
            tmp2 = (_luminance < 0.5 and [_luminance*(1.0 + _saturation)] or [(_luminance+_saturation) - (_luminance*_saturation)])[0]
            tmp1 = 2.0 * _luminance - tmp2
            tmp3R = _hue + 120.0
            
            if tmp3R > 360.0:
                tmp3R -= 360.0
            if tmp3R < 60.0:
                red = tmp1 + (tmp2 - tmp1) * tmp3R / 60.0
            elif tmp3R < 180.0:
                red = tmp2
            elif tmp3R < 240.0:
                red = tmp1 + (tmp2 - tmp1) * (240.0 - tmp3R) / 60.0
            else:
                red = tmp1

            tmp3G = _hue
            
            if tmp3G > 360.0:
                tmp3G -= 360.0
            if tmp3G < 60.0:
                green = tmp1 + (tmp2 - tmp1) * tmp3G / 60.0
            elif tmp3G < 180.0:
                green = tmp2
            elif tmp3G < 240.0:
                green = tmp1 + (tmp2 - tmp1) * (240.0 - tmp3G) / 60.0
            else:
                green = tmp1

            tmp3B = _hue + 240.0
            
            if tmp3B > 360.0:
                tmp3B -= 360.0
            if tmp3B < 60.0:
                blue = tmp1 + (tmp2 - tmp1) * tmp3B / 60.0
            elif tmp3B < 180.0:
                blue = tmp2
            elif tmp3B < 240.0:
                blue = tmp1 + (tmp2 - tmp1) * (240.0 - tmp3B) / 60.0
            else:
                blue = tmp1
        
        return wx.Colour(red * 255.0, green * 255.0, blue * 255.0)


    def Darker(self, delta):

        return self.Lighter(-delta)


    def MakeDarker(self, delta):

        self.luminance -= delta
        return self


    def Lighter(self, delta):

        return RibbonHSLColour(self.hue, self.saturation, self.luminance + delta)


    def Saturated(self, delta):

        return RibbonHSLColour(self.hue, self.saturation + delta, self.luminance)


    def Desaturated(self, delta):

        return self.Saturated(-delta)


    def ShiftHue(self, delta):

        return RibbonHSLColour(self.hue + delta, self.saturation, self.luminance)


class RibbonPageTabInfo(object):
    
    def __init__(self):
            
        self.page = -1
        self.active = False
        self.hovererd = False
        self.rect = wx.Rect()
        self.ideal_width = 0
        self.small_begin_need_separator_width = 0
        self.small_must_have_separator_width = 0
        self.minimum_width = 0
        
        
                  
