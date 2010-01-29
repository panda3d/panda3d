"""
UI for object property control
"""
import wx
import os

from wx.lib.scrolledpanel import ScrolledPanel
from wx.lib.agw.cubecolourdialog import *
from direct.wxwidgets.WxSlider import *
from pandac.PandaModules import *
import ObjectGlobals as OG

class AnimFileDrop(wx.FileDropTarget):
    def __init__(self, editor):
        wx.FileDropTarget.__init__(self)
        self.editor = editor

    def OnDropFiles(self, x, y, filenames):
        obj = self.editor.objectMgr.findObjectByNodePath(base.direct.selected.last)
        if obj is None:
            return

        objDef = obj[OG.OBJ_DEF]
        if not objDef.actor:
            return

        objNP = obj[OG.OBJ_NP]

        for filename in filenames:
            name = os.path.basename(filename)
            animName = Filename.fromOsSpecific(filename).getFullpath()
            if animName not in objDef.anims:
                objDef.anims.append(animName)

            objNP.loadAnims({name:animName})
            objNP.loop(name)
            obj[OG.OBJ_ANIM] = animName
            self.editor.ui.objectPropertyUI.updateProps(obj)
            
class ObjectPropUI(wx.Panel):
    """
    Base class for ObjectPropUIs,
    It consists of label area and ui area.
    """
    def __init__(self, parent, label):
        wx.Panel.__init__(self, parent)
        self.parent = parent
        self.label = wx.StaticText(self, label=label)
        self.uiPane = wx.Panel(self)
        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.label)
        sizer.Add(self.uiPane, 1, wx.EXPAND, 0)
        self.SetSizer(sizer)

    def setValue(self, value):
        self.ui.SetValue(value)

    def getValue(self):
        return self.ui.GetValue()

    def bindFunc(self, inFunc, outFunc, valFunc = None):
        self.ui.Bind(wx.EVT_ENTER_WINDOW, inFunc)
        self.ui.Bind(wx.EVT_LEAVE_WINDOW, outFunc)
        if valFunc:
            self.ui.Bind(self.eventType, valFunc)

class ObjectPropUIEntry(ObjectPropUI):
    """ UI for string value properties """
    def __init__(self, parent, label):
        ObjectPropUI.__init__(self, parent, label)
        self.ui = wx.TextCtrl(self.uiPane, -1)
        self.eventType = wx.EVT_TEXT_ENTER
        self.Layout()

    def setValue(self, value):
        self.ui.SetValue(str(value))

class ObjectPropUISlider(ObjectPropUI):
    """ UI for float value properties """
    def __init__(self, parent, label, value, minValue, maxValue):
        ObjectPropUI.__init__(self, parent, label)
        self.ui = WxSlider(self.uiPane, -1, value, minValue, maxValue,
                           pos = (0,0), size=(140, -1),
                           style=wx.SL_HORIZONTAL | wx.SL_LABELS)
        self.ui.Enable()
        self.Layout()

    def bindFunc(self, inFunc, outFunc, valFunc = None):
        self.ui.Bind(wx.EVT_ENTER_WINDOW, inFunc)
        self.ui.Bind(wx.EVT_LEAVE_WINDOW, outFunc)
        self.ui.textValue.Bind(wx.EVT_ENTER_WINDOW, inFunc)
        self.ui.textValue.Bind(wx.EVT_LEAVE_WINDOW, outFunc)

        if valFunc:
            self.ui.bindFunc(valFunc)


class ObjectPropUISpinner(ObjectPropUI):
    """ UI for int value properties """
    def __init__(self, parent, label, value, minValue, maxValue):
        ObjectPropUI.__init__(self, parent, label)
        self.ui = wx.SpinCtrl(self.uiPane, -1, "", min=minValue, max=maxValue, initial=value)
        self.eventType = wx.EVT_SPIN
        self.Layout()


