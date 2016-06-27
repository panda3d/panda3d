"""
Defines Graph Editor
"""
import wx
import math
from .PaletteTreeCtrl import *
from . import ObjectGlobals as OG
from . import AnimGlobals as AG
from wx.lib.embeddedimage import PyEmbeddedImage

property =  [
    "translateX",
    "translateY",
    "translateZ"
    ]

#----------------------------------------------------------------------
ZoomIn = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAIAAAD9b0jDAAAAA3NCSVQICAjb4U/gAAACgklE"
    "QVRIid3VW0vjQBQA4DlJMziZlCKCSB/9Bf4a3wRtNU6SHyXK6pay2nVbQbasskXFO15SqmwL"
    "LtYnn7Zg20zSyT50wZikXsCHxXk9Z74ZZuacgebtLXrvIb27+PFQjDEADIoqiiLL8htQAMAY"
    "Hx4dPTw8SHEuIaRWqzWbzUQi8SoUAIaGhgqFgn15SSn1Iwmqqh4fH5c2NpLJpO8/xhORzEeR"
    "EJLL5VqtViaT8TxPCBFMoJTu7e1tb28zxpLJpOu6L6D9PS6vrHQ6nWw267puVNzZ2alUKqZh"
    "UE0LivFoX/y0vOxynpmZ4ZxHxZ+Vyu7urmmaqqqGxBhUkiSM8eLiIgDMTE87EVHTtB9bW4cH"
    "B5ZpEkKiYhjt3/XCwoKiKFNTU47jRPf4vVw+PTmxLAtjHCs+QQEAAJaWljRNm5yc7Ha7wQtF"
    "CGGMy+WybduWZSmK4nlerIiCTwoAOOfX19cTExMSgPDDT0iSJNu2x8fHU6nUM+ITVAhBCLFM"
    "83MuV63VKCGhVM65ruv1RqNYLFJKX4UihFzXTafTc7Oz+XzerlZVVQ1GhRCyLJuGcXV1VSqV"
    "QtGBKEKo6zhjY2Pzur66tnZxcRGa2ev1AMA0zXqj8W3wfmPK1HGc0dFRNj//dX397Ows6iKE"
    "DMZ+39wUCgVKabTXxNe+4zgjIyMGY8VS6eT0NOr6vs8Yu7u7+7K6SggJuQMbCud8eHjYYGxz"
    "c3N/f19RlJDb6/V0Xb+/v8/n829ofZzzVCqlz82dn5//abUk6UmyEEIIkc1k2u32r3o9uCq8"
    "+PHJsuz7vud5IfTffABZljnnwejLnb9/grEiQih2vVd9J36kup4fH+w3/S/QvwejQg8ibHgo"
    "AAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
ZoomOut = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAIAAAD9b0jDAAAAA3NCSVQICAjb4U/gAAACmUlE"
    "QVRIieXVz0sbQRQH8NmdH8smh3jwEIIlB+tNEW1NVps0WxPBv0BQXKQx4N2/xR2xTYQ9iIHc"
    "/A+shyqevHiQVBGLeqgoxM1kNjHTQ8DayWb14KX0Hfe9+fDd4bGr/Ly4AK9d6quL/y6qKIqm"
    "aQHThBBVfT7HnwlFUTjn3/f3MUKKoviMqurh4WG9XkcIvRQVQoRCodPT0+1yWdO0bhchxD2v"
    "WCxyzjHGL0IBAA8PD9biIgDAcRxN06Q39Tzvk2kahmHbNmMswP3rmBCi0WhYlkUI2dzcxBhL"
    "ruu6mUwmnU6v2bbrur1c+dY77sLCQjgcLpVKGKFuN5VKmaZpU1qr1Qghz6OP7vz8fF9f39di"
    "Efq5H6amcrkcpfTu7q7b9d8PIQRjbG5urr+//8vGBoSw200mErOzs3R9/fb2VnLh6uqqrwsA"
    "aLVao6OjV9fX3/b2RkZGJLfZbMbj8UgkUqlU3g4NhcNhIURQ0k612+12u/1ufPxHtcoY6157"
    "z/OGh4fv7++vLi8hhI/Pg9ZY1/Xz83PHcVZWViKRSLPZfNpFCHmeRynNZrNjY2ONRuOx1TOp"
    "rutnZ2eO41iWNTg4KIkYY875mm2/n5iYnp5+KvZMqut6tVrd2tpaWlqKx+OMMUmsM0YpnZqc"
    "NE3TdV3puA+q6/rJycl2ufw5n38zMCClwBjXXdem9GMmk06lukUfNBQKHR8fVyqVwvJyLBaT"
    "REJIrVbr3KNhGL6ijGKMj46OdnZ2CoVCNBqVRAjhr5ubUqmUm5kxksleIpA+fYyxg4ODfD4f"
    "jUY559IohHB3dzebzSYTiQARAKA8/fEJIVRVhRC2Wi3faSEEIUTahKCknbAAgF4iAEBV1WdF"
    "Ge1kCZgO7vZEX6X+c/Q3Qy8zmmCLWqIAAAAASUVORK5CYII=")

