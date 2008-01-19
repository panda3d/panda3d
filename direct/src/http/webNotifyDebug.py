from direct.task import Task
from direct.http import WebRequest
from direct.directnotify import DirectNotifyGlobal
import random, string

class webNotifyDebug:
    def __init__(self, portNumber = 8888, username = None, password = None):

        self.portNumber = portNumber
        self.username = username
        self.password = password
        self.passwordProtect = False
        self.pageToHit = 'debug'
        self.authTokens = []
        self.web = WebRequest.WebRequestDispatcher()
        self.web.listenOnPort(int(self.portNumber))
        # 'debug' will be the name of the page we have to hit
        # If both a username and password should be specified, then
        # we will need to present a username and password prompt to the user
        if self.username and self.password:
            # set self.passwordProtect to True
            self.passwordProtect = True
            # Register 'debug' with the password prompt
            self.web.registerGETHandler('debug', self.passwordPrompt)
            self.web.registerGETHandler('authDebug', self.authDebug)
            self.pageToHit = 'authDebug'
        else:
            self.web.registerGETHandler('debug', self.debug)
        self.startCheckingIncomingHTTP()

    def passwordPrompt(self, replyTo, **kw):
        # This should get called if we need to prompt the user for
        # a username and password.
        try:
            username = kw['username']
            password = kw['password']
        except KeyError:
            # the user is probably making their initial connection to the
            # password protected site. Present them with the login page
            replyTo.respond('<HTML>\n<HEAD><TITLE>Direct Notify Web Interface - Username and Password Required</TITLE></HEAD>\n<BODY>\n<FONT SIZE=4>Username/Password authentication has been enabled. You must provide the following before gaining access to the system:<P><FORM action="debug" method="get">\nUsername: <INPUT type="text" name="username"><BR>\nPassword: <INPUT type="password" name="password"><BR>\n<input type=submit name="Submit" text="Login"></form>\n</BODY></HTML>')
            return

        # If the username and password are correct, we need to generate an
        # auth token and place it in self.authTokens. If the username and
        # password are incorrect. Return an error message indicating such.

        if username == self.username and password == self.password:
            # Username and Password match
            # Generate auth token
            authToken = self.genToken()
            # Place the authToken in the list of valid auth tokens
            self.authTokens.append(authToken)
            
            replyTo.respond('<HTML><HEAD><TITLE>Username and Password Good</TITLE></HEAD><BODY>Username and Password are good, please remember to logout when done. <A HREF=authDebug?authToken=%s>Click here to continue</a></BODY></HTML>' % (authToken))
            return
        else:
            replyTo.respond('Username and/or password are incorrect')
            return

    def listAllCategories(self, replyTo, optionalMessage = None, authToken = None):
        # Return a web page with a list of all registered notify categories
        # along with an HTML widget to chage their debug state

        completeList = DirectNotifyGlobal.directNotify.getCategories()

        # Define the static head of the response

        if not optionalMessage:
            head = '<html>\n<head>\n<meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">\n<title>DirectNotify - List All Categories</title>\n</head>\n<body>\n<h1 style="text-align: center;">DirectNotify - Listing All Categories</h1>\n<CENTER><table style="text-align: left;" border="1" cellpadding="2" cellspacing="2">\n<tbody>\n<tr><th>Category</th><th>Debug Status</th></tr>\n'
        else:
            head = '<html>\n<head>\n<meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">\n<title>DirectNotify - List All Categories</title>\n</head>\n<body>\n<h1 style="text-align: center;">DirectNotify - Listing All Categories</h1>\n<CENTER><HR>%s<HR><BR><table style="text-align: left;" border="1" cellpadding="2" cellspacing="2">\n<tbody>\n<tr><th>Category</th><th>Debug Status</th></tr>\n' % (optionalMessage)

        # define the static foot

        if authToken:
            foot = '</tbody></table></CENTER><BR><A HREF="%s?authToken=%s">Main Menu</a></body></html>' % (self.pageToHit, authToken)
        else:
            foot = '</tbody></table></CENTER><BR><A HREF="%s">Main Menu</a></body></html>' % self.pageToHit

        # Sort our catagory list into alpha order

        completeList.sort()

        # Now generate the body of the page response

        body = ''
        for item in completeList:
            select = '<tr><td>%s</td><td style="text-align: center;">' % (item)
            tempCategory = DirectNotifyGlobal.directNotify.getCategory(item)
            debugStatus = tempCategory.getDebug()
            if debugStatus == 0:
                if authToken:
                    body = '%s%s<A HREF="%s?command=on&item=%s&authToken=%s">Off</a></td></tr>' % (body, select, self.pageToHit, item, authToken)
                else:
                    body = '%s%s<A HREF="%s?command=on&item=%s">Off</a></td></tr>' % (body, select, self.pageToHit, item)
            else:
                if authToken:
                    body = '%s%s<A HREF="%s?command=off&item=%s&authToken=%s">On</a></td></tr>' % (body, select, self.pageToHit, item, authToken)
                else:
                    body = '%s%s<A HREF="%s?command=off&item=%s">On</a></td></tr>' % (body, select, self.pageToHit, item)

        replyTo.respond('%s\n%s\n%s\n' % (head, body, foot))

    def turnCatOn(self, item, replyTo, sString = None, authToken = None):
        # Used to turn a catagory (item), to the on state
        try:
            notifyItem = DirectNotifyGlobal.directNotify.getCategory(item)
            notifyItem.setDebug(1)
            updateMessage = 'Category <b>%s</b>, has been turned on' % (item)
            if not sString:
                self.listAllCategories(replyTo, updateMessage, authToken)
            else:
                self.searchForCat(sString, replyTo, updateMessage, authToken)
        except AttributeError:
            replyTo.respond('Invalid Category Passed')

    def turnCatOff(self, item, replyTo, sString = None, authToken = None):
        # Used to turn a catagory (item), to the off state
        try:
            notifyItem = DirectNotifyGlobal.directNotify.getCategory(item)
            notifyItem.setDebug(0)
            updateMessage = 'Category <b>%s</b>, has been turned off' % (item)
            if not sString:
                self.listAllCategories(replyTo, updateMessage, authToken)
            else:
                self.searchForCat(sString, replyTo, updateMessage, authToken)
        except AttributeError:
            replyTo.respond('Invalid Category Passed')

    def searchForCat(self, searchString, replyTo, toggle = None, authToken = None):
        # Used to execute a substring search for a category
        completeList = DirectNotifyGlobal.directNotify.getCategories()
        resultList = []
        while completeList:
            item = completeList.pop()
            if item.find(searchString) != -1:
                resultList.append(item)
        # Now that we have the results, present them
        # First, sort the list
        resultList.sort()
        if not toggle:
            head = '<html>\n<head>\n<meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">\n<title>DirectNotify - Search Results</title>\n</head>\n<body>\n<h1 style="text-align: center;">DirectNotify - Listing All Categories</h1>\n<CENTER><table style="text-align: left;" border="1" cellpadding="2" cellspacing="2">\n<tbody>\n<tr><th>Category</th><th>Debug Status</th></tr>\n'
        else:
            head = '<html>\n<head>\n<meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">\n<title>DirectNotify - Search Results</title>\n</head>\n<body>\n<h1 style="text-align: center;">DirectNotify - Listing All Categories</h1>\n<CENTER><HR>%s<HR><br><table style="text-align: left;" border="1" cellpadding="2" cellspacing="2">\n<tbody>\n<tr><th>Category</th><th>Debug Status</th></tr>\n' % (toggle)
        if authToken:
            foot = '</tbody></table></CENTER><BR><A HREF="authDebug?authToken=%s">Main Menu</a></body></html>' % (authToken)
        else:
            foot = '</tbody></table></CENTER><BR><A HREF="debug">Main Menu</a></body></html>'
        body = ''
        for item in resultList:
            select = '<tr><td>%s</td><td style="text-align: center;">' % (item)
            tempCategory = DirectNotifyGlobal.directNotify.getCategory(item)
            debugStatus = tempCategory.getDebug()
            if debugStatus == 0:
                if authToken:
                    body = '%s%s<A HREF="%s?command=on&item=%s&sString=%s&authToken=%s">Off</a></td></tr>' % (body, select, self.pageToHit, item, searchString, authToken)
                else:
                    body = '%s%s<A HREF="%s?command=on&item=%s&sString=%s">Off</a></td></tr>' % (body, select, self.pageToHit, item, searchString)
            else:
                if authToken:
                    body = '%s%s<A HREF="%s?command=off&item=%s&sString=%s&authToken=%s">On</a></td></tr>' % (body, select, self.pageToHit, item, searchString, authToken)
                else:
                    body = '%s%s<A HREF="%s?command=off&item=%s&sString=%s">On</a></td></tr>' % (body, select, self.pageToHit, item, searchString)

        replyTo.respond('%s\n%s\n%s\n' % (head, body, foot))

    def debug(self, replyTo, **kw):
        try:
            authToken = kw['authToken']
        except KeyError:
            authToken = None
        try:
            command = kw['command']
            if command == 'listAll':
                if self.passwordProtect:
                    self.listAllCategories(replyTo, None, authToken)
                else:
                    self.listAllCategories(replyTo)
            elif command == 'on':
                item = kw['item']
                try:
                    sString = kw['sString']
                    if self.passwordProtect:
                        self.turnCatOn(item, replyTo, sString, authToken)
                    else:
                        self.turnCatOn(item, replyTo, sString)
                except KeyError:
                    if self.passwordProtect:
                        self.turnCatOn(item, replyTo, None, authToken)
                    else:
                        self.turnCatOn(item, replyTo)
            elif command == 'off':
                item = kw['item']
                try:
                    sString = kw['sString']
                    if self.passwordProtect:
                        self.turnCatOff(item, replyTo, sString, authToken)
                    else:
                        self.turnCatOff(item, replyTo, sString)
                except KeyError:
                    if self.passwordProtect:
                        self.turnCatOff(item, replyTo, None, authToken)
                    else:
                        self.turnCatOff(item, replyTo)
            elif command == 'search':
                searchString = kw['searchString']
                if self.passwordProtect:
                    self.searchForCat(searchString, replyTo, None, authToken)
                else:
                    self.searchForCat(searchString, replyTo)
            elif command == 'logOff' and authToken:
                self.logOut(replyTo, authToken)
            else:
                replyTo.respond('Error: Invalid args')
            return
        except KeyError:
            pass
        # Basic Index Page

        if not authToken:
            replyTo.respond('<html><head>\n<meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">\n<title>DirectNotify Web Interface</title>\n</head>\n<body>\n<div style="text-align: center;">\n<h1>DirectNotify Web Interface</h1>\n</div>\n<hr style="height: 2px;">\n<form method="get" action="debug" name="searchfom"><INPUT TYPE=HIDDEN NAME="command" VALUE="search">Search for a DirectNotify Category: <input name="searchString"> <input type=submit name="Submit"></button><br>\n</form>\n<br>\n<A HREF="%s?command=listAll">Display all DirectNotify Categories</a>\n</body>\n</html>' % (self.pageToHit))
        else:
            replyTo.respond('<html><head>\n<meta content="text/html; charset=ISO-8859-1" http-equiv="content-type">\n<title>DirectNotify Web Interface</title>\n</head>\n<body>\n<div style="text-align: center;">\n<h1>DirectNotify Web Interface</h1>\n</div>\n<hr style="height: 2px;">\n<form method="get" action="authDebug" name="searchfom"><INPUT TYPE=HIDDEN NAME="command" VALUE="search"><INPUT TYPE=HIDDEN NAME="authToken" VALUE="%s">Search for a DirectNotify Category: <input name="searchString"> <input type=submit name="Submit"></button><br>\n</form>\n<br>\n<A HREF="%s?command=listAll&authToken=%s">Display all DirectNotify Categories</a><BR>\n<A HREF="authDebug?command=logOff&authToken=%s">Log Off</a></body>\n</html>' % (authToken, self.pageToHit, authToken, authToken))

    def logOut(self, replyTo, authToken):
        # Delete token from auth list
        self.authTokens.remove(authToken)
        replyTo.respond('<HTML><HEAD><TITLE>Logout Sucessful</TITLE></HEAD>\n<BODY>Logout complete. You will need to login again to use the system</BODY>\n</HTML>')

    def authDebug(self, replyTo, **kw):
        try:
            authToken = kw['authToken']
            try:
                 pos = self.authTokens.index(authToken)
            except ValueError:
                # authToken passed is not in the list
                replyTo.respond('Error: Client not authorized')
                return
        except (ValueError, KeyError):
            # authToken not passed in GET. Return an error
            replyTo.respond('Error: No auth token passed from client')
            return

        # If we've gotten this far, we have determined that an auth token was
        # passed in the HTTP GET and it is on the list of auth tokens.
        # Now we can pass this to the normal debug URI
        kw['authToken'] = authToken
        self.debug(replyTo, **kw)

    def startCheckingIncomingHTTP(self):
        taskMgr.remove('pollDirectDebugHTTPTask')
        taskMgr.doMethodLater(0.3,self.pollDirectDebugHTTPTask,'pollDirectDebugHTTPTask')

    def stopCheckingIncomingHTTP(self):
        taskMgr.remove('pollDirectDebugHTTPTask')

    def pollDirectDebugHTTPTask(self,task):
        self.web.poll()
        return Task.again

    def genToken(self):
        alpha = string.letters.upper()
        num = string.digits
        ranNumber = ''
        ranAlpha = ''
        for i in range(3):
            ranNumberOne = ranNumber + random.choice(num)
        for i in range(3):
            ranAlphaOne = ranAlpha + random.choice(alpha)
        for i in range(3):
            ranNumberTwo = ranNumber + random.choice(num)
        for i in range(3):
            ranAlphaTwo = ranAlpha + random.choice(alpha)
        token = "%s%s%s%s" % (ranAlphaOne, ranNumberOne, ranAlphaTwo, ranNumberTwo)
        return token
