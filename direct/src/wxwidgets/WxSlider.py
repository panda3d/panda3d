"""
WxSlider Class: Extended wx.Slider supporting floating point values
                you should call Enable() after binding any event with this control
"""

__all__ = ['WxSlider']

import wx

class WxSlider(wx.Slider):
    def __init__(self, parent, id, value, minValue, maxValue,\
                 pos=wx.DefaultPosition, size=wx.DefaultSize,\
                 style=wx.SL_HORIZONTAL, validator=wx.DefaultValidator, name="slider"):

        self.maxValue = maxValue
        self.minValue = minValue
        intVal = 100.0 / (self.maxValue - self.minValue) * (value - self.minValue)

        intMin = 0
        intMax = 100
        self.textValue = None
        
        if style & wx.SL_HORIZONTAL:
            newStyle = wx.SL_HORIZONTAL
            if style & wx.SL_LABELS:
                wx.StaticText(parent, -1, "%.2f"%minValue, (pos[0], pos[1]))
                strMaxValue = "%.2f"%maxValue
                wx.StaticText(parent, -1, strMaxValue, (pos[0] + size[0] - len(strMaxValue) * 8 , pos[1]))
                strValue = "%.2f"%value
                self.textValue = wx.TextCtrl(parent, -1, strValue,\
                                             (pos[0] + size[0] /2 - 20, pos[1]), (40, 20),\
                                             wx.TE_CENTER | wx.TE_PROCESS_ENTER)

                self.textValue.Disable()
                newPos = (pos[0], pos[1] + 20)
        else:
            newStyle = wx.SL_VERTICAL
            newPos = (pos[0], pos[1] + 40)

        if style & wx.SL_AUTOTICKS:
            newStyle |= wx.SL_AUTOTICKS

        wx.Slider.__init__(self, parent, id, intVal, intMin, intMax, newPos, size, style=newStyle)
        self.Disable()

    def GetValue(self):
        # overriding wx.Slider.GetValue()
        return (wx.Slider.GetValue(self) * (self.maxValue - self.minValue) / 100.0 + self.minValue)

    def SetValue(self, value):
        # overriding wx.Slider.SetValue()
        self.textValue.SetValue("%.2f"%value)
        intVal = 100.0 / (self.maxValue - self.minValue) * (value - self.minValue)
        wx.Slider.SetValue(self, intVal)

    def onChange(self, event):
        # update textValue from slider
        self.textValue.Clear()
        self.textValue.WriteText("%.2f"%self.GetValue())
        event.Skip()

    def onEnter(self, event):
        # update slider from textValue
        if self.textValue is None:
            return
        intVal = 100.0 / (self.maxValue - self.minValue) * (float(self.textValue.GetValue()) - self.minValue)
        wx.Slider.SetValue(self, intVal)
        event.Skip()

    def Disable(self):
        # overriding wx.Slider.Disable()
        wx.Slider.Disable(self)
        self.textValue.Disable()
        
    def Enable(self):
        # overriding wx.Slider.Enable()
        wx.Slider.Enable(self)
        self.Bind(wx.EVT_SLIDER, self.onChange)
        
        if not self.textValue is None:
            self.textValue.Enable()
            self.textValue.Bind(wx.EVT_TEXT_ENTER, self.onEnter)        

