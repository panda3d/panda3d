from libpandaexpress import BufferedDatagramConnection
from libpandaexpress import SocketAddress
from libpandaexpress import SocketIP
from libpandaexpress import SocketTCP
from libpandaexpress import SocketTCPListen
from libpandaexpress import SocketUDPOutgoing
from libpandaexpress import SocketUDPIncoming


SocketIP.InitNetworkDriver();

addr = SocketAddress()
addr.setHost("127.0.0.1",8080)
print(addr.getIpPort())


inbound = SocketTCPListen()
inbound.OpenForListen(addr);
##inbound.SetNonBlocking();

while 1 == 1:
    newsession = SocketTCP()
    source = SocketAddress()
    if inbound.GetIncomingConnection(newsession,source) :
        #newsession.SetNonBlocking();
        print(source.getIpPort())
        newsession.SendData("Hello From the Listener\n\r");

        s = newsession.RecvData(10);
        print(s)
        print(newsession.GetLastError())
        if newsession.ErrorIsWouldBlocking(newsession.GetLastError()) :
            print("Reading Would Block")
        else:
            print("Not A Blocking Error")

        newsession.SendData("GoodBy From the Listener\n\r");
        newsession.Close();
