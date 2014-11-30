'''
Created on Oct 3, 2010

@authors: Daphna Rosenbom,Gitty Zinger,Moshe Cohavi and Yoav Glazner
The widget is contributed by NDS ltd under the same license as wxPython

items_picker.ItemsPicker:
  - Displays available items to choose from,</LI>
  - Selection is done by the Add button or Double Click,</LI>
  - De-Selection is done by the Remove button or Double Click,</LI>

  Derived from wxPanel
'''
import wx
__version__ = 0.1

IP_DEFAULT_STYLE = 0
IP_SORT_CHOICES = 1
IP_SORT_SELECTED = 2
IP_REMOVE_FROM_CHOICES = 4


wxEVT_IP_SELECTION_CHANGED = wx.NewEventType()
EVT_IP_SELECTION_CHANGED = wx.PyEventBinder(wxEVT_IP_SELECTION_CHANGED, 1)
LB_STYLE = wx.LB_EXTENDED|wx.LB_HSCROLL


class IpSelectionChanged(wx.PyCommandEvent):
    def __init__(self, id, items, object = None):
        wx.PyCommandEvent.__init__(self, wxEVT_IP_SELECTION_CHANGED, id)
        self.__items = items
        self.SetEventObject(object)

    def GetItems(self):
        return self.__items


