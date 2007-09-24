import sys

"""
Class to extract system information from a Linux Box via /proc

The following variables are accessable:

os
cpu
totalRAM
availableRAM
totalVM
availableVM
loadAvg

Example:

s = SystemInformation()
print s.os

s.refresh() # If you need to refresh the dynamic data

"""

class SystemInformation:
	def __init__(self):

		# Just in case sombody called this class by accident, we should
		# check to make sure the OS is Linux before continuing

		assert sys.platform == 'linux2', "Not a Linux based system. This class should not be called"

		self.os = self.__getOS()
		self.cpu = self.__getCPU()
		self.totalRAM, self.availableRAM, self.totalVM, self.availableVM = self.__getMemory()
		self.loadAvg = self.__getLoadAvg()

	def refresh(self):
		self.totalRAM, self.availableRAM, self.totalVM, self.availableVM = self.__getMemory()
		self.loadAvg = self.__getLoadAvg()

	def __getloadAvg(self):
		loadAvg = open('/proc/loadavg')
		procloadAvg = loadAvg.read()
		loadAvg.close()
		# Lets remove the \n from the raw string
		procloadAvg = procloadAvg.replace('\n','')
		return procloadAvg

	def __getOS(self):
		procOS = open('/proc/version')
		procOSRaw = procOS.read()
		procOS.close()
		# Lets remove the \n before returning the version string
		procOSRaw = procOSRaw.replace('\n', '')
		return procOSRaw

	def __getCPU(self):
		procCPU = open('/proc/cpuinfo')
		procCPURaw = procCPU.read()
		procCPU.close()
		del procCPU
		procCPURaw = procCPURaw.split('\n')
		# Lets first figure out how many CPUs are in the system
		cpuCount = 0
		modelName = ''
		cpuMHz = ''
		for element in procCPURaw:
			if element.find('processor') != -1:
				cpuCount += 1
		# cpuCount now has the total number of CPUs

		# Next, on to the Model of the Processor
		for element in procCPURaw:
			if element.find('model name') != -1:
				modelName = element[element.find(':')+2:]
				break

		# Next, on to the clock speed
		for element in procCPURaw:
			if element.find('cpu MHz') != -1:
				cpuMHz = element[element.find(':')+2:]
				break
		# Now that we have the info, time to return a string

		return "%s\t%d @ %s MHz" % (modelName, cpuCount, cpuMHz)


	def __getMemory(self):
		procMemory = open('/proc/meminfo')
		procMemoryRaw = procMemory.read()
		procMemory.close()
		del procMemory
		procMemoryRaw = procMemoryRaw.split('\n')
		# We are looking for the following:
		# MemTotal, MemFree, SwapTotal, SwapFree

		# Lets start with MemTotal first

		memTotal = ''
		for element in procMemoryRaw:
			if element.find('MemTotal:') != -1:
				memTotal = element.split(':')[1].replace(' ','')
				break
		# Next MemFree:

		memFree = ''
		for element in procMemoryRaw:
			if element.find('MemFree:') != -1:
				memFree = element.split(':')[1].replace(' ','')
                                break

		# SwapTotal:

		swapTotal = ''
		for element in procMemoryRaw:
			if element.find('SwapTotal:') != -1:
				memFree = element.split(':')[1].replace(' ','')
                                break

		# SwapFree:

		swapFree = ''
		for element in procMemoryRaw:
                        if element.find('SwapFree:') != -1:
                                memFree = element.split(':')[1].replace(' ','')
                                break
		return memTotal, memFree, swapTotal, swapFree

if __name__ == "__main__":
	s = SystemInformation()
	print s.cpu
	print s.totalRAM
	print s.os
