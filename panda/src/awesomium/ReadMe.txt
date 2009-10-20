See http://www.sirikata.com/wiki/index.php?title=Compiling_Awesomium

But the Chromium revision 22725.
The awesomium git repository used is at: http://github.com/pathorn/awesomium/network
The version of awesomium is from the master August 7 checkin, tree fc3239b4f49031285682cb5d78256d56e9001b66

Any method that had a wstring in the parameter, a second method was created which just accepts all std::string. This fixes the VC7 - VC9 linker error.

