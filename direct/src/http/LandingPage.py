import LandingPageHTML

class LandingPage:
    def __init__(self):
        self.headerTemplate = LandingPageHTML.header
        self.footerTemplate = LandingPageHTML.footer
        
        self.menu = {}

        self.uriToTitle = {}

        self.quickStats = [[],{}]

        self.addQuickStat("Pages Served", 0, 0)

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
    
    def skin(self, body, uri):
        title = self.uriToTitle.get(uri,"Services")
        return self.getHeader(title) + body + self.getFooter()

    def addQuickStat(self,item,value,position):
        if item in self.quickStats[1]:
            self.notify.warning("Ignoring duplicate addition of quickstat %s." % item)
            return
                                
        self.quickStats[0].insert(position,item)
        self.quickStats[1][item] = value
        
    def updateQuickStat(self,item,value):
        assert item in self.quickStats[1]

        self.quickStats[1][item] = value

    def incrementQuickStat(self,item):
        assert item in self.quickStats[1]

        self.quickStats[1][item] += 1
