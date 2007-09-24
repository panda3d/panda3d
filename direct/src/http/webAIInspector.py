"""This is a web based inspector for the AI System. It can be accessed via
http://hostname.domain:port/ai

The hostname.domain would of course be the computer that the AI is running on.
The port will need to be defined when the instance is inited.

"""

import string, time, direct, inspect
from direct.directnotify.DirectNotifyGlobal import directNotify
from direct.http import WebRequest
from socket import gethostname
from direct.task.Task import Task
from sys import platform

# Need to figure out which systeminfo module to import
if platform == 'win32':
    from windowsSystemInfo import SystemInformation
else:
    from linuxSystemInfo import SystemInformation

if __debug__:
    notify = directNotify.newCategory('AIWebInterface')

class aiWebServer(SystemInformation):
    def __init__(self, air, listenPort=8080):
        SystemInformation.__init__(self)
        self.listenPort = listenPort
        # self.taskMgr = Task.TaskManager()
        if __debug__:
            print "Listen port set to: %d" % self.listenPort
        # Start dispatcher
        self.web = WebRequest.WebRequestDispatcher()
        self.web.listenOnPort(self.listenPort)
        self.localHostName = gethostname()
        self.web.registerGETHandler('inspect', self.inspect)
        self.web.registerGETHandler('systemInfo', self.systemInfo)
        self.web.registerGETHandler('oType', self.oType)
        self.web.registerGETHandler('oInst', self.oInst)
        self.web.registerGETHandler('blank', self.blank)
        self.startCheckingIncomingHTTP()

    def timeStamp(self):
        # Returns the local time in the following string format:
        # Month-Day-Year Hour:Minute:Seconds
        # Example: 09-17-2007 15:36:04
        return time.strftime("%m-%d-%Y %H:%M:%S", time.localtime())

    def blank(self, replyTo, **kw):
        # This simple generates a blank page for the middle and right
        # frames;( for when the page is first accessed)
        replyTo.respond('<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">\n<html>\n<head>\n<meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">\n<TITLE>BLANK</title>\n</head><body></body>\n</HTML>')

    def oInst(self, replyTo, **kw):
        # This will populate the middle frame with list of the members of
        # the object selected in the left frame

        #print "%s|oInst Frame Accessed, Request ID %s" % (self.timeStamp(), str(kw))
        
        head = '<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">\n<html>\n<head>\n<meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">\n<title>member List</title>\n</head>\n<body>\n<UL>'
        foot = '</ul></body></HTML>'
        body = ''
        doIdRequested = ''
        for j, k in kw.iteritems():
            doIdRequested = int(k)
            #print j,k
        memberList = inspect.getmembers(simbase.air.doId2do[doIdRequested])
        memberList.sort()
        memberList.reverse()
        while memberList:
             tempMember = memberList.pop()
             if (type(tempMember[1]) == str or type(tempMember[1]) == int or type(tempMember[1]) == float or type(tempMember[1]) == dict) and (tempMember[0] != '__doc__') or (tempMember[0] != 'DtoolClassDict') :
                 body = '%s<LI>%s\n' % (body, str(tempMember))
        replyTo.respond('%s%s%s' % (head,body,foot))
        

    def oType(self, replyTo, **kw):
        # This will populate the left frame with a alpha sorted list of
        # objects.

        #print "%s|oType Frame Accessed" % self.timeStamp()
        head = '<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">\n<html>\n<head>\n<meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">\n<title>Object List</title>\n</head>\n<body>\n<UL>'
        foot = '</ul></body></HTML>'
        objList = self.generateSortedIDList()
        objList.reverse()
        body = ''
        # Pop off the Null entry
        while objList:
            tempObjElement = objList.pop()
            #tempObjElement[0].replace('<','')
            #tempObjElement[0].replace('>','')
            if str(tempObjElement[0]).find('render') == -1:
                body = '%s<LI><A HREF="oInst?id=%s" target="oInst">%s</A>\n' % (body, tempObjElement[1], str(tempObjElement[0]).replace('<','').replace('>',''))
        replyTo.respond('%s%s%s' % (head,body,foot))    

    def inspect(self, replyTo, **kw):
        # This is the index. Basically, it will generate the frames for the
        # other functions to populate: systemInfo, oType, oInst, oAttrib
        # Three frames on the bottom row
        # frameset = '<frameset rows="35\%,65\%">\n<frame src="systemInfo" name="systemInfo" frameborder=1>\n<frameset cols="25\%,25\%,50\%">\n<frame src="oType" name="oType" frameborder=1>\n<frame src="blank" name="oInst" frameborder=1>\n<frame src="blank" name="oAttrib" frameborder=1>\n</frameset>\n</frameset>\n</html>'
        # Two Frames on the bottom row
        frameset = '<frameset rows="35\%,65\%">\n<frame src="systemInfo" name="systemInfo" frameborder=1>\n<frameset cols="50\%,50\%">\n<frame src="oType" name="oType" frameborder=1>\n<frame src="blank" name="oInst" frameborder=1>\n</frameset>\n</frameset>\n</html>'
        #print "%s|Index Frame Accessed" % self.timeStamp()
        # print str(simbase.air.doid2do)
        replyTo.respond('<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN">\n<html lang="en">\n<head>\n<title>AI HTTP Interface: %s</title>\n</head>\n%s' % (self.localHostName, frameset))

    def systemInfo(self, replyTo, **kw):
        # This is the contents of the top frame; i.e. system information

        self.refresh()
        #print "%s|SystemInfo Frame Accessed" % self.timeStamp()
        replyTo.respond('<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">\n<html>\n<head>\n<meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">\n<title>System Info</title>\n</head>\n<body>\n<center><table style="text-align: left; width: 443px; height: 128px;" border="1" cellpadding="2" cellspacing="2">\n<tbody>\n<tr>\n<td style="text-align: center;" colspan="4">Hostname: %s<br>\nOperating System: %s<br>\nCPU: %s</td>\n</tr>\n<tr>\n<td>Total RAM:</td>\n<td>%d</td>\n<td>Total VM</td>\n<td>%d</td>\n</tr>\n<tr>\n<td>Available RAM:</td>\n<td>%d</td>\n<td>Available VM</td>\n<td>%d</td>\n</tr>\n</tbody>\n</table></center>\n</body>\n</html>' % (self.localHostName, self.os, self.cpu, self.totalRAM, self.totalVM, self.availableRAM, self.availableVM))

    def startCheckingIncomingHTTP(self):
        taskMgr.remove('pollHTTPTask')
        taskMgr.doMethodLater(0.3,self.pollHTTPTask,'pollHTTPTask')

    def stopCheckingIncomingHTTP(self):
        taskMgr.remove('pollHTTPTask')

    def pollHTTPTask(self,task):
        """
        Task that polls the HTTP server for new requests.
        """
        # print 'Polling...'
        self.web.poll()
        #taskMgr.doMethodLater(0.3,self.pollHTTPTask,'pollHTTPTask')
        return Task.again

    def generateSortedIDList(self):
        # looks at the simbase.air.doID2do dict, and returns a list
        # sorted by alpha order.
        IDlist = []
        for key, val in simbase.air.doId2do.iteritems():
            IDlist.append([val,key])
        IDlist.sort()
        return IDlist


