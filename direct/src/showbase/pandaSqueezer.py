import os
import pandaSqueezeTool

# Assumption: We will be squeezing the files from C:\Panda\lib\py

def getSqueezeableFiles():
        directDir = os.getenv('DIRECT')
	fileList = os.listdir(directDir + "\lib\py")
	newFileList = []
	for i in fileList:
		if i[-4:] == ".pyc":
			j = directDir + "/lib/py/" + i
			newFileList.append(j)
	return newFileList

def squeezePandaFiles():
	l = getSqueezeableFiles()
	pandaSqueezeTool.squeeze("panda", "ShowBaseGlobal", l)

squeezePandaFiles()