class ObjectPropUICheck(ObjectPropUI):
    def __init__(self, parent, label, value):
        ObjectPropUI.__init__(self, parent, label)
        self.ui = wx.CheckBox(self.uiPane, -1, "", size=(50, 30))
        self.setValue(value)
        self.eventType = wx.EVT_CHECKBOX
        self.Layout()


class ObjectPropUIRadio(ObjectPropUI):
    def __init__(self, parent, label, value, valueList):
        ObjectPropUI.__init__(self, parent, label)
        self.ui = wx.RadioBox(self.uiPane, -1, "", choices=valueList, majorDimension=1, style=wx.RA_SPECIFY_COLS)
        self.setValue(value)
        self.eventType = wx.EVT_RADIOBOX
        self.Layout()

    def setValue(self, value):
        self.ui.SetStringSelection(value)

    def getValue(self):
        return self.ui.GetStringSelection()


class ObjectPropUICombo(ObjectPropUI):
    def __init__(self, parent, label, value, valueList):
        ObjectPropUI.__init__(self, parent, label)
        self.ui = wx.Choice(self.uiPane, -1, choices=valueList)
        self.setValue(value)
        self.eventType = wx.EVT_CHOICE
        self.Layout()

    def setValue(self, value):
        self.ui.SetStringSelection(value)

    def getValue(self):
        return self.ui.GetStringSelection()

class ColorPicker(CubeColourDialog):
    def __init__(self, parent, colourData=None, style=CCD_SHOW_ALPHA, alpha = 255, callback=None):
        self.callback=callback
        CubeColourDialog.__init__(self, parent, colourData, style)
        self.okButton.Hide()
        self.cancelButton.Hide()
        self._colour.alpha = alpha
        self.alphaSpin.SetValue(self._colour.alpha)
        self.DrawAlpha()

    def SetPanelColours(self):
        self.oldColourPanel.RefreshColour(self._oldColour)
        self.newColourPanel.RefreshColour(self._colour)
        if self.callback:
            self.callback(self._colour.r, self._colour.g, self._colour.b, self._colour.alpha)

