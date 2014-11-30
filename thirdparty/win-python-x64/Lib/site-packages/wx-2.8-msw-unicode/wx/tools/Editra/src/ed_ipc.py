###############################################################################
# Name: ed_ipc.py                                                             #
# Purpose: Editra IPC client/server                                           #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008-2009 Cody Precord <staff@editra.org>                    #
# License: wxWindows License                                                  #
###############################################################################

"""
Classes and utilities for handling IPC between running instances of Editra. The
IPC is done through sockets using the TCP protocol. Message packets have a
specified format and authentication method that is described in L{EdIpcServer}.

Remote Control Protocol:

This server and its relationship with the main application object allows for
some limited remote control of Editra. The server's basic message protocol
requirements are as follows.

SESSION_KEY;xml;MSGEND

Where the SESSION_KEY is the unique authentication key created by the app that
started the server. This key is stored in the user profile and only valid for
the current running session of Editra. The MSGEND indicator is the L{MSGEND}
string defined in this file (*EDEND*). If both of these parts of the message
are found and correct the server will forward the messages that are packed in
between to the app.

Message Format:

<edipc>
   <filelist>
      <file name="absolute_filepath"/>
   </filelist>
   <arglist>
      <arg name="g" value="2"/>
   </arglist>
</edipc>

@summary: Editra's IPC Library

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: ed_ipc.py 67991 2011-06-20 23:48:01Z CJP $"
__revision__ = "$Revision: 67991 $"

#-----------------------------------------------------------------------------#
# Imports
import sys
import wx
import threading
import socket
import time
#import select

# Editra Libs
import util
import ed_xml
import ebmlib

#-----------------------------------------------------------------------------#
# Globals

# Port choosing algorithm ;)
EDPORT = (10 * int('ed', 16) + sum(ord(x) for x in "itr") + int('a', 16)) * 10
MSGEND = "*EDEND*"

# Xml Implementation
EDXML_IPC       = "edipc"
EDXML_FILELIST  = "filelist"
EDXML_FILE      = "file"
EDXML_ARGLIST   = "arglist"
EDXML_ARG       = "arg"

#-----------------------------------------------------------------------------#

edEVT_COMMAND_RECV = wx.NewEventType()
EVT_COMMAND_RECV = wx.PyEventBinder(edEVT_COMMAND_RECV, 1)
class IpcServerEvent(wx.PyCommandEvent):
    """Event to signal the server has recieved some commands"""
    def __init__(self, etype, eid, values=None):
        """Creates the event object"""
        wx.PyCommandEvent.__init__(self, etype, eid)
        self._value = values

    def GetCommands(self):
        """Returns the list of commands sent to the server
        @return: the value of this event

        """
        return self._value

#-----------------------------------------------------------------------------#

class EdIpcServer(threading.Thread):
    """Create an instance of IPC server for Editra. IPC is handled through
    a socket connection to an instance of this server listening on L{EDPORT}.
    The server will recieve commands and dispatch them to the app.
    Messages sent to the server must be in the following format.
    
      AuthenticationKey;Message Data;MSGEND

    The _AuthenticationKey_ is the same as the key that started the server it
    is used to validate that messages are coming from a legitimate source.

    _Message Data_ is a string of data where items are separated by a single
    ';' character. If you use L{SendCommands} to communicate with the server
    then this message separators are handled internally by that method.

    L{MSGEND} is the token to signify that the client is finished sending
    commands to the server. When using L{SendCommands} this is also 
    automatically handled.

    @todo: investigate possible security issues

    """
    def __init__(self, app, key, port=EDPORT):
        """Create the server thread
        @param app: Application object the server belongs to
        @param key: Unique user authentication key (string)
        @keyword port: TCP port to attempt to connect to

        """
        super(EdIpcServer, self).__init__()

        # Attributes
        self._exit = False
        self.__key = key
        self.app = app
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # Setup
        ## Try new ports till we find one that we can use
        while True:
            try:
                self.socket.bind(('127.0.0.1', port))
                break
            except:
                port += 1

        global EDPORT
        EDPORT = port
        self.socket.listen(5)

    def Shutdown(self):
        """Tell the server to exit"""
        self._exit = True
        # Wake up the server in case its waiting
        # TODO: should add a specific exit event message
        SendCommands(IPCCommand(), self.__key)

    def run(self):
        """Start the server. The server runs in blocking mode, this
        shouldn't be an issue as it should rarely need to respond to
        anything.

        """
        while not self._exit:
            try:
                client, addr = self.socket.accept()

                if self._exit:
                    break

                # Block for up to 2 seconds while reading
                start = time.time()
                recieved = u''
                while time.time() < start + 2:
                    recieved += client.recv(4096)
                    if recieved.endswith(MSGEND):
                        break

                # If message key is correct and the message is ended, process
                # the input and dispatch to the app.
                if recieved.startswith(self.__key) and recieved.endswith(MSGEND):
                    # Strip the key
                    recieved = recieved.replace(self.__key, '', 1)
                    # Strip the end token
                    xmlstr = recieved.rstrip(MSGEND).strip(";")

                    # Parse the xml
                    exml = IPCCommand()
                    try:
                        # Well formed xml must be utf-8 string not unicode
                        xmlstr = xmlstr.encode('utf-8')
                        exml = IPCCommand.parse(xmlstr)
                    except Exception, msg:
                        # Log and ignore parsing errors
                        logmsg = "[ed_ipc][err] Parsing failed: %s\n" % msg
                        xmlstr = xmlstr.replace('\n', '').strip()
                        logmsg += "Bad xml was: %s" % repr(xmlstr)
                        util.Log(logmsg)
                        continue

                    evt = IpcServerEvent(edEVT_COMMAND_RECV, wx.ID_ANY, exml)
                    wx.CallAfter(wx.PostEvent, self.app, evt)
            except socket.error:
                # TODO: Better error handling
                self._exit = True

        # Shutdown Server
        try:
            self.socket.shutdown(socket.SHUT_RDWR)
        except:
            pass

        self.socket.close()

#-----------------------------------------------------------------------------#

def SendCommands(xmlobj, key):
    """Send commands to the running instance of Editra
    @param xmlobj: EditraXml Object
    @param key: Server session authentication key
    @return: bool

    """
    assert isinstance(xmlobj, ed_xml.EdXml), "SendCommands expects an xml object"

    # Build the edipc protocol msg
    cmds = list()
    cmds.insert(0, key)
    cmds.append(xmlobj.GetXml())
    cmds.append(MSGEND)

    try:
        # Setup the client socket
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.connect(('127.0.0.1', EDPORT))

        # Server expects commands delimited by ;
        message = ";".join(cmds)
        client.send(message)
        client.shutdown(socket.SHUT_RDWR)
        client.close()
    except Exception, msg:
        return False
    else:
        return True

#-----------------------------------------------------------------------------#
# Command Serialization

class IPCFile(ed_xml.EdXml):
    """Xml object for holding the list of files
    <file value="/path/to/file"/>

    """
    class meta:
        tagname = EDXML_FILE
    value = ed_xml.String(required=True)

class IPCArg(ed_xml.EdXml):
    """Xml object for holding the list of args
       <arg name="test" value="x"/>

    """
    class meta:
        tagname = EDXML_ARG
    name = ed_xml.String(required=True)
    value = ed_xml.String(required=True)

class IPCCommand(ed_xml.EdXml):
    """IPC XML Command"""
    class meta:
        tagname = EDXML_IPC
    filelist = ed_xml.List(ed_xml.Model(IPCFile))
    arglist = ed_xml.List(ed_xml.Model(IPCArg))
