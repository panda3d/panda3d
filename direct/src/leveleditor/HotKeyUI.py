import wx
from wx.lib.scrolledpanel import ScrolledPanel

class EditHotKeyDialog(wx.Dialog):
    def __init__(self, parent, id, title, key):
        wx.Dialog.__init__(self, parent, id, title, size=(250, 240))
        self.currKey = key
        self.panel = wx.Panel(self, -1)
        self.updateUI()
        vbox = wx.BoxSizer(wx.VERTICAL)
        vbox.Add(self.panel, 1, wx.EXPAND, 0)
        self.SetSizer(vbox)
        self.Layout()

    def updateUI(self):
        vbox = wx.BoxSizer(wx.VERTICAL)
        self.label = wx.StaticText(self.panel, label='')
        vbox.Add(self.label)
        self.modifierRadio = wx.RadioBox(self.panel, -1, "", choices=['None', 'Shift', 'Control'], majorDimension=1, style=wx.RA_SPECIFY_ROWS)
        self.modifierRadio.Bind(wx.EVT_RADIOBOX, self.onChangeModifier)
        vbox.Add(self.modifierRadio)
        itemPanel = wx.Panel(self.panel)
        hbox = wx.BoxSizer(wx.HORIZONTAL)
        keyList = ['']
        keyList.extend(base.direct.specialKeys)
        self.specialKeyCombo = wx.Choice(itemPanel, -1, choices=keyList)
        self.specialKeyCombo.Bind(wx.EVT_CHOICE, self.onChangeSpecialKey)
        self.keyEntry = wx.TextCtrl(itemPanel, -1, size=(30, 20))
        button = wx.Button(itemPanel, -1, 'Apply', size=(50, 20))
        button.Bind(wx.EVT_BUTTON, self.onApply)
        hbox.Add(self.specialKeyCombo)
        hbox.Add(self.keyEntry)
        hbox.Add(button)
        itemPanel.SetSizer(hbox)
        vbox.Add(itemPanel)
        self.panel.SetSizer(vbox)

        keyDesc = base.direct.hotKeyMap[self.currKey]
        self.label.SetLabel(keyDesc[0])
        if 'shift' in self.currKey:
            self.modifierRadio.SetStringSelection('Shift')
            self.specialKeyCombo.Enable(False)
            keyStr = self.currKey[len('shift-'):]
        elif 'control' in self.currKey:
            self.modifierRadio.SetStringSelection('Control')
            self.specialKeyCombo.Enable(False)
            keyStr = self.currKey[len('control-'):]
        else:
            self.modifierRadio.SetStringSelection('None')
            self.specialKeyCombo.Enable(True)
            keyStr = self.currKey

        if keyStr in base.direct.specialKeys:
            self.keyEntry.SetValue('')
            self.keyEntry.Enable(False)
            self.specialKeyCombo.SetStringSelection(keyStr)
        else:
            self.specialKeyCombo.SetStringSelection('')
            self.keyEntry.SetValue(keyStr)

    def onChangeModifier(self, evt):
        if evt.GetString() == 'None':
            self.specialKeyCombo.Enable(True)
        else:
            self.specialKeyCombo.SetStringSelection('')
            self.specialKeyCombo.Enable(False)
            self.keyEntry.Enable(True)

    def onChangeSpecialKey(self, evt):
        if evt.GetString() != '':
            self.keyEntry.SetValue('')
            self.keyEntry.Enable(False)
        else:
            self.keyEntry.Enable(True)

    def onApply(self, evt):
        modifier = self.modifierRadio.GetStringSelection()
        if modifier == 'Shift':
            prefix = 'shift-'
        elif modifier == 'Control':
            prefix = 'control-'
        else:
            prefix = ''

        specialKey = self.specialKeyCombo.GetStringSelection()
        if specialKey == '':
            newKeyStr= prefix + self.keyEntry.GetValue().lower()
        else:
            newKeyStr = specialKey

        if newKeyStr != self.currKey:
            if newKeyStr in list(base.direct.hotKeyMap.keys()):
                print('a hotkey is to be overridden with %s' % newKeyStr)
                oldKeyDesc = base.direct.hotKeyMap[newKeyStr]
                msg = 'The hotkey is already assigned to %s\n'%oldKeyDesc[0] +\
                      'Do you want to override this?'

                dialog = wx.MessageDialog(None, msg, 'Hot Key exists!',
                                        wx.YES_NO | wx.NO_DEFAULT | wx.ICON_QUESTION)
                result = dialog.ShowModal()
                if result == wx.ID_YES:
                    base.direct.hotKeyMap[newKeyStr] = base.direct.hotKeyMap[self.currKey]
                    base.direct.hotKeyMap['__removed__' + newKeyStr] = oldKeyDesc
                    del base.direct.hotKeyMap[self.currKey]
            else:
                base.direct.hotKeyMap[newKeyStr] = base.direct.hotKeyMap[self.currKey]
                del base.direct.hotKeyMap[self.currKey]

        self.Destroy()

class HotKeyPanel(ScrolledPanel):
    def __init__(self, parent):
        ScrolledPanel.__init__(self, parent, -1)
        self.parent = parent
        self.updateUI()

    def updateUI(self):
        vbox = wx.BoxSizer(wx.VERTICAL)
        keys = list(base.direct.hotKeyMap.keys())
        keys.sort()
        for key in keys:
            keyDesc = base.direct.hotKeyMap[key]
            itemPanel = wx.Panel(self)
            sizer = wx.BoxSizer(wx.HORIZONTAL)
            space = wx.StaticText(itemPanel, label='', size=(10,20))
            hotKey = wx.StaticText(itemPanel, label=key, size=(100, 20))
            desc = wx.StaticText(itemPanel, label=keyDesc[0], size=(380, 20))
            button = wx.Button(itemPanel, -1, 'Edit', size=(40, 20))
            button.Bind(wx.EVT_BUTTON, lambda p0 = None, p1 = key: self.onEdit(p0, p1))
            sizer.Add(button)
            sizer.Add(space)
            sizer.Add(hotKey)
            sizer.Add(desc, 1, wx.EXPAND)
            itemPanel.SetSizer(sizer)
            vbox.Add(itemPanel)
        self.SetSizer(vbox)
        self.Layout()
        self.SetupScrolling(self, scroll_y=True, rate_y=20)
        self.parent.parent.updateMenu()

    def onEdit(self, evt, key):
        base.le.ui.bindKeyEvents(False)
        editUI = EditHotKeyDialog(self, -1, 'Edit Hot Key', key)
        editUI.ShowModal()
        editUI.Destroy()

        sizer = self.GetSizer()
        if sizer is not None:
            sizer.DeleteWindows()
            self.SetSizer(None)
        base.le.ui.bindKeyEvents(True)
        self.updateUI()

class HotKeyUI(wx.Dialog):
    def __init__(self, parent, id, title):
        wx.Dialog.__init__(self, parent, id, title, size=(550, 500))
        self.parent = parent
        panel = HotKeyPanel(self)
        vbox = wx.BoxSizer(wx.VERTICAL)
        vbox.Add(panel, 1, wx.EXPAND, 0)
        self.SetSizer(vbox)
        self.Layout()
