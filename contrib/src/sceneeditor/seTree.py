#################################################################
# seTree.py
# Originally from Tree.py
# Altered by Yi-Hong Lin, yihhongl@andrew.cmu.edu, 2004
#
# This class actually decides the behavior of the sceneGraphExplorer
# You might feel it realy looks like the original one, but we actually did a lots of change in it.
# such as, when selection happend in other place, such as picking directly inside the scene. or,
# when user removed something by hot key.
# The rename process has also been changed. It won't be rename in here anymore.
# Instead, here we will send out a message to sceneEditor to reaname the target.
#
#################################################################

from direct.tkwidgets.Tree import TreeNode, TreeItem
from direct.showbase.TkGlobal import Label

class SeTreeNode(TreeNode):

    def __init__(self, canvas, parent, item, menuList = []):
        TreeNode.__init__(self, canvas, parent, item, menuList)

    def select(self, event=None):
        if self.selected:
            return
        self.deselectall()
        self.selected = 1
        self.canvas.delete(self.image_id)
        self.drawicon()
        self.drawtext()
        self.item.OnSelect(event)

    def drawtext(self, text=None):
        textx = self.x+20-1
        texty = self.y-1
        labeltext = self.item.GetLabelText()
        if labeltext:
            id = self.canvas.create_text(textx, texty, anchor="nw",
                                         text=labeltext)
            self.canvas.tag_bind(id, "<1>", self.select)
            self.canvas.tag_bind(id, "<Double-1>", self.flip)
            x0, y0, x1, y1 = self.canvas.bbox(id)
            textx = max(x1, 200) + 10
        if text==None:
            text = self.item.GetText() or "<no text>"
        try:
            self.entry
        except AttributeError:
            pass
        else:
            self.edit_finish()
        try:
            label = self.label
        except AttributeError:
            # padding carefully selected (on Windows) to match Entry widget:
            self.label = Label(self.canvas, text=text, bd=0, padx=2, pady=2)
        if self.selected:
            self.label.configure(fg="white", bg="darkblue")
        elif self.setAsTarget:
            self.label.configure(fg="white", bg="red")
        else:
            fg = self.item.GetTextFg()
            bg = self.item.GetTextBg()
            self.label.configure(fg=fg, bg=bg)
        id = self.canvas.create_window(textx, texty,
                                       anchor="nw", window=self.label)
        self.label.bind("<1>", self.select_or_edit)
        self.label.bind("<Double-1>", self.flip)
        self.label.bind("<3>", self.popupMenu)
        # Update text if necessary
        if text != self.label['text']:
            self.label['text'] = text
        self.text_id = id

    def select_or_edit(self, event=None):
        if self.selected and self.item.IsEditable():
            text = self.item.GetTextForEdit()
            self.label['text'] = text
            self.drawtext(text)
        TreeNode.select_or_edit(self)

    def GetTextForEdit(self):
        """Called before editting the item."""
