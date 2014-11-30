# Name:         generate.py
# Purpose:      Code-generation related classes
# Author:       Roman Rolinsky <rolinsky@femagsoft.com>
# Created:      25.07.2007
# RCS-ID:       $Id$

from globals import *
from presenter import Presenter

class PythonOptions(wx.Dialog):

    def __init__(self, parent, cfg, dataFile):
        pre = wx.PreDialog()
        g.res.LoadOnDialog(pre, parent, "PYTHON_OPTIONS")
        self.PostCreate(pre)

        self.cfg = cfg
        self.dataFile = dataFile

        self.AutoGenerateCB = xrc.XRCCTRL(self, "AutoGenerateCB")
        self.EmbedCB = xrc.XRCCTRL(self, "EmbedCB")
        self.GettextCB = xrc.XRCCTRL(self, "GettextCB")
        self.MakeXRSFileCB = xrc.XRCCTRL(self, "MakeXRSFileCB")
        self.FileNameTC = xrc.XRCCTRL(self, "FileNameTC")
        self.BrowseBtn = xrc.XRCCTRL(self, "BrowseBtn")
        self.GenerateBtn = xrc.XRCCTRL(self, "GenerateBtn")
        self.SaveOptsBtn = xrc.XRCCTRL(self, "SaveOptsBtn")

        self.Bind(wx.EVT_BUTTON, self.OnBrowse, self.BrowseBtn)
        self.Bind(wx.EVT_BUTTON, self.OnGenerate, self.GenerateBtn)
        self.Bind(wx.EVT_BUTTON, self.OnSaveOpts, self.SaveOptsBtn)

        if self.cfg.Read("filename", "") != "":
            self.FileNameTC.SetValue(self.cfg.Read("filename"))
        else:
            name = os.path.splitext(os.path.split(dataFile)[1])[0]
            name += '_xrc.py'
            self.FileNameTC.SetValue(name)
        self.AutoGenerateCB.SetValue(self.cfg.ReadBool("autogenerate", False))
        self.EmbedCB.SetValue(self.cfg.ReadBool("embedResource", False))
        self.MakeXRSFileCB.SetValue(self.cfg.ReadBool("makeXRS", False))
        self.GettextCB.SetValue(self.cfg.ReadBool("genGettext", False))
        
                  
    def OnBrowse(self, evt):
        path = self.FileNameTC.GetValue()
        dirname = os.path.abspath(os.path.dirname(path))
        name = os.path.split(path)[1]
        dlg = wx.FileDialog(self, 'Save As', dirname, name, '*.py',
                               wx.SAVE | wx.OVERWRITE_PROMPT)
        if dlg.ShowModal() == wx.ID_OK:
            path = dlg.GetPath()
            self.FileNameTC.SetValue(path)
        dlg.Destroy()
    

    def OnGenerate(self, evt):
        pypath = self.FileNameTC.GetValue()
        embed = self.EmbedCB.GetValue()
        genGettext = self.GettextCB.GetValue()
        Presenter.generatePython(self.dataFile, pypath, embed, genGettext)
        self.OnSaveOpts()

    
    def OnSaveOpts(self, evt=None):
        self.cfg.Write("filename", self.FileNameTC.GetValue())
        self.cfg.WriteBool("autogenerate", self.AutoGenerateCB.GetValue())
        self.cfg.WriteBool("embedResource", self.EmbedCB.GetValue())
        self.cfg.WriteBool("makeXRS", self.MakeXRSFileCB.GetValue())
        self.cfg.WriteBool("genGettext", self.GettextCB.GetValue())

        self.EndModal(wx.ID_OK)
    