class ObjectPropertyUI(ScrolledPanel):
    def __init__(self, parent, editor):
        self.editor = editor
        self.colorPicker = None
        ScrolledPanel.__init__(self, parent)

        parentSizer = wx.BoxSizer(wx.VERTICAL)
        parentSizer.Add(self, 1, wx.EXPAND, 0)
        parent.SetSizer(parentSizer); parent.Layout()

        self.SetDropTarget(AnimFileDrop(self.editor))

    def clearPropUI(self):
        sizer = self.GetSizer()
        if sizer is not None:
            sizer.Remove(self.propPane)
            self.propPane.Destroy()
            self.SetSizer(None)
        self.Layout()
        self.SetupScrolling(self, scroll_y = True, rate_y = 20)

    def colorPickerCB(self, rr, gg, bb, aa):
        r = rr / 255.0
        g = gg / 255.0
        b = bb / 255.0
        a = aa / 255.0
        self.propCR.setValue(r)
        self.propCG.setValue(g)
        self.propCB.setValue(b)
        self.propCA.setValue(a)        

        self.editor.objectMgr.updateObjectColor(r, g, b, a)

    def onColorSlider(self, evt):
        r = float(self.editor.ui.objectPropertyUI.propCR.getValue())
        g = float(self.editor.ui.objectPropertyUI.propCG.getValue())
        b = float(self.editor.ui.objectPropertyUI.propCB.getValue())
        a = float(self.editor.ui.objectPropertyUI.propCA.getValue())

        if self.colorPicker:
            evtObj = evt.GetEventObject()
            if evtObj == self.propCR.ui or\
               evtObj == self.propCR.ui.textValue:
                self.colorPicker.redSpin.SetValue(r * 255)
                self.colorPicker.AssignColourValue('r', r * 255, 255, 0)
            elif evtObj == self.propCG.ui or\
                 evtObj == self.propCG.ui.textValue:
                self.colorPicker.greenSpin.SetValue(g * 255)
                self.colorPicker.AssignColourValue('g', g * 255, 255, 0)
            elif evtObj == self.propCB.ui or\
                 evtObj == self.propCB.ui.textValue:
                self.colorPicker.blueSpin.SetValue(b * 255)
                self.colorPicker.AssignColourValue('b', b * 255, 255, 0)
            else:
                self.colorPicker._colour.alpha = a * 255
                self.colorPicker.alphaSpin.SetValue(self.colorPicker._colour.alpha)
                self.colorPicker.DrawAlpha()

        self.editor.objectMgr.updateObjectColor(r, g, b, a)
        
    def openColorPicker(self, evt, colourData, alpha):
        if self.colorPicker:
            self.colorPicker.Destroy()

        self.colorPicker = ColorPicker(self, colourData, alpha=alpha, callback=self.colorPickerCB)
        self.colorPicker.GetColourData().SetChooseFull(True)
        self.colorPicker.Show()
        
    def updateProps(self, obj):
        self.clearPropUI()
        
        self.propPane = wx.Panel(self)
        mainSizer = wx.BoxSizer(wx.VERTICAL)
        mainSizer.Add(self.propPane, 1, wx.EXPAND, 0)
        self.SetSizer(mainSizer)

        self.propX = ObjectPropUIEntry(self.propPane, 'X')
        self.propY = ObjectPropUIEntry(self.propPane, 'Y')
        self.propZ = ObjectPropUIEntry(self.propPane, 'Z')

        self.propH = ObjectPropUISlider(self.propPane, 'H', 0, 0, 360)
        self.propP = ObjectPropUISlider(self.propPane, 'P', 0, 0, 360)
        self.propR = ObjectPropUISlider(self.propPane, 'R', 0, 0, 360)

        self.propSX = ObjectPropUIEntry(self.propPane, 'SX')
        self.propSY = ObjectPropUIEntry(self.propPane, 'SY')
        self.propSZ = ObjectPropUIEntry(self.propPane, 'SZ')

        transformProps = [self.propX, self.propY, self.propZ, self.propH, self.propP, self.propR,
                       self.propSX, self.propSY, self.propSZ]

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.AddMany(transformProps)

        for transformProp in transformProps:
            transformProp.bindFunc(self.editor.objectMgr.onEnterObjectPropUI,
                                   self.editor.objectMgr.onLeaveObjectPropUI,
                                   self.editor.objectMgr.updateObjectTransform)

        objNP = obj[OG.OBJ_NP]
        objNP.setTransparency(1)
        colorScale = objNP.getColorScale()
        self.propCR = ObjectPropUISlider(self.propPane, 'CR', colorScale.getX(), 0, 1)
        self.propCG = ObjectPropUISlider(self.propPane, 'CG', colorScale.getY(), 0, 1)
        self.propCB = ObjectPropUISlider(self.propPane, 'CB', colorScale.getZ(), 0, 1)        
        self.propCA = ObjectPropUISlider(self.propPane, 'CA', colorScale.getW(), 0, 1) 
        colorProps = [self.propCR, self.propCG, self.propCB, self.propCA]

        for colorProp in colorProps:
            colorProp.ui.bindFunc(self.onColorSlider)

        sizer.AddMany(colorProps)
        button = wx.Button(self.propPane, -1, 'Color Picker', (0,0), (140, 20))
        _colourData = wx.ColourData()
        _colourData.SetColour(wx.Colour(colorScale.getX() * 255, colorScale.getY() * 255, colorScale.getZ() * 255))
        button.Bind(wx.EVT_BUTTON, lambda p0=None, p1=_colourData, p2=colorScale.getW() * 255: self.openColorPicker(p0, p1, p2))

        sizer.Add(button)

        if self.colorPicker:
            self.openColorPicker(None, _colourData, colorScale.getW() * 255)

        objDef = obj[OG.OBJ_DEF]

        if objDef.model is not None:
            propUI = ObjectPropUICombo(self.propPane, 'model', obj[OG.OBJ_MODEL], objDef.models)
            sizer.Add(propUI)            

            propUI.bindFunc(self.editor.objectMgr.onEnterObjectPropUI,
                            self.editor.objectMgr.onLeaveObjectPropUI,
                            lambda p0=None, p1=obj: self.editor.objectMgr.updateObjectModelFromUI(p0, p1))

        if len(objDef.anims) > 0:
            propUI = ObjectPropUICombo(self.propPane, 'anim', obj[OG.OBJ_ANIM], objDef.anims)
            sizer.Add(propUI)            

            propUI.bindFunc(self.editor.objectMgr.onEnterObjectPropUI,
                            self.editor.objectMgr.onLeaveObjectPropUI,
                            lambda p0=None, p1=obj: self.editor.objectMgr.updateObjectAnimFromUI(p0, p1))

        for key in objDef.properties.keys():
            propDef = objDef.properties[key]
            propType = propDef[OG.PROP_TYPE]
            propDataType = propDef[OG.PROP_DATATYPE]
            value = obj[OG.OBJ_PROP].get(key)

            if propType == OG.PROP_UI_ENTRY:
                propUI = ObjectPropUIEntry(self.propPane, key)
                propUI.setValue(value)
                sizer.Add(propUI)

            elif propType == OG.PROP_UI_SLIDE:
                if len(propDef) <= OG.PROP_RANGE:
                    continue
                propRange = propDef[OG.PROP_RANGE]

                if value is None:
                    continue

                propUI = ObjectPropUISlider(self.propPane, key, value, propRange[OG.RANGE_MIN], propRange[OG.RANGE_MAX])
                sizer.Add(propUI)

            elif propType == OG.PROP_UI_SPIN:
                if len(propDef) <= OG.PROP_RANGE:
                    continue
                propRange = propDef[OG.PROP_RANGE]

                if value is None:
                    continue

                propUI = ObjectPropUISpinner(self.propPane, key, value, propRange[OG.RANGE_MIN], propRange[OG.RANGE_MAX])
                sizer.Add(propUI)                

            elif propType == OG.PROP_UI_CHECK:
                if value is None:
                    continue

                propUI = ObjectPropUICheck(self.propPane, key, value)
                sizer.Add(propUI)                  

            elif propType == OG.PROP_UI_RADIO:
                if len(propDef) <= OG.PROP_RANGE:
                    continue
                propRange = propDef[OG.PROP_RANGE]
                
                if value is None:
                    continue

                if propDataType != OG.PROP_STR:
                    for i in range(len(propRange)):
                        propRange[i] = str(propRange[i])

                    value = str(value)

                propUI = ObjectPropUIRadio(self.propPane, key, value, propRange)
                sizer.Add(propUI)

            elif propType == OG.PROP_UI_COMBO:
                if len(propDef) <= OG.PROP_RANGE:
                    continue
                propRange = propDef[OG.PROP_RANGE]                

                if value is None:
                    continue

                if propDataType != OG.PROP_STR:
                    for i in range(len(propRange)):
                        propRange[i] = str(propRange[i])

                    value = str(value)

                propUI = ObjectPropUICombo(self.propPane, key, value, propRange)
                sizer.Add(propUI)

            else:
                # unspported property type
                continue

            propUI.bindFunc(self.editor.objectMgr.onEnterObjectPropUI,
                            self.editor.objectMgr.onLeaveObjectPropUI,
                            lambda p0=None, p1=obj, p2=key: self.editor.objectMgr.updateObjectProperty(p0, p1, p2))


        self.propPane.SetSizer(sizer);
        self.Layout()
        self.SetupScrolling(self, scroll_y = True, rate_y = 20)
