from PandaObject import *
from ClusterClient import *
import string

# this is an array of offsets for the various servers.  For example,
# mono-modelcave-pipe0 has one server with both a pos and hpr of 0.
# For mono-modelcave-pipe0, I decided to set the offsets in the
# actual configuration for the display.

ClientConfigs = {'mono-modelcave-pipe0': [ [Vec3(0,0,0),Vec3(0,0,0)] ]

# The following is a fake configuration to show an example with two servers.
# 'two-server' : [ [Vec3(0,0,0),Vec3(-60,0,0)],
#                  [Vec3(0,0,0),Vec3(60,0,0) ] ]

def createCluster():
    # setup camera offsets based on chanconfig
    # This should ideally be independent of chan-config.  In other
    # words, there might be some other configuration for clustering,
    # from which the number and offset of the servers' cameras are set.
    chanConfig = base.config.GetString('chan-config', 'single')
    
    if base.config.GetBool('display-client'):
        if not ClientConfigs.has_key(chanConfig):
            base.notify.warning('Display Client flag set, but %s is not a display client configuration.' % chanConfig)
            return None
        thisClientConfig = ClientConfigs[chanConfig]
        displayConfigs = []
        for i in range(len(thisClientConfig)):
            configString = 'display%d' % i
            serverString = base.config.GetString(configString, '')
            if serverString == '':
                base.notify.warning('%s should be defined in Configrc to use %s as display client.' % [configString,chanConfig])
                base.notify.warning('%s will not be used.' % configString)
            else:
                serverInfo = string.split(serverString)
                if len(serverInfo) > 1:
                    port = int(serverInfo[1])
                else:
                    port = CLUSTER_PORT
                displayConfigs.append(ClusterConfigItem(configString,
                    serverInfo[0], thisClientConfig[i][0], thisClientConfig[i][1],port))
                
        synced = base.config.GetBool('sync-display',0)
        if synced:
            base.win.setSync(1)
            return ClusterManagerSync(displayConfigs)
        else:
            return ClusterManager(displayConfigs)
    