class ItemsPicker(wx.Panel):
    '''
    ItemsPicker is a widget that allows the user to form a set of picked
    items out of a given list
    '''
    def __init__(self, parent, id=wx.ID_ANY, choices = [],
                 label = '', selectedLabel = '',
                 ipStyle = IP_DEFAULT_STYLE,
                 *args, **kw):
        '''
        ItemsPicker(parent, choices = [], label = '', selectedLabel = '',
                    ipStyle = IP_DEFAULT_STYLE)
        '''
        wx.Panel.__init__(self, parent, id, *args, **kw)
        self._ipStyle = ipStyle
        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.Add(self._CreateSourceList(choices, label), 1,
                   wx.EXPAND|wx.ALL, 5)
        sizer.Add(self._CreateButtons(), 0, wx.ALIGN_CENTER|wx.ALL, 5)
        sizer.Add(self._CreateDestList(selectedLabel), 1,
                   wx.EXPAND|wx.ALL, 5)
        self.SetSizer(sizer)
        
        
    def SetItems(self, items):
        '''SetItems(self, items)=> None
        items - Sequence of strings that the user can pick from''' 
        return self._source.SetItems(items)    
    
    
    def GetItems(self): 
        '''GetItems(self)=> items
        returns list of strings that the user can pick from''' 
        return self._source.GetItems()
    
    
    Items = property(fget = GetItems,
                     fset = SetItems,
                     doc = 'See GetItems/SetItems')
    
     
    def GetSelections(self):
        '''GetSelections(self)=>items
        returns list of strings that were selected
        ''' 
        return self._dest.GetItems()
    
    
    def SetSelections(self, items): 
        '''SetSelections(self, items)=>None
        items - Sequence of strings to be selected
        The items are displayed in the selection part of the widget'''
        assert len(items)==len(set(items)),"duplicate items are not allowed"
        if items != self._dest.GetItems():
            self._dest.SetItems(items)
            self._FireIpSelectionChanged()
    
        
    Selections = property(fget = GetSelections,
                     fset = SetSelections,
                     doc = 'See GetSelections/SetSelections')
    
    
    def _CreateButtons(self):
        sizer = wx.BoxSizer(wx.VERTICAL)
        self.bAdd = wx.Button(self, -1, label = 'Add ->')
        self.bAdd.Bind(wx.EVT_BUTTON, self._OnAdd)
        self.bRemove = wx.Button(self, -1, label = '<- Remove')
        self.bRemove.Bind(wx.EVT_BUTTON, self._OnRemove)
        sizer.Add(self.bAdd, 0, wx.EXPAND|wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5)
        sizer.Add(self.bRemove, 0, wx.EXPAND|wx.ALL, 5)
        return sizer
    
    
    def _set_add_button_label(self, label=None):
        if label is None:
            return
        self.bAdd.SetLabel(label)
    
    add_button_label = property(fset = _set_add_button_label, fget = lambda x:x)
    
    
    def _set_remove_button_label(self, label=None):
        if label is None:
            return
        self.bRemove.SetLabel(label)
    
    remove_button_label = property(fset = _set_remove_button_label, fget = lambda x:x)
    
    
    def _OnAdd(self, e):
        if self._ipStyle & IP_REMOVE_FROM_CHOICES:
            self._MoveItems(self._source,self._dest)
        else:
            self._AddSelectedItems()
            
    def _MoveItems(self,source,dest):
        selections = source.GetSelections()
        selectedItems = map(source.GetString, selections)
        dest.SetItems(dest.GetItems() + selectedItems)
        selections = set(selections)
        source.SetItems([item for i, item in enumerate(source.GetItems())\
                      if i not in selections])
        self._FireIpSelectionChanged()
            
    def _AddSelectedItems(self):
        newItems = map(self._source.GetString, self._source.GetSelections())
        items = self._dest.GetItems()
        oldItems = set(items)
        for newItem in newItems:
            if newItem not in oldItems:
                items.append(newItem)    
        self.SetSelections(items)
    
    
    def _FireIpSelectionChanged(self):
            self.GetEventHandler().ProcessEvent(
                        IpSelectionChanged(self.GetId(), 
                                            self._dest.GetItems(),
                                             self ))
        
            
    def _OnRemove(self, e):
        if self._ipStyle & IP_REMOVE_FROM_CHOICES:
            self._MoveItems(self._dest, self._source)
        else:
            self._RemoveSelected()
    
    def _RemoveSelected(self):
        selections = self._dest.GetSelections()
        if selections:
            allItems = self._dest.GetItems()
            items = [item for i, item in enumerate(allItems)\
                      if i not in selections]
            self.SetSelections(items)
            self._FireIpSelectionChanged()
       
        
    def _CreateSourceList(self, items, label):
        style = LB_STYLE
        if self._ipStyle & IP_SORT_CHOICES:
            style |= wx.LB_SORT
        sizer = wx.BoxSizer(wx.VERTICAL)
        if label:
            sizer.Add(wx.StaticText(self, label = label), 0,
                       wx.ALIGN_LEFT|wx.ALL, 5)
        self._source = wx.ListBox(self, -1, style = style)
        self._source.Bind(wx.EVT_LISTBOX_DCLICK, self._OnDClick)
        self._source.SetItems(items)
        sizer.Add(self._source, 1, wx.EXPAND|wx.ALL, 5)
        return sizer
    
    
    def _CreateDestList(self, label):
        style = LB_STYLE
        if self._ipStyle & IP_SORT_SELECTED:
            style |= wx.LB_SORT
        sizer = wx.BoxSizer(wx.VERTICAL)
        if label:
            sizer.Add(wx.StaticText(self, label = label), 0,
                       wx.ALIGN_LEFT|wx.ALL, 5)
        self._dest = wx.ListBox(self, -1, style = style)
        self._dest.Bind(wx.EVT_LISTBOX_DCLICK,  self._OnDClick) 
        sizer.Add(self._dest, 1, wx.EXPAND|wx.ALL, 5)
        return sizer
       
        
    def _OnDClick(self, e):
        lb = e.GetEventObject()
        selections = lb.GetSelections()
        if len(selections) != 1:
            return #DCLICK only works on one item
        if e.GetSelection() != selections[0]:
            #this can happen using ^DCLICK when two items are selected
            return
        if lb  == self._source: 
            self._OnAdd(e)
        else:
            self._OnRemove(e)


        

if __name__ == '__main__':
    test = wx.PySimpleApp(0)
    frame = wx.Frame(None, -1)
    d = wx.Dialog(frame, style = wx.RESIZE_BORDER|wx.DEFAULT_DIALOG_STYLE)
    
    d.sizer = wx.BoxSizer(wx.VERTICAL)
    d.sizer.Add(wx.StaticText(d, -1, label = 'Example of the ItemsPicker'),
                 0, wx.ALL, 10)
    ip = ItemsPicker(d, -1, ['pop', 'cool', 'lame'],
                      'Stuff:', 'Selected stuff:',IP_SORT_SELECTED|IP_SORT_CHOICES|IP_REMOVE_FROM_CHOICES)
    ip.add_button_label = u'left -> right'
    ip.remove_button_label = u'right -> left'
    d.sizer.Add(ip, 1, wx.EXPAND, 1)
    d.SetSizer(d.sizer)
    test.SetTopWindow(frame)
    def callback(e):
        print 'selected items', e.GetItems()
    d.Bind(EVT_IP_SELECTION_CHANGED, callback)
    d.ShowModal()
    d.Destroy()
    frame.Close()
    
    
