#----------------------------------------------------------------------
# Name:        wx.lib.flashwin
# Purpose:     A class that allows the use of the Shockwave Flash
#              ActiveX control
#
# Author:      Robin Dunn
#
# Created:     22-March-2004
# RCS-ID:      $Id: flashwin.py 54040 2008-06-08 23:03:22Z RD $
# Copyright:   (c) 2008 by Total Control Software
# Licence:     wxWindows license
#----------------------------------------------------------------------

import wx
import wx.lib.activex
import comtypes.client as cc

import sys
if not hasattr(sys, 'frozen'):
    cc.GetModule( ('{D27CDB6B-AE6D-11CF-96B8-444553540000}', 1, 0) )
from comtypes.gen import ShockwaveFlashObjects


clsID = '{D27CDB6E-AE6D-11CF-96B8-444553540000}'
progID = 'ShockwaveFlash.ShockwaveFlash.1'



class FlashWindow(wx.lib.activex.ActiveXCtrl):
    def __init__(self, parent, id=-1, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=0, name='FlashWindow'):
        wx.lib.activex.ActiveXCtrl.__init__(self, parent, progID,
                                            id, pos, size, style, name)
        
    def SetZoomRect(self, left, top, right, bottom):
        return self.ctrl.SetZoomRect(left, top, right, bottom)

    def Zoom(self, factor):
        return self.ctrl.Zoom(factor)

    def Pan(self, x, y, mode):
        return self.ctrl.Pan(x, y, mode)

    def Play(self):
        return self.ctrl.Play()

    def Stop(self):
        return self.ctrl.Stop()

    def Back(self):
        return self.ctrl.Back()

    def Forward(self):
        return self.ctrl.Forward()

    def Rewind(self):
        return self.ctrl.Rewind()

    def StopPlay(self):
        return self.ctrl.StopPlay()

    def GotoFrame(self, FrameNum):
        return self.ctrl.GotoFrame(FrameNum)

    def CurrentFrame(self):
        return self.ctrl.CurrentFrame()

    def IsPlaying(self):
        return self.ctrl.IsPlaying()

    def PercentLoaded(self):
        return self.ctrl.PercentLoaded()

    def FrameLoaded(self, FrameNum):
        return self.ctrl.FrameLoaded(FrameNum)

    def FlashVersion(self):
        return self.ctrl.FlashVersion()

    def LoadMovie(self, layer, url):
        return self.ctrl.LoadMovie(layer, url)

    def TGotoFrame(self, target, FrameNum):
        return self.ctrl.TGotoFrame(target, FrameNum)

    def TGotoLabel(self, target, label):
        return self.ctrl.TGotoLabel(target, label)

    def TCurrentFrame(self, target):
        return self.ctrl.TCurrentFrame(target)

    def TCurrentLabel(self, target):
        return self.ctrl.TCurrentLabel(target)

    def TPlay(self, target):
        return self.ctrl.TPlay(target)

    def TStopPlay(self, target):
        return self.ctrl.TStopPlay(target)

    def SetVariable(self, name, value):
        return self.ctrl.SetVariable(name, value)

    def GetVariable(self, name):
        return self.ctrl.GetVariable(name)

    def TSetProperty(self, target, property, value):
        return self.ctrl.TSetProperty(target, property, value)

    def TGetProperty(self, target, property):
        return self.ctrl.TGetProperty(target, property)

    def TCallFrame(self, target, FrameNum):
        return self.ctrl.TCallFrame(target, FrameNum)

    def TCallLabel(self, target, label):
        return self.ctrl.TCallLabel(target, label)

    def TSetPropertyNum(self, target, property, value):
        return self.ctrl.TSetPropertyNum(target, property, value)

    def TGetPropertyNum(self, target, property):
        return self.ctrl.TGetPropertyNum(target, property)

    def TGetPropertyAsNumber(self, target, property):
        return self.ctrl.TGetPropertyAsNumber(target, property)

    # Getters, Setters and properties
    def _get_ReadyState(self):
        return self.ctrl.ReadyState
    readystate = property(_get_ReadyState, None)

    def _get_TotalFrames(self):
        return self.ctrl.TotalFrames
    totalframes = property(_get_TotalFrames, None)

    def _get_Playing(self):
        return self.ctrl.Playing
    def _set_Playing(self, Playing):
        self.ctrl.Playing = Playing
    playing = property(_get_Playing, _set_Playing)

    def _get_Quality(self):
        return self.ctrl.Quality
    def _set_Quality(self, Quality):
        self.ctrl.Quality = Quality
    quality = property(_get_Quality, _set_Quality)

    def _get_ScaleMode(self):
        return self.ctrl.ScaleMode
    def _set_ScaleMode(self, ScaleMode):
        self.ctrl.ScaleMode = ScaleMode
    scalemode = property(_get_ScaleMode, _set_ScaleMode)

    def _get_AlignMode(self):
        return self.ctrl.AlignMode
    def _set_AlignMode(self, AlignMode):
        self.ctrl.AlignMode = AlignMode
    alignmode = property(_get_AlignMode, _set_AlignMode)

    def _get_BackgroundColor(self):
        return self.ctrl.BackgroundColor
    def _set_BackgroundColor(self, BackgroundColor):
        self.ctrl.BackgroundColor = BackgroundColor
    backgroundcolor = property(_get_BackgroundColor, _set_BackgroundColor)

    def _get_Loop(self):
        return self.ctrl.Loop
    def _set_Loop(self, Loop):
        self.ctrl.Loop = Loop
    loop = property(_get_Loop, _set_Loop)

    def _get_Movie(self):
        return self.ctrl.Movie
    def _set_Movie(self, Movie):
        self.ctrl.Movie = Movie
    movie = property(_get_Movie, _set_Movie)

    def _get_FrameNum(self):
        return self.ctrl.FrameNum
    def _set_FrameNum(self, FrameNum):
        self.ctrl.FrameNum = FrameNum
    framenum = property(_get_FrameNum, _set_FrameNum)

    def _get_WMode(self):
        return self.ctrl.WMode
    def _set_WMode(self, WMode):
        self.ctrl.WMode = WMode
    wmode = property(_get_WMode, _set_WMode)

    def _get_SAlign(self):
        return self.ctrl.SAlign
    def _set_SAlign(self, SAlign):
        self.ctrl.SAlign = SAlign
    salign = property(_get_SAlign, _set_SAlign)

    def _get_Menu(self):
        return self.ctrl.Menu
    def _set_Menu(self, Menu):
        self.ctrl.Menu = Menu
    menu = property(_get_Menu, _set_Menu)

    def _get_Base(self):
        return self.ctrl.Base
    def _set_Base(self, Base):
        self.ctrl.Base = Base
    base = property(_get_Base, _set_Base)

    def _get_Scale(self):
        return self.ctrl.Scale
    def _set_Scale(self, Scale):
        self.ctrl.Scale = Scale
    scale = property(_get_Scale, _set_Scale)

    def _get_DeviceFont(self):
        return self.ctrl.DeviceFont
    def _set_DeviceFont(self, DeviceFont):
        self.ctrl.DeviceFont = DeviceFont
    devicefont = property(_get_DeviceFont, _set_DeviceFont)

    def _get_EmbedMovie(self):
        return self.ctrl.EmbedMovie
    def _set_EmbedMovie(self, EmbedMovie):
        self.ctrl.EmbedMovie = EmbedMovie
    embedmovie = property(_get_EmbedMovie, _set_EmbedMovie)

    def _get_BGColor(self):
        return self.ctrl.BGColor
    def _set_BGColor(self, BGColor):
        self.ctrl.BGColor = BGColor
    bgcolor = property(_get_BGColor, _set_BGColor)

    def _get_Quality2(self):
        return self.ctrl.Quality2
    def _set_Quality2(self, Quality2):
        self.ctrl.Quality2 = Quality2
    quality2 = property(_get_Quality2, _set_Quality2)

    def _get_SWRemote(self):
        return self.ctrl.SWRemote
    def _set_SWRemote(self, SWRemote):
        self.ctrl.SWRemote = SWRemote
    swremote = property(_get_SWRemote, _set_SWRemote)

    def _get_FlashVars(self):
        return self.ctrl.FlashVars
    def _set_FlashVars(self, FlashVars):
        self.ctrl.FlashVars = FlashVars
    flashvars = property(_get_FlashVars, _set_FlashVars)

    def _get_AllowScriptAccess(self):
        return self.ctrl.AllowScriptAccess
    def _set_AllowScriptAccess(self, AllowScriptAccess):
        self.ctrl.AllowScriptAccess = AllowScriptAccess
    allowscriptaccess = property(_get_AllowScriptAccess, _set_AllowScriptAccess)

    def _get_MovieData(self):
        return self.ctrl.MovieData
    def _set_MovieData(self, MovieData):
        self.ctrl.MovieData = MovieData
    moviedata = property(_get_MovieData, _set_MovieData)

