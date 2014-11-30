###############################################################################
# Name: style_editor.py                                                       #
# Purpose: Syntax Highlighting configuration dialog                           #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008-2011 Cody Precord <staff@editra.org>                    #
# License: wxWindows License                                                  #
###############################################################################

"""
Provides an editor dialog for graphically editing how the text is presented in 
the editor when syntax highlighting is turned on. It does this by taking the 
data from the controls and formating it into an Editra Style Sheet that the 
editor can load to configure the styles of the text.

@summary: Gui for creating custom Editra Style Sheets

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: style_editor.py 67139 2011-03-07 02:44:26Z CJP $"
__revision__ = "$Revision: 67139 $"

#--------------------------------------------------------------------------#
# Imports
import os
import glob
import wx

# Editra Imports
import ed_glob
from profiler import Profile_Get, Profile_Set
import ed_basestc
from ed_style import StyleItem
import util
import syntax.syntax as syntax
import eclib
import ed_basewin

# Function Aliases
_ = wx.GetTranslation

# Global Values
ID_FORE_COLOR = wx.NewId()
ID_BACK_COLOR = wx.NewId()
ID_BOLD = wx.NewId()
ID_ITALIC = wx.NewId()
ID_EOL = wx.NewId()
ID_ULINE = wx.NewId()
ID_FONT = wx.NewId()
ID_FONT_SIZE = wx.NewId()

SETTINGS_IDS = [ ID_FORE_COLOR, ID_BACK_COLOR, ID_BOLD, ID_ITALIC,
                 ID_EOL, ID_ULINE, ID_FONT, ID_FONT_SIZE ]

# Modification Flags
MOD_NONE           = 0
MOD_DOESNT_EXIST   = 1
MOD_CHANGE_PRESENT = 2

#--------------------------------------------------------------------------#

class StyleEditor(ed_basewin.EdBaseDialog):
    """This class creates the window that contains the controls
    for editing/configuring the syntax highlighting styles it acts
    as a graphical way to interact with the L{ed_style.StyleMgr}.

    @see: ed_style.StyleMgr
    """
    def __init__(self, parent, id_=wx.ID_ANY, title=_("Style Editor"),
                 style=wx.DEFAULT_DIALOG_STYLE|wx.RESIZE_BORDER):
        super(StyleEditor, self).__init__(parent, id_, title, style=style)

        # Attributes
        self.LOG = wx.GetApp().GetLog()
        self._panel = StyleEditorBox(self) #TODO

        # Layout
        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self._panel, 1, wx.EXPAND)

        # Create Buttons
        b_sizer = wx.BoxSizer(wx.HORIZONTAL)
        ok_b = wx.Button(self, wx.ID_OK, _("Ok"))
        ok_b.SetDefault()
        b_sizer.AddMany([(wx.Button(self, wx.ID_CANCEL, _("Cancel")), 0),
                         ((5, 5), 0),
                         (wx.Button(self, wx.ID_SAVE, _("Save")), 0),
                         ((5, 5), 0), (ok_b, 0)])
        sizer.Add(b_sizer, 0, wx.ALIGN_RIGHT |
                  wx.ALIGN_CENTER_VERTICAL | wx.ALL, 5)

        # Finish the Layout
        self.SetSizer(sizer)
        self.SetInitialSize()

        # Event Handlers
        self.Bind(wx.EVT_BUTTON, self.OnCancel, id=wx.ID_CANCEL)
        self.Bind(wx.EVT_BUTTON, self.OnOk, id=wx.ID_OK)
        self.Bind(wx.EVT_BUTTON, self.OnSave, id=wx.ID_SAVE)
        self.Bind(wx.EVT_CLOSE, self.OnClose)

    #--- End Init ---#

    def ExportStyleSheet(self):
        """Writes the style sheet data out to a style sheet
        @return: bool

        """
        saved = self._panel.SaveStyleSheet()
        return saved

    def OnCancel(self, evt):
        """Catches the cancel button clicks and checks if anything
        needs to be done before closing the window.
        @param evt: event that called this handler

        """
        self.LOG('[style_editor][evt] Cancel Clicked Closing Window')
        evt.Skip()

    def OnClose(self, evt):
        """Handles the window closer event
        @param evt: event that called this handler

        """
        self.LOG("[style_editor][evt] Dialog closing...")
        self.OnOk(evt)

    def OnOk(self, evt):
        """Catches the OK button click and checks if any changes need to be
        saved before the window closes.
        @param evt: event that called this handler

        """
        self.LOG('[style_editor][evt] Ok Clicked Closing Window')
        modtype = self._panel.CheckForModifications()
        result = wx.ID_NO
        msg = None
        if modtype == MOD_CHANGE_PRESENT:
            msg = _("Some styles have been changed would "
                    "you like to save before exiting?")
        elif modtype == MOD_DOESNT_EXIST:
            msg = _("The new style sheet '%s' has not been saved "
                    "would you like to save before exiting?") % self._panel.StyleTheme
        if msg is not None:
            dlg = wx.MessageDialog(self, msg, _("Save Styles"),
                                   style=wx.YES_NO | wx.YES_DEFAULT | \
                                         wx.CANCEL | wx.ICON_INFORMATION)
            dlg.CenterOnParent()
            result = dlg.ShowModal()
            dlg.Destroy()

        if result == wx.ID_NO:
            # Get Current Selection to update buffers
            sheet = self._panel.StyleTheme
            path = self._panel.GetStyleSheetPath(sheet)
            if os.path.exists(path):
                UpdateBufferStyles(sheet)
            evt.Skip()
        elif result == wx.ID_CANCEL:
            self.LOG('[style_editor][info] canceled closing')
        else:
            result = self.ExportStyleSheet()
            if result != wx.ID_CANCEL:
                evt.Skip()

    def OnSave(self, evt):
        """Catches save button event
        @param evt: event that called this handler

        """
        self.LOG('[style_editor][evt] Export Clicked')
        self.ExportStyleSheet()

#-----------------------------------------------------------------------------#

class StyleEditorBox(eclib.ControlBox):
    """StyleEditor main Panel"""
    def __init__(self, parent):
        super(StyleEditorBox, self).__init__(parent)

        # Attributes
        self._prevTheme = None
        ctrlbar = self.CreateControlBar(wx.TOP)
        ss_lst = util.GetResourceFiles(u'styles', get_all=True)
        ss_lst = [sheet for sheet in ss_lst if not sheet.startswith('.')]
        self._style_ch = wx.Choice(ctrlbar, ed_glob.ID_PREF_SYNTHEME,
                                   choices=sorted(ss_lst))
        bmp = wx.ArtProvider.GetBitmap(str(ed_glob.ID_ADD), wx.ART_MENU)
        if not bmp.IsOk():
            bmp = None
        self._addbtn = eclib.PlateButton(ctrlbar, label=_("New"),
                                         bmp=bmp, style=eclib.PB_STYLE_NOBG)
        bmp = wx.ArtProvider.GetBitmap(str(ed_glob.ID_REMOVE), wx.ART_MENU)
        if not bmp.IsOk():
            bmp = None
        self._delbtn = eclib.PlateButton(ctrlbar, label=_("Remove"),
                                         bmp=bmp, style=eclib.PB_STYLE_NOBG)

        # Setup
        ss_lbl = wx.StaticText(ctrlbar, label=_("Style Theme") + u": ")
        ctrlbar.AddControl(ss_lbl, wx.ALIGN_LEFT)
        self.StyleTheme = Profile_Get('SYNTHEME', 'str')
        ctrlbar.AddControl(self._style_ch, wx.ALIGN_LEFT)
        ctrlbar.AddControl(self._addbtn, wx.ALIGN_LEFT)
        self._addbtn.SetToolTipString(_("Create a new style theme"))
        ctrlbar.AddControl(self._delbtn, wx.ALIGN_LEFT)
        self._delbtn.SetToolTipString(_("Remove Style"))
        self.SetWindow(StyleEditorPanel(self))

        # Events
        self.Bind(wx.EVT_CHOICE, self.OnThemeChoice, self._style_ch)
        self.Bind(wx.EVT_BUTTON, self.OnButton)
        self.Bind(wx.EVT_UPDATE_UI,
                  lambda evt: evt.Enable(not self.IsSystemStyleSheet()),
                  self._delbtn)

    #--- Properties ----#
    def __setStyleTheme(self, theme):
        self._prevTheme = theme # Tracking for choice change
        self._style_ch.SetStringSelection(theme)
    StyleTheme = property(lambda self: self._style_ch.GetStringSelection(),
                          lambda self, val: self.__setStyleTheme(val))
    SyntaxSheets = property(lambda self: self._style_ch.GetItems(),
                            lambda self, val: self._style_ch.SetItems(sorted(val)))

    #---- Public Api ----#
    def CheckForModifications(self, sheet_name=None):
        """Check for any unsaved modifications to the styling information
        @return: modification type

        """
        if sheet_name is None:
            sheet_name = self.StyleTheme # currently selected
        modtype = MOD_NONE
        if self.Window.DiffStyles():
            modtype = MOD_CHANGE_PRESENT
        elif not self.SheetExistOnDisk(sheet_name):
            modtype = MOD_DOESNT_EXIST
        return modtype

    def DoChangeStyleSheet(self, sheet_name):
        """Change the StyleEditor for the given style sheet"""
        if not self.SheetExistOnDisk(sheet_name):
            # Changing to a fully transient style sheet that has
            # not yet been written to disk.
            self.SetDisplayForTransientSheet(sheet_name)
        else:
            self.Window.ChangeStyleSheet(sheet_name)
            self.StyleTheme = sheet_name

    def GetStyleSheetPath(self, sheet, syspath=False):
        """Get the on disk path to where the style sheet should
        be written to.
        @param sheet: sheet name

        """
        cfgdir = ed_glob.CONFIG['STYLES_DIR']
        if syspath:
            cfgdir = ed_glob.CONFIG['SYS_STYLES_DIR']
        sheet_path = os.path.join(cfgdir, sheet)
        if not sheet_path.endswith(u"ess"):
            sheet_path += u".ess"
        return sheet_path

    def IsSystemStyleSheet(self):
        """Is the given style sheet a system provided one
        @return: bool

        """
        # If it exists in user space it is not a system one
        path = self.GetStyleSheetPath(self.StyleTheme)
        return not os.path.exists(path)

    def RefreshStyleSheets(self):
        """Update the list of style sheets"""
        ss_lst = util.GetResourceFiles(u'styles', get_all=True)
        ss_lst = [sname for sname in ss_lst if not sname.startswith('.')]
        self.SyntaxSheets = ss_lst

    def SaveStyleSheet(self, sheetname=None):
        """Save the changes to the currently selected StyleSheet
        @return: bool

        """
        # Ensure user styles directory exists to save style sheet to
        if ed_glob.CONFIG['STYLES_DIR'] == ed_glob.CONFIG['SYS_STYLES_DIR']:
            path = util.GetUserConfigBase()
            user_config = os.path.join(path, 'styles')
            if not os.path.exists(user_config):
                try:
                    os.mkdir(user_config)
                except (OSError, IOError), msg:
                    util.Log("[style_editor][err] %s" % msg)
                else:
                    ed_glob.CONFIG['STYLES_DIR'] = user_config

        rval = False
        if sheetname is None:
            sheetname = self.StyleTheme
        sheet_path = self.GetStyleSheetPath(sheetname)
        if self.WriteStyleSheet(sheet_path):
            # Update Style Sheet Control
            self.RefreshStyleSheets()
            sheet = u".".join(os.path.basename(sheet_path).split(u'.')[:-1])
            self.StyleTheme = sheet
            self.Window.ResetTransientStyleData()
            util.Log("[style_editor][info] Successfully exported: %s" % sheet)

            if sheet_path.startswith(ed_glob.CONFIG['STYLES_DIR']) or \
               sheet_path.startswith(ed_glob.CONFIG['SYS_STYLES_DIR']):
                # Update editor windows/buffer to use new style sheet
                UpdateBufferStyles(sheet)
            rval = True
        return rval

    def SetDisplayForTransientSheet(self, sheet_name):
        """Setup the display and editor data for a transient style sheet"""
        self.Window.SetBlankStyle()
        themes = self.SyntaxSheets
        if sheet_name not in themes:
            themes.append(sheet_name)
            self.SyntaxSheets = themes
        self.StyleTheme = sheet_name

    def SheetExistOnDisk(self, sheet_name):
        """Check if the given style sheet exists on disk
        @param sheet_name: style sheet name
        @return: bool

        """
        path = self.GetStyleSheetPath(sheet_name) # User path
        syspath = self.GetStyleSheetPath(sheet_name, True) # System path
        exists = os.path.exists(path) or os.path.exists(syspath)
        return exists

    def WriteStyleSheet(self, path):
        """Write the current style data to the given path
        @param path: string
        @return: bool

        """
        bOk = True
        try:
            writer = util.GetFileWriter(path)
            writer.write(self.Window.GenerateStyleSheet())
            writer.close()
        except (AttributeError, IOError), msg:
            util.Log('[style_editor][err] Failed to export style sheet')
            util.Log('[style_editor][err] %s' % msg)
            bOk = False
        return bOk

    #---- Event Handlers ----#

    def OnButton(self, evt):
        """Handle the Add/Remove Buttons"""
        e_obj = evt.GetEventObject()
        if e_obj is self._addbtn:
            # TODO: warn about unsaved changes
            fname = wx.GetTextFromUser(_("Enter style sheet name"),
                                       _("New Style Sheet"),
                                       parent=self)
            if fname:
                # Case insensitive check
                if fname.lower() in [name.lower() for name in self.SyntaxSheets]:
                    # Already exists
                    wx.MessageBox(_("The style %s already exists. Please choose a different name.") % fname,
                                  style=wx.OK|wx.CENTER|wx.ICON_INFORMATION)
                else:
                    # Create it
                    self.SetDisplayForTransientSheet(fname)
        elif e_obj is self._delbtn:
            path = self.GetStyleSheetPath(self.StyleTheme)
            try:
                os.remove(path)
            except OSError, msg:
                wx.MessageBox(_("Failed to delete style sheet:\nError:\n%s") % msg,
                              style=wx.OK|wx.CENTER|wx.ICON_ERROR)
            else:
                self.RefreshStyleSheets()
                self.StyleTheme = u"Default" # select the default style
                self.DoChangeStyleSheet(self.StyleTheme)
        else:
            evt.Skip()

    def OnThemeChoice(self, evt):
        """Check if current style sheet has been saved when switching sheets"""
        oldTheme = self._prevTheme
        newTheme = self.StyleTheme # newly selected theme
        msg = None
        modtype = self.CheckForModifications(self._prevTheme)
        if modtype == MOD_CHANGE_PRESENT:
            # prompt to save before changing
            msg = _("Would you like to save the changes to '%s' before changing themes?\n\n"
                    "Selecting No will result in all changes being lost.")
            msg = msg % oldTheme
        elif modtype == MOD_DOESNT_EXIST:
            # prompt to save unsaved sheet
            msg = _("The new style theme '%s' has not been saved.\n\n"
                    "Would you like to save it before changing themes?")
            msg = msg % oldTheme

        if msg is not None:
            dlg = wx.MessageDialog(self, msg, _("Save Styles"),
                                   style=wx.YES_NO | wx.YES_DEFAULT | \
                                         wx.ICON_INFORMATION)
            dlg.CenterOnParent()
            result = dlg.ShowModal()
            dlg.Destroy()
            if result == wx.YES:
                # Save the style sheet
                self.SaveStyleSheet(oldTheme)

        # Change the style sheet to the newly selected one
        self.DoChangeStyleSheet(newTheme)

#-----------------------------------------------------------------------------#

class StyleEditorPanel(wx.Panel):
    """Main panel for the editor portion of the StyleEditor"""
    def __init__(self, parent):
        super(StyleEditorPanel, self).__init__(parent)

        # Attributes
        self._settings = SettingsPanel(self)
        self.preview = PreviewPanel(self)
        self.prebuff = self.preview.GetPreviewBuffer() # TEMP HACK
        self.styles_orig = DuplicateStyleDict(self.prebuff.GetStyleSet())
        self.styles_new = DuplicateStyleDict(self.styles_orig)
        self.prebuff.SetStyles('preview', self.styles_new, True)
        self.preview.OpenPreviewFile('cpp')

        # Setup
        self.StyleTags = self.styles_orig.keys()
        self.__DoLayout()
        self.EnableSettings(False)

        # Event Handlers
        self.Bind(wx.EVT_LISTBOX, self.OnListBox)
        self.Bind(wx.EVT_CHOICE, self.OnChoice)
        self.Bind(wx.EVT_CHECKBOX, self.OnCheck)
        self.Bind(eclib.EVT_COLORSETTER, self.OnColor)
        self.prebuff.Bind(wx.EVT_LEFT_UP, self.OnTextRegion)
        self.prebuff.Bind(wx.EVT_KEY_UP, self.OnTextRegion)

    def __DoLayout(self):
        """Layout the window"""
        vsizer = wx.BoxSizer(wx.VERTICAL)
        vsizer.Add(self._settings, 0, wx.EXPAND|wx.ALL, 5)
        vsizer.Add(self.preview, 1, wx.EXPAND)
        self.SetSizer(vsizer)

    #---- Properties ----#

    StyleTags = property(lambda self: self._settings.TagList.GetItems(),
                         lambda self, val: self._settings.TagList.SetItems(sorted(val)))
    SettingsPanel = property(lambda self: self._settings)

    #---- Public API ----#

    def ChangeStyleSheet(self, sheet_name):
        """Change the style sheet that is being edited
        @param sheet_name: style sheet name (without extension)

        """
        self.prebuff.UpdateAllStyles(sheet_name)
        self.ResetTransientStyleData()
        tag = self._settings.TagList.GetStringSelection()
        if tag != wx.EmptyString:
            self.UpdateSettingsPane(self.styles_new[tag])

    def DiffStyles(self):
        """Checks if the current style set is different from the
        original set. Used internally to check if a save prompt needs
        to be brought up. Returns True if the style sets are different.
        @return: whether style set has been modified or not
        @rtype: bool

        """
        diff = False
        for key in self.styles_orig:
            if self.styles_orig[key] != self.styles_new[key]:
                diff = True
                break
        return diff

    def EnableSettings(self, enable=True):
        """Enables/Disables all settings controls
        @keyword enable: whether to enable/disable settings controls

        """
        for child in self.SettingsPanel.GetChildren():
            if not isinstance(child, wx.ListBox):
                child.Enable(enable)

    def GenerateStyleSheet(self):
        """Generates a style sheet from the dialogs style data
        @return: The dictionary of L{StyleItem} in self.styles_new transformed
                 into a string that is in Editra Style Sheet format.

        """
        sty_sheet = list()
        ditem = self.styles_new.get('default_style', StyleItem())
        dvals = ';\n\t\t'.join([item.replace(',', ' ')
                                for item in ditem.GetAsList() ]) + ';'
        sty_sheet.append(''.join(['default_style {\n\t\t', dvals, '\n\n}\n\n']))

        tags = sorted(self.styles_new.keys())
        for tag in tags:
            item = self.styles_new[tag]
            if item.IsNull() or tag == 'default_style':
                continue

            stage1 = wx.EmptyString
            for attr in ('fore', 'back', 'face', 'size'):
                ival = item.GetNamedAttr(attr)
                if attr in ('fore', 'back'):
                    ival = ival.upper()

                if ival is None or ival == ditem.GetNamedAttr(attr):
                    continue

                stage1 = ''.join((stage1, attr, u':',
                                  ival.replace(',', ' '), u';'))

            # Add any modifiers to the modifier tag
            modifiers = item.GetModifiers()
            if len(modifiers):
                stage1 += (u"modifiers:" + modifiers + u";").replace(',', ' ')

            # If the StyleItem had any set attributes add it to the stylesheet
            if len(stage1):
                sty_sheet.append(tag + u" {\n")
                stage2 = u"\t\t" + stage1[0:-1].replace(u";", u";\n\t\t") + u";"
                sty_sheet.append(stage2)
                sty_sheet.append(u"\n}\n\n")

        return u"".join(sty_sheet)

    def ResetTransientStyleData(self):
        """Reset the transient style data to mark the changes as not dirty"""
        self.styles_new = DuplicateStyleDict(self.prebuff.GetStyleSet())
        self.styles_orig = DuplicateStyleDict(self.styles_new)

    def SetBlankStyle(self):
        """Clear all the transient style data to a blank style set"""
        self.styles_orig = self.prebuff.BlankStyleDictionary()
        self.styles_new = DuplicateStyleDict(self.styles_orig)
        self.prebuff.SetStyles('preview', self.styles_new, nomerge=True)
        self.prebuff.UpdateAllStyles('preview')

        # For some reason this causes the text display to refresh
        # properly when nothing else would work.
        self.OnTextRegion()

    def UpdateSettingsPane(self, syntax_data):
        """Updates all the settings controls to hold the
        values of the selected tag.
        @param syntax_data: syntax data set to configure panel from

        """
        val_str = unicode(syntax_data)
        val_map = { ID_FORE_COLOR : syntax_data.GetFore(),
                    ID_BACK_COLOR : syntax_data.GetBack(),
                    ID_BOLD       : "bold" in val_str,
                    ID_ITALIC     : "italic" in val_str,
                    ID_EOL        : "eol" in val_str,
                    ID_ULINE      : "underline" in val_str,
                    ID_FONT       : syntax_data.GetFace(),
                    ID_FONT_SIZE  : syntax_data.GetSize()
                  }

        # Fall back to defaults for color values
        # that we may not be able to understand
        if u"#" not in val_map[ID_FORE_COLOR]:
            val_map[ID_FORE_COLOR] = self.prebuff.GetDefaultForeColour(as_hex=True)
        if u"#" not in val_map[ID_BACK_COLOR]:
            val_map[ID_BACK_COLOR] = self.prebuff.GetDefaultBackColour(as_hex=True)

        for sid in SETTINGS_IDS:
            ctrl = self.FindWindowById(sid)
            if isinstance(ctrl, wx.CheckBox):
                ctrl.SetValue(val_map[sid])
            elif isinstance(ctrl, wx.Choice):
                ctrl.SetStringSelection(val_map[sid])
            elif isinstance(ctrl, eclib.ColorSetter):
                ctrl.SetLabel(val_map[sid][:7])
        return True

    def UpdateStyleSet(self, id_):
        """Updates the value of the style tag to reflect any changes
        made in the settings controls.
        @param id_: identifier of the style tag in the list

        """
        # Get the tag that has been modified
        tag = self._settings.TagList.GetStringSelection()
        if not tag:
            return False

        # Get the modified value
        ctrl = self.FindWindowById(id_)
        if isinstance(ctrl, wx.CheckBox):
            val = ctrl.GetValue()
        elif isinstance(ctrl, wx.Choice):
            val = ctrl.GetStringSelection()
        elif isinstance(ctrl, eclib.ColorSetter):
            val = ctrl.GetLabel()
        else:
            return False

        # Update the value of the modified tag
        val_map = { ID_FONT       : u"face",
                    ID_FONT_SIZE  : u"size",
                    ID_BOLD       : u"bold",
                    ID_EOL        : u"eol",
                    ID_ITALIC     : u"italic",
                    ID_ULINE      : u"underline",
                    ID_FORE_COLOR : u"fore",
                    ID_BACK_COLOR : u"back"
                  }

        if id_ in [ ID_FONT, ID_FONT_SIZE, ID_FORE_COLOR, ID_BACK_COLOR ]:
            self.styles_new[tag].SetNamedAttr(val_map[id_], val)
        elif id_ in [ ID_BOLD, ID_ITALIC, ID_ULINE, ID_EOL ]:
            self.styles_new[tag].SetExAttr(val_map[id_], val)
        else:
            return False

        # Update the Preview Area
        self.prebuff.SetStyleTag(tag, self.styles_new[tag])
        self.prebuff.RefreshStyles()

    #---- Event Handlers ----#

    def OnCheck(self, evt):
        """Update Model for changes to styling settings"""
        e_id = evt.GetId()
        if e_id in [ID_BOLD, ID_EOL, ID_ULINE, ID_ITALIC]:
            self.UpdateStyleSet(e_id)
        else:
            evt.Skip()

    def OnChoice(self, evt):
        """Update Model for changes to styling settings"""
        e_id = evt.GetId()
        if e_id in [ID_FONT, ID_FONT_SIZE]:
            self.UpdateStyleSet(e_id)
        else:
            evt.Skip()

    def OnColor(self, evt):
        """Handles color selection events
        @param evt: event that called this handler

        """
        # Update The Style data for current tag
        self.UpdateStyleSet(evt.GetId())

    def OnListBox(self, evt):
        """Catches the selection of a style tag in the listbox
        and updates the style window appropriately.
        @param evt: event that called this handler

        """
        tag = evt.GetEventObject().GetStringSelection()
        if tag != u"" and tag in self.styles_new:
            self.UpdateSettingsPane(self.styles_new[tag])
            self.EnableSettings()
        else:
            self.EnableSettings(False)

    def OnTextRegion(self, evt=None):
        """Processes clicks in the preview control and sets the style
        selection in the style tags list to the style tag of the area
        the cursor has moved into.
        @param evt: event that called this handler

        """
        if evt is not None:
            evt.Skip()

        style_id = self.prebuff.GetStyleAt(self.prebuff.GetCurrentPos())
        data = self.prebuff.FindTagById(style_id)
        if data != wx.EmptyString and data in self.styles_new:
            self._settings.TagList.SetStringSelection(data)
            if wx.Platform == '__WXGTK__':
                self._settings.TagList.SetFirstItemStr(data)
            self.UpdateSettingsPane(self.styles_new[data])
            self.EnableSettings()

#-----------------------------------------------------------------------------#

class SettingsPanel(wx.Panel):
    """Panel holding all settings controls for changing the font,
    colors, styles, ect.. in the style set.

    """
    def __init__(self, parent):
        """Create the settings panel"""
        super(SettingsPanel, self).__init__(parent)

        # Attributes
        self._tag_list = wx.ListBox(self,
                                    size=(-1, 150),
                                    style=wx.LB_SINGLE)

        # Layout
        self.__DoLayout()

    TagList = property(lambda self: self._tag_list)

    def __DoLayout(self):
        """Layout the controls in the panel"""
        hsizer = wx.BoxSizer(wx.HORIZONTAL)

        # Setup Left hand side with Style Tag List
        ss_v = wx.BoxSizer(wx.VERTICAL)
        style_lbl = wx.StaticText(self, label=_("Style Tags") + u": ")
        ss_v.AddMany([(style_lbl, 0, wx.ALIGN_LEFT),
                      (self._tag_list, 1, wx.EXPAND)])
        hsizer.Add(ss_v, 0, wx.EXPAND|wx.ALL, 5)

        # Add divider line
        hsizer.Add(wx.StaticLine(self, size=(-1, 2), style=wx.LI_VERTICAL),
                   0, wx.ALIGN_CENTER_HORIZONTAL|wx.EXPAND|wx.TOP|wx.BOTTOM, 5)
        
        # Setup the Right side
        setting_sizer = wx.BoxSizer(wx.VERTICAL)
        setting_top = wx.BoxSizer(wx.HORIZONTAL)

        # Settings top
        sbox = wx.StaticBox(self, label=_("Color"))
        cbox_sizer = wx.StaticBoxSizer(sbox, wx.VERTICAL)

        # Foreground
        fground_sizer = wx.BoxSizer(wx.HORIZONTAL)
        fground_lbl = wx.StaticText(self, label=_("Foreground") + u": ")
        fground_sel = eclib.ColorSetter(self, ID_FORE_COLOR, wx.BLACK)
        fground_sizer.AddMany([((5, 5)),
                               (fground_lbl, 0, wx.ALIGN_CENTER_VERTICAL),
                               ((2, 2), 0),
                               (fground_sel, 0, wx.ALIGN_CENTER_VERTICAL),
                               ((5, 5))])
        cbox_sizer.AddMany([(fground_sizer, 0, wx.ALIGN_LEFT | wx.EXPAND),
                            ((10, 10))])

        # Background
        bground_sizer = wx.BoxSizer(wx.HORIZONTAL)
        bground_lbl = wx.StaticText(self, label=_("Background") + u": ")
        bground_sel = eclib.ColorSetter(self, ID_BACK_COLOR, wx.WHITE)
        bground_sizer.AddMany([((5, 5)),
                               (bground_lbl, 0, wx.ALIGN_CENTER_VERTICAL),
                               ((2, 2), 0),
                               (bground_sel, 0, wx.ALIGN_CENTER_VERTICAL),
                               ((5, 5))])
        cbox_sizer.Add(bground_sizer, 0, wx.EXPAND)
        setting_top.AddMany([(cbox_sizer, 0, wx.ALIGN_TOP), ((10, 10), 0)])

        # Attrib Box
        attrib_box = wx.StaticBox(self, label=_("Attributes"))
        abox_sizer = wx.StaticBoxSizer(attrib_box, wx.VERTICAL)

        # Attributes
        bold_cb = wx.CheckBox(self, ID_BOLD, _("bold"))
        eol_cb = wx.CheckBox(self, ID_EOL, _("eol"))
        ital_cb = wx.CheckBox(self, ID_ITALIC, _("italic"))
        uline_cb = wx.CheckBox(self, ID_ULINE, _("underline"))
        abox_sizer.AddMany([(bold_cb, 0),
                            (eol_cb, 0),
                            (ital_cb, 0),
                            (uline_cb, 0)])
        setting_top.Add(abox_sizer, 0, wx.ALIGN_TOP)

        # Font
        fh_sizer = wx.BoxSizer(wx.HORIZONTAL)
        font_box = wx.StaticBox(self, label=_("Font Settings"))
        fbox_sizer = wx.StaticBoxSizer(font_box, wx.VERTICAL)

        # Font Face Name
        fsizer = wx.BoxSizer(wx.HORIZONTAL)
        flbl = wx.StaticText(self, label=_("Font") + u": ")
        fontenum = wx.FontEnumerator()
        if wx.Platform == '__WXMAC__':
            # FixedWidthOnly Asserts on wxMac
            fontenum.EnumerateFacenames(fixedWidthOnly=False)
        else:
            fontenum.EnumerateFacenames(fixedWidthOnly=True)
        font_lst = [u"%(primary)s", u"%(secondary)s"]
        font_lst.extend(sorted(fontenum.GetFacenames()))
        fchoice = wx.Choice(self, ID_FONT, choices=font_lst)
        fsizer.AddMany([((5, 5), 0), (flbl, 0, wx.ALIGN_CENTER_VERTICAL),
                        (fchoice, 0, wx.ALIGN_CENTER_VERTICAL), ((5, 5),0)])
        fbox_sizer.Add(fsizer, 0)

        # Font Size
        fsize_sizer = wx.BoxSizer(wx.HORIZONTAL)
        fsize_lbl = wx.StaticText(self, label=_("Size") + u": ")
        fsizes = [u"%(size)d", u"%(size2)d"]
        fsizes.extend([ unicode(x) for x in range(4, 21) ])
        fs_choice = wx.Choice(self, ID_FONT_SIZE, choices=fsizes)
        fsize_sizer.AddMany([((5, 5), 0),
                             (fsize_lbl, 0, wx.ALIGN_CENTER_VERTICAL),
                             (fs_choice, 1, wx.EXPAND|wx.ALIGN_CENTER_VERTICAL),
                             ((5, 5), 0)])
        fbox_sizer.AddMany([((5, 5), 0), (fsize_sizer, 0, wx.EXPAND)])
        fh_sizer.AddMany([(fbox_sizer, 0, wx.ALIGN_CENTER_HORIZONTAL), ((10, 10), 0)])

        # Build Section
        setting_sizer.AddMany([(setting_top, 0, wx.ALIGN_CENTER_HORIZONTAL),
                               ((10, 10), 0),
                               (fh_sizer, 0, wx.ALIGN_CENTER_HORIZONTAL)])

        # Setup Right hand side with the settings controls
        hsizer.AddStretchSpacer()
        hsizer.Add(setting_sizer, 0, wx.EXPAND|wx.ALL, 5)
        hsizer.AddStretchSpacer()

        self.SetSizer(hsizer)

#-----------------------------------------------------------------------------#

class PreviewPanel(ed_basewin.EdBaseCtrlBox):
    """Panel to hold the preview window and selector"""
    def __init__(self, parent):
        super(PreviewPanel, self).__init__(parent)

        # Attributes
        self.LOG = wx.GetApp().GetLog()
        self.preview = ed_basestc.EditraBaseStc(self, size=(-1, 200),
                                                style=wx.SUNKEN_BORDER)

        # Setup
        self.preview.SetEdgeColumn(80)
        self.preview.SetEdgeMode(wx.stc.STC_EDGE_LINE)
        self.preview.SetCaretLineVisible(True)
        self.__DoLayout()

        # Event Handlers
        self.Bind(wx.EVT_CHOICE, self.OnChoice)

    def __DoLayout(self):
        """Layout the Panel"""
        # Create the ControlBar
        cbar = self.CreateControlBar(wx.TOP)

        # Setup the ControlBar's controls
        lexer_lbl = wx.StaticText(cbar, label=_("Preview File") + u": ")
        lexer_lst = wx.Choice(cbar, ed_glob.ID_LEXER,
                              choices=syntax.GetLexerList())
        lexer_lst.SetToolTip(wx.ToolTip(_("Set the preview file type")))
        lexer_lst.SetStringSelection(u"CPP")
        cbar.AddControl(lexer_lbl)
        cbar.AddControl(lexer_lst)

        self.SetWindow(self.preview)

    def GetPreviewBuffer(self):
        """Get the STC instance"""
        return self.preview

    def OnChoice(self, evt):
        """Update the preview file"""
        if evt.GetId() == ed_glob.ID_LEXER:
            e_obj = evt.GetEventObject()
            val = e_obj.GetStringSelection()
            self.OpenPreviewFile(val)
        else:
            evt.Skip()

    def OpenPreviewFile(self, file_lbl):
        """Opens a file using the names in the Syntax Files choice
        control as a search query.
        @param file_lbl: name of file to open in test data directory

        """
        fname = file_lbl.replace(u" ", u"_").replace(u"/", u"_").lower()
        fname = fname.replace('#', 'sharp')
        try:
            fname = glob.glob(ed_glob.CONFIG['TEST_DIR'] + fname + ".*")[0]
        except IndexError:
            self.LOG('[style_editor][err] File %s Does not exist' % fname)
            return False

        self.preview.SetFileName(fname)
        self.preview.ClearAll()
        self.preview.LoadFile(fname)
        self.preview.FindLexer()
        self.preview.EmptyUndoBuffer()
        return True

#-----------------------------------------------------------------------------#
# Utility functions
def DuplicateStyleDict(style_dict):
    """Duplicates the style dictionary to make a true copy of
    it, as simply assigning the dictionary to two different variables
    only copies a reference leaving both variables pointing to the
    same object.
    @param style_dict: dictionary of tags->StyleItems
    @return: a copy of the given styleitem dictionary

    """
    new_dict = dict()
    for tag in style_dict:
        new_dict[tag] = style_dict[tag].Clone()
    return new_dict

def UpdateBufferStyles(sheet):
    """Update the style used in all buffers
    @param sheet: Style sheet to use

    """
    # Only update if the sheet has changed
    if sheet is not None:
        Profile_Set('SYNTHEME', sheet)
