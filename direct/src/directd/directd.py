#!python
"""directd stuff"""

from AIBaseGlobal import *
import DirectNotifyGlobal
import Datagram
import DatagramIterator


print "hello"


class DirectDServer:
    notify = DirectNotifyGlobal.directNotify.newCategory("DirectD")

    def __init__(self):
        pass

    def connect(self, ip, port, timeout=35000):
        self.qcm=QueuedConnectionManager()
        self.tcpConn=self.qcm.openTCPClientConnection(ip, port, timeout)
        
        if self.tcpConn == None:
            # Bad Connection
            print "noConnection"
        else:
            # Good Connection
            # Set up connection reader and writer
            self.qcr=QueuedConnectionReader(self.qcm, 0)
            self.qcr.addConnection(self.tcpConn)
            self.cw=ConnectionWriter(self.qcm, 0)
            while 1:
                s=raw_input("send: ")
                d=Datagram.Datagram()
                d.addString(s)
                self.cw.send(d, self.tcpConn)
                while self.qcr.dataAvailable():
                    nd=NetDatagram()
                    self.qcr.getData(nd)
                    print "address", nd.getAddress().getIp()
                    nd.dumpHex(ostream)
            #self.startReaderPollTask()
            
            # Register our channel
            #self.registerForChannel(self.ourChannel)

def foo():
    dds = DirectDServer()
    dds.connect("127.0.0.1", 8000)
