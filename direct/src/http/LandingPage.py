import os
from direct.directnotify.DirectNotifyGlobal import directNotify
from pandac.PandaModules import VirtualFileSystem
from pandac.PandaModules import Filename
from pandac.PandaModules import DSearchPath
import LandingPageHTML

class LandingPage:
    notify  = directNotify.newCategory("LandingPage")
    def __init__(self):
        self.headerTemplate = LandingPageHTML.header
        self.footerTemplate = LandingPageHTML.footer
        
        self.menu = {}

        self.uriToTitle = {}

        self.quickStats = [[],{}]

        self.addQuickStat("Pages Served", 0, 0)

        self.favicon = self.readFavIcon()
        

    def readFavIcon(self):
        vfs = VirtualFileSystem.getGlobalPtr()
        filename = Filename('favicon.ico')

        searchPath = DSearchPath()
        searchPath.appendDirectory(Filename('.'))
        searchPath.appendDirectory(Filename('etc'))
        searchPath.appendDirectory(Filename.fromOsSpecific(os.path.expandvars('$DIRECT/src/http')))
        searchPath.appendDirectory(Filename.fromOsSpecific(os.path.expandvars('direct/src/http')))
        searchPath.appendDirectory(Filename.fromOsSpecific(os.path.expandvars('direct/http')))
        found = vfs.resolveFilename(filename,searchPath)
        if not found:
            raise "Couldn't find direct/http/favicon.ico"

        return vfs.readFile(filename, 1)


    def addTab(self, title, uri):
        self.menu[title] = uri
        self.uriToTitle[uri] = title

    def getMenu(self, activeTab):
        return LandingPageHTML.getTabs(self.menu,activeTab)

    def getHeader(self, activeTab = "Main"):
        s = self.headerTemplate % {'titlestring' : LandingPageHTML.title,
                                   'menustring' : self.getMenu(activeTab)}
        return s
        

    def getFooter(self):
        return self.footerTemplate % {'contact' : LandingPageHTML.contactInfo}

    def getServicesPage(self, uriToHandler):
        output = ""
        
        uriList = uriToHandler.keys()

        uriList.sort()

        autoList = []

        if "/" in uriList:
            uriList.remove("/")
            autoList.append("/")
        if "/services" in uriList:
            uriList.remove("/services")
            autoList.append("/services")
        if "/default.css" in uriList:
            uriList.remove("/default.css")
            autoList.append("/default.css")
        if "/favicon.ico" in uriList:
            uriList.remove("/favicon.ico")
            autoList.append("/favicon.ico")

        output += LandingPageHTML.getURITable(title="Application",uriList=uriList,uriToHandler=uriToHandler)

        output += LandingPageHTML.getURITable(title="Admin",uriList=autoList,uriToHandler=uriToHandler)
        
        return output

    def populateMainPage(self, body):
        LandingPageHTML.mainPageBody = body

    def setTitle(self, title):
        if LandingPageHTML.title == LandingPageHTML.defaultTitle:
            LandingPageHTML.title = title
        else:
            LandingPageHTML.title = LandingPageHTML.title + " + " + title

    def setDescription(self,desc):
        if LandingPageHTML.description == LandingPageHTML.defaultDesc:
            LandingPageHTML.description = desc
        else:
            LandingPageHTML.description = LandingPageHTML.description + "</P>\n<P>" + desc

    def setContactInfo(self,info):
        LandingPageHTML.contactInfo = info

    def getDescription(self):
        return LandingPageHTML.description

    def getQuickStatsTable(self):
        return LandingPageHTML.getQuickStatsTable(self.quickStats)

    def getMainPage(self):
        return LandingPageHTML.mainPageBody % {"description" : self.getDescription(),
                                               "quickstats" : self.getQuickStatsTable()}

    def getStyleSheet(self):
        return LandingPageHTML.stylesheet

    def getFavIcon(self):
        return self.favicon
    
    def skin(self, body, uri):
        title = self.uriToTitle.get(uri,"Services")
        return self.getHeader(title) + body + self.getFooter()

    def addQuickStat(self,item,value,position):
        if item in self.quickStats[1]:
            assert self.notify.warning("Ignoring duplicate addition of quickstat %s." % item)
            return
                                
        self.quickStats[0].insert(position,item)
        self.quickStats[1][item] = value
        
    def updateQuickStat(self,item,value):
        assert item in self.quickStats[1]

        self.quickStats[1][item] = value

    def incrementQuickStat(self,item):
        assert item in self.quickStats[1]

        self.quickStats[1][item] += 1
