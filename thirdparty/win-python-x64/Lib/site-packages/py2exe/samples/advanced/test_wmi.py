import wmi # Tim Golden's wmi module.

computer = wmi.WMI()

for item in computer.Win32_Process()[:2]:
    print item