#----------------------------------------------------------------------
OneTangent = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAIAAAD9b0jDAAAAA3NCSVQICAjb4U/gAAACKUlE"
    "QVRIia2Wz2saURDH5/3Yt6aVsE3SomArGO0phxCSU6A2EFgSklP+qPwrXkOQkBCoaG8eQrBQ"
    "KRYVU1tabUij3V1337weAtZI1FX3e9qF2c/OzJv9zpKbRgMGRAhRSsF8oo9uOCeUMk0LDEo1"
    "rVUuF46Pv2SzhDEgJBhoLZer5/OViwu32yWUjnlsvHj/SrluPJ222m3GORMC5ugsGTwoyjkT"
    "4nuxeFetvj06ko4zG/RRjeh5rmVFNjeZrt8UCnxhIQAoAIBSnm0n9vb+1Gq/y2Wm60FAAUAp"
    "UGr18LCRz9vtNuH8iZipoQAopW4Yb3Z2vmazgDjtJIyMlo7zIpUyksnq2RkTIhgoAHiWFdve"
    "RsTmx4IuNP/ZToiUvV7CNNt3natb61b57cKEMIXIOeu+N0uof5IaEl8f7+R3K1SG8sKc0FqF"
    "oPTjCZOhEiDF0Vyk8da3+occC4UCgAKAAhBe73X6Xa/T+VEs8klcv0eKCtD1kgcHP6+v7xsN"
    "OtZzp5hqhUh1PbG/Xz0/l5ZFGAsACgDous+j0cjWVuX0lI428qmdWNr2q/X10NJS/fJyVHNn"
    "sXfPtuO7u39brV+lkgiHmRBDKZOhbepThDHpOJ8zmZ7jLMZiq6YJhPSXxYyLSEkZMgx9ebl8"
    "clLKZO6bTTbgkFN7ZV+e47xcW4tubIQjkWcrKyjl/zpmK/9BlDH0vIfZUogBZAoAKCWhVCk1"
    "tHrnggLAk/9I/wC0eekA1bLbIQAAAABJRU5ErkJggg==")

#----------------------------------------------------------------------
TwoTangents = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAIAAAD9b0jDAAAAA3NCSVQICAjb4U/gAAACL0lE"
    "QVRIie3UzU8TQRQA8DczO1vatRgh1YhWD/WD+kG91FgOXLhx4+CpiYnE9L8g/gVwJHhp4lUP"
    "pPHgQRMOGiEaDabV1LRAtdjUAG23m3W/Znc8YEo/trRNOGjCO+7M++3MezODdopFOO7Axy6e"
    "oC0oAqAEY4QGyCcE4faVtaII7dQUy7b7dDEheq3GdB0R4o76KH2+kZ158uzxyzcO56iXS0Rx"
    "N5tdnZ9fW1w0FaXZPUQxRl/Le7+q8mruh2paQsem2kLweCq5XGVrq7K5adTruGk+ahx+AeOS"
    "rLz4ktctdtYvPYje0i3Gu22cUr1a3VheHg6FhoPBC9EoBwDO21EOQDH2igIALL39NOrz3r8z"
    "rppWp3jQmQ8LC5enp4NTU0zTmGG0/PJwKgBzHEU3VcN6eHfie7X++ltBEmknSoaG0snkSDg8"
    "Njmp12ptIrieU4dzAEjEIu8KPz8Wy14qNI9SScqvrHCA67Ozlqq6F8f1K3McL6WPYpFUOre9"
    "L3uEv50VfL7S+vpuJjMxN8cMo1HEvlAAMG37nF+KR28+fZ/e/62JBAuiWC8U8qlUJJFAhHDH"
    "6ZZ71LnRLXY1MDJzI5Rc+6wwR5PlTDIZjsd9gYBjuTSwEajneypR4dV2qXxq1F/dG5fLodg9"
    "U9OOThGOHgYAg7FrF8fKKqjng/4rl+xeYl+ozeEMsNsiwmCftnnXQg6EcgABICxyAGAcut2x"
    "wdADt+uFdYv/9OU/Qf9d9A9dhOgRU8MmewAAAABJRU5ErkJggg==")

#----------------------------------------------------------------------

