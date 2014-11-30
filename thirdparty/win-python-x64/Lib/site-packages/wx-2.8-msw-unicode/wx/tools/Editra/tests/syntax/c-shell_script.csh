# Some Comments about this file

echo "Hello, World!"

# Look for the file specified as arg 1 and 
# say if we find it or not
set f = $1
foreach d (*)
 if (-e $d/$f) then
       echo FOUND: $d/$f
       exit(0)
 endif
end
echo $f not found in subdirectories
