import LandingPageHTML

class LandingPage:
    def __init__(self):
        self.headerTemplate = LandingPageHTML.header
        self.footerTemplate = LandingPageHTML.footer
        
        #self.title = LandingPageHTML.title
        #self.contactInfo = LandingPageHTML.contactInfo

        self.menu = {}

        self.uriToTitle = {}

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

        output += LandingPageHTML.getURITable(title="Application",uriList=uriList,uriToHandler=uriToHandler)

        output += LandingPageHTML.getURITable(title="Admin",uriList=autoList,uriToHandler=uriToHandler)
        
        return output

    def populateMainPage(self, body):
        LandingPageHTML.mainPageBody = body

    def setTitle(self, title):
        LandingPageHTML.title = title

    def setDescription(self,desc):
        LandingPageHTML.description = desc

    def setContactInfo(self,info):
        LandingPageHTML.contactInfo = info

    def getDescription(self):
        return LandingPageHTML.description

    def getMainPage(self):
        return LandingPageHTML.mainPageBody

    def getQuickStatsTable(self, quickStats):
        return LandingPageHTML.getQuickStatsTable(quickStats)

    def skin(self, body, uri):
        title = self.uriToTitle.get(uri,"Services")
        return self.getHeader(title) + body + self.getFooter()
