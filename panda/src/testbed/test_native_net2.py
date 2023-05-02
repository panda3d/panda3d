from libpandaexpress import BufferedDatagramConnection
from libpandaexpress import SocketAddress
from libpandaexpress import SocketIP
from libpandaexpress import SocketTCP
from libpandaexpress import SocketTCPListen
from libpandaexpress import SocketUDPOutgoing
from libpandaexpress import SocketUDPIncoming
from libpandaexpress import Datagram

import time



SocketIP.InitNetworkDriver();

addr = SocketAddress()
addr.setHost("127.0.0.1",6666)
print(addr.getIpPort())

MyConection = BufferedDatagramConnection(0,4096000,4096000,102400);
#help(BufferedDatagramConnection)

MyConection.AddAddress(addr)

dg = Datagram();
dg.addUint8(1)
dg.addUint64(4001)
dg.addUint16(2001)
dg.addUint64(123456)

##MyConection.SendMessage(dg);


dg1 = Datagram();
dg1.addUint8(1)
dg1.addUint64(123456)
dg1.addUint64(12340)
dg1.addUint16(1000)
dg1.addUint16(54321)


while 1==1:
    for x in range(200000):
        if  not MyConection.SendMessage(dg1):
            print("Error Sending Message")

    MyConection.Flush();
    time.sleep(1)
    print("loop")