def inspectObject(anObject):
    inspector = inspectorFor(anObject)
    # inspectorWindow = InspectorWindow(inspector)
    # inspectorWindow.open()
    # return inspectorWindow
    return inspector

### private

def inspectorFor(anObject):
    typeName = string.capitalize(type(anObject).__name__) + 'Type'
    if _InspectorMap.has_key(typeName):
        inspectorName = _InspectorMap[typeName]
    else:
        print "Can't find an inspector for " + typeName
        inspectorName = 'Inspector'
    inspector = eval(inspectorName + '(anObject)')
    return inspector

def initializeInspectorMap():
    global _InspectorMap
    notFinishedTypes = ['BufferType',  'EllipsisType',  'FrameType', 'TracebackType', 'XRangeType']

    _InspectorMap = {
        'Builtin_function_or_methodType': 'FunctionInspector',
        'BuiltinFunctionType': 'FunctionInspector',
        'BuiltinMethodType': 'FunctionInspector',
        'ClassType': 'ClassInspector',
        'CodeType': 'CodeInspector',
        'ComplexType': 'Inspector',
        'DictionaryType': 'DictionaryInspector',
        'DictType': 'DictionaryInspector',
        'FileType': 'Inspector',
        'FloatType': 'Inspector', 
        'FunctionType': 'FunctionInspector',
        'Instance methodType': 'InstanceMethodInspector',
        'InstanceType': 'InstanceInspector',
        'IntType': 'Inspector',
        'LambdaType': 'Inspector',
        'ListType': 'SequenceInspector',
        'LongType': 'Inspector',
        'MethodType': 'FunctionInspector',
        'ModuleType': 'ModuleInspector',
        'NoneType': 'Inspector',
        'SliceType': 'SliceInspector',
        'StringType': 'SequenceInspector',
        'TupleType': 'SequenceInspector',
        'TypeType': 'Inspector',
         'UnboundMethodType': 'FunctionInspector',
        'DistributedshipcannonaiType': 'ClassInspector'}

    for each in notFinishedTypes:
        _InspectorMap[each] = 'Inspector'

