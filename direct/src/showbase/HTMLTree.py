from direct.showbase import ElementTree as ET

class HTMLTree(ET.ElementTree):
    def __init__(self, title):
        root = ET.Element('HTML')
        ET.ElementTree.__init__(self, root)

        head = ET.SubElement(root, 'HEAD')
        title = ET.SubElement(head, 'TITLE')
        title.text = title

        body = ET.SubElement(root, 'BODY')

    def getBody(self):
        return self.find('BODY')

    def getHead(self):
        return self.find('HEAD')
    
