

from .DistributedObjectUD import DistributedObjectUD
from direct.directnotify.DirectNotifyGlobal import directNotify

import sys

class DistributedObjectGlobalUD(DistributedObjectUD):
    notify = directNotify.newCategory('DistributedObjectGlobalUD')

    doNotDeallocateChannel = 1
    isGlobalDistObj = 1

    def __init__(self, air):
        DistributedObjectUD.__init__(self, air)
        self.ExecNamespace = {"self":self}

    def announceGenerate(self):
        self.air.registerForChannel(self.doId)
        DistributedObjectUD.announceGenerate(self)

    def delete(self):
        self.air.unregisterForChannel(self.doId)
        ## self.air.removeDOFromTables(self)
        DistributedObjectUD.delete(self)

    def execCommand(self, command, mwMgrId, avId, zoneId):
        text = str(self.__execMessage(command))[:config.GetInt("ai-debug-length",300)]
        self.notify.info(text)

    def __execMessage(self, message):
        if not self.ExecNamespace:
            # Import some useful variables into the ExecNamespace initially.
            import panda3d.core

            for key, value in panda3d.core.__dict__.items():
                if not key.startswith('__'):
                    self.ExecNamespace[key] = value
            #self.importExecNamespace()

        # Now try to evaluate the expression using ChatInputNormal.ExecNamespace as
        # the local namespace.
        try:
            return str(eval(message, globals(), self.ExecNamespace))

        except SyntaxError:
            # Maybe it's only a statement, like "x = 1", or
            # "import math".  These aren't expressions, so eval()
            # fails, but they can be exec'ed.
            try:
                exec(message, globals(), self.ExecNamespace)
                return 'ok'
            except:
                exception = sys.exc_info()[0]
                extraInfo = sys.exc_info()[1]
                if extraInfo:
                    return str(extraInfo)
                else:
                    return str(exception)
        except:
            exception = sys.exc_info()[0]
            extraInfo = sys.exc_info()[1]
            if extraInfo:
                return str(extraInfo)
            else:
                return str(exception)
