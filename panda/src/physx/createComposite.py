
import os
import os.path

def createComposite():
    filesToOmit = (
        "physx_composite.cxx",
        "physxTemplate.cxx",
    )

    compositeFile = file( "physx_composite.cxx", "w" )
    compositeFile.write( "\n" )

    for filename in os.listdir( os.getcwd() ):
        if filename.endswith( ".cxx" ) and (filename not in filesToOmit):
            compositeFile.write( '#include "%s"\n' % filename )

if __name__ == "__main__":
    createComposite()
