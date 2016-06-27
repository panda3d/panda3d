"""
   This is the GUI for the Curve Animation
"""
import wx

from direct.interval.IntervalGlobal import *
from direct.actor.Actor import *
from . import ObjectGlobals as OG


class CurveAnimUI(wx.Dialog):
    """
    This is the Curve Animation Panel implementation.
    """
    def __init__(self, parent, editor):
        wx.Dialog.__init__(self, parent, id=wx.ID_ANY, title="Curve Animation",
                           pos=wx.DefaultPosition, size=(430, 140))

        self.editor = editor
        self.nodePath = None
        self.curve = None

        self.mainPanel = wx.Panel(self, -1)

        self.chooseNode = wx.StaticText( self.mainPanel, -1, "Choose NodePath:")
        self.chooseNodeTxt = wx.TextCtrl( self.mainPanel, -1, "")
        self.chooseNodeButton = wx.Button( self.mainPanel, -1, "Choose..")

        self.chooseCurve = wx.StaticText( self.mainPanel, -1, "Choose attch Curve:")
        self.chooseCurveTxt = wx.TextCtrl( self.mainPanel, -1, "")
        self.chooseCurveButton = wx.Button( self.mainPanel, -1, "Choose..")

        self.duritionTime = wx.StaticText( self.mainPanel, -1, "Durition(Frame):")
        self.duritionTimeSpin = wx.SpinCtrl( self.mainPanel, -1, "",size = (70,25), min=24, max=10000)

        self.createAnimButton = wx.Button( self.mainPanel, -1, "Creat")
        self.saveAnimButton = wx.Button( self.mainPanel, -1, "Save Animation")

        self.SetProperties()
        self.DoLayout()

        self.Bind(wx.EVT_BUTTON, self.OnChooseNode, self.chooseNodeButton)
        self.Bind(wx.EVT_BUTTON, self.OnChooseCurve, self.chooseCurveButton)
        self.Bind(wx.EVT_BUTTON, self.OnCreateAnim, self.createAnimButton)
        self.Bind(wx.EVT_BUTTON, self.OnSaveAnim, self.saveAnimButton)

        self.Bind(wx.EVT_CLOSE, self.OnExit)

    def SetProperties(self):
        self.duritionTimeSpin.SetValue(24)
        self.chooseNodeTxt.SetMinSize((200,21))
        self.chooseCurveTxt.SetMinSize((200,21))
        self.saveAnimButton.SetToolTipString("Save the animation to the global animation control")

    def DoLayout(self):
        dialogSizer = wx.BoxSizer(wx.VERTICAL)
        mainSizer = wx.FlexGridSizer(4, 3, 0, 0)

        mainSizer.Add(self.chooseNode, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 10)
        mainSizer.Add(self.chooseNodeTxt, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 5)
        mainSizer.Add(self.chooseNodeButton, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 5)

        mainSizer.Add(self.chooseCurve, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 10)
        mainSizer.Add(self.chooseCurveTxt, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 5)
        mainSizer.Add(self.chooseCurveButton, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 5)

        mainSizer.Add(self.duritionTime, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 10)
        mainSizer.Add(self.duritionTimeSpin, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 5)
        mainSizer.Add(self.createAnimButton, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 5)

        mainSizer.Add(self.saveAnimButton, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 5)

        self.mainPanel.SetSizerAndFit(mainSizer)

        dialogSizer.Add(self.mainPanel, 1, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

        self.SetSizer(dialogSizer)
        self.Layout()

    def OnChooseNode(self, evt):
        if base.direct.selected.last == None or base.direct.selected.last.hasTag('Controller') or not base.direct.selected.last.hasTag('OBJRoot'):
            dlg = wx.MessageDialog(None, 'Please select an object.', 'NOTICE', wx.OK )
            dlg.ShowModal()
            dlg.Destroy()
        else:
            obj = self.editor.objectMgr.findObjectByNodePath(base.direct.selected.last)
            if obj[OG.OBJ_DEF].name == '__Curve__':
                dlg = wx.MessageDialog(None, 'Please select an object, not a curve.', 'NOTICE', wx.OK )
                dlg.ShowModal()
                dlg.Destroy()
            else:
                self.nodePath = obj
                self.chooseNodeTxt.SetValue(str(self.nodePath[OG.OBJ_UID]))

    def OnChooseCurve(self, evt):
        if base.direct.selected.last == None or base.direct.selected.last.hasTag('Controller') or not base.direct.selected.last.hasTag('OBJRoot'):
            dlg = wx.MessageDialog(None, 'Please select a curve.', 'NOTICE', wx.OK )
            dlg.ShowModal()
            dlg.Destroy()
        else:
            obj = self.editor.objectMgr.findObjectByNodePath(base.direct.selected.last)
            if obj[OG.OBJ_DEF].name != '__Curve__':
                dlg = wx.MessageDialog(None, 'Please select a curve, not an object.', 'NOTICE', wx.OK )
                dlg.ShowModal()
                dlg.Destroy()
            elif obj[OG.OBJ_DEF].name == '__Curve__':
                self.curve = obj
                self.chooseCurveTxt.SetValue(str(self.curve[OG.OBJ_UID]))

    def OnCreateAnim(self, evt):
        self.time = self.duritionTimeSpin.GetValue()
        if self.nodePath == None or self.curve == None:
            dlg = wx.MessageDialog(None, 'Please select an object and a curve first.', 'NOTICE', wx.OK )
            dlg.ShowModal()
            dlg.Destroy()
        else:
            self.curveSequence = self.editor.animMgr.singleCurveAnimation(self.nodePath, self.curve, self.time)
            self.curveSequence.start()

    def OnSaveAnim(self,evt):
        if not self.curveSequence:
            dlg = wx.MessageDialog(None, 'Please create a animation first.', 'NOTICE', wx.OK )
            dlg.ShowModal()
            dlg.Destroy()
        else:
            if self.editor.animMgr.curveAnimation == {}:
                self.editor.animMgr.curveAnimation[(self.nodePath[OG.OBJ_UID],self.curve[OG.OBJ_UID])] = (self.nodePath[OG.OBJ_UID],self.curve[OG.OBJ_UID],self.time)
                self.editor.updateStatusReadout('Sucessfully saved to global animation list')
                return

            hasKey = False
            for key in self.editor.animMgr.curveAnimation:
                if key == (self.nodePath[OG.OBJ_UID], self.curve[OG.OBJ_UID]):
                    dlg = wx.MessageDialog(None, 'Already have the animation for this object attach to this curve.', 'NOTICE', wx.OK )
                    dlg.ShowModal()
                    dlg.Destroy()
                    hasKey = True
                    return
                elif self.nodePath[OG.OBJ_UID] == key[0]:
                    dlg = wx.MessageDialog(None, 'This object is already attached to a curve.', 'NOTICE', wx.OK )
                    dlg.ShowModal()
                    dlg.Destroy()
                    hasKey = True
                    return

            if hasKey == False and self.editor.animMgr.curveAnimation != {}:
                self.editor.animMgr.curveAnimation[(self.nodePath[OG.OBJ_UID],self.curve[OG.OBJ_UID])] = (self.nodePath[OG.OBJ_UID],self.curve[OG.OBJ_UID],self.time)
                self.editor.updateStatusReadout('Sucessfully saved to global animation list')

    def OnExit(self,evt):
        self.Destroy()
        self.editor.ui.curveAnimMenuItem.Check(False)