class GraphEditorWindow(wx.Window):
    """
    This is the main graph editor window.
    """
    def __init__(self, parent, windowSize, property, xRange, yRange, curFrame, object):
        wx.Window.__init__(self, parent, size = windowSize, style = wx.SUNKEN_BORDER)

        self._mainDialog = wx.GetTopLevelParent(self)
        self.w,self.h = self.GetClientSize()

        self.zoom = float(2)
        self._mouseIn = False
        self._selectRec = False
        self._selectHandler = False
        self._OneTangent = True

        self.object = object
        self.curFrame = curFrame
        self.property = property

        self.zeroPos = (float(0), self.h/float(2))
        self.zero = 0
        self.unitWidth = self.w/float(xRange)
        self.unitHeight = self.h/float(yRange)

        self.generateInfo()
        self.InitBuffer()

        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_MIDDLE_DOWN, self.OnMiddleDown)
        self.Bind(wx.EVT_MIDDLE_UP, self.OnMiddleUp)
        self.Bind(wx.EVT_MOTION, self.OnMotion)

    def refresh(self):
        self._mouseIn = False
        self._selectRec = False
        self._selectHandler = False
        self.generateInfo()

    def generateInfo(self):
        #generate the information for animation curve generation
        self.X = []
        self.Y = []
        self.Z = []

        if self._mainDialog.editor.animMgr.keyFramesInfo != {}:
            self.keyFramesInfo = self._mainDialog.editor.animMgr.keyFramesInfo
            for key in self.keyFramesInfo:
                if key == (self.object[OG.OBJ_UID], 'X'):
                    for i in range(len(self.keyFramesInfo[key])):
                        item = self.keyFramesInfo[key][i]
                        handler = self.generateHandler(item)
                        self.X.append([key, i, handler[0], handler[1], handler[2], handler[3], handler[4]])
                if key == (self.object[OG.OBJ_UID], 'Y'):
                    for i in range(len(self.keyFramesInfo[key])):
                        item = self.keyFramesInfo[key][i]
                        handler = self.generateHandler(item)
                        self.Y.append([key, i, handler[0], handler[1], handler[2], handler[3], handler[4]])
                if key == (self.object[OG.OBJ_UID], 'Z'):
                    for i in range(len(self.keyFramesInfo[key])):
                        item = self.keyFramesInfo[key][i]
                        handler = self.generateHandler(item)
                        self.Z.append([key, i, handler[0], handler[1], handler[2], handler[3], handler[4]])

    def generateHandler(self, item):
        #generate the position for the control handler
        x1 = self.zeroPos[0] + float(item[AG.FRAME])*self.unitWidth
        y1 = self.zeroPos[1] - float(item[AG.VALUE])*self.unitHeight

        t1x = item[AG.INSLOPE][0]*self.unitWidth
        t1y = item[AG.INSLOPE][1]*self.unitHeight

        t2x = item[AG.OUTSLOPE][0]*self.unitWidth
        t2y = item[AG.OUTSLOPE][1]*self.unitHeight

        tanA = t1y/t1x
        temp1 = float(1)/(tanA**2+1)
        if t1x <0 :
            cosA = -math.sqrt(abs(temp1))
        if t1x >=0:
            cosA = math.sqrt(abs(temp1))
        temp2 = (tanA**2)*temp1
        if t1y <0 :
            sinA = -math.sqrt(abs(temp2))
        if t1y >=0:
            sinA = math.sqrt(abs(temp2))

        x2 = x1-float(self.unitWidth*self.zoom)*cosA
        y2 = y1+float(self.unitWidth*self.zoom)*sinA

        tanA = t2y/t2x
        temp1 = float(1)/(tanA**2+1)
        if t2x <0 :
            cosA = -math.sqrt(abs(temp1))
        if t2x >=0:
            cosA = math.sqrt(abs(temp1))
        temp2 = (tanA**2)*temp1
        if t2y <0 :
            sinA = -math.sqrt(abs(temp2))
        if t2y >=0:
            sinA = math.sqrt(abs(temp2))

        x3 = x1+float(self.unitWidth*self.zoom)*cosA
        y3 = y1-float(self.unitWidth*self.zoom)*sinA

        return [[(x1,y1),0],[(x2,y2),0],[(x3,y3),0],[t1x,t1y],[t2x,t2y]]

    def InitBuffer(self):
        self.buffer = wx.EmptyBitmap(self.w, self.h)
        dc = wx.BufferedDC(wx.ClientDC(self), self.buffer)
        self.DrawXCoord(dc)
        self.DrawYCoord(dc)
        self.DrawFrame(dc)
        self.DrawCurve(dc)
        self.DrawSelectRec(dc)

    def SetGraphEditorData(self, property, curFrame = 1):
        self.curFrame = curFrame
        self.property = property

        self.InitBuffer()

    def OnPaint(self, evt):
        dc = wx.BufferedPaintDC(self, self.buffer)

    def DrawXCoord(self,dc):
        dc.SetBackground(wx.Brush(wx.Colour(200, 200, 200)))
        dc.Clear()

        dc.SetPen(wx.BLACK_PEN)
        dc.SetBrush(wx.BLACK_BRUSH)
        dc.SetFont(wx.Font(8, wx.SWISS, wx.NORMAL, wx.NORMAL))

        dc.DrawLine(self.zeroPos[0], float(0), self.zeroPos[0], self.h)
        st = str(self.zero)
        self.tw,self.th = dc.GetTextExtent(st)
        dc.DrawText(st, self.zeroPos[0]+1.0, self.h-self.th-0.5)

        dc.SetPen(wx.Pen(wx.Colour(150, 150, 150)))
        dc.SetBrush(wx.Brush(wx.Colour(150, 150, 150)))

        if self.unitWidth >= 25:
            posPos = self.zeroPos[0]+self.unitWidth
            posNum = self.zero + 1
            while posPos <= self.w:
                dc.DrawLine(posPos, float(0), posPos, self.h)
                st = str(posNum)
                self.drawXNumber(dc, st, posPos)
                posPos += self.unitWidth
                posNum += 1

            negPos = self.zeroPos[0]-self.unitWidth
            negNum = self.zero - 1
            while negPos >= float(0):
                dc.DrawLine(negPos, float(0), negPos, self.h)
                st = str(negNum)
                self.drawXNumber(dc, st, negPos)
                negPos -= self.unitWidth
                posNum -= 1

        elif self.unitWidth >= 10 and self.unitWidth <= 25:
            posPos = self.zeroPos[0]+self.unitWidth*float(2)
            posNum = self.zero + 2
            while posPos <= self.w:
                dc.DrawLine(posPos, float(0), posPos, self.h)
                st = str(posNum)
                self.drawXNumber(dc, st, posPos)
                posPos += self.unitWidth*float(2)
                posNum += 2

            negPos = self.zeroPos[0]-self.unitWidth*float(2)
            negNum = self.zero - 2
            while negPos >= float(0):
                dc.DrawLine(negPos, float(0), negPos, self.h)
                st = str(negNum)
                self.drawXNumber(dc, st, negPos)
                negPos -= self.unitWidth*float(2)
                posNum -= 2

        elif self.unitWidth >= 2 and self.unitWidth <= 10:
            posPos = self.zeroPos[0]+self.unitWidth*float(5)
            posNum = self.zero + 5
            while posPos <= self.w:
                dc.DrawLine(posPos, float(0), posPos, self.h)
                st = str(posNum)
                self.drawXNumber(dc, st, posPos)
                posPos += self.unitWidth*float(5)
                posNum += 5

            negPos = self.zeroPos[0]-self.unitWidth*float(5)
            negNum = self.zero - 5
            while negPos >= float(0):
                dc.DrawLine(negPos, float(0), negPos, self.h)
                st = str(negNum)
                self.drawXNumber(dc, st, negPos)
                negPos -= self.unitWidth*float(5)
                posNum -= 5

    def DrawYCoord(self,dc):
        dc.SetPen(wx.BLACK_PEN)
        dc.SetBrush(wx.BLACK_BRUSH)
        dc.SetFont(wx.Font(8, wx.SWISS, wx.NORMAL, wx.NORMAL))

        dc.DrawLine(float(0), self.zeroPos[1], self.w, self.zeroPos[1])
        st = str(self.zero)
        dc.DrawText(st, 5.0, self.zeroPos[1]-1.0)

        dc.SetPen(wx.Pen(wx.Colour(150, 150, 150)))
        dc.SetBrush(wx.Brush(wx.Colour(150, 150, 150)))
        dc.SetLogicalFunction(wx.AND)

        posPos = self.zeroPos[1]-self.unitHeight*float(5)
        posNum = self.zero + 5
        while posPos >= float(0):
            dc.DrawLine(float(0), posPos, self.w, posPos)
            st = str(posNum)
            self.drawYNumber(dc, st, posPos)
            posPos -= self.unitHeight*float(5)
            posNum += 5

        negPos = self.zeroPos[1]+self.unitHeight*float(5)
        negNum = self.zero - 5
        while negPos <= self.h:
            dc.DrawLine(float(0), negPos, self.w, negPos)
            st = str(negNum)
            self.drawYNumber(dc, st, negPos)
            negPos += self.unitHeight*float(5)
            negNum -= 5

    def drawXNumber(self, dc, st, pos):
        oldPen, oldBrush, oldMode = dc.GetPen(), dc.GetBrush(), dc.GetLogicalFunction()

        dc.SetPen(wx.BLACK_PEN)
        dc.SetBrush(wx.BLACK_BRUSH)
        dc.DrawText(st, pos+1.0, self.h-self.th-0.5)

        dc.SetPen(oldPen)
        dc.SetBrush(oldBrush)
        dc.SetLogicalFunction(oldMode)

    def drawYNumber(self, dc, st, pos):
        oldPen, oldBrush, oldMode = dc.GetPen(), dc.GetBrush(), dc.GetLogicalFunction()

        dc.SetPen(wx.BLACK_PEN)
        dc.SetBrush(wx.BLACK_BRUSH)
        dc.DrawText(st, 5.0, pos-1.0)

        dc.SetPen(oldPen)
        dc.SetBrush(oldBrush)
        dc.SetLogicalFunction(oldMode)

    def DrawFrame(self, dc):
        if self._mainDialog.editor.mode == self._mainDialog.editor.ANIM_MODE:
            curFramePos = self.zeroPos[0]+self.curFrame*self.unitWidth
            dc.SetPen(wx.Pen("red"))
            dc.SetBrush(wx.Brush("red"))
            dc.DrawLine(curFramePos, float(0), curFramePos, self.h)
        else:
            pass

    def drawX(self, dc):
        dc.SetPen(wx.Pen("red"))
        dc.SetBrush(wx.Brush("red"))
        self.drawSingleCurve(self.X, dc)
        self.drawKeys(self.X, dc)
        self.drawHandler(self.X, dc)

    def drawY(self, dc):
        dc.SetPen(wx.Pen("green"))
        dc.SetBrush(wx.Brush("green"))
        self.drawSingleCurve(self.Y, dc)
        self.drawKeys(self.Y, dc)
        self.drawHandler(self.Y, dc)

    def drawZ(self, dc):
        dc.SetPen(wx.Pen("blue"))
        dc.SetBrush(wx.Brush("blue"))
        self.drawSingleCurve(self.Z, dc)
        self.drawKeys(self.Z, dc)
        self.drawHandler(self.Z, dc)

    def DrawCurve(self, dc):
        if self.property == self._mainDialog.namestr:
           self.drawX(dc)
           self.drawY(dc)
           self.drawZ(dc)
           return
        if self.property == property[AG.X]:
           self.drawX(dc)
           return
        if self.property == property[AG.Y]:
           self.drawY(dc)
           return
        if self.property == property[AG.Z]:
           self.drawZ(dc)
           return

    def drawSingleCurve(self, list, dc):
        if len(list) == 1:
            dc.DrawPoint(list[0][AG.KEYFRAME][AG.LOCAL_VALUE][0], list[0][AG.KEYFRAME][AG.LOCAL_VALUE][1])
            return

        if len(list) == 2:
            dc.DrawLine(list[0][AG.KEYFRAME][AG.LOCAL_VALUE][0], list[0][AG.KEYFRAME][AG.LOCAL_VALUE][1], list[1][AG.KEYFRAME][AG.LOCAL_VALUE][0], list[1][AG.KEYFRAME][AG.LOCAL_VALUE][1])
            return

        if len(list)>=3 :
            for i in range(len(list)-1):
                x1 = list[i][AG.KEYFRAME][AG.LOCAL_VALUE][0]
                y1 = list[i][AG.KEYFRAME][AG.LOCAL_VALUE][1]

                x4 = list[i+1][AG.KEYFRAME][AG.LOCAL_VALUE][0]
                y4 = list[i+1][AG.KEYFRAME][AG.LOCAL_VALUE][1]

                t1x = list[i][AG.OUT_SLOPE][0]
                t1y = list[i][AG.OUT_SLOPE][1]

                t2x = list[i+1][AG.IN_SLOPE][0]
                t2y = list[i+1][AG.IN_SLOPE][1]

                x2 = x1 + (x4 - x1) / float(3);
                scale1 = (x2 - x1) / t1x;
                y2 = y1 - t1y * scale1;


                x3 = x4 - (x4 - x1) / float(3);
                scale2 = (x4 - x3) / t2x;
                y3 = y4 + t2y * scale2;

                ax = - float(1) * x1 + float(3) * x2 - float(3) * x3 + float(1) * x4;
                bx =   float(3) * x1 - float(6) * x2 + float(3) * x3 + float(0) * x4;
                cx = - float(3) * x1 + float(3) * x2 + float(0) * x3 + float(0) * x4;
                dx =   float(1) * x1 + float(0) * x2 - float(0) * x3 + float(0) * x4;

                ay = - float(1) * y1 + float(3) * y2 - float(3) * y3 + float(1) * y4;
                by =   float(3) * y1 - float(6) * y2 + float(3) * y3 + float(0) * y4;
                cy = - float(3) * y1 + float(3) * y2 + float(0) * y3 + float(0) * y4;
                dy =   float(1) * y1 + float(0) * y2 - float(0) * y3 + float(0) * y4;

                preX = x1
                preY = y1
                t = 0.001

                while t<=float(1):
                    x = ax * t*t*t + bx * t*t + cx * t + dx;
                    y = ay * t*t*t + by * t*t + cy * t + dy;

                    curX = x
                    curY = y

                    dc.DrawLine(preX, preY, curX, curY)

                    preX = curX
                    preY = curY

                    t += 0.001

    def drawKeys(self, list, dc):
        for i in range(len(list)):
            pointX = list[i][AG.KEYFRAME][AG.LOCAL_VALUE][0]
            pointY = list[i][AG.KEYFRAME][AG.LOCAL_VALUE][1]

            if list[i][AG.KEYFRAME][AG.SELECT] == 0:
                dc.SetPen(wx.Pen("black",3))
                dc.SetBrush(wx.Brush("black"))
                dc.DrawCircle(pointX, pointY, 2)

            if list[i][AG.KEYFRAME][AG.SELECT] == 1:
                dc.SetPen(wx.Pen("cyan",3))
                dc.SetBrush(wx.Brush("cyan"))
                dc.DrawCircle(pointX, pointY, 2)

    def drawHandler(self, list, dc):
        for i in range(len(list)):
            if list[i][AG.KEYFRAME][AG.SELECT] == 1:
                X1 = list[i][AG.KEYFRAME][AG.LOCAL_VALUE][0]
                Y1 = list[i][AG.KEYFRAME][AG.LOCAL_VALUE][1]
                if self._OneTangent == True:
                    for j in range(3,5):
                        X = list[i][j][AG.LOCAL_VALUE][0]
                        Y = list[i][j][AG.LOCAL_VALUE][1]
                        if list[i][j][AG.SELECT] == 1:
                            dc.SetPen(wx.Pen("cyan",3))
                            dc.SetBrush(wx.Brush("cyan"))
                            dc.DrawCircle(X, Y, 2)

                            dc.SetPen(wx.Pen("cyan",1))
                            dc.DrawLine(X1, Y1, X, Y)

                        if list[i][j][AG.SELECT] == 0:
                            dc.SetPen(wx.Pen("brown",3))
                            dc.SetBrush(wx.Brush("brown"))
                            dc.DrawCircle(X, Y, 2)

                            dc.SetPen(wx.Pen("brown",1))
                            dc.DrawLine(X1, Y1, X, Y)

                if self._OneTangent == False:
                    if list[i][AG.IN_TANGENT][AG.SELECT] == 1:
                        X = list[i][AG.IN_TANGENT][AG.LOCAL_VALUE][0]
                        Y = list[i][AG.IN_TANGENT][AG.LOCAL_VALUE][1]
                        dc.SetPen(wx.Pen("cyan",3))
                        dc.SetBrush(wx.Brush("cyan"))
                        dc.DrawCircle(X, Y, 2)

                        dc.SetPen(wx.Pen("cyan",1))
                        dc.DrawLine(X1, Y1, X, Y)

                    if list[i][AG.IN_TANGENT][AG.SELECT] == 0:
                        X = list[i][AG.IN_TANGENT][AG.LOCAL_VALUE][0]
                        Y = list[i][AG.IN_TANGENT][AG.LOCAL_VALUE][1]
                        dc.SetPen(wx.Pen("navy",3))
                        dc.SetBrush(wx.Brush("navy"))
                        dc.DrawCircle(X, Y, 2)

                        dc.SetPen(wx.Pen("navy",1))
                        dc.DrawLine(X1, Y1, X, Y)

                    if list[i][AG.OUT_TANGENT][AG.SELECT] == 1:
                        X = list[i][AG.OUT_TANGENT][AG.LOCAL_VALUE][0]
                        Y = list[i][AG.OUT_TANGENT][AG.LOCAL_VALUE][1]
                        dc.SetPen(wx.Pen("cyan",3))
                        dc.SetBrush(wx.Brush("cyan"))
                        dc.DrawCircle(X, Y, 2)

                        dc.SetPen(wx.Pen("cyan",1))
                        dc.DrawLine(X1, Y1, X, Y)

                    if list[i][AG.OUT_TANGENT][AG.SELECT] == 0:
                        X = list[i][AG.OUT_TANGENT][AG.LOCAL_VALUE][0]
                        Y = list[i][AG.OUT_TANGENT][AG.LOCAL_VALUE][1]
                        dc.SetPen(wx.Pen("brown",3))
                        dc.SetBrush(wx.Brush("brown"))
                        dc.DrawCircle(X, Y, 2)

                        dc.SetPen(wx.Pen("brown",1))
                        dc.DrawLine(X1, Y1, X, Y)

    def DrawSelectRec(self, dc):
        if self._selectRec == True:
            dc.SetPen(wx.Pen("navy", 1))
            dc.SetBrush(wx.Brush("navy"))
            ## dc.SetLogicalFunction(wx.AND)
            dc.DrawLine(self.pos[0], self.pos[1], self.pos[0], self.newPos[1])
            dc.DrawLine(self.pos[0], self.pos[1], self.newPos[0], self.pos[1])
            dc.DrawLine(self.newPos[0], self.newPos[1], self.pos[0], self.newPos[1])
            dc.DrawLine(self.newPos[0], self.newPos[1], self.newPos[0], self.pos[1])

    def OnSize(self,evt):
        self.InitBuffer()

    def OnLeftDown(self,evt):
        point = (evt.GetX(), evt.GetY())

        if point[1]>= float(0) and point[1]<= float(self.h):
            if point[0]>= float(0) and point[0]<= float(self.w):
                self._mouseIn = True

        if self._mouseIn:
            self.CaptureMouse()
            self.pos = point

    def OnLeftUp(self,evt):
        if self.GetCapture():
            self.ReleaseMouse()
            self._mouseIn = False
            self._selectRec = False
            self.setSelection()
            self.SetGraphEditorData(self.property, self.curFrame)

    def OnMiddleDown(self,evt):
        point = (evt.GetX(), evt.GetY())

        if point[1]>= float(0) and point[1]<= float(self.h):
            if point[0]>= float(0) and point[0]<= float(self.w):
                self._mouseIn = True

        if self._mouseIn:
            self.CaptureMouse()
            self.midPos = point

    def OnMiddleUp(self, evt):
        if self.GetCapture():
            self.ReleaseMouse()

    def OnMotion(self,evt):
        self._mouseIn = False
        if evt.Dragging() and evt.LeftIsDown():
            self.newPos = (evt.GetX(), evt.GetY())
            if self.newPos[1]>= float(0) and self.newPos[1]<= float(self.h):
                if self.newPos[0]>= float(0) and self.newPos[0]<= float(self.w):
                    self._mouseIn = True

            if self._mouseIn:
                if self.newPos == self.pos:
                    evt.Skip()
                    self._mouseIn = False
                else:
                    self._selectRec = True
                    self.SetGraphEditorData(self.property,  self.curFrame)

        if evt.Dragging() and evt.MiddleIsDown():
            self.newMidPos = (evt.GetX(), evt.GetY())
            if self.newMidPos[1]>= float(0) and self.newMidPos[1]<= float(self.h):
                if self.newMidPos[0]>= float(0) and self.newMidPos[0]<= float(self.w):
                    self._mouseIn = True

            if self._mouseIn:
                if self.newMidPos == self.midPos:
                    evt.Skip()
                    self._mouseIn = False
                else:
                    self.recalculateSlope()
                    self.onAnimation()
                    self.midPos = self.newMidPos

        evt.Skip()
        self._mouseIn = False
        self._selectRec = False

    def setExistKey(self, list):
        flag = False
        for i in range(len(list)):
            if list[i][AG.KEYFRAME][AG.SELECT] == 1:
                inside = self.inside(self.pos, self.newPos, (list[i][AG.KEYFRAME][AG.LOCAL_VALUE][0], list[i][AG.KEYFRAME][AG.LOCAL_VALUE][1]))
                if inside == True:
                    list[i][AG.KEYFRAME][AG.SELECT] = 0
                if inside == False:
                    find = False
                    for j in range(3,5):
                        inside = self.inside(self.pos, self.newPos, (list[i][j][AG.LOCAL_VALUE][0], list[i][j][AG.LOCAL_VALUE][1]))
                        if inside == False:
                            list[i][j][AG.SELECT] = 0
                        if inside == True:
                            list[i][j][AG.SELECT] = 1
                            find = True
                            flag = True
                    if find == False:
                        list[i][AG.KEYFRAME][AG.SELECT] == 0

        return flag

    def setNewKey(self, list):
        for i in range(len(list)):
            inside = self.inside(self.pos, self.newPos, (list[i][2][0][0], list[i][2][0][1]))
            if inside == True:
                list[i][AG.KEYFRAME][AG.SELECT] = 1
            if inside == False:
                list[i][AG.KEYFRAME][AG.SELECT] = 0

    def setSelection(self):
        if self.property == self._mainDialog.namestr:
           self.setSelectionBase(self.X)
           self.setSelectionBase(self.Y)
           self.setSelectionBase(self.Z)
           return
        if self.property == property[AG.X]:
           self.setSelectionBase(self.X)
           return
        if self.property == property[AG.Y]:
           self.setSelectionBase(self.Y)
           return
        if self.property == property[AG.Z]:
           self.setSelectionBase(self.Z)
           return

    def setSelectionBase(self, list):
        self.setExistKey(list)
        if self.setExistKey(list) == True:
            return
        else:
            self.setNewKey(list)

    def inside(self, point0, point1, point):
        if point0[0]<=point1[0] and point0[1]<=point1[1]:
            if point0[0]<point[0] and point[0]<point1[0] and point0[1]<point[1] and point[1]<point1[1]:
                return True
            else:
                return False
        elif point1[0]<=point0[0] and point0[1]<=point1[1]:
            if point1[0]<point[0] and point[0]<point0[0] and point0[1]<point[1] and point[1]<point1[1]:
                return True
            else:
                return False
        elif point0[0]<=point1[0] and point1[1]<=point0[1]:
            if point0[0]<point[0] and point[0]<point1[0] and point1[1]<point[1] and point[1]<point0[1]:
                return True
            else:
                return False
        elif point1[0]<=point0[0] and point1[1]<=point0[1]:
            if point1[0]<point[0] and point[0]<point0[0] and point1[1]<point[1] and point[1]<point0[1]:
                return True
            else:
                return False
        else:
            return False

    def recalculateSlope(self):
        if self.property == self._mainDialog.namestr:
           self.recalculateSlopeBase(self.X)
           self.recalculateSlopeBase(self.Y)
           self.recalculateSlopeBase(self.Z)
           return
        if self.property == property[AG.X]:
           self.recalculateSlopeBase(self.X)
           return
        if self.property == property[AG.Y]:
           self.recalculateSlopeBase(self.Y)
           return
        if self.property == property[AG.Z]:
           self.recalculateSlopeBase(self.Z)
           return

    def recalculateSlopeBase(self, list):
        #recalculate the tangent slope
        moveX = self.newMidPos[0]-self.midPos[0]
        moveY = self.newMidPos[1]-self.midPos[1]

        for i in range(len(list)):
            if list[i][AG.KEYFRAME][AG.SELECT] == 1:
                if list[i][AG.IN_TANGENT][AG.SELECT] == 1:
                    newPointX = list[i][AG.IN_TANGENT][AG.LOCAL_VALUE][0] + moveX
                    newPointY = list[i][AG.IN_TANGENT][AG.LOCAL_VALUE][1] + moveY

                    newSlope = [list[i][AG.KEYFRAME][AG.LOCAL_VALUE][0] - newPointX , newPointY - list[i][AG.KEYFRAME][AG.LOCAL_VALUE][1]]

                    temp0 = self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]][AG.INSLOPE][0]
                    temp1 = self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]][AG.INSLOPE][1]
                    self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]][AG.INSLOPE][0] = newSlope[0]/self.unitWidth
                    self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]][AG.INSLOPE][1] = newSlope[1]/self.unitHeight
                    handler = self.generateHandler(self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]])
                    if handler[1][0][0]>= list[i][AG.KEYFRAME][AG.LOCAL_VALUE][0]:
                        self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]][AG.INSLOPE][0] = temp0
                        self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]][AG.INSLOPE][1] = temp1
                        return
                    if handler[1][0][0] < list[i][AG.KEYFRAME][AG.LOCAL_VALUE][0]:
                        if self._OneTangent == False:
                            list[i][AG.IN_TANGENT][0] = handler[1][0]
                            list[i][AG.IN_SLOPE][0] = handler[3][0]
                            list[i][AG.IN_SLOPE][1] = handler[3][1]

                        if self._OneTangent == True:
                            self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]][AG.OUTSLOPE][0] = newSlope[0]/self.unitWidth
                            self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]][AG.OUTSLOPE][1] = newSlope[1]/self.unitHeight
                            handler = self.generateHandler(self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]])
                            list[i][AG.IN_TANGENT][0] = handler[1][0]
                            list[i][AG.OUT_TANGENT][0] = handler[2][0]
                            list[i][AG.IN_SLOPE][0] = handler[3][0]
                            list[i][AG.IN_SLOPE][1] = handler[3][1]
                            list[i][AG.OUT_SLOPE][0] = handler[4][0]
                            list[i][AG.OUT_SLOPE][1] = handler[4][1]

                        self.SetGraphEditorData(self.property, self.curFrame)

                if list[i][AG.OUT_TANGENT][AG.SELECT] == 1:
                    newPointX = list[i][AG.OUT_TANGENT][AG.LOCAL_VALUE][0] + moveX
                    newPointY = list[i][AG.OUT_TANGENT][AG.LOCAL_VALUE][1] + moveY

                    newSlope = [newPointX  - list[i][AG.KEYFRAME][AG.LOCAL_VALUE][0] , list[i][AG.KEYFRAME][AG.LOCAL_VALUE][1] - newPointY]

                    temp0 = self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]][AG.OUTSLOPE][0]
                    temp1 = self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]][AG.OUTSLOPE][1]
                    self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]][AG.OUTSLOPE][0] = newSlope[0]/self.unitWidth
                    self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]][AG.OUTSLOPE][1] = newSlope[1]/self.unitHeight

                    handler = self.generateHandler(self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]])
                    if handler[2][0][0] <= list[i][AG.KEYFRAME][AG.LOCAL_VALUE][0]:
                        self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]][AG.OUTSLOPE][0] = temp0
                        self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]][AG.OUTSLOPE][1] = temp1
                        return
                    if handler[2][0][0] > list[i][AG.KEYFRAME][AG.LOCAL_VALUE][0]:
                        if self._OneTangent == False:
                            list[i][AG.OUT_TANGENT][0] = handler[2][0]
                            list[i][AG.OUT_SLOPE][0] = handler[4][0]
                            list[i][AG.OUT_SLOPE][1] = handler[4][1]

                        if self._OneTangent == True:
                            self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]][AG.INSLOPE][0] = newSlope[0]/self.unitWidth
                            self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]][AG.INSLOPE][1] = newSlope[1]/self.unitHeight
                            handler = self.generateHandler(self._mainDialog.editor.animMgr.keyFramesInfo[list[i][AG.KEY]][list[i][AG.I]])
                            list[i][AG.IN_TANGENT][0] = handler[1][0]
                            list[i][AG.OUT_TANGENT][0] = handler[2][0]
                            list[i][AG.IN_SLOPE][0] = handler[3][0]
                            list[i][AG.IN_SLOPE][1] = handler[3][1]
                            list[i][AG.OUT_SLOPE][0] = handler[4][0]
                            list[i][AG.OUT_SLOPE][1] = handler[4][1]

                        self.SetGraphEditorData(self.property,  self.curFrame)

    def selectHandler(self):
        self._selectHandler = False

    def onAnimation(self):
        if self._mainDialog.editor.mode == self._mainDialog.editor.ANIM_MODE:
            self._mainDialog.editor.ui.animUI.OnAnimation(self._mainDialog.editor.ui.animUI.curFrame)
        else:
            pass


