#! /bin/sh

rm -f panda-standalone.zip

for dir in $DTOOL $PANDA $DIRECT $PANDATOOL; do
  zip -j panda-standalone.zip $dir/lib/*.dll $dir/bin/*.exe || exit
done
zip -j panda-standalone.zip $WINTOOLS/lib/*.dll $WINTOOLS/sdk/python/Python-2.2.2/PCbuild/*.dll || exit

cp $DIRECT/src/directscripts/standalone-Configrc /tmp/Configrc || exit
zip -j -m panda-standalone.zip /tmp/Configrc || exit
cp $DIRECT/src/directscripts/standalone-setup.bat /tmp/setup.bat || exit
zip -j -m panda-standalone.zip /tmp/setup.bat || exit

echo Success!

