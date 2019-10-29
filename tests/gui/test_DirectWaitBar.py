# coding=utf-8
from direct.gui.DirectWaitBar import DirectWaitBar

def test_create_waitbar():
    bar=DirectWaitBar(pos=(0,0,0),scale=(0.1,0.1,0.1))
    bar["range"]=1.0
    bar["value"]=0.0
    bar.setRange()
    bar.setValue()
    #position = (0.0, 0.0, -0.6)
    #text = "hello"
    #scale = (0.1, 0.1, 0.1)
    #label = DirectLabel(text=text,pos=position,scale=scale,textMayChange=1)
