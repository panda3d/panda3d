from DirectGuiBase import *

class DirectLabel(DirectGuiBase, PGItem):
    def __init__(self, parent = None, **kw):
        # Pass in a background texture, and/or a geometry object,
        # and/or a text string to be used as the visible
        # representation of the label
        optiondefs = (
            ('image',         None,       self.setImage),
            ('geom',          None,       self.setGeom),
            ('text',          None,       self.setText),
            ('pos',           (0,0,0),    self.setPos),
            ('scale',         (1,1,1),    self.setScale),
            ('bounds',        None,       self.setBounds),
            ('imagePos',      (0,0,0),    self.setImagePos),
            ('imageScale',    (1,1,1),    self.setImagePos),
            ('geomPos',       (0,0,0),    self.setGeomPos),
            ('geomScale',     (1,1,1),    self.setGeomPos),
            ('textPos',       (0,0,0),    self.setTextPos),
            ('textScale',     (1,1,1),    self.setTextPos),
            )
        apply(DirectGuiBase.__init__, (self, optiondefs, ()), kw)            
        self.initialiseoptions(DirectLabel)

    def setImage(self):
        pass
    def setGeom(self):
        pass
    def setText(self):
        pass
    def setPos(self):
        pass
    def setScale(self):
        pass
    def setBounds(self):
        pass
    def setImagePos(self):
        pass
    def setImagePos(self):
        pass
    def setGeomPos(self):
        pass
    def setGeomPos(self):
        pass
    def setTextPos(self):
        pass
    def setTextPos(self):
        pass
    def setState(self):
        pass