class Inspector:
    def __init__(self, anObject):
        self.object = anObject
        self.lastPartNumber = 0
        self.initializePartsList()
        self.initializePartNames()

    def __str__(self):
        return __name__ + '(' + str(self.object) + ')'

    def initializePartsList(self):
        self._partsList = []
        keys = self.namedParts()
        keys.sort()
        for each in keys:
            self._partsList.append(each)
            #if not callable(eval('self.object.' + each)):
            #    self._partsList.append(each)  

    def initializePartNames(self):
        self._partNames = ['up'] + map(lambda each: str(each), self._partsList)

    def title(self):
        "Subclasses may override."
        return string.capitalize(self.objectType().__name__)

    def getLastPartNumber(self):
        return self.lastPartNumber

    def selectedPart(self):
        return self.partNumber(self.getLastPartNumber())
        
    def namedParts(self):
        return dir(self.object)

    def stringForPartNumber(self, partNumber):
        object = self.partNumber(partNumber)
        doc = None
        if callable(object):
            try:
                doc = object.__doc__
            except:
                pass
        if doc:
            return (str(object) + '\n' + str(doc))
        else:
            return str(object)

    def partNumber(self, partNumber):
        self.lastPartNumber = partNumber
        if partNumber == 0:
            return self.object
        else:
            part = self.privatePartNumber(partNumber)
            return eval('self.object.' + part)

    def inspectorFor(self, part):
        return inspectorFor(part)

    def privatePartNumber(self, partNumber):
        return self._partsList[partNumber - 1]        

    def partNames(self):
        return self._partNames
    
    def objectType(self):
        return type(self.object)

###
    
class ModuleInspector(Inspector):
    def namedParts(self):
        return ['__dict__']

class ClassInspector(Inspector):
    def namedParts(self):
        return ['__bases__'] + self.object.__dict__.keys()

    def title(self):
        return self.object.__name__ + ' Class'

class InstanceInspector(Inspector):
    def title(self):
        return self.object.__class__.__name__
    def namedParts(self):
        return ['__class__'] + dir(self.object)

###
    
class FunctionInspector(Inspector):
    def title(self):
        return self.object.__name__ + "()"

class InstanceMethodInspector(Inspector):
    def title(self):
        return str(self.object.im_class) + "." + self.object.__name__ + "()"

class CodeInspector(Inspector):
    def title(self):
        return str(self.object)

###

class ComplexInspector(Inspector):
    def namedParts(self):
        return ['real', 'imag']

###

class DictionaryInspector(Inspector):

    def initializePartsList(self):
        Inspector.initializePartsList(self)
        keys = self.object.keys()
        keys.sort()
        for each in keys:
            self._partsList.append(each)

    def partNumber(self, partNumber):
        self.lastPartNumber = partNumber
        if partNumber == 0:
            return self.object
        key = self.privatePartNumber(partNumber)
        if self.object.has_key(key):
            return self.object[key]
        else:
            return eval('self.object.' + key)
        
class SequenceInspector(Inspector):
    def initializePartsList(self):
        Inspector.initializePartsList(self)
        for each in range(len(self.object)):
            self._partsList.append(each)

    def partNumber(self, partNumber):
        self.lastPartNumber = partNumber
        if partNumber == 0:
            return self.object
        index = self.privatePartNumber(partNumber)
        if type(index) == IntType:
            return self.object[index]
        else:
            return eval('self.object.' + index)
    
class SliceInspector(Inspector):
    def namedParts(self):
        return ['start', 'stop', 'step']

### Initialization
initializeInspectorMap()
