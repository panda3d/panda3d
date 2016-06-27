from direct.wxwidgets.WxAppShell import *
import os
from . import ObjectGlobals as OG

CLOSE_STDIN = "<CLOSE STDIN>"

class StartupError(Exception):
    pass

class Process:
    def __init__(self, parent, cmd, end_callback):
        self.process = wx.Process(parent)
        self.process.Redirect()
        self.process.pid = wx.Execute(cmd, wx.EXEC_ASYNC|wx.EXEC_MAKE_GROUP_LEADER, self.process)
        self.b = []
        if self.process.pid:
            #what was up with wx.Process.Get*Stream names?
            self.process._stdin_ = self.process.GetOutputStream()
            self.process._stdout_ = self.process.GetInputStream()
            self.process._stderr_ = self.process.GetErrorStream()
            self.process.Bind(wx.EVT_END_PROCESS, end_callback)
            return
        raise StartupError

    def Poll(self, input=''):
        if (input or self.b) and self.process and self.process._stdin_:
            if self.b or len(input) > 512:
                if input:
                    #if we don't chop up our input into resonably sized chunks,
                    #some platforms (like Windows) will send some small number
                    #of bytes per .write() call (sometimes 2 in the case of
                    #Windows).
                    self.b.extend([input[i:i+512] for i in range(0, len(input), 512)])
                input = self.b.pop(0)
            self.process._stdin_.write(input)
            if hasattr(self.process._stdin_, "LastWrite"):
                y = self.process._stdin_.LastWrite()
                if y != len(input):
                    self.b.insert(0, input[y:])
        x = []
        for s in (self.process._stderr_, self.process._stdout_):
            if s and s.CanRead():
                x.append(s.read())
            else:
                x.append('')
        return x

    def CloseInp(self):
        if self.process and self.process._stdin_:
            self.process.CloseOutput()
            self.process._stdin_ = None

    def Kill(self, ks='SIGKILL'):
        errors = {wx.KILL_BAD_SIGNAL: "KILL_BAD_SIGNAL",
                  wx.KILL_ACCESS_DENIED: "KILL_ACCESS_DENIED",
                  wx.KILL_ERROR: "KILL_ERROR"}
        if self.process:
            if ks == CLOSE_STDIN:
                self.CloseInp()
                return 1, None
            elif wx.Process.Exists(self.process.pid):
                signal = getattr(wx, ks)
                r = wx.Process.Kill(self.process.pid, signal, flags=wx.KILL_CHILDREN)
            else:
                r = 65535
                self.CloseInp()
                return 1, None

            if r not in (wx.KILL_OK, wx.KILL_NO_PROCESS, 65535):
                return 0, (self.process.pid, signal, errors.get(r, "UNKNOWN_KILL_ERROR %s"%r))
            else:
                return 1, None

FROM_MAYA_TO_EGG = 0
FROM_BAM_TO_MAYA = 1

class MayaConverter(wx.Dialog):
    def __init__(self, parent, editor, mayaFile, callBack=None, obj=None, isAnim=False, convertMode=FROM_MAYA_TO_EGG):
        wx.Dialog.__init__(self, parent, id=wx.ID_ANY, title="Maya Converter",
                           pos=wx.DefaultPosition, size=(300, 200))

        self.editor = editor
        self.obj = obj
        self.isAnim = isAnim
        self.callBack = callBack
        self.mayaFile = mayaFile

        self.mainPanel = wx.Panel(self, -1)
        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.mainPanel, 1, wx.EXPAND, 0)
        self.SetSizer(sizer)

        self.output = wx.TextCtrl(self.mainPanel, -1, style = wx.TE_MULTILINE, pos = (0, 0), size = (100, 400))
        sizer2 = wx.BoxSizer(wx.VERTICAL)
        sizer2.Add(self.output, 1, wx.EXPAND, 0)
        self.mainPanel.SetSizer(sizer2)

        if convertMode == FROM_MAYA_TO_EGG:
            self.convertFromMaya()
        elif convertMode == FROM_BAM_TO_MAYA:
            self.convertToMaya()
        else:
            pass

        self.timer = wx.Timer(self, -1)
        self.Bind(wx.EVT_TIMER, self.onPoll, self.timer)
        self.timer.Start(100)

    def convertFromMaya(self):
        if self.isAnim:
            if self.obj:
                command = 'maya2egg -uo ft -a chan %s -o %s.anim.egg'%(self.mayaFile, self.mayaFile)
                self.process = Process(self, command, lambda p0=None: self.onProcessEnded(p0))
            else:
                command = 'maya2egg -uo ft -a model %s -o %s.model.egg'%(self.mayaFile, self.mayaFile)
                self.process = Process(self, command, lambda p0=None: self.onModelProcessEnded(p0))
        else:
            command = 'maya2egg -uo ft %s -o %s.egg'%(self.mayaFile, self.mayaFile)
            self.process = Process(self, command, lambda p0=None: self.onProcessEnded(p0))

    def convertToMaya(self):
        bamFileName = self.mayaFile + ".bam"
        eggFileName = self.mayaFile + ".egg"
        command = 'bam2egg %s -o %s'%(bamFileName, eggFileName)
        self.process = Process(self, command, lambda p0=None: self.onBam2EggEnded(p0))

    def onEgg2MayaEnded(self, evt):
        self.process.CloseInp()
        for i in self.process.Poll():
            self.output.AppendText(i)
        self.process = None

    def onBam2EggEnded(self, evt):
        self.process.CloseInp()
        for i in self.process.Poll():
            self.output.AppendText(i)
        eggFileName = self.mayaFile + ".egg"
        command = 'egg2maya -ui ft -uo ft %s -o %s'%(eggFileName, self.mayaFile)
        self.process = Process(self, command, lambda p0=None: self.onEgg2MayaEnded(p0))

    def onPoll(self, evt):
        if self.process:
            for i in self.process.Poll():
                self.output.AppendText(i)

    def onModelProcessEnded(self, evt):
        self.process.CloseInp()
        for i in self.process.Poll():
            self.output.AppendText(i)
        self.process = None
        command = 'maya2egg -uo ft -a chan %s -o %s.anim.egg'%(self.mayaFile, self.mayaFile)
        self.process = Process(self, command, lambda p0 = None: self.onProcessEnded(p0))

    def onProcessEnded(self, evt):
        self.process.CloseInp()
        for i in self.process.Poll():
            self.output.AppendText(i)

        self.output.AppendText('Converting %s is finished\n'%self.mayaFile)
        self.process = None

        name = os.path.basename(self.mayaFile)
        if self.isAnim:
            if self.obj:
                objDef = self.obj[OG.OBJ_DEF]
                objNP = self.obj[OG.OBJ_NP]
                animName = "%s.anim.egg"%self.mayaFile
                if animName not in objDef.anims:
                    objDef.anims.append(animName)
                name = os.path.basename(animName)
                objNP.loadAnims({name:animName})
                objNP.loop(name)
                self.obj[OG.OBJ_ANIM] = animName
                self.editor.ui.objectPropertyUI.updateProps(self.obj)
                return
            else:
                modelName = "%s.model.egg"%self.mayaFile
                animName = "%s.anim.egg"%self.mayaFile
                result = [name, modelName, animName]
        else:
            modelName = "%s.egg"%self.mayaFile
            result = [name, modelName]

        if self.callBack:
            self.callBack(result)