class GraphEditorUI(wx.Dialog):
    """
    This is the graph editor main class implementation.
    """
    def __init__(self, parent, editor, object):
        wx.Dialog.__init__(self, parent, id=wx.ID_ANY, title="Graph Editor",
                           pos=wx.DefaultPosition, size=(735, 535))

        self.editor = editor
        self.editor.GRAPH_EDITOR = True
        self.object = object
        self.xRange = 24+1
        self.yRange = 50
        if self.editor.mode == self.editor.ANIM_MODE:
            self.curFrame = self.editor.ui.animUI.curFrame
        self.curFrame = 1

        self.mainPanel1 = wx.Panel(self, -1)

        bmpZoomIn = ZoomIn.GetBitmap()
        bmpZoomOut = ZoomOut.GetBitmap()
        bmpOneTangent = OneTangent.GetBitmap()
        bmpTwoTangents = TwoTangents.GetBitmap()

        self.buttonZoomIn = wx.BitmapButton(self.mainPanel1, -1, bmpZoomIn, size = (30,30),style = wx.BU_AUTODRAW)
        self.buttonZoomOut = wx.BitmapButton(self.mainPanel1, -1, bmpZoomOut, size = (30,30),style = wx.BU_AUTODRAW)
        self.buttonOneTangent = wx.BitmapButton(self.mainPanel1, -1, bmpOneTangent, size = (30,30),style = wx.BU_AUTODRAW)
        self.buttonTwoTangents = wx.BitmapButton(self.mainPanel1, -1, bmpTwoTangents, size = (30,30),style = wx.BU_AUTODRAW)

        self.mainPanel2 = wx.Panel(self, -1)

        self.tree =  self.tree = wx.TreeCtrl(self.mainPanel2, id=-1, pos=wx.DefaultPosition,size=wx.Size(200, 450), style=wx.TR_MULTIPLE|wx.TR_DEFAULT_STYLE,validator=wx.DefaultValidator, name="treeCtrl")
        self.namestr = "%s"%(object[OG.OBJ_DEF].name)
        self.root = self.tree.AddRoot(self.namestr)
        self.AddTreeNodes(self.root, property)
        self.tree.Expand(self.root)
        self.tree.SelectItem(self.root,select=True)
        self.str = self.tree.GetItemText(self.root)

        self.graphEditorWindow =GraphEditorWindow(self.mainPanel2, wx.Size(500, 450), str(object[OG.OBJ_DEF].name), self.xRange, self.yRange, self.curFrame, self.object)

        self.SetProperties()
        self.DoLayout()

        self.Bind(wx.EVT_BUTTON, self.OnZoomIn, self.buttonZoomIn)
        self.Bind(wx.EVT_BUTTON, self.OnZoomOut, self.buttonZoomOut)
        self.Bind(wx.EVT_BUTTON, self.OnOneTangent, self.buttonOneTangent)
        self.Bind(wx.EVT_BUTTON, self.OnTwoTangents, self.buttonTwoTangents)

        self.Bind(wx.EVT_CLOSE, self.OnExit)
        self.tree.Bind(wx.EVT_TREE_SEL_CHANGED, self.OnSelChanged)

    def SetProperties(self):
        pass

    def DoLayout(self):
        dialogSizer = wx.BoxSizer(wx.VERTICAL)
        mainSizer1 = wx.FlexGridSizer(1, 4, 0, 0)
        mainSizer2 = wx.FlexGridSizer(1, 2, 0, 0)

        mainSizer1.Add(self.buttonOneTangent, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT, 570)
        mainSizer1.Add(self.buttonTwoTangents, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT, 10)
        mainSizer1.Add(self.buttonZoomIn, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT, 10)
        mainSizer1.Add(self.buttonZoomOut, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 10)

        mainSizer2.Add(self.tree, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 10)
        mainSizer2.Add(self.graphEditorWindow, 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 10)

        self.mainPanel1.SetSizerAndFit(mainSizer1)
        self.mainPanel2.SetSizer(mainSizer2)

        dialogSizer.Add(self.mainPanel2, 0, wx.ALIGN_CENTER_VERTICAL|wx.TOP, 10)
        dialogSizer.Add(self.mainPanel1, 0, wx.ALIGN_CENTER_VERTICAL|wx.TOP, 5)

        self.SetSizer(dialogSizer)
        self.Layout()

        self.dialogSizer = dialogSizer

    def AddTreeNodes(self, parentItem, items):
        for item in items:
            if type(item) == str:
                self.tree.AppendItem(parentItem, item)

    def OnSelChanged(self, evt):
        item = evt.GetItem()
        if item:
            self.str = self.tree.GetItemText(item)
            self.graphEditorWindow.refresh()
            self.graphEditorWindow.SetGraphEditorData(self.str, self.curFrame)

    def OnZoomIn(self,evt):
        self.graphEditorWindow.zoom = self.graphEditorWindow.zoom/float(1.2)
        self.graphEditorWindow.unitWidth = self.graphEditorWindow.unitWidth*float(1.2)
        self.graphEditorWindow.unitHeight = self.graphEditorWindow.unitHeight*float(1.2)
        self.graphEditorWindow.generateInfo()
        self.graphEditorWindow.SetGraphEditorData(self.str, self.curFrame)

    def OnZoomOut(self,evt):
        self.graphEditorWindow.zoom = self.graphEditorWindow.zoom*float(1.2)
        self.graphEditorWindow.unitWidth = self.graphEditorWindow.unitWidth/float(1.2)
        self.graphEditorWindow.unitHeight = self.graphEditorWindow.unitHeight/float(1.2)
        self.graphEditorWindow.generateInfo()
        self.graphEditorWindow.SetGraphEditorData(self.str, self.curFrame)

    def OnOneTangent(self,evt):
        self.graphEditorWindow._OneTangent = True
        self.graphEditorWindow.SetGraphEditorData(self.str, self.curFrame)

    def OnTwoTangents(self,evt):
        self.graphEditorWindow._OneTangent = False
        self.graphEditorWindow.SetGraphEditorData(self.str, self.curFrame)

    def curFrameChange(self):
        if self.editor.mode == self.editor.ANIM_MODE:
            self.curFrame = self.editor.ui.animUI.curFrame
            self.graphEditorWindow.SetGraphEditorData(self.str, self.curFrame)
        else:
            pass

    def OnExit(self,evt):
        self.Destroy()
        self.editor.ui.graphEditorMenuItem.Check(False)
        self.object = None
        self.editor.GRAPH_EDITOR = False








