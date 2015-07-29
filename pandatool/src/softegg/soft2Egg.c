// Filename: soft2Egg.c
// Created by:  masad (26Sep03)
// 
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////

#include <SAA.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "pandatoolbase.h"

int init_soft2egg(int, char **);

#if 0
// DWD includes
#include "eggBase.h"
#include <eggParametrics.h>
#include <animTable.h>
#include <linMathOutput.h>

// system includes
#include <fstream.h>
#include <strstream.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <ieeefp.h>

// Performer includes
#include <Performer/pr/pfLinMath.h>

// SoftImage includes
#include <SAA.h>
#include <SI_macros.h>

static const int    TEX_PER_MAT = 1;
static FILE            *outStream  = stdout;
//static FILE        *outStream  = stderr;

class soft2egg : public EggBase
{
  public:

    soft2egg() : EggBase("r:d:s:m:t:P:b:e:f:T:S:M:A:N:v:o:FhknpaxiucCD")
    {
        rsrc_path = "/ful/ufs/soft371_mips2/3D/rsrc";
        database_name = NULL;
        scene_name = NULL;
        model_name = NULL;
        animFileName = NULL;
        eggFileName = NULL;
        tex_path = NULL;
        eggGroupName = NULL;
        tex_filename = NULL;
        search_prefix = NULL;
        result = SI_SUCCESS;
    
        skeleton = new EggGroup();
        foundRoot = FALSE;
        animRoot = NULL;
        morphRoot = NULL;
        geom_as_joint = 0;
        make_anim = 0;
        make_nurbs = 0;
        make_poly = 0;
        make_soft = 0;
        make_morph = 1;
        make_duv = 1;
        make_dart = TRUE;
        has_morph = 0;
        make_pose = 0;
        animData.is_z_up = FALSE;
        nurbs_step = 1;
        anim_start = -1000;
        anim_end = -1000;
        anim_rate = 24;
        pose_frame = -1;
        verbose = 0;
        flatten = 0;
        shift_textures = 0;
        ignore_tex_offsets = 0;
        use_prefix = 0;
    }

    virtual void Help();
    virtual void Usage();
    virtual void ShowOpts();

    virtual boolean UseOutputSwitch() const {
       return false;
    }

    virtual boolean
    HandleGetopts(char flag, char *optarg, int &optind, int argc, char **argv);

    int   isNum( float );
    char *GetRootName( const char * );
    char *RemovePathName( const char * );
    char *GetSliderName( const char * );
    char *GetFullName( SAA_Scene *, SAA_Elem * );
    char *GetName( SAA_Scene *, SAA_Elem * );
    char *GetModelNoteInfo( SAA_Scene *, SAA_Elem * );
    char *MakeTableName( const char *, int );
    char *DepointellizeName( char * );
    SAA_Elem *FindModelByName( char *, SAA_Scene *, SAA_Elem *, int );
    char *ConvertTexture( SAA_Scene *, SAA_Elem * );
    int  *FindClosestTriVert( EggVertexPool *, SAA_DVector *, int );
    int  *MakeIndexMap( int *, int, int );
    int     findShapeVert( SAA_DVector, SAA_DVector *, int );
    void LoadSoft();
    void MakeEgg( EggGroup *, EggJoint *, AnimGroup *, SAA_Scene *, SAA_Elem * );
    void MakeSurfaceCurve(  SAA_Scene *, SAA_Elem *, EggGroup *,
        EggNurbsSurface *&, int , SAA_SubElem *, bool );

    EggNurbsCurve *MakeUVNurbsCurve( int, long *,  double *, double *, 
        EggGroup *, char * );

    EggNurbsCurve *MakeNurbsCurve( SAA_Scene *, SAA_Elem *, EggGroup *,
        float [4][4], char * );

    void AddKnots( perf_vector<double> &, double *, int, SAA_Boolean, int );
    void MakeJoint( SAA_Scene *, EggJoint *&, AnimGroup *&, SAA_Elem *, char * );
    void MakeSoftSkin( SAA_Scene *, SAA_Elem *, SAA_Elem *, int, char * );
    void CleanUpSoftSkin( SAA_Scene *, SAA_Elem *, char * );
    void MakeAnimTable( SAA_Scene *, SAA_Elem *, char * );
    void MakeVertexOffsets( SAA_Scene *, SAA_Elem *, SAA_ModelType type,
        int, int, SAA_DVector *, float (*)[4], char * );
    void MakeMorphTable( SAA_Scene *, SAA_Elem *, SAA_Elem *, int,  char *, 
        float );
    void MakeLinearMorphTable( SAA_Scene *, SAA_Elem *, int, char *, float );
    void MakeWeightedMorphTable( SAA_Scene *, SAA_Elem *, SAA_Elem *, int,  
        int, char *, float );
    void MakeExpressionMorphTable( SAA_Scene *, SAA_Elem *, SAA_Elem *, int,  
        int, char *, float );
    void MakeTexAnim( SAA_Scene *, SAA_Elem *, char * );

  private:

    char        *rsrc_path;
    char        *database_name;
    char        *scene_name;
    char        *model_name;
    char        *eggFileName;
    char         *animFileName;
    char         *eggGroupName;
    char        *tex_path;
    char        *tex_filename;
    char        *search_prefix;

    SI_Error            result;
    SAA_Scene           scene;
    SAA_Elem            model;
    SAA_Database        database;
    EggGroup           *dart;
    EggGroup           *skeleton;
    AnimGroup          *rootAnim;
    EggJoint           *rootJnt;
    AnimGroup          *animRoot;
    AnimGroup          *morphRoot;
    EggData             animData;

    int                    nurbs_step;
    int                    anim_start;
    int                    anim_end;
    int                    anim_rate;
    int                    pose_frame;
    int                    verbose;
    int                    flatten;
    int                    shift_textures;
    int                    ignore_tex_offsets;
    int                    use_prefix;
 
    bool                foundRoot;
    bool                geom_as_joint;
    bool                make_anim;
    bool                make_nurbs;
    bool                make_poly;
    bool                make_soft;
    bool                make_morph;
    bool                make_duv;
    bool                make_dart;
    bool                has_morph;
    bool                make_pose;

    ofstream    eggFile;
    ofstream    animFile;
    ofstream    texFile;

};


////////////////////////////////////////////////////////////////////
//     Function: Help
//       Access: Public, Virtual
//  Description: Displays the "what is this program" message, along
//               with the usage message.  Should be overridden in base
//               classes to describe the current program.
////////////////////////////////////////////////////////////////////
void soft2egg::
Help() 
{
    cerr <<
    "soft2egg takes a SoftImage scene or model\n"
    "and outputs its contents as an egg file\n";

    Usage();
}

////////////////////////////////////////////////////////////////////
//     Function: Usage
//       Access: Public, Virtual
//  Description: Displays the usage message.
////////////////////////////////////////////////////////////////////
void soft2egg::
Usage() {
  cerr << "\nUsage:\n"
       << _commandName << " [opts] (must specify -m or -s)\n\n"
       << "Options:\n";

  ShowOpts();
  cerr << "\n";
}



////////////////////////////////////////////////////////////////////
//     Function: ShowOpts
//       Access: Public, Virtual
//  Description: Displays the valid options.  Should be extended in
//               base classes to show additional options relevant to
//               the current program.
////////////////////////////////////////////////////////////////////
void soft2egg::
ShowOpts() 
{
    cerr <<
    "  -r <path>  - Used to provide soft with the resource\n" 
    "               Defaults to 'c:/Softimage/SOFT_3.9.2/3D/test'.\n"
      //    "               Defaults to '/ful/ufs/soft371_mips2/3D/rsrc'.\n"
    "  -d <path>  - Database path.\n"
    "  -s <scene> - Indicates that a scene will be converted.\n"
    "  -m <model> - Indicates that a model will be converted.\n"
    "  -t <path>  - Specify path to place converted textures.\n"
    "  -T <name>  - Specify filename for texture map listing.\n"
    "  -S <step>  - Specify step for nurbs surface triangulation.\n"
    "  -M <name>  - Specify model output filename. Defaults to scene name.\n"
    "  -A <name>  - Specify anim output filename. Defaults to scene name.\n"
    "  -N <name>  - Specify egg group name.\n"
    "  -k         - Enable soft assignment for geometry.\n"
    "  -n         - Specify egg NURBS representation instead of poly's.\n"
    "  -p         - Specify egg polygon output for geometry.\n"
    "  -P <frame> - Specify frame number for static pose.\n"
    "  -b <frame> - Specify starting frame for animation (default = first).\n"
    "  -e <frame> - Specify ending frame for animation (default = last).\n"
    "  -f <fps>   - Specify frame rate for animation playback.\n"
    "  -a         - Compile animation tables if animation present.\n"
    "  -F         - Ignore hierarchy and build a completely flat skeleton.\n"
    "  -v <level> - Set debug level.\n"
    "  -x         - Shift NURBS parameters to preserve Alias textures.\n"
    "  -i         - Ignore Soft texture uv offsets.\n"
    "  -u         - Use Soft prefix in model names.\n"
    "  -c         - Cancel morph conversion.\n"
    "  -C         - Cancel duv conversion.\n"
    "  -D         - Don't make the output model a character.\n"
    "  -o <prefix>- Convert only models with given prefix.\n";

      EggBase::ShowOpts();
}


////////////////////////////////////////////////////////////////////
//     Function: HandleGetopts
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
boolean soft2egg::
HandleGetopts(char flag, char *optarg, int &optind, int argc, char **argv) 
{
    boolean okflag = true;

    switch (flag) 
    {
      case 'r':       // Set the resource path for soft.
        if ( strcmp( optarg, "" ) )
        {
            // Get the path.
            rsrc_path = optarg;
            fprintf( outStream, "using rsrc path %s\n", rsrc_path );
        }
        break;

    case 'd':       // Set the database path.
        if ( strcmp( optarg, "" ) )
        {
            // Get the path.
            database_name = optarg;
            fprintf( outStream, "using database %s\n", database_name );
        }
        break;

    case 's':     // Check if its a scene.
        if ( strcmp( optarg, "" ) )
        {
            // Get scene name.
            scene_name = optarg;
            fprintf( outStream, "loading scene %s\n", scene_name );
        }
        break;
    
    case 'm':     // Check if its a model.
        if ( strcmp( optarg, "" ) )
        {
            // Get model name.
            model_name = optarg;
            fprintf( outStream, "loading model %s\n", model_name );
        }
        break;
    
    case 't':     // Get converted texture path.
        if ( strcmp( optarg, "" ) )
        {
            // Get tex path name.
            tex_path = optarg;
            fprintf( outStream, "texture path:  %s\n", tex_path );
        }
        break;

    case 'T':      // Specify texture list filename. 
        if ( strcmp( optarg, "") )
        {
            // Get the name.
            tex_filename = optarg;
            fprintf( outStream, "creating texture list file: %s\n", 
                tex_filename );
        }
        break;
    case 'S':     // Set NURBS step.
        if ( strcmp( optarg, "" ) )
        {
            nurbs_step = atoi(optarg);
            fprintf( outStream, "NURBS step:  %d\n", nurbs_step );
        }
        break;
 
    case 'M':     // Set model output file name.
        if ( strcmp( optarg, "" ) )
        {
            eggFileName = optarg;
            fprintf( outStream, "Model output filename:  %s\n", eggFileName );
        }
        break;
 
    case 'A':     // Set anim output file name.
        if ( strcmp( optarg, "" ) )
        {
            animFileName = optarg;
            fprintf( outStream, "Anim output filename:  %s\n", animFileName );
        }
        break;
 
    case 'N':     // Set egg model name.
        if ( strcmp( optarg, "" ) )
        {
            eggGroupName = optarg;
            fprintf( outStream, "Egg group name:  %s\n", eggGroupName );
        }
        break;

    case 'o':     // Set search_prefix.
        if ( strcmp( optarg, "" ) )
        {
            search_prefix = optarg;
            fprintf( outStream, "Only converting models with prefix:  %s\n", 
                search_prefix );
        }
        break;
 
    case 'h':    // print help message
        Help();
        exit(1);
        break;
    
    case 'c':    // Cancel morph animation conversion
        make_morph = FALSE;
        fprintf( outStream, "canceling morph conversion\n" );
        break;

    case 'C':    // Cancel uv animation conversion
        make_duv = FALSE;
        fprintf( outStream, "canceling uv animation conversion\n" );
        break;
    
    case 'D':    // Omit the Dart flag
        make_dart = FALSE;
        fprintf( outStream, "making a non-character model\n" );
        break;
    
    case 'k':    // Enable soft skinning
        //make_soft = TRUE;
        //fprintf( outStream, "enabling soft skinning\n" );
        fprintf( outStream, "-k flag no longer necessary\n" );
        break;
    
    case 'n':    // Generate egg NURBS output
        make_nurbs = TRUE;
        fprintf( outStream, "outputting egg NURBS info\n" );
        break;
    
    case 'p':    // Generate egg polygon output
        make_poly = TRUE;
        fprintf( outStream, "outputting egg polygon info\n" );
        break;
    
    case 'P':    // Generate static pose from given frame
        if ( strcmp( optarg, "" ) )
        {
            make_pose = TRUE;
            pose_frame = atoi(optarg);
            fprintf( outStream, "generating static pose from frame %d\n",
                pose_frame );
        }
        break;
    
    case 'a':     // Compile animation tables.
        make_anim = TRUE;
        fprintf( outStream, "attempting to compile anim tables\n" );
        break;

    case 'F':     // Build a flat skeleton.
        flatten = TRUE;
        fprintf( outStream, "building a flat skeleton!!!\n" );
        break;

    case 'x':     // Shift NURBS parameters to preserve Alias textures.
        shift_textures = TRUE;
        fprintf( outStream, "shifting NURBS parameters...\n" );
        break;

    case 'i':     // Ignore Soft uv texture offsets 
        ignore_tex_offsets = TRUE;
        fprintf( outStream, "ignoring texture offsets...\n" );
        break;

    case 'u':     // Use Soft prefix in model names 
        use_prefix = TRUE;
        fprintf( outStream, "using prefix in model names...\n" );
        break;


    case 'v':     // print debug messages.
        if ( strcmp( optarg, "" ) )
        {
            verbose = atoi(optarg);
            fprintf( outStream, "using debug level %d\n", verbose );
        }
        break;

    case 'b':     // Set animation start frame.
        if ( strcmp( optarg, "" ) )
        {
            anim_start = atoi(optarg);
            fprintf( outStream, "animation starting at frame:  %d\n", 
                anim_start );
        }
        break;

    case 'e':     /// Set animation end frame.
        if ( strcmp( optarg, "" ) )
        {
            anim_end = atoi(optarg);
            fprintf( outStream, "animation ending at frame:  %d\n", anim_end );
        }
        break;
 
    case 'f':     /// Set animation frame rate.
        if ( strcmp( optarg, "" ) )
        {
            anim_rate = atoi(optarg);
            fprintf( outStream, "animation frame rate:  %d\n", anim_rate );
        }
        break;
 
    default:
        okflag = EggBase::HandleGetopts(flag, optarg, optind, argc, argv);
  }

  return (okflag);
}



////////////////////////////////////////////////////////////////////
//     Function: isNum
//       Access: Public, Virtual
//  Description: Take a float and make sure it is of the body. 
////////////////////////////////////////////////////////////////////
int soft2egg::
isNum( float num ) 
{
    return( ( num < HUGE_VAL ) && finite( num ) );
}


////////////////////////////////////////////////////////////////////
//     Function: GetRootName
//       Access: Public
//  Description: Given a string, return a copy of the string up to
//                 the first occurence of '-'. 
////////////////////////////////////////////////////////////////////
char *soft2egg::
GetRootName( const char *name )
{
    char *hyphen;
    char *root;
    int      len;

    hyphen = strchr( name, '-' );
    len = hyphen-name;

    if ( (hyphen != NULL) && len )
    {
        root = (char *)malloc(sizeof(char)*(len+1));
        strncpy( root, name, len );
        root[sizeof(char)*(len)] = '\0';
    }
    else
    {
        root = (char *)malloc( sizeof(char)*(strlen(name)+1));
        strcpy( root, name );
    }

    return( root );
}


////////////////////////////////////////////////////////////////////
//     Function: RemovePathName
//       Access: Public
//  Description: Given a string, return a copy of the string after 
//                 the last occurence of '/ 
////////////////////////////////////////////////////////////////////
char *soft2egg::
RemovePathName( const char *name )
{
    char *slash;
    char *root;

    if ( *name != NULL )
    {
        slash = strrchr( name, '/' );

        root = (char *)malloc( sizeof(char)*(strlen(name)+1));

        if ( slash != NULL )
            strcpy( root, ++slash );
        else
            strcpy( root, name );

        return( root );
    }

    fprintf( stderr, "Error: RemovePathName received NULL string!\n" );
    return ( (char *)name );
}

////////////////////////////////////////////////////////////////////
//     Function: GetSliderName
//       Access: Public
//  Description: Given a string, return that part of the string after 
//               the first occurence of '-' and before the last
//                 occurance of '.' 
////////////////////////////////////////////////////////////////////
char *soft2egg::
GetSliderName( const char *name )
{
    if ( name != NULL )
    {
        strstream newStr;
        char *hyphen;
        char *end;

        hyphen = strchr( name, '-' );

        // pull off stuff before first hyphen
        if (hyphen != NULL)
        {
            newStr << ++hyphen;
            end = newStr.str();
        }

        char *lastPeriod;

        lastPeriod = strrchr( end, '.' );

        // ignore stuff after last period
        if ( lastPeriod != NULL )
        {
            *lastPeriod = '\0';
        }

        if ( verbose >= 1 )
            fprintf( stdout, "slider name: '%s'\n", end );

        return( end );
    }
    
    return( (char *)name );
}

////////////////////////////////////////////////////////////////////
//     Function: GetName
//       Access: Public
//  Description: Given an element, return a copy of the element's 
//                 name WITHOUT prefix. 
////////////////////////////////////////////////////////////////////
char *soft2egg::
GetName( SAA_Scene *scene, SAA_Elem *element )
{
    int    nameLen;
    char *name;

    // get the name
    SAA_elementGetNameLength( scene, element, &nameLen ); 
    name = (char *)malloc(sizeof(char)*++nameLen);
    SAA_elementGetName( scene, element, nameLen, name );
    
    return name;
}

////////////////////////////////////////////////////////////////////
//     Function: GetFullName
//       Access: Public
//  Description: Given an element, return a copy of the element's 
//                 name complete with prefix. 
////////////////////////////////////////////////////////////////////
char *soft2egg::
GetFullName( SAA_Scene *scene, SAA_Elem *element )
{
    int    nameLen;
    char *name;

    // get the name
    SAA_elementGetNameLength( scene, element, &nameLen ); 
    name = (char *)malloc(sizeof(char)*++nameLen);
    SAA_elementGetName( scene, element, nameLen, name );
    
    int prefixLen;
    char *prefix;

    // get the prefix
    SAA_elementGetPrefixLength( scene, element, &prefixLen ); 
    prefix = (char *)malloc(sizeof(char)*++prefixLen);
    SAA_elementGetPrefix( scene, element, prefixLen, prefix );

    strstream fullNameStrm;

    // add 'em together
    fullNameStrm << prefix << "-" << name << ends;

    //free( name );
    //free( prefix );
    
    return fullNameStrm.str();
}

////////////////////////////////////////////////////////////////////
//     Function: GetModelNoteInfo
//       Access: Public
//  Description: Given an element, return a string containing the
//                 contents of its MODEL NOTE entry 
////////////////////////////////////////////////////////////////////
char *soft2egg::
GetModelNoteInfo( SAA_Scene *scene, SAA_Elem *model )
{

    int         size;
    char     *modelNote = NULL;
    SAA_Boolean bigEndian;


    SAA_elementGetUserDataSize( scene, model, "MNOT", &size );

    if ( size != 0 )
    {
        // allocate modelNote string
        modelNote = (char *)malloc(sizeof(char)*(size + 1));

        // get ModelNote data from this model
        SAA_elementGetUserData( scene, model, "MNOT", size, 
            &bigEndian, (void *)modelNote );

        //strip off newline, if present
        char *eol = strchr( modelNote, '\n' );
        if ( eol != NULL)
            *eol = '\0';
        else
            modelNote[size] = '\0';

        if ( verbose >= 1 )
            fprintf( outStream, "\nmodelNote = %s\n", 
                modelNote );    
    }

    return modelNote;
}


////////////////////////////////////////////////////////////////////
//     Function: MakeTableName
//       Access: Public
//  Description: Given a string, and a number, return a new string
//                 consisting of "string.number". 
////////////////////////////////////////////////////////////////////
char *soft2egg::
MakeTableName( const char *name, int number )
{
    strstream namestrm;

    namestrm << name << "." << number << ends;
    return namestrm.str();
}

////////////////////////////////////////////////////////////////////
//     Function: FindModelByName
//       Access: Public
//  Description: Given a string, find the model in the scene 
//                 whose name corresponds to the given string.                  
////////////////////////////////////////////////////////////////////
SAA_Elem *soft2egg::
FindModelByName( char *name, SAA_Scene *scene, SAA_Elem *models, 
    int numModels )
{
    char     *foundName;
    SAA_Elem *foundModel = NULL;
    
    for ( int model = 0; model < numModels; model++ )
    {
        foundName = GetName( scene, &models[model] );
    
        if ( !strcmp( name, foundName ) )
        {
            if ( verbose >= 1 )
                fprintf( outStream, "foundModel: '%s' = '%s'\n",
                    name, foundName );

            foundModel = &models[model];
            return( foundModel );
        }    
    }    

    fprintf( outStream, "findModelByName: failed to find model named: '%s'\n",
        name );

    return ( foundModel );    
}


////////////////////////////////////////////////////////////////////
//     Function: DepointellizeName
//       Access: Public
//  Description: Given a string, return the string up to the first
//                 period. 
////////////////////////////////////////////////////////////////////
char *soft2egg::
DepointellizeName( char *name )
{
    char    *endPtr;
    char    *newName;

    newName = (char *)malloc(sizeof(char)*(strlen(name)+1));
    sprintf( newName, "%s", name );

    endPtr = strchr( newName, '.' );
    if ( endPtr != NULL ) 
      *endPtr = '\0';

    return ( newName );
}


////////////////////////////////////////////////////////////////////
//     Function: ConvertTexture
//       Access: Public
//  Description: Given a string, return a copy of the string without
//               the leading file path, and make an rgb file of the
//                 same name in the tex_path directory. 
////////////////////////////////////////////////////////////////////
char *soft2egg::
ConvertTexture( SAA_Scene *scene, SAA_Elem *texture )
{
  char *fileName = NULL;
  int  fileNameLen = 0;

  // get the texture's name
  SAA_texture2DGetPicNameLength( scene, texture, &fileNameLen);

  if ( fileNameLen )
  {
        fileName = (char *)malloc(sizeof(char)*++fileNameLen);
        SAA_texture2DGetPicName( scene, texture, fileNameLen, fileName );
  }

  // make sure we are not being passed a NULL image, an empty image
  // string or the default image created by egg2soft
  if ( (fileName != NULL) && strlen( fileName ) && strcmp( fileName, 
        "/fat/people/gregw/new_test/PICTURES/default") &&
                                ( strstr( fileName, "noIcon" ) == NULL) )
  {
    char *texName = NULL;
    char *texNamePath = NULL;
    char *tmpName = NULL;
    char *fileNameExt = NULL;

    // strip off path and add .rgb
    tmpName = strrchr( fileName, '/' );

    if ( tmpName == NULL )
        tmpName = fileName;
    else
        tmpName++;

    float transp;

        // check for alpha
        SAA_texture2DGetTransparency( scene, texture, &transp );

        if ( transp != 0.0f ) {
        texName = (char *)malloc(sizeof(char)*(strlen(tmpName)+6));
        sprintf( texName, "%s.rgba", tmpName );
        } else {
        texName = (char *)malloc(sizeof(char)*(strlen(tmpName)+5));
        sprintf( texName, "%s.rgb", tmpName );
        }

    fileNameExt = (char *)malloc(sizeof(char)*(strlen(fileName)+5));
    sprintf( fileNameExt, "%s.pic", fileName ); 

    if ( verbose >= 1 )
        fprintf( outStream, "Looking for texture file: '%s'\n", fileNameExt );

    // try to make conversion of file
    int found_file = ( access( fileNameExt, F_OK ) == 0);
    
    if ( found_file )
    {
        if ( tex_path )
        {
            texNamePath = (char *)malloc(sizeof(char)*(strlen(tex_path) +
                strlen(texName) + 2));

            sprintf( texNamePath, "%s/%s", tex_path, texName );    

            if ( texFile )
                texFile << texNamePath << ": " << fileNameExt << "\n";

            // make sure conversion doesn't already exist    
            if ( (access( texNamePath, F_OK ) != 0)  && !texFile )    
            {
                char *command = (char *)malloc(sizeof(char)*
                    (strlen(fileNameExt) + strlen(texNamePath) + 20));

                sprintf( command, "image-resize -1 %s %s",  
                    fileNameExt, texNamePath );

                if ( verbose >=1 )
                    fprintf( outStream, "executing %s\n", command );
            
                system( command );    

                //free( command );
            }
            else
                if ( verbose >=1 )
                    fprintf( outStream, "%s already exists!\n", texNamePath );
        }
        else
        {
            if ( verbose >= 1 )
            {
                fprintf( outStream, "Warning: No texture path defined" );
                fprintf( outStream, " - No automatic conversion performed\n" );
            }
        }
    }
    else
    {
        fprintf( outStream, "Warning: Couldn't find texture file: %s\n", 
            fileNameExt );
    }

    //free( fileNameExt );

    if (tex_path)
        return( texNamePath );
    else
        return( texName );
  }
  else
  {
    fprintf( outStream, "Warning: ConvertTexture received NULL fileName\n" );
    return( NULL );
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FindClosestTriVert
//       Access: Public
//  Description: Given an egg vertex pool, map each vertex therein to 
//                 a vertex within an array of SAA model vertices of
//                 size numVert. Mapping is done by closest proximity.
////////////////////////////////////////////////////////////////////
int *soft2egg::
FindClosestTriVert( EggVertexPool *vpool, SAA_DVector *vertices, int numVert )
{
    int    *vertMap = NULL; 
    int     vpoolSize = vpool->NumVertices();
    int     i,j;
    float   thisDist;
    float   closestDist;
    int     closest;
    

    vertMap = (int *)malloc(sizeof(int)*vpoolSize);

    // for each vertex in vpool
    for ( i = 0; i < vpoolSize; i++ )
    {
        // find closest model vertex 
        for ( j = 0; j < numVert-1; j++ ) 
        {
            // calculate distance
            thisDist =    sqrtf( 
                powf( vpool->Vertex(i)->position[0] - vertices[j].x , 2 ) + 
                powf( vpool->Vertex(i)->position[1] - vertices[j].y , 2 ) + 
                powf( vpool->Vertex(i)->position[2] - vertices[j].z , 2 ) ); 

            // remember this if its the closest so far
            if ( !j || ( thisDist < closestDist ) )
            {
                closest = j;
                closestDist = thisDist;
            }
        } 
        vertMap[i] = closest;

        if ( verbose >= 2 )
        {
            fprintf( outStream, "mapping v %d of %d:( %f, %f, %f )\n", i, 
                vpoolSize, vpool->Vertex(i)->position[0], 
                vpool->Vertex(i)->position[1],
                vpool->Vertex(i)->position[2] ); 
            fprintf( outStream, "to cv %d of %d:( %f, %f, %f )\tdelta = %f\n",
                closest, numVert-1, vertices[closest].x, vertices[closest].y, 
                vertices[closest].z, closestDist );
        }
                }

        return( vertMap );
}


////////////////////////////////////////////////////////////////////
//     Function: MakeIndexMap
//       Access: Public
//  Description: Given an array of indices that is a map from one
//                 set of vertices to another, return an array that
//                 performs the reverse mapping of the indices array 
////////////////////////////////////////////////////////////////////
int *soft2egg::
MakeIndexMap( int *indices, int numIndices, int mapSize )
{
    int i, j;

    // allocate map array
    int *map = (int *)malloc(sizeof(int)*mapSize); 

    if ( map != NULL )
    {
        for ( i = 0; i < mapSize; i++ )
        {
            j = 0;
            int found = 0;
            while( j < numIndices )
            {
                if ( indices[j] == i )
                {
                    map[i] = j;
                    if ( verbose >= 2 )
                        fprintf( outStream, "map[%d] = %d\n", i, map[i] );
                    found = 1;
                    break;
                }
                j++;
            }
            if ( !found) 
            {
                if ( verbose >= 2 )
                    fprintf( outStream, "Warning: orphan vertex (%d)\n", i );
                // default to -1 for now
                map[i] = -1;
            }
        }
    }
    else
        fprintf( outStream, "Not enough Memory for index Map...\n");
    

    return( map );
}

    
////////////////////////////////////////////////////////////////////
//     Function: findShapeVert
//       Access: Public
//  Description: given a vertex, find its corresponding shape vertex
//                 and return its index.
////////////////////////////////////////////////////////////////////
int     soft2egg::
findShapeVert( SAA_DVector vertex, SAA_DVector *vertices, int numVert )
{
    int i;
    int found = 0;

    for ( i = 0; i < numVert && !found ; i++ )
    {
        if ( ( vertex.x == vertices[i].x ) && 
             ( vertex.y == vertices[i].y ) && 
             ( vertex.z == vertices[i].z ) )
        {
            found = 1;

            if ( verbose >= 2)
                fprintf( outStream, "found shape vert at index %d\n", i );
        }
             
    } 

    if (!found )
        i = -1;
    else
        i--;

    return( i );
}


////////////////////////////////////////////////////////////////////
//     Function: LoadSoft
//       Access: Public
//  Description: Open the SI database and grab the scene & model info
////////////////////////////////////////////////////////////////////
void soft2egg::
LoadSoft()
{
    int      i;

    if ( (scene_name == NULL && model_name == NULL) || database_name == NULL )
    {
        Usage();
        exit( 1 );
    }

    if ((result = SAA_Init(rsrc_path, FALSE)) != SI_SUCCESS)
    {
        fprintf( outStream, "Error: Couldn't get resource path!\n");
        exit( 1 );
    }

    if ((result = SAA_databaseLoad(database_name, &database)) != SI_SUCCESS)
    {
        fprintf( outStream, "Error: Couldn't load database!\n");
        exit( 1 );
    }

    if ((result = SAA_sceneGetCurrent(&scene)) == SI_SUCCESS)
    {
        // load scene if present
        if ( scene_name != NULL )
        {
            SAA_sceneLoad( &database, scene_name, &scene );

            // if no egg filename specified, make up a name
            if ( eggFileName == NULL )
            {
                eggFileName = (char *)malloc(sizeof(char)*
                    (strlen( scene_name ) + 14 ));
                sprintf( eggFileName, "%s", DepointellizeName(scene_name) );
                if ( make_nurbs )
                    strcat( eggFileName, "-nurb" );
                strcat( eggFileName, "-mod.egg" );
            }

            // open an output file for the geometry if necessary 
            if ( make_poly || make_nurbs )
            {
                unlink( eggFileName );
                eggFile.open( eggFileName, ios::out, 0666 );

                if ( !eggFile )
                {
                    fprintf( outStream, "Couldn't open output file: %s\n", 
                        eggFileName ); 
                    exit( 1 );
                }
            }

            // open an output file for texture list if specified
            if ( tex_filename != NULL )
            {
                unlink( tex_filename );
                texFile.open( tex_filename, ios::out, 0666 );
                
                if ( !texFile )
                {    
                    fprintf( outStream, "Couldn't open output file: %s\n",
                        tex_filename );
                    exit( 1 );
                }
            }

            if ( SAA_updatelistGet( &scene ) == SI_SUCCESS )
            {
                float time;

                fprintf( outStream, "setting Scene to frame %d...\n", pose_frame );
                //SAA_sceneSetPlayCtrlCurrentFrame( &scene, pose_frame );
                SAA_frame2Seconds( &scene, pose_frame, &time );
                SAA_updatelistEvalScene( &scene, time );
                sginap( 100 );
                SAA_updatelistEvalScene( &scene, time );
                if ( make_pose )
                    SAA_sceneFreeze( &scene );
            } 

            int            numModels;
            SAA_Elem    *models;

            SAA_sceneGetNbModels( &scene, &numModels ); 
            fprintf( outStream, "Scene has %d model(s)...\n", numModels );
        
            if ( numModels )
            {
                // allocate array of models
                models = (SAA_Elem *)malloc(sizeof(SAA_Elem)*numModels);

                if ( models != NULL )
                {
                    char *rootName = GetRootName( eggFileName );
                    
                
                    if ( eggGroupName == NULL )    
                        dart = _data.CreateGroup( NULL, rootName );
                    else
                        dart = _data.CreateGroup( NULL, eggGroupName );

                    if (make_dart)
                      dart->flags |= EF_DART;

                    AnimGroup    *rootTable;

                    rootTable = animData.CreateTable( NULL, eggFileName );    

                    if ( eggGroupName == NULL )
                        animRoot = animData.CreateBundle( rootTable, rootName );
                    else
                        animRoot = animData.CreateBundle( rootTable, 
                            eggGroupName );

                    // propagate commet to anim data
                    animData.root_group.children.push_front(
                        new EggComment( _commandLine ) );

                    if ( verbose >= 1 )
                        fprintf( outStream, "made animRoot: %s\n", rootName );

                    SAA_sceneGetModels( &scene, numModels, models );

                    for ( i = 0; i < numModels; i++ )
                    {
                        int level;

                        SAA_elementGetHierarchyLevel( &scene, &models[i], &level );
                        if ( !level )
                        {
                            if ( verbose >= 1 )
                                fprintf( outStream,
                                    "\negging scene model[%d]\n", i );

                            MakeEgg( dart, NULL, NULL,  &scene, &models[i] );
                        }
                    }

            if ( make_poly || make_nurbs )
            {
                // generate soft skinning assignments if desired
                //
                //disabled 1/1/99 to streamline joint assignments.
                // all joint assignments now done here. Hard & Soft.
                //if ( make_soft)
                {
                    char        *name;
                    char        *fullname;
                    SAA_Boolean isSkeleton;

                    // search through models and look for skeleton parts
                    for ( i = 0; i < numModels; i++ )
                    {
                        SAA_modelIsSkeleton( &scene, &models[i], &isSkeleton );

                        // get fullname for splitting files, but
                        // only use it in file if requested
                        fullname = GetFullName( &scene, &models[i] );
                        if ( use_prefix )
                            name = fullname;
                        else
                            name = GetName( &scene, &models[i] );

                        // split
                        if ( strstr( fullname, search_prefix ) != NULL )
                        {
                            // for every skel part: get soft skin info 
                            if ( isSkeleton )
                                MakeSoftSkin( &scene, &models[i], models,
                                    numModels, name );
                        }

                        //free( name );
                    }

                    // make sure all vertices were assigned
                    // via soft skinning - if not hard assign them
                    for ( i = 0; i < numModels; i++ )
                    {
                        // get fullname for splitting files, but
                        // only use it in file if requested
                        fullname = GetFullName( &scene, &models[i] );
                        if ( use_prefix )
                            name = fullname;
                        else
                            name = GetName( &scene, &models[i] );

                        // split
                        if ( strstr( fullname, search_prefix ) != NULL )
                            CleanUpSoftSkin( &scene, &models[i], name );

                        //free( name );
                    }

                }


                // put the skeleton data into the egg data
                dart->StealChildren( *skeleton );

                // make sure all elements have unique names
                _data.UniquifyNames();
    
                // write out the geometry data if requested    
                //if ( make_poly || make_nurbs )
                //{
                    eggFile << _data << "\n";
                    fprintf( outStream, "\nwriting out %s...\n", eggFileName );
                    eggFile.close();
                }

                // close texture list file if opened
                if ( texFile )
                    texFile.close();

                // generate animation data if desired
                if ( make_anim )
                {
                if ( animFileName == NULL )
                {
                    animFileName = (char *)malloc(sizeof(char)*
                        (strlen(scene_name)+ 10 ));
                    sprintf( animFileName, "%s", DepointellizeName(scene_name) );
                    strcat( animFileName, "-chan.egg" );
                }

                unlink( animFileName );
                animFile.open( animFileName, ios::out, 0666 );

                if ( !animFile )
                {
                    fprintf( outStream, "Couldn't open output file: %s\n", 
                        animFileName ); 
                    exit( 1 );
                }

                int frame;
                //int frameStep;
                float time;

                // get all the animation frame info if not specified
                // on the command line
                if (anim_start == -1000) 
                    SAA_sceneGetPlayCtrlStartFrame( &scene, &anim_start );

                if (anim_end == -1000)
                    SAA_sceneGetPlayCtrlEndFrame( &scene, &anim_end );

                //SAA_sceneGetPlayCtrlFrameStep( &scene, &frameStep );

                fprintf( outStream, "\nframeStart = %d\n", anim_start );
                fprintf( outStream, "frameEnd = %d\n", anim_end );
                //fprintf( outStream, "frameStep = %d\n", frameStep );

                // start at first frame and go to last
                for ( frame = anim_start; frame <= anim_end; 
                        frame += 1)
                {
                    SAA_frame2Seconds( &scene, frame, &time );
                    SAA_updatelistEvalScene( &scene, time );
                    sginap( 100 );
                    SAA_updatelistEvalScene( &scene, time );
                    fprintf( outStream, "\n> animating frame %d\n", frame );

                    // for each model
                    for ( i = 0; i < numModels; i++ )
                    {
                        char           *name;
                        char           *fullname;
                        SAA_Boolean     isSkeleton;
                        SAA_ModelType     type;

                        SAA_modelIsSkeleton( &scene, &models[i], &isSkeleton );

                        // get fullname for splitting files, but
                        // only use it in file if requested
                        fullname = GetFullName( &scene, &models[i] );
                        if ( use_prefix )
                            name = fullname;
                        else
                            name = GetName( &scene, &models[i] );

                        // split
                        if ( strstr( fullname, search_prefix ) != NULL )
                        {
                            // make the morph table for this critter
                            if ( make_morph )
                            {
                                MakeMorphTable( &scene, &models[i], models,
                                    numModels, name, time );
                            }
                        }

                        // find out what type of node we're dealing with
                        result = SAA_modelGetType( &scene, &models[i], &type );

                        int size;

                        // check for uv texture animation
                        SAA_elementGetUserDataSize( &scene, &models[i], 
                            "TEX_OFFSETS", &size );

                        // if so, update for this frame if desired
                        if ( ( size != 0 ) && make_duv )    
                            MakeTexAnim( &scene, &models[i], name );

                        // if we have a skeleton or something that acts
                        // like one - build anim tables
                        if ( isSkeleton  ||
                            ( strstr( name, "joint") != NULL ) )
                                MakeAnimTable( &scene, &models[i], name ); 

                        //free( name );
                    }

                    if ( verbose >= 1 )
                        fprintf( outStream, "\n" );
                }

                animFile << animData << "\n";
                fprintf( outStream, "\nwriting out %s...\n", animFileName );
                animFile.close();
                }

                //free( models );

                }
                else
                    fprintf( outStream, "Error: Not enough Memory for models...\n");
            }
        }
        // otherwise try to load a model
        else if ( model_name != NULL )
        {

            if ( eggFileName == NULL )
            {    
                eggFileName = 
                    (char *)malloc(sizeof(char)*(strlen( model_name )+13));
                sprintf( eggFileName, "%s", DepointellizeName( model_name ) );

                if ( make_nurbs )
                    strcat( eggFileName, "-nurb" );
                strcat( eggFileName, "-mod.egg" );
            }

            eggFile.open( eggFileName );    

            if ( !eggFile )
            {
                fprintf( outStream, "Couldn't open output file: %s\n", 
                    eggFileName ); 
                exit( 1 );
            }

            if ((result = 
                SAA_elementLoad(&database, &scene, model_name, &model))
                == SI_SUCCESS)
            {
                fprintf( outStream, "Loading single model...\n");
                MakeEgg( NULL, NULL, NULL,  &scene, &model );
            }

            eggFile << _data << "\n";
        }
    }

}

////////////////////////////////////////////////////////////////////
//     Function: MakeEgg
//       Access: Public
//  Description: Make egg geometry from a given model. This include
//                 textures, tex coords, colors, normals, and joints. 
////////////////////////////////////////////////////////////////////
void soft2egg::
MakeEgg( EggGroup *parent, EggJoint *lastJoint, AnimGroup *lastAnim,
            SAA_Scene *scene, SAA_Elem *model )
{
    char        *name;
    char        *fullname;
    SAA_ModelType type;
    int         id = 0;
    int         numShapes;
    int         numTri;
    int         numVert;
    int         numTexLoc = 0;
    int         numTexGlb = 0;
    int         i, j;
    float        matrix[4][4];
    float        *uScale = NULL; 
    float        *vScale = NULL;
    float        *uOffset = NULL;
    float        *vOffset = NULL;
    SAA_Boolean    uv_swap = FALSE;
    void        *relinfo;
    SAA_SubElem *triangles = NULL;
    SAA_Elem    *materials = NULL;
    SAA_SubElem *cvertices = NULL;
    SAA_DVector *cvertPos = NULL;
    SAA_DVector *vertices = NULL;
    SAA_DVector *normals = NULL;
    int            *indices = NULL;
    int            *indexMap = NULL;
    int            *numTexTri = NULL;
    SAA_Elem    *textures = NULL;
    char        **texNameArray;
    float        *uCoords = NULL;
    float        *vCoords = NULL;
    SAA_GeomType gtype = SAA_GEOM_ORIGINAL;
    SAA_Boolean    visible;

    /////////////////////////////////////////////////
    // find out what type of node we're dealing with
    /////////////////////////////////////////////////
    result = SAA_modelGetType( scene, model, &type );

    if ( verbose >= 1 )
    {
        if ( type == SAA_MNILL )
            fprintf( outStream, "encountered null\n"); 
        else if ( type == SAA_MPTCH )
            fprintf( outStream, "encountered patch\n" );
        else if ( type == SAA_MFACE )
            fprintf( outStream, "encountered face\n" );
        else if ( type == SAA_MSMSH )
            fprintf( outStream, "encountered mesh\n" );
        else if ( type == SAA_MJNT )
            fprintf( outStream, "encountered joint\n" );
        else if ( type == SAA_MSPLN )
            fprintf( outStream, "encountered spline\n" );
        else if ( type == SAA_MMETA )
            fprintf( outStream, "encountered meta element\n" );
        else if ( type == SAA_MBALL )
            fprintf( outStream, "encountered metaball\n" );
        else if ( type == SAA_MNCRV )
            fprintf( outStream, "encountered nurb curve\n" );
        else if ( type == SAA_MNSRF )
            fprintf( outStream, "encountered nurbs surf\n" );
        else 
            fprintf( outStream, "encountered unknown type: %d\n", type );
    }

    /////////////////////////////
    // Get the name of the model
    /////////////////////////////

    // Get the FULL name of the model
    fullname = GetFullName( scene, model );

    if ( use_prefix )
    {
        // Get the FULL name of the trim curve
        name = fullname; 
    }
    else
    {
        // Get the name of the trim curve
        name = GetName( scene, model );
    }

    if ( verbose >= 1 )
        fprintf( outStream, "element name <%s>\n", name );

    fflush( outStream );

    // get the model's matrix    
    SAA_modelGetMatrix( scene, model, SAA_COORDSYS_GLOBAL,  matrix );

    if ( verbose >= 2 )
    {
        fprintf( outStream, "model matrix = %f %f %f %f\n", matrix[0][0],
             matrix[0][1],  matrix[0][2],  matrix[0][3] ); 
        fprintf( outStream, "model matrix = %f %f %f %f\n", matrix[1][0],
             matrix[1][1],  matrix[1][2],  matrix[1][3] ); 
        fprintf( outStream, "model matrix = %f %f %f %f\n", matrix[2][0],
             matrix[2][1],  matrix[2][2],  matrix[2][3] ); 
        fprintf( outStream, "model matrix = %f %f %f %f\n", matrix[3][0],
             matrix[3][1],  matrix[3][2],  matrix[3][3] ); 
    }
    
    ///////////////////////////////////////////////////////////////////////
    // check to see if this is a branch we don't want to descend - this
    // will prevent creating geometry for animation control structures
    ///////////////////////////////////////////////////////////////////////
    if ( (strstr( name, "con-" ) == NULL) && 
         (strstr( name, "con_" ) == NULL) && 
         (strstr( name, "fly_" ) == NULL) && 
         (strstr( name, "fly-" ) == NULL) && 
         (strstr( name, "camRIG" ) == NULL) &&
         (strstr( name, "bars" ) == NULL) && 
         // split
         (strstr( fullname, search_prefix ) != NULL) )
    {

    // if making a pose - get deformed geometry
    if ( make_pose )
        gtype = SAA_GEOM_DEFORMED;
        
    // Get the number of key shapes
    SAA_modelGetNbShapes( scene, model, &numShapes );
    if ( verbose >= 1 )
        fprintf( outStream, "MakeEgg: num shapes: %d\n", numShapes);

    ///////////////////////////////////////////////////////////////////////
    // if multiple key shapes exist create table entries for each
    ///////////////////////////////////////////////////////////////////////
    if ( (numShapes > 0) && make_morph )
    {
        has_morph = 1;

        // make sure root morph table exists
        if ( morphRoot == NULL )
            morphRoot = animData.CreateTable( animRoot, "morph" ); 
    
        char   *tableName;

        // create morph table entry for each key shape
        // (start at second shape - as first is the original geometry)
        for ( i = 1; i < numShapes; i++ )
        {
            tableName = MakeTableName( name, i );
            SAnimTable *table = new SAnimTable( );     
            table->name = tableName;
            table->fps = anim_rate;
            morphRoot->children.push_back( table );
            if ( verbose >= 1 )
                fprintf( outStream, "created table named: '%s'\n", tableName );    
        }

        //free( tableName );
    }

    SAA_modelGetNodeVisibility( scene, model, &visible ); 
    if ( verbose >= 1 )
        fprintf( outStream, "model visibility: %d\n", visible ); 
    
    ///////////////////////////////////////////////////////////////////////
    // Only create egg polygon data if: the node is visible, and its not
    // a NULL or a Joint, and we're outputing polys (or if we are outputing 
    // NURBS and the model is a poly mesh or a face) 
    ///////////////////////////////////////////////////////////////////////
    if ( visible &&  
         (type != SAA_MNILL) &&
         (type != SAA_MJNT) && 
         ((make_poly || 
         (make_nurbs && ((type == SAA_MSMSH) || (type == SAA_MFACE )) )) 
         || (!make_poly && !make_nurbs && make_duv && 
            ((type == SAA_MSMSH) || (type == SAA_MFACE )) ))
       )
    {
      // If the model is a NURBS in soft, set its step before tesselating
      if ( type == SAA_MNSRF )
        SAA_nurbsSurfaceSetStep( scene, model, nurbs_step, nurbs_step );

      // If the model is a PATCH in soft, set its step before tesselating
      else if ( type == SAA_MPTCH )
        SAA_patchSetStep( scene, model, nurbs_step, nurbs_step );
 
      // Get the number of triangles    
      result = SAA_modelGetNbTriangles( scene, model, gtype, id, &numTri);
      if ( verbose >= 1 )
          fprintf( outStream, "triangles: %d\n", numTri);

      if ( result != SI_SUCCESS )
      {
        if ( verbose >= 1 ) {
            fprintf( outStream, 
            "Error: couldn't get number of triangles!\n" );
            fprintf( outStream, "\tbailing on model: '%s'\n", name );
        }
        return;    
      }

      // check to see if surface is also skeleton...
      SAA_Boolean isSkeleton = FALSE;

      SAA_modelIsSkeleton( scene, model, &isSkeleton );

      // check to see if this surface is used as a skeleton
      // or is animated via constraint only ( these nodes are
      // tagged by the animator with the keyword "joint"
      // somewhere in the nodes name)
      if ( isSkeleton || (strstr( name, "joint" ) != NULL) )
      {
          if ( verbose >= 1 )
              fprintf( outStream, "animating Polys as joint!!!\n" );

            MakeJoint( scene, lastJoint, lastAnim, model, name );
      }
    
      // model is not a null and has no triangles!
      if ( !numTri )
      {
        if ( verbose >= 1 )
            fprintf( outStream, "no triangles!\n"); 
      }
      else 
      {
        // allocate array of triangles
        triangles = (SAA_SubElem *)malloc(sizeof(SAA_SubElem)*numTri);
        if ( triangles != NULL )
        {
            // triangulate model and read the triangles into array
            SAA_modelGetTriangles( scene, model, gtype, id, numTri, triangles );
        }
        else
            fprintf( outStream, "Not enough Memory for triangles...\n");

        // allocate array of materials
        materials = (SAA_Elem *)malloc(sizeof(SAA_Elem)*numTri);
        if ( materials != NULL )
        {
            // read each triangle's material into array  
            SAA_triangleGetMaterials( scene, model, numTri, triangles, 
                materials );
        }
        else
            fprintf( outStream, "Not enough Memory for materials...\n");

        // allocate array of textures per triangle
        numTexTri = (int *)malloc(sizeof(int)*numTri);

        // find out how many local textures per triangle
        for ( i = 0; i < numTri; i++ )
        {    
            result = SAA_materialRelationGetT2DLocNbElements( scene, 
                                                        &materials[i], FALSE, &relinfo, &numTexTri[i] );

            // polytex    
            if ( result == SI_SUCCESS )
                numTexLoc += numTexTri[i];
        }

        // don't need this anymore...
        //free( numTexTri ); 

        // get local textures if present
        if ( numTexLoc )
        {
            // ASSUME only one texture per material
            textures = (SAA_Elem *)malloc(sizeof(SAA_Elem)*numTri);

            for ( i = 0; i < numTri; i++ )
            {
                // and read all referenced local textures into array
                SAA_materialRelationGetT2DLocElements( scene, &materials[i],
                    TEX_PER_MAT , &textures[i] ); 
            }

            if ( verbose >= 1 )
                fprintf( outStream, "numTexLoc = %d\n", numTexLoc);    
        }
        // if no local textures, try to get global textures
        else
        {
            SAA_modelRelationGetT2DGlbNbElements( scene, model,
                FALSE, &relinfo, &numTexGlb );

            if ( numTexGlb )
            {
                // ASSUME only one texture per model
                textures = (SAA_Elem *)malloc(sizeof(SAA_Elem));

                // get the referenced texture
                SAA_modelRelationGetT2DGlbElements( scene, model, 
                    TEX_PER_MAT, textures ); 

                if ( verbose >= 1 )
                    fprintf( outStream, "numTexGlb = %d\n", numTexGlb);    
            }
        }

        // allocate array of control vertices 
        cvertices = (SAA_SubElem *)malloc(sizeof(SAA_SubElem)*numTri*3);
        if ( cvertices != NULL )
        {
            // read each triangle's control vertices into array
            SAA_triangleGetCtrlVertices( scene, model, gtype, id,
                numTri, triangles, cvertices );

            if ( verbose >= 2 )
            {
                cvertPos = (SAA_DVector *)malloc(sizeof(SAA_DVector)*numTri*3);
                SAA_ctrlVertexGetPositions(  scene, model, numTri*3, 
                    cvertices, cvertPos);

                for ( i=0; i < numTri*3; i++ )
                {
                    fprintf( outStream, "cvert[%d] = %f %f %f %f\n", i, 
                        cvertPos[i].x, cvertPos[i].y, cvertPos[i].z, 
                        cvertPos[i].w );
                }
            }
        }
        else
            fprintf( outStream, "Not enough Memory for control vertices...\n");

        // allocate array of control vertex indices
        // this array maps from the redundant cvertices array into
        // the unique vertices array (cvertices->vertices)
        indices = (int *)malloc(sizeof(int)*numTri*3);
        if ( indices != NULL )
        {
            for ( i=0; i < numTri*3; i++ )
                indices[i] = 0;

            SAA_ctrlVertexGetIndices( scene, model, numTri*3, 
                cvertices, indices );
       
            if ( verbose >= 2 ) 
                for ( i=0; i < numTri*3; i++ )
                    fprintf( outStream, "indices[%d] = %d\n", i, indices[i] );
        }
        else
            fprintf( outStream, "Not enough Memory for indices...\n");

        // get number of UNIQUE vertices in model
        SAA_modelGetNbTriVertices( scene, model, &numVert );

        if ( verbose >= 2 )
            fprintf( outStream, "num unique verts = %d\n", numVert );

        //allocate array of vertices
        vertices = (SAA_DVector *)malloc(sizeof(SAA_DVector)*numVert);

        // get the UNIQUE vertices of all triangles in model
        SAA_modelGetTriVertices( scene, model, numVert, vertices );

        if ( verbose >= 2 )
        {
            for ( i=0; i < numVert; i++ )
            {
                fprintf( outStream, "vertices[%d] = %f ", i, vertices[i].x );
                fprintf( outStream, "%f %f %f\n", vertices[i].y, 
                vertices[i].z, vertices[i].w );
            }
        }

        // allocate indexMap array
        // we contruct this array to map from the unique vertices
        // array to the redundant cvertices array - it will save
        // us from doing repetitive searches later
        indexMap = MakeIndexMap( indices, numTri*3, numVert ); 

        // allocate array of normals
        normals = (SAA_DVector *)malloc(sizeof(SAA_DVector)*numTri*3);
        if ( normals != NULL )
        {
            // read each control vertex's normals into an array
            SAA_ctrlVertexGetNormals( scene, model, numTri*3,
                cvertices, normals );
        }
        else
            fprintf( outStream, "Not enough Memory for normals...\n");

        if ( verbose >= 2 )
        {
            for ( i=0; i<numTri*3; i++ )
                fprintf( outStream, "normals[%d] = %f %f %f %f\n", i,
                    normals[i].x, normals[i].y, normals[i].z, normals[i].w );
        }

        int uRepeat, vRepeat;

        // make sure we have textures before we get t-coords
        if ( numTexLoc )
        {
            // allocate arrays for u & v coords
            uCoords = (float *)malloc(sizeof(float)*numTri*numTexLoc*3); 
            vCoords = (float *)malloc(sizeof(float)*numTri*numTexLoc*3); 
          
            // read the u & v coords into the arrays
            if ( uCoords != NULL && vCoords != NULL)
            {
              for ( i = 0; i < numTri*numTexLoc*3; i++ )
                uCoords[i] = vCoords[i] = 0.0f;
 
                SAA_ctrlVertexGetUVTxtCoords( scene, model, numTri*3,
                    cvertices, numTexLoc*3, uCoords, vCoords );
            }
            else
                fprintf( outStream, "Not enough Memory for texture coords...\n");

            if ( verbose >= 2 )
            {
                for ( i=0; i<numTexLoc*3; i++ )
                    fprintf( outStream, "texcoords[%d] = ( %f , %f )\n", i, 
                        uCoords[i], vCoords[i] );
            }

            // allocate arrays of texture info
            uScale = ( float *)malloc(sizeof(float)*numTri);
            vScale = ( float *)malloc(sizeof(float)*numTri);
            uOffset = ( float *)malloc(sizeof(float)*numTri);
            vOffset = ( float *)malloc(sizeof(float)*numTri);
            texNameArray = ( char **)malloc(sizeof(char *)*numTri);

            for ( i = 0; i < numTri; i++ )
            {
                // initialize the array value
                texNameArray[i] = NULL;

                SAA_Boolean    valid = FALSE;
                // check to see if texture is present
                result = SAA_elementIsValid( scene, &textures[i], &valid );
               
                if ( result != SI_SUCCESS )
                    fprintf( outStream, "SAA_elementIsValid failed!!!!\n" );
 
                // texture present - get the name and uv info 
                if ( valid )
                {
                    texNameArray[i] = ConvertTexture( scene, &textures[i] );
                   
                    if ( verbose >= 2 ) 
                        fprintf( outStream, " tritex[%d] named: %s\n", i, 
                            texNameArray[i] );

                    SAA_texture2DGetUVSwap( scene, &textures[i], &uv_swap );

                    if ( verbose >= 2 )
                        if ( uv_swap == TRUE )
                            fprintf( outStream, " swapping u and v...\n" );

                    SAA_texture2DGetUScale( scene, &textures[i], &uScale[i] );
                    SAA_texture2DGetVScale( scene, &textures[i], &vScale[i] );
                    SAA_texture2DGetUOffset( scene, &textures[i], &uOffset[i] );
                    SAA_texture2DGetVOffset( scene, &textures[i], &vOffset[i] );

                    if ( verbose >= 2 )
                    {    
                        fprintf(outStream, "tritex[%d] uScale: %f vScale: %f\n",                             i, uScale[i], vScale[i] );
                        fprintf(outStream, " uOffset: %f vOffset: %f\n", 
                            uOffset[i], vOffset[i] );
                    }


                    SAA_texture2DGetRepeats(  scene, &textures[i], &uRepeat,
                        &vRepeat );

                    if ( verbose >= 2 )
                    {
                        fprintf(outStream, "uRepeat = %d, vRepeat = %d\n",
                            uRepeat, vRepeat );
                    }
                }
                else 
                {
                    if ( verbose >= 2 ) 
                    {
                        fprintf( outStream, "Invalid texture...\n");
                        fprintf( outStream, " tritex[%d] named: (null)\n", i );
                    }
                }
            }

            //debug
            //for ( i = 0; i < numTri; i++ )
            //{
                //if ( texNameArray[i] != NULL )
                    //fprintf( outStream, " tritex[%d] named: %s\n", i, 
                        //texNameArray[i] );
                //else
                    //fprintf( outStream, " tritex[%d] named: (null)\n", i );
            //}
        }
        // make sure we have textures before we get t-coords
        else if ( numTexGlb )
        {
            SAA_Boolean    valid;

            // check to see if texture is present
            SAA_elementIsValid( scene, textures, &valid );
                
            // texture present - get the name and uv info 
            if ( valid )
            {
                SAA_texture2DGetUVSwap( scene, textures, &uv_swap );

                if ( verbose >= 1 )
                    if ( uv_swap == TRUE )
                        fprintf( outStream, " swapping u and v...\n" );

                // allocate arrays for u & v coords
                uCoords = (float *)malloc(sizeof(float)*numTri*numTexGlb*3); 
                vCoords = (float *)malloc(sizeof(float)*numTri*numTexGlb*3); 

                for ( i = 0; i < numTri*numTexGlb*3; i++ )
                {
                    uCoords[i] = vCoords[i] = 0.0f;
                }                
                    
                // read the u & v coords into the arrays
                if ( uCoords != NULL && vCoords != NULL)
                {
                    SAA_triCtrlVertexGetGlobalUVTxtCoords( scene, model, 
                        numTri*3, cvertices, numTexGlb, textures, 
                        uCoords, vCoords );
                }
                else
                    fprintf( outStream, "Not enough Memory for texture coords...\n");

                if ( verbose >= 2 )
                  {
                    for ( i=0; i<numTri*numTexGlb*3; i++ )
                      fprintf( outStream, "texcoords[%d] = ( %f , %f )\n", i, 
                               uCoords[i], vCoords[i] );
                  }
                
                texNameArray = ( char **)malloc(sizeof(char *));
                *texNameArray = ConvertTexture( scene, textures );

                if ( verbose >= 1 )    
                    fprintf( outStream, " global tex named: %s\n", 
                        texNameArray );
        
                // allocate arrays of texture info
                uScale = ( float *)malloc(sizeof(float));
                vScale = ( float *)malloc(sizeof(float));
                uOffset = ( float *)malloc(sizeof(float));
                vOffset = ( float *)malloc(sizeof(float));

                SAA_texture2DGetUScale( scene, textures, uScale );
                SAA_texture2DGetVScale( scene, textures, vScale );
                SAA_texture2DGetUOffset( scene, textures, uOffset );
                SAA_texture2DGetVOffset( scene, textures, vOffset );

                if ( verbose >= 1 ) 
                {
                    fprintf( outStream, " global tex uScale: %f vScale: %f\n", 
                        *uScale, *vScale );
                    fprintf( outStream, " uOffset: %f vOffset: %f\n", 
                        *uOffset, *vOffset );
                }

                SAA_texture2DGetRepeats(  scene, textures, &uRepeat,
                    &vRepeat );

                if ( verbose >= 2 )
                {
                    fprintf(outStream, "uRepeat = %d, vRepeat = %d\n",
                        uRepeat, vRepeat );
                }
            }
            else fprintf( outStream, "Invalid texture...\n");
        }

        // make the egg vertex pool 
        EggVertexPool *pool = _data.CreateVertexPool( parent, name );

        for ( i = 0; i < numVert; i++ )
        {
            pfVec3    eggVert; 
            pfVec3    eggNorm; 
            
            //convert to global coords
            SAA_DVector local = vertices[i];
            SAA_DVector global;

            _VCT_X_MAT( global, local, matrix );

            // set vertices array to reflect global coords
            //vertices[i].x = global.x;
            //vertices[i].y = global.y;
            //vertices[i].z = global.z;

            //eggVert.set( vertices[i].x, vertices[i].y, vertices[i].z );

            // we'll preserve original verts for now
            eggVert.set( global.x, global.y, global.z );

            local = normals[indexMap[i]];

            _VCT_X_MAT( global, local, matrix );

            eggNorm.set( global.x, global.y, global.z );
            eggNorm.normalize();

            pool->AddVertex( eggVert, i );
            pool->Vertex(i)->attrib.SetNormal( eggNorm );

            // translate local uv's to global and add to vertex pool
            if ( numTexLoc && (uCoords != NULL && vCoords !=NULL ))
            {
                float u, v;

                if ( ignore_tex_offsets ) {
                  u = uCoords[indexMap[i]];
                  v = 1.0f - vCoords[indexMap[i]];
                } else {
                  u = (uCoords[indexMap[i]] - uOffset[indexMap[i]/3]) /
                    uScale[indexMap[i]/3];
                  
                  v = 1.0f - ((vCoords[indexMap[i]] - vOffset[indexMap[i]/3]) /
                              vScale[indexMap[i]/3]);
                }
                
                if ( isNum(u) && isNum(v) )
                { 
                    if ( uv_swap == TRUE )
                        pool->Vertex(i)->attrib.SetUV( v, u );
                    else
                        pool->Vertex(i)->attrib.SetUV( u, v );
                }
            }
            else if ( numTexGlb && (uCoords != NULL && vCoords !=NULL ) )
            {
                float u, v;
            
                if ( ignore_tex_offsets ) {
                  u = uCoords[indexMap[i]];
                  v = 1.0f - vCoords[indexMap[i]];
                } else {
                  u = (uCoords[indexMap[i]] - *uOffset) / *uScale;
                  v = 1.0f - (( vCoords[indexMap[i]] - *vOffset ) / *vScale);
                }

                if ( isNum(u) && isNum(v) )
                { 
                    if ( uv_swap == TRUE )
                        pool->Vertex(i)->attrib.SetUV( v, u );
                    else
                        pool->Vertex(i)->attrib.SetUV( u, v );
                }
                        
            }
             
            // if we've encountered textures and we desire duv anims
            if (( numTexLoc || numTexGlb ) && make_duv )
            {
                int            numExp;
                SAA_Elem   *tex;

                // grab the current texture
                if ( numTexLoc ) 
                    tex = &textures[0];
                else
                    tex = textures;

                // find how many expressions for this shape
                SAA_elementGetNbExpressions( scene, tex, NULL, FALSE, 
                    &numExp );

                // if it has expressions we'll assume its animated
                if ( numExp )
                {
                    // if animated object make base duv's, animtables
                    // for the duv's and store the original offsets
                    strstream uName, vName;

                    // create duv target names
                    uName << name << ".u" << ends;
                    vName << name << ".v" << ends;

                    // only create tables and store offsets
                    // on a per model basis (not per vertex)
                    if ( !i )
                    {

                        // make sure root morph table exists
                        if ( morphRoot == NULL )
                            morphRoot = animData.CreateTable( animRoot, 
                                "morph" ); 

                        // create morph table entry for each duv
                        SAnimTable *uTable = new SAnimTable( );     
                        uTable->name = uName.str();
                        uTable->fps = anim_rate;
                        morphRoot->children.push_back( uTable );
                        if ( verbose >= 1 )
                            fprintf( outStream, "created duv table named: %s\n",                                uName.str() );    

                        SAnimTable *vTable = new SAnimTable( );     
                        vTable->name = vName.str();
                        vTable->fps = anim_rate;
                        morphRoot->children.push_back( vTable );
                        if ( verbose >= 1 )
                            fprintf( outStream, "created duv table named: %s\n",                                vName.str() );    

                        float    texOffsets[4];

                        if ( numTexGlb )
                        {
                            texOffsets[0] = *uOffset;
                            texOffsets[1] = *vOffset;
                            texOffsets[2] = *uScale;
                            texOffsets[3] = *vScale;
                        }
                        else
                        {
                            texOffsets[0] = uOffset[indexMap[i]/3];
                            texOffsets[1] = vOffset[indexMap[i]/3];
                            texOffsets[2] = uScale[indexMap[i]/3];
                            texOffsets[3] = vScale[indexMap[i]/3];
                        }

                        // remember original texture offsets future reference
                        SAA_elementSetUserData( scene, model, "TEX_OFFSETS", 
                            sizeof( texOffsets ), TRUE, (void  **)&texOffsets );
                    }

                    EggMorphOffset *duvU;
                    EggMorphOffset *duvV;

                    // generate base duv's for this vertex
                    duvU = new EggMorphOffset( uName.str(), 1.0 , 0.0 );    
                    pool->Vertex(i)->attrib.uv_morphs.push_back( *duvU );
            
                    duvV = new EggMorphOffset( vName.str(), 0.0 , 1.0 );    
                    pool->Vertex(i)->attrib.uv_morphs.push_back( *duvV );

                } // if ( numExp )

            } // if ( numTexLoc || numTexGlb )

        } // for ( i = 0; i < numVert; i++ )

        // if model has key shapes, generate vertex offsets
        if ( has_morph && make_morph )
            MakeVertexOffsets( scene, model, type, numShapes, numVert, 
                vertices, matrix,  name );


        // create vertex ref list for all polygons in the model
        EggVertexRef *vref;

        vref = new EggVertexRef( pool);
        for ( i = 0; i < numVert; i++ )
        {
            //add each vert in pool to last joint for hard skinning
            vref->indices.push_back( EggVertexIndex( i ) );
        }

        // hard assign poly geometry if no soft-skinning requested
        //
        //disabled 1/1/99 to streamline joint assignments.
        // all hard-skinning now done in CleanUpSoftSkin.
        //if ( !make_soft )
        //{
            //if ( lastJoint != NULL )
            //{
                //lastJoint->vrefs.AddUniqueNode( *vref );

                //if ( verbose >= 1 )
                    //fprintf( outStream, "hard-skinning %s (%d vertices)\n", 
                        //name, i+1 );
            //}
        //}

        // make an egg group to hold all triangles
        EggGroup *group = _data.CreateGroup( parent, name);

        // make this group the current parent
        parent = group;

        EggPolygon *poly = NULL;
        EggColor *cref = NULL;
        EggTexture *tref = NULL;

        // for each triangle
        for ( i = 0; i < numTri*3; i+=3 )
        {
            float    r,g,b,a;
            pfVec4    color;

            // make egg poly for each traingle and reference
            // the appropriate vertex in the pool
            poly = _data.CreatePolygon( group, pool );
            poly->AddVertex(indices[i]);
            poly->AddVertex(indices[i+1]);
            poly->AddVertex(indices[i+2]);

            // check for back face flag in model note info
            char *modelNoteStr = GetModelNoteInfo( scene, model );

            if ( modelNoteStr != NULL )
            {
                if ( strstr( modelNoteStr, "bface" ) != NULL )
                    poly->flags |= EG_BFACE;
            }
            
            // check to see if material is present
            SAA_Boolean    valid;
            SAA_elementIsValid( scene, &materials[i/3], &valid );

            // material present - get the color 
            if ( valid )
            {
                SAA_materialGetDiffuse( scene, &materials[i/3], &r, &g, &b );
                SAA_materialGetTransparency( scene, &materials[i/3], &a );
                color.set( r, g, b, 1.0f - a );
            }
            // no material - default to white
            else
                color.set( 1.0, 1.0, 1.0, 1.0 );

            cref = _data.CreateColor(color);
            poly->attrib.SetCRef(cref);

            strstream uniqueTexName;

            if (numTexLoc)
            {
                // polytex
                if ( (texNameArray[i/3] != NULL) &&
                        (strcmp(texNameArray[i/3], "NULL") != 0) )
                {
                    // append unique identifier to texname for
                    // this particular object
                    uniqueTexName << name << "-" 
                        << RemovePathName(texNameArray[i/3]);

                    tref = _data.CreateTexture( texNameArray[i/3], 
                        uniqueTexName.str() ); 

                    if ( verbose >= 1 )
                        fprintf( outStream, " tritex[%d] named: %s\n", i/3, 
                            texNameArray[i/3] );
                }
            }
            else if ( numTexGlb )
            {
                if ( texNameArray != NULL )
                {
                    // append unique identifier to texname for
                    // this particular object
                    uniqueTexName << name << "-"
                        << RemovePathName(*texNameArray);

                    tref = _data.CreateTexture( *texNameArray, 
                        uniqueTexName.str() );

                    if ( verbose >= 1 )
                        fprintf( outStream, " tritex named: %s\n",
                             *texNameArray );
                }
            }

            // set the clamp on the texture
            if ( tref != NULL )
            {
                if ( uRepeat > 0 )
                    tref->wrapu = EggTexture::WM_repeat;
                else
                    tref->wrapu = EggTexture::WM_clamp;

                if ( vRepeat > 1 )
                    tref->wrapv = EggTexture::WM_repeat;
                else
                    tref->wrapv = EggTexture::WM_clamp;

                poly->attrib.SetTRef(tref);
            }

        }

        // we're done - trash triangles...
        SAA_modelClearTriangles( scene, model );

        // free molloc'd memory
        //free( triangles );
        //free( materials );
        //free( normals );
        //free( cvertices );
        //free( vertices );
        //free( indices );
        //free( indexMap );

        // free these only if they were malloc'd for textures
        if (numTexLoc || numTexGlb)
        {
            //free( textures );
            //free( uCoords );
            //free( vCoords );
            //free( texNameArray );
            //free( uScale );
            //free( vScale );
            //free( uOffset );
            //free( vOffset );
        }
      }
    }
    else
    {
        ///////////////////////////////////////
        // check to see if its a nurbs surface
        ///////////////////////////////////////
        if ( (type == SAA_MNSRF) && ( visible ) && (( make_nurbs ) 
            || ( !make_nurbs && !make_poly &&  make_duv )) )
        {
            // check to see if NURBS is also skeleton...
            SAA_Boolean isSkeleton = FALSE;

            SAA_modelIsSkeleton( scene, model, &isSkeleton );

            // check to see if this NURBS is used as a skeleton
            // or is animated via constraint only ( these nodes are
            // tagged by the animator with the keyword "joint"
            // somewhere in the nodes name)
            if ( isSkeleton || (strstr( name, "joint" ) != NULL) )
            {
                MakeJoint( scene, lastJoint, lastAnim, model, name );
                geom_as_joint = 1;
                if ( verbose >= 1 )
                    fprintf( outStream, "animating NURBS as joint!!!\n" );
            }

            EggNurbsSurface    *eggNurbsSurf = new EggNurbsSurface( name );
            int uDegree, vDegree;

            // create nurbs representation of surface
            SAA_nurbsSurfaceGetDegree( scene, model, &uDegree, &vDegree );
            eggNurbsSurf->u_order = uDegree + 1;
            eggNurbsSurf->v_order = vDegree + 1;
            if ( verbose >= 1 )
            {
                fprintf( outStream, "nurbs degree: %d u, %d v\n", 
                    uDegree, vDegree );
                fprintf( outStream, "nurbs order: %d u, %d v\n", 
                    uDegree + 1, vDegree + 1 );
            }

            SAA_Boolean    uClosed = FALSE;
            SAA_Boolean    vClosed = FALSE;

            SAA_nurbsSurfaceGetClosed( scene, model, &uClosed, &vClosed);    

            if ( verbose >= 1 )
            {    
                if ( uClosed )
                    fprintf( outStream, "nurbs is closed in u...\n");
                if ( vClosed )
                    fprintf( outStream, "nurbs is closed in v...\n");
            }    
            
            int uRows, vRows;
            SAA_nurbsSurfaceGetNbVertices( scene, model, &uRows, &vRows );
            if ( verbose >= 1 )
                fprintf( outStream, "nurbs vertices: %d u, %d v\n", 
                    uRows, vRows );
            
            int uCurves, vCurves;
            SAA_nurbsSurfaceGetNbCurves( scene, model, &uCurves, &vCurves );
            if ( verbose >= 1 )
                fprintf( outStream, "nurbs curves: %d u, %d v\n", 
                    uCurves, vCurves );

            if ( shift_textures )
            {
            if ( uClosed )
                // shift starting point on NURBS surface for correct textures
                SAA_nurbsSurfaceShiftParameterization( scene, model, -2, 0 );

            if ( vClosed )
                // shift starting point on NURBS surface for correct textures
                SAA_nurbsSurfaceShiftParameterization( scene, model, 0, -2 );
            }

            SAA_nurbsSurfaceSetStep( scene, model, nurbs_step, nurbs_step );

            // check for back face flag in model note info
            char *modelNoteStr = GetModelNoteInfo( scene, model );

            if ( modelNoteStr != NULL )
            {
                if ( strstr( modelNoteStr, "bface" ) != NULL )
                    eggNurbsSurf->flags |= EG_BFACE;
            }
            
            int numKnotsU, numKnotsV;
            
            SAA_nurbsSurfaceGetNbKnots( scene, model, &numKnotsU, &numKnotsV );
            if ( verbose >= 1 )
                fprintf( outStream, "nurbs knots: %d u, %d v\n", 
                    numKnotsU, numKnotsV );

            double *knotsU, *knotsV;    
            knotsU = (double *)malloc(sizeof(double)*numKnotsU);
            knotsV = (double *)malloc(sizeof(double)*numKnotsV);
            SAA_nurbsSurfaceGetKnots( scene, model, gtype, 0, 
                numKnotsU, numKnotsV, knotsU, knotsV );

            if ( verbose >= 2 )
                fprintf( outStream, "u knots:\n" );

            AddKnots( eggNurbsSurf->u_knots, knotsU, numKnotsU, uClosed, uDegree ); 
            if ( verbose >= 2 )
                fprintf( outStream, "v knots:\n" );

            AddKnots( eggNurbsSurf->v_knots, knotsV, numKnotsV, vClosed, vDegree); 

            //free( knotsU );
            //free( knotsV );

            // set sub_div so we can see it in perfly
            eggNurbsSurf->u_subdiv = (uRows-1)*nurbs_step;
            eggNurbsSurf->v_subdiv = (vRows-1)*nurbs_step;

            SAA_modelGetNbVertices( scene, model, &numVert );
        
            if ( verbose >= 2 )    
                fprintf( outStream, "%d CV's\n", numVert ); 

            // get the CV's
            vertices = (SAA_DVector *)malloc(sizeof(SAA_DVector)*numVert);
            SAA_modelGetVertices( scene, model, gtype, 0,
                numVert, vertices );

            // create pool of NURBS vertices 
            EggVertexPool *pool = _data.CreateVertexPool( parent, name );
            eggNurbsSurf->SetVertexPool( pool );

            // create vertex ref list for all cv's in the model
            EggVertexRef *vref;

            vref = new EggVertexRef( pool);

            for ( int k = 0; k<numVert; k++ )
            {
                if ( verbose >= 2 )
                {
                    fprintf( outStream, "original cv[%d] = %f %f %f %f\n", k, 
                        vertices[k].x, vertices[k].y, vertices[k].z, 
                        vertices[k].w );
                }

                pfVec4  eggVert;

                // convert to global coords
                SAA_DVector global;

                _VCT_X_MAT( global, vertices[k], matrix );

                //preserve original weight
                global.w = vertices[k].w;

                // normalize coords to weight
                global.x *= global.w;
                global.y *= global.w;
                global.z *= global.w;

                // this code is commented out because I
                // am no longer sending global data to
                // the other routines (ie makevertexoffset)

                // set vertices array to reflect global coords
                //vertices[k].x = global.x;
                //vertices[k].y = global.y;
                //vertices[k].z = global.z;
                //vertices[k].w = global.w;

                //if ( verbose >= 2 )
                //{
                    //fprintf( outStream, "global cv[%d] = %f %f %f %f\n", k, 
                        //vertices[k].x, vertices[k].y, vertices[k].z, 
                        //vertices[k].w );
                //}

                //eggVert.set( vertices[k].x, vertices[k].y, vertices[k].z, 
                    //vertices[k].w );

                if ( verbose >= 2 )
                {
                    fprintf( outStream, "global cv[%d] = %f %f %f %f\n", k, 
                        global.x, global.y, global.z, 
                        global.w );
                }

                eggVert.set( global.x, global.y, global.z, 
                    global.w );

                // populate vertex pool
                pool->AddVertex( eggVert, k );

                // add vref's to NURBS info 
                eggNurbsSurf->AddVertex( k ); 

                //add each vert in pool to vref for hard skinning
                vref->indices.push_back( EggVertexIndex( k ) );
                
                // check to see if the NURB is closed in u    
                if ( uClosed )
                {
                    // add first uDegree verts to end of row
                    if ( (k % uRows) == ( uRows - 1) )
                    for ( int i = 0; i < uDegree; i++ ) 
                    {
                        // add vref's to NURBS info 
                        eggNurbsSurf->AddVertex( i+((k/uRows)*uRows) ); 

                        //add each vert to vref 
                        vref->indices.push_back( 
                            EggVertexIndex( i+((k/uRows)*uRows) ) );
                    }
                } 
            }

            // if hard skinned or this nurb is also a joint
            //
            //disabled 1/1/99 to streamline joint assignments.
            // all hard skinning now done in CleanUpSoftSkin.
            //if (!make_soft || geom_as_joint)
            //{
                //add the new cv references to the last 
                //joint for hard skinning only 
                //if ( lastJoint != NULL )
                //{
                    //lastJoint->vrefs.AddUniqueNode( *vref );
                    //geom_as_joint = 0;    
                    //if ( verbose >= 1 )    
                        //fprintf( outStream, "Doing NURBS hard skinning...\n");
                //}
            //}

            // check to see if the NURB is closed in v    
            if ( vClosed && !uClosed )
            {
                // add first vDegree rows of verts to end of list
                for ( int i = 0; i < vDegree*uRows; i++ ) 
                    eggNurbsSurf->AddVertex( i ); 
            }
            // check to see if the NURB is closed in u and v    
            else if ( vClosed && uClosed )
            {
                // add the first (degree) v verts and a few
                // extra - for good measure
                for ( i = 0; i < vDegree; i++ ) 
                {
                    // add first vDegree rows of verts to end of list
                    for ( j = 0; j < uRows; j++ )
                        eggNurbsSurf->AddVertex( j+(i*uRows) );

                    // if u is closed to we have added uDegree
                    // verts onto the ends of the rows - add them here too
                    for ( k = 0; k < uDegree; k++ )
                        eggNurbsSurf->AddVertex( k+(i*uRows)+((k/uRows)*uRows) );
                }

            }

            // get the color of the NURBS surface
            int numNurbMats;
            EggColor *nurbCref;
            pfVec4    nurbColor;

            SAA_modelRelationGetMatNbElements( scene, model, FALSE, &relinfo,
                &numNurbMats ); 

            if ( verbose >= 1 )
                fprintf( outStream, "nurbs surf has %d materials\n", 
                    numNurbMats );

            if ( numNurbMats )
            {
                float r,g,b,a;

                materials = (SAA_Elem *)malloc(sizeof(SAA_Elem)*numNurbMats);
                
                SAA_modelRelationGetMatElements( scene, model, relinfo, 
                    numNurbMats, materials ); 

                SAA_materialGetDiffuse( scene, &materials[0], &r, &g, &b );
                SAA_materialGetTransparency( scene, &materials[0], &a );
                nurbColor.set( r, g, b, 1.0f - a );
                //nurbColor.set( r, g, b, 1.0 );

                nurbCref = _data.CreateColor(nurbColor);
                eggNurbsSurf->attrib.SetCRef(nurbCref);
        
                //get the texture of the NURBS surface from the material
                int numNurbTexLoc = 0;
                int numNurbTexGlb = 0;

                // ASSUME only one texture per material
                SAA_Elem nurbTex;

                // find out how many local textures per NURBS surface
                // ASSUME it only has one material
                SAA_materialRelationGetT2DLocNbElements( scene, &materials[0],
                    FALSE, &relinfo, &numNurbTexLoc );

                // if present, get local textures
                if ( numNurbTexLoc )
                {
                    if ( verbose >= 1 )
                        fprintf( outStream, "%s had %d local tex\n", name, 
                            numNurbTexLoc );

                    // get the referenced texture
                    SAA_materialRelationGetT2DLocElements( scene, &materials[0],
                        TEX_PER_MAT, &nurbTex ); 

                }
                // if no locals, try to get globals
                else
                {
                    SAA_modelRelationGetT2DGlbNbElements( scene, model,
                        FALSE, &relinfo, &numNurbTexGlb );

                    if ( numNurbTexGlb )
                    {
                        if ( verbose >= 1 )
                            fprintf( outStream, "%s had %d global tex\n", name, 
                                numNurbTexGlb );

                            // get the referenced texture
                            SAA_modelRelationGetT2DGlbElements( scene, 
                                model, TEX_PER_MAT, &nurbTex ); 
                        }
                }

                // add tex ref's if we found any textures
                if ( numNurbTexLoc || numNurbTexGlb) 
                {
                    char    *texName = NULL;
                    char    *uniqueTexName = NULL;
                    EggTexture *tref;
                    pfMatrix  nurbTexMat;


                    // convert the texture to .rgb and adjust name 
                    texName = ConvertTexture( scene, &nurbTex );

                    // append unique identifier to texname for
                    // this particular object
                    uniqueTexName = (char *)malloc(sizeof(char)*
                        (strlen(name)+strlen(texName)+3) );
                    sprintf( uniqueTexName, "%s-%s", name, 
                        RemovePathName(texName) );

                    if ( verbose >= 1 )
                    {
                        fprintf( outStream, "creating tref %s\n", 
                            uniqueTexName );
                    }

                    tref = _data.CreateTexture( texName, uniqueTexName );

                    uScale = ( float *)malloc(sizeof(float));
                    vScale = ( float *)malloc(sizeof(float));
                    uOffset = ( float *)malloc(sizeof(float));
                    vOffset = ( float *)malloc(sizeof(float));

                    // get texture offset info
                    SAA_texture2DGetUScale( scene, &nurbTex, uScale );
                    SAA_texture2DGetVScale( scene, &nurbTex, vScale );
                    SAA_texture2DGetUOffset( scene, &nurbTex, uOffset );
                    SAA_texture2DGetVOffset( scene, &nurbTex, vOffset );
                    SAA_texture2DGetUVSwap( scene, &nurbTex, &uv_swap );


                    if ( verbose >= 1 )
                    {
                        fprintf( outStream, "nurbTex uScale: %f\n", *uScale );
                        fprintf( outStream, "nurbTex vScale: %f\n", *vScale );
                        fprintf( outStream, "nurbTex uOffset: %f\n", *uOffset );
                        fprintf( outStream, "nurbTex vOffset: %f\n", *vOffset );
                        if ( uv_swap )
                            fprintf( outStream, "nurbTex u & v swapped!\n" );
                        else
                            fprintf( outStream, "nurbTex u & v NOT swapped\n" );
                    }

                    nurbTexMat.makeIdent();

                    if ( !ignore_tex_offsets )
                    {
                        if ( uv_swap )
                        {
                            nurbTexMat[0][0] = 0.0f;
                            nurbTexMat[1][1] = 0.0f;
                            nurbTexMat[0][1] = 1 / *vScale;
                            nurbTexMat[1][0] = 1 / *uScale;
                            nurbTexMat[2][1] = -(*uOffset / *uScale);
                            nurbTexMat[2][0] = -(*vOffset / *vScale);
                        }
                        else
                        {
                            nurbTexMat[0][0] = 1 / *uScale;
                            nurbTexMat[1][1] = 1 / *vScale;
                            nurbTexMat[2][0] = -(*uOffset / *uScale);
                            nurbTexMat[2][1] = -(*vOffset / *vScale);
                        }
                    }


    //call printMat
    if ( verbose >= 2 )
    {    
    fprintf( outStream, "nurb tex matrix = %f %f %f %f\n", nurbTexMat[0][0],
         nurbTexMat[0][1],  nurbTexMat[0][2],  nurbTexMat[0][3] ); 
    fprintf( outStream, "nurb tex matrix = %f %f %f %f\n", nurbTexMat[1][0],
         nurbTexMat[1][1],  nurbTexMat[1][2],  nurbTexMat[1][3] ); 
    fprintf( outStream, "nurb tex matrix = %f %f %f %f\n", nurbTexMat[2][0],
         nurbTexMat[2][1],  nurbTexMat[2][2],  nurbTexMat[2][3] ); 
    fprintf( outStream, "nurb tex matrix = %f %f %f %f\n", nurbTexMat[3][0],
         nurbTexMat[3][1],  nurbTexMat[3][2],  nurbTexMat[3][3] ); 
    }


                    tref->tex_mat = nurbTexMat;
                    tref->flags |= EFT_TRANSFORM;

                    eggNurbsSurf->attrib.SetTRef(tref);

                }

                // if we've encountered textures and we desire duv anims
                if (( numNurbTexLoc || numNurbTexGlb ) && make_duv )
                {
                    int            numExp;

                     // find how many expressions for this shape
                    SAA_elementGetNbExpressions( scene, &nurbTex, NULL, FALSE, 
                        &numExp );

                    // if it has expressions we'll assume its animated
                    if ( numExp )
                    {
                        if ( verbose > 1 )
                            printf( "nurbTex has %d expressions...\n", numExp );

                        // if animated object make base duv's, animtables
                        // for the duv's and store the original offsets
                        strstream uName, vName;

                        // create duv target names
                        uName << name << ".u" << ends;
                        vName << name << ".v" << ends;

                        // make sure root morph table exists
                        if ( morphRoot == NULL )
                            morphRoot = animData.CreateTable( animRoot, 
                                "morph" ); 

                        // create morph table entry for each duv
                        SAnimTable *uTable = new SAnimTable( );     
                        uTable->name = uName.str();
                        uTable->fps = anim_rate;
                        morphRoot->children.push_back( uTable );
                        if ( verbose >= 1 )
                            fprintf( outStream, "created duv table named: %s\n",                                uName.str() );    

                        SAnimTable *vTable = new SAnimTable( );     
                        vTable->name = vName.str();
                        vTable->fps = anim_rate;
                        morphRoot->children.push_back( vTable );
                        if ( verbose >= 1 )
                            fprintf( outStream, "created duv table named: %s\n",                                vName.str() );    

                        float    texOffsets[4];

                        texOffsets[0] = *uOffset;
                        texOffsets[1] = *vOffset;
                        texOffsets[2] = *uScale;
                        texOffsets[3] = *vScale;

                        // remember original texture offsets future reference
                        SAA_elementSetUserData( scene, model, "TEX_OFFSETS", 
                            sizeof( texOffsets ), TRUE, (void  **)&texOffsets );

                        // create UV's and duv's for each vertex 
                        for( i = 0; i < numVert; i++ )
                        {
                            pfVec2            tmpUV;
                            EggMorphOffset *duvU;
                            EggMorphOffset *duvV;

                            //create uv's so we can store duv's
                            eggNurbsSurf->CalcActualUV( i, tmpUV );
                            pool->Vertex(i)->attrib.SetUV( tmpUV[0], tmpUV[1] );
                            
                            // generate base duv's for this vertex
                            duvU = new EggMorphOffset(uName.str(), 1.0 , 0.0);    
                            pool->Vertex(i)->attrib.uv_morphs.push_back(*duvU);
                    
                            duvV = new EggMorphOffset(vName.str(), 0.0 , 1.0);    
                            pool->Vertex(i)->attrib.uv_morphs.push_back(*duvV);
                        }

                  } // if ( numExp )
                } // if ( numTexLoc || numTexGlb )

                //free( uScale );
                //free( vScale );
                //free( uOffset );
                //free( vOffset );

                //free( materials );
            }
            else
            {
                // no material present - default to white 
                nurbColor.set( 1.0, 1.0, 1.0, 1.0 );
            }

            //////////////////////////////////////////
            // check NURBS surface for trim curves
            //////////////////////////////////////////
            int     numTrims;
            bool    isTrim = TRUE;
            SAA_SubElem *trims;
        
            SAA_nurbsSurfaceGetNbTrimCurves( scene, model, SAA_TRIMTYPE_TRIM,
                &numTrims );

            if ( verbose >= 1 )
                fprintf( outStream, "nurbs surf has %d trim curves\n", 
                    numTrims );

            if ( numTrims)
            {
                trims = (SAA_SubElem *)malloc(sizeof(SAA_SubElem)*numTrims);

                if ( trims )
                {
                    SAA_nurbsSurfaceGetTrimCurves( scene, model, 
                        gtype, 0, SAA_TRIMTYPE_TRIM, numTrims, 
                        trims );

                    MakeSurfaceCurve( scene, model, parent, eggNurbsSurf, 
                        numTrims, trims, isTrim );
                }

                //free( trims );
            }

            //////////////////////////////////////////
            // check NURBS surface for surface curves
            //////////////////////////////////////////
            isTrim = FALSE;

            SAA_nurbsSurfaceGetNbTrimCurves( scene, model, 
                SAA_TRIMTYPE_PROJECTION, &numTrims );

            if ( verbose >= 1 )
                fprintf( outStream, "nurbs surf has %d surface curves\n", 
                    numTrims );

            if ( numTrims)
            {
                trims = (SAA_SubElem *)malloc(sizeof(SAA_SubElem)*numTrims);

                if ( trims )
                {
                    SAA_nurbsSurfaceGetTrimCurves( scene, model, 
                        gtype, 0, SAA_TRIMTYPE_PROJECTION, 
                        numTrims, trims );

                    MakeSurfaceCurve( scene, model, parent, eggNurbsSurf, 
                        numTrims, trims, isTrim );
                }

                //free( trims );
            }

            // push the NURBS into the egg data
            parent->children.push_back( eggNurbsSurf );    

            // if model has key shapes, generate vertex offsets
            if ( has_morph && make_morph )
                MakeVertexOffsets( scene, model, type, numShapes, numVert, 
                    vertices, matrix, name );


            //free( vertices );

        }
        /////////////////////////////////////
        // check to see if its a NURBS curve
        /////////////////////////////////////
        else if ( (type == SAA_MNCRV) && ( visible ) && ( make_nurbs ) )
        {
            // ignore for now
            // make the NURBS curve and push it into the egg data
            //parent->children.push_back( MakeNurbsCurve( scene, model, parent, 
                //matrix, name ) );
           }
        else if ( type == SAA_MJNT )
        {
            MakeJoint( scene, lastJoint, lastAnim, model, name );
            if ( verbose >= 1 )
                fprintf( outStream, "encountered IK joint: %s\n", name );
        }
        /////////////////////
        // it must be a NULL
        /////////////////////
        else 
        {
            SAA_AlgorithmType    algo;
        
            SAA_modelGetAlgorithm( scene, model, &algo );
            if ( verbose >= 1 )
                fprintf( outStream, "null algorithm: %d\n", algo );

            if ( algo == SAA_ALG_INV_KIN )
            {
                MakeJoint( scene, lastJoint, lastAnim,  model, name );
                if ( verbose >= 1 )
                    fprintf( outStream, "encountered IK root: %s\n", name );
            }
            else if ( algo == SAA_ALG_INV_KIN_LEAF )
            {
                MakeJoint( scene, lastJoint, lastAnim, model, name );
                if ( verbose >= 1 )
                    fprintf( outStream, "encountered IK leaf: %s\n", name );
            }
            else if ( algo == SAA_ALG_STANDARD )
            {
                SAA_Boolean isSkeleton = FALSE;

                if ( verbose >= 1 )
                    fprintf( outStream, "encountered Standard null: %s\n", name);

                SAA_modelIsSkeleton( scene, model, &isSkeleton );

                // check to see if this NULL is used as a skeleton
                // or is animated via constraint only ( these nodes are
                // tagged by the animator with the keyword "joint"
                // somewhere in the nodes name)
                if ( isSkeleton || (strstr( name, "joint" ) != NULL) )
                {
                    MakeJoint( scene, lastJoint, lastAnim, model, name );
                    if ( verbose >= 1 )
                        fprintf( outStream, "animating Standard null!!!\n" );
                }
                
            }
            else
                if ( verbose >= 1 )
                    fprintf( outStream, "encountered some other NULL: %d\n", 
                        algo );
        }
    }


    // check for children...
    int        numChildren;
    int        thisChild;
    SAA_Elem *children;

    SAA_modelGetNbChildren( scene, model, &numChildren );
    if ( verbose >= 1 )
        fprintf( outStream, "Model children: %d\n", numChildren );

    if ( numChildren )
    {
        children = (SAA_Elem *)malloc(sizeof(SAA_Elem)*numChildren);
        SAA_modelGetChildren( scene, model, numChildren, children );
        if ( children != NULL )
        {
            for ( thisChild = 0; thisChild < numChildren; thisChild++ )
            {
                if ( verbose >= 1 )
                    fprintf( outStream, "\negging child %d...\n", thisChild);
                MakeEgg( parent, lastJoint, lastAnim, scene, 
                    &children[thisChild] );
            }
        }
        else
            fprintf( outStream, "Not enough Memory for children...\n");
        //free( children );
    }
    fflush( outStream );
  }
  else
    if ( verbose >= 1 )
        fprintf( outStream, "Don't descend this branch!\n" );
    
  // we are done for the most part - start cleaning up memory
  //free( name );
}


////////////////////////////////////////////////////////////////////
//     Function: MakeSurfaceCurve
//       Access: Public 
//  Description: Given a scene and lists of u and v samples create a
//                an egg NURBS curve of degree two from the samples 
////////////////////////////////////////////////////////////////////
void  soft2egg::
MakeSurfaceCurve(  SAA_Scene *scene, SAA_Elem *model, EggGroup *parent, 
    EggNurbsSurface *&nurbsSurf, int numTrims, SAA_SubElem *trims,
    bool isTrim )
{
    int      i;
    long      totalSamples = 0;
    long    *numSamples;
    double    *uSamples;
    double    *vSamples;
    SAA_Elem *trimCurves;
    char    *name;

    //get UV coord data
    numSamples = (long *)malloc(sizeof(long)*numTrims);

    SAA_surfaceCurveGetNbLinearSamples( scene, model,  numTrims, trims,
        numSamples );

    for ( i = 0; i < numTrims; i++ )
    {
        totalSamples += numSamples[i];
        if ( verbose >= 2 )
            fprintf( outStream, "numSamples[%d] = %d\n", i, numSamples[i] );
    }

    if ( verbose >= 2 )
        fprintf( outStream, "total samples = %ld\n", totalSamples );

    uSamples = (double *)malloc(sizeof(double)*totalSamples);
    vSamples = (double *)malloc(sizeof(double)*totalSamples);

    SAA_surfaceCurveGetLinearSamples( scene, model, numTrims, trims,
        numSamples, uSamples, vSamples ); 

    if ( verbose >= 2 )
        for ( long li = 0; li < totalSamples; li++ )
            fprintf( outStream, "master list cv[%ld] = %f, %f\n", li, 
                uSamples[li], vSamples[li] ); 

    trimCurves = (SAA_Elem *)malloc(sizeof(SAA_Elem)*numTrims);

    SAA_surfaceCurveExtract( scene, model, numTrims, trims, trimCurves );

    // if it's a trim create a trim to assign trim curves to
    EggNurbsSurface::Trim *eggTrim = new EggNurbsSurface::Trim();

    // for each trim curve, make an egg curve and 
    // add it to the trims of the NURBS surface
    for ( i = 0; i < numTrims; i++ )
    {
        if ( use_prefix )
        {
            // Get the FULL name of the trim curve
            name = GetFullName( scene, &trimCurves[i] );
        }
        else
        {
            // Get the name of the trim curve
            name = GetName( scene, &trimCurves[i] );
        }

        if ( isTrim )
        {
            // add to trim list
            EggNurbsSurface::Loop *eggLoop = new EggNurbsSurface::Loop();
            eggLoop->push_back( MakeUVNurbsCurve( i, numSamples, uSamples, 
                vSamples, parent, name ) );
            eggTrim->push_back( *eggLoop );
        }
        else
            // add to curve list
            nurbsSurf->curves.push_back( MakeUVNurbsCurve( i, numSamples,                         uSamples, vSamples, parent, name ) );
    } 

    if ( isTrim )
        // pus trim list onto trims list
        nurbsSurf->trims.push_back( *eggTrim );

    //free( name );
    //free( trimCurves );
    //free( uSamples );
    //free( vSamples );
}

////////////////////////////////////////////////////////////////////
//     Function: MakeUVNurbsCurve
//       Access: Public 
//  Description: Given a scene and lists of u and v samples create a
//                an egg NURBS curve of degree two from the samples 
////////////////////////////////////////////////////////////////////
EggNurbsCurve  *soft2egg::
MakeUVNurbsCurve( int numCurve, long *numSamples, double *uSamples, 
    double *vSamples, EggGroup *parent, char *name )
{
    EggNurbsCurve    *eggNurbsCurve = new EggNurbsCurve( name );

    eggNurbsCurve->order = 2;


    if ( verbose >= 2 )
        fprintf( outStream, "nurbs UV curve %s:\n", name );

    //set sub_div so we can see it in perfly
    //eggNurbsCurve->subdiv = numSamples[numCurve]/4;
    // perfly chokes on big numbers - keep it reasonable
    eggNurbsCurve->subdiv = 150; 

    //create pool of NURBS vertices 
    EggVertexPool *pool = _data.CreateVertexPool( parent, name );
    eggNurbsCurve->SetVertexPool( pool );

    // calculate offset to this curve's samples
    // in list of all curve samples
    int offset = 0;

    for ( int o = 0; o < numCurve; o++ )
        offset += numSamples[o];
    
    for ( int k = 0; k<numSamples[numCurve]; k++ )
    {
        pfVec3  eggVert;

        // index into the array of samples for this curve
        eggVert.set( uSamples[k+offset], vSamples[k+offset], 1.0f );

        if ( verbose >= 2 )
            fprintf( outStream, "cv[%d] = %f %f %f\n", k, eggVert[0], 
                eggVert[1], eggVert[2] ); 

        //populate vertex pool
        pool->AddVertex( eggVert, k );

        //add vref's to NURBS info 
        eggNurbsCurve->AddVertex( k ); 
    }

    // create numSamples[numCurve]+2 knots
    eggNurbsCurve->knots.push_back( 0 );
    for ( k = 0; k < numSamples[numCurve]; k++ )
        eggNurbsCurve->knots.push_back( k  );
    eggNurbsCurve->knots.push_back( numSamples[numCurve] - 1 );

    //set color to bright green for now
    EggColor *nurbCref;
    pfVec4    nurbColor;

    nurbColor.set( 0.5, 1.0, 0.5, 1.0 );
    nurbCref = _data.CreateColor(nurbColor);
    eggNurbsCurve->attrib.SetCRef(nurbCref);

    return( eggNurbsCurve );
}

////////////////////////////////////////////////////////////////////
//     Function: MakeNurbsCurve
//       Access: Public 
//  Description: Given a scene and a NURBS curve model create the
//                 the appropriate egg structures  
////////////////////////////////////////////////////////////////////
EggNurbsCurve  *soft2egg::
MakeNurbsCurve( SAA_Scene *scene, SAA_Elem *model, EggGroup *parent, 
    float matrix[4][4], char *name )
{
    EggNurbsCurve    *eggNurbsCurve = new EggNurbsCurve( name );
    int degree;

    if ( verbose >= 2 )
        fprintf( outStream, "nurbs curve %s:\n", name );

    //create nurbs representation of surface
    SAA_nurbsCurveGetDegree( scene, model, &degree );
    eggNurbsCurve->order = degree + 1;
    if ( verbose >= 2 )
        fprintf( outStream, "nurbs curve order: %d\n", degree + 1 );
    
    SAA_nurbsCurveSetStep( scene, model, nurbs_step );

    SAA_Boolean    closed = FALSE;

    SAA_nurbsCurveGetClosed( scene, model, &closed );    
    if ( closed )
        if ( verbose >= 2 )
            fprintf( outStream, "nurbs curve is closed...\n");

    int numKnots;
    
    SAA_nurbsCurveGetNbKnots( scene, model, &numKnots );
    if ( verbose >= 2 )
        fprintf( outStream, "nurbs curve knots: %d\n", numKnots );
    double *knots;    
    knots = (double *)malloc(sizeof(double)*numKnots);
    SAA_nurbsCurveGetKnots( scene, model, SAA_GEOM_ORIGINAL, 0, 
        numKnots, knots );

    AddKnots( eggNurbsCurve->knots, knots, numKnots, closed, degree );

    //free( knots );

    int    numCV;

    SAA_modelGetNbVertices( scene, model, &numCV );
    if ( verbose >= 2 )
        fprintf( outStream, "%d CV's (=? %d)\n", numCV, (numKnots-(degree+1)) );

    //set sub_div so we can see it in perfly
    eggNurbsCurve->subdiv = (numCV-1)*nurbs_step;

    // get the CV's
    SAA_DVector *cvArray;
    cvArray = (SAA_DVector *)malloc(sizeof(SAA_DVector)*numCV);
    SAA_modelGetVertices( scene, model, SAA_GEOM_ORIGINAL, 0,
        numCV, cvArray );

    //create pool of NURBS vertices 
    EggVertexPool *pool = _data.CreateVertexPool( parent, name );
    eggNurbsCurve->SetVertexPool( pool );

    for ( int k = 0; k<numCV; k++ )
    {
        if ( verbose >= 2 )
            fprintf( outStream, "cv[%d] = %f %f %f %f\n", k, cvArray[k].x,
                cvArray[k].y, cvArray[k].z, cvArray[k].w );

        pfVec4  eggVert;

        //convert to global coords
        SAA_DVector local = cvArray[k];
        SAA_DVector global;

        _HVCT_X_MAT( global, local, matrix );

        eggVert.set( global.x, global.y, global.z, global.w );

        //populate vertex pool
        pool->AddVertex( eggVert, k );

        //add vref's to NURBS info 
        eggNurbsCurve->AddVertex( k ); 
    }

    if ( closed )
    {
        // need to replicate first (degree) vertices
        for ( k = 0; k < degree; k++ )
        {
            eggNurbsCurve->AddVertex( k );
            if ( verbose >= 2 )
                fprintf( outStream, "adding cv[%d] = %f %f %f %f\n", k, 
                    cvArray[k].x, cvArray[k].y, cvArray[k].z, cvArray[k].w );
        }
    }

    //free( cvArray );

    //set color to bright green for now
    EggColor *nurbCref;
    pfVec4    nurbColor;

    nurbColor.set( 0.5, 1.0, 0.5, 1.0 );
    nurbCref = _data.CreateColor(nurbColor);
    eggNurbsCurve->attrib.SetCRef(nurbCref);

    return( eggNurbsCurve );
}

////////////////////////////////////////////////////////////////////
//     Function: AddKnots
//       Access: Public 
//  Description: Given a parametric surface, and its knots, create
//                 the appropriate egg structure by filling in Soft's
//                 implicit knots and assigning the rest to eggKnots. 
////////////////////////////////////////////////////////////////////
void soft2egg::
AddKnots( perf_vector<double> &eggKnots, double *knots, int numKnots, 
    SAA_Boolean closed, int degree ) 
{
    int k = 0;
    double lastKnot = knots[0];
    double    *newKnots;

    // add initial implicit knot(s)
    if ( closed ) 
    {
        int i = 0;
        newKnots = (double *)malloc(sizeof(double)*degree);

        // need to add (degree) number of knots 
        for ( k = numKnots - 1; k >= numKnots - degree; k-- )
        {
            // we have to know these in order to calculate
            // next knot value so hold them in temp array
            newKnots[i] =  lastKnot - (knots[k] - knots[k-1]);
            lastKnot = newKnots[i];
            i++;
        }
        for ( k = degree - 1; k >= 0; k-- )
        {
            eggKnots.push_back( newKnots[k] );
            if ( verbose >= 2 )
                fprintf( outStream, "knots[%d] = %f\n", k, newKnots[k] ); 
        }

        //free( newKnots );
    }
    else
    {
        eggKnots.push_back( knots[k] );
        if ( verbose >= 2 )
            fprintf( outStream, "knots[%d] = %f\n", k, knots[k] ); 
    }

    // add the regular complement of knots
    for (k = 0; k < numKnots; k++)
    {
        eggKnots.push_back( knots[k] );
        if ( verbose >= 2 )
            fprintf( outStream, "knots[%d] = %f\n", k+1, knots[k] ); 
    }

    lastKnot = knots[numKnots-1];

    // add trailing implicit knots
    if ( closed ) 
    {

        // need to add (degree) number of knots 
        for ( k = 1; k <= degree; k++ )
        {
            eggKnots.push_back( lastKnot + (knots[k] - knots[k-1]) );
            if ( verbose >= 2 )
                fprintf( outStream, "knots[%d] = %f\n", k, 
                    lastKnot + (knots[k] - knots[k-1]) ); 
            lastKnot = lastKnot + (knots[k] - knots[k-1]);
        }
    }
    else
    {
        eggKnots.push_back( knots[k-1] );
        if ( verbose >= 2 )
            fprintf( outStream, "knots[%d] = %f\n", k+1, knots[k-1] ); 
    }
}

////////////////////////////////////////////////////////////////////
//     Function: MakeJoint
//       Access: Public 
//  Description: Given a name, a parent and a model create a new 
//                 a new EggJoint for that model. 
////////////////////////////////////////////////////////////////////
void soft2egg::
MakeJoint( SAA_Scene *scene, EggJoint *&lastJoint, AnimGroup *&lastAnim, 
    SAA_Elem *model, char *name )
{
    float     matrix[4][4];
    pfMatrix  Matrix;
    EggJoint *joint;
    SAA_Boolean    globalFlag = FALSE;
    int    scale_joint = 0;


    // this is a quick fix to make scaled skeletons possible
    // if the parent contains the keyword "scale" make this joint
    // a global root joint instead of a child...
    if (lastJoint != NULL)
    {
        if ( strstr( lastJoint->name.Str(), "scale" ) != NULL )
        {
            scale_joint = 1;    
            if ( verbose >= 1 )
                fprintf( outStream, "scale joint flag set!\n" );
        }
    }

    // if not root, flatten is false, and last joint had no scaling
    // applied to it, then create joint in skeleton tree
    if ( (lastJoint != NULL) && !flatten && !scale_joint )
    {
        if ( verbose >= 1 )
        {
            fprintf( outStream, "lastJoint = %s\n", lastJoint->name.Str() );
            fprintf( outStream, "getting local transform\n" );
        }

        SAA_elementSetUserData( scene, model, "GLOBAL", sizeof( SAA_Boolean ),
            TRUE, (void  **)&globalFlag );

        // get the local matrix
        SAA_modelGetMatrix( scene, model, SAA_COORDSYS_LOCAL,  matrix );

        // make this into a pfMatrix
        Matrix[0][0] = matrix[0][0];
        Matrix[0][1] = matrix[0][1];
        Matrix[0][2] = matrix[0][2];
        Matrix[0][3] = matrix[0][3];
        Matrix[1][0] = matrix[1][0];
        Matrix[1][1] = matrix[1][1];
        Matrix[1][2] = matrix[1][2];
        Matrix[1][3] = matrix[1][3];
        Matrix[2][0] = matrix[2][0];
        Matrix[2][1] = matrix[2][1];
        Matrix[2][2] = matrix[2][2];
        Matrix[2][3] = matrix[2][3];
        Matrix[3][0] = matrix[3][0];
        Matrix[3][1] = matrix[3][1];
        Matrix[3][2] = matrix[3][2];
        Matrix[3][3] = matrix[3][3];

        joint = _data.CreateJoint( lastJoint, name ); 
        joint->transform = Matrix;
    }
    // if we already have a root attach this joint to it
    else if (foundRoot)
    {
        if ( verbose >= 1 )
            fprintf( outStream, "getting global transform\n" );

        globalFlag = TRUE;

        SAA_elementSetUserData( scene, model, "GLOBAL", sizeof( SAA_Boolean ),
            TRUE, (void *)&globalFlag );

        // get the global matrix
        SAA_modelGetMatrix( scene, model, SAA_COORDSYS_GLOBAL,  matrix );

        // make this into a pfMatrix
        Matrix[0][0] = matrix[0][0];
        Matrix[0][1] = matrix[0][1];
        Matrix[0][2] = matrix[0][2];
        Matrix[0][3] = matrix[0][3];
        Matrix[1][0] = matrix[1][0];
        Matrix[1][1] = matrix[1][1];
        Matrix[1][2] = matrix[1][2];
        Matrix[1][3] = matrix[1][3];
        Matrix[2][0] = matrix[2][0];
        Matrix[2][1] = matrix[2][1];
        Matrix[2][2] = matrix[2][2];
        Matrix[2][3] = matrix[2][3];
        Matrix[3][0] = matrix[3][0];
        Matrix[3][1] = matrix[3][1];
        Matrix[3][2] = matrix[3][2];
        Matrix[3][3] = matrix[3][3];

        if ( verbose >= 1 )
            fprintf( outStream, "attaching orphan chain to root\n" );

        joint = _data.CreateJoint( rootJnt, name ); 
        joint->transform = Matrix;
        lastAnim = rootAnim;    
    }
    // if root, make a seperate tree for skeleton and
    // create required Table for the Egg heirarchy
    else 
    {
        if ( verbose >= 1 )
            fprintf( outStream, "getting global transform\n" );

        globalFlag = TRUE;

        SAA_elementSetUserData( scene, model, "GLOBAL", sizeof( SAA_Boolean ),
            TRUE, (void *)&globalFlag );

        // get the global matrix
        SAA_modelGetMatrix( scene, model, SAA_COORDSYS_GLOBAL,  matrix );

        // make this into a pfMatrix
        Matrix[0][0] = matrix[0][0];
        Matrix[0][1] = matrix[0][1];
        Matrix[0][2] = matrix[0][2];
        Matrix[0][3] = matrix[0][3];
        Matrix[1][0] = matrix[1][0];
        Matrix[1][1] = matrix[1][1];
        Matrix[1][2] = matrix[1][2];
        Matrix[1][3] = matrix[1][3];
        Matrix[2][0] = matrix[2][0];
        Matrix[2][1] = matrix[2][1];
        Matrix[2][2] = matrix[2][2];
        Matrix[2][3] = matrix[2][3];
        Matrix[3][0] = matrix[3][0];
        Matrix[3][1] = matrix[3][1];
        Matrix[3][2] = matrix[3][2];
        Matrix[3][3] = matrix[3][3];

        rootJnt = _data.CreateJoint( skeleton, "root" );
        rootJnt->transform.makeIdent();
        if ( verbose >= 1 )
            fprintf( outStream, "setting skeleton root\n" );
        rootJnt->flags |= EF_TRANSFORM;

        joint = _data.CreateJoint( rootJnt, name ); 
        joint->transform = Matrix;
        foundRoot = TRUE;
        if ( verbose >= 1 )
            fprintf( outStream, "found first chain\n" );

        // make skeleton table
        AnimGroup *skeletonTable;
        skeletonTable = animData.CreateTable( animRoot, "<skeleton>" ); 
        rootAnim = animData.CreateTable( skeletonTable, "root" ); 
        XfmSAnimTable *table = new XfmSAnimTable( );     
        table->name = "xform";
        table->fps = anim_rate;
        rootAnim->children.push_back( table );
        lastAnim = rootAnim;    
    }

    joint->flags |= EF_TRANSFORM;

    //if ( make_anim)
    //{
        AnimGroup *anim = animData.CreateTable( lastAnim, name );
        XfmSAnimTable *table = new XfmSAnimTable( );     
        if ( verbose >= 1 )
            fprintf( outStream, "created anim table: %s\n", "xform" );
        table->name = "xform";
        table->fps = anim_rate;
        anim->children.push_back( table );
        lastAnim = anim;
    //}

    // make this joint current parent of chain
    lastJoint = joint;
}


////////////////////////////////////////////////////////////////////
//     Function: MakeSoftSkin
//       Access: Public 
//  Description: Given a skeleton part find its envelopes (if any)
//                 get the vertices associated with the envelopes and
//                 their weights and make vertex ref's for the joint 
////////////////////////////////////////////////////////////////////
void soft2egg::
MakeSoftSkin( SAA_Scene *scene, SAA_Elem *model, SAA_Elem *models,
    int numModels, char *name )
{
    int            numEnv;
    SAA_ModelType   type;
    SAA_Elem    *envelopes;

    if ( verbose >= 1 )
        fprintf( outStream, "\n>found skeleton part( %s )!\n", name );

    SAA_skeletonGetNbEnvelopes( scene, model, &numEnv );
    
    if ( numEnv )
    {
        // it's got envelopes - must be soft skinned
        if ( verbose >= 1 )
            fprintf( outStream, "numEnv = %d\n", numEnv );

        // allocate envelope array
        envelopes = ( SAA_Elem *)malloc( sizeof( SAA_Elem )*numEnv );

        if ( envelopes != NULL )
        {
            int         thisEnv;
            SAA_EnvType envType;
            bool        hasEnvVertices = 0;

            SAA_skeletonGetEnvelopes( scene, model, numEnv, envelopes );

            for ( thisEnv = 0; thisEnv < numEnv; thisEnv++ )
            {
                if ( verbose >= 1 )
                    fprintf( outStream, "env[%d]: ", thisEnv );

                SAA_envelopeGetType( scene, &envelopes[thisEnv], &envType );

                if ( envType == SAA_ENVTYPE_NONE )
                {
                    if ( verbose >= 1 )
                        fprintf( outStream, "envType = none\n" );
                }
                else if ( envType == SAA_ENVTYPE_FLXLCL )
                {
                    if ( verbose >= 1 )
                        fprintf( outStream, "envType = flexible, local\n" );
                    hasEnvVertices = 1;
                }
                else if ( envType == SAA_ENVTYPE_FLXGLB )
                {
                    if ( verbose >= 1 )
                        fprintf( outStream, "envType = flexible, global\n" );
                    hasEnvVertices = 1;
                }
                else if ( envType == SAA_ENVTYPE_RGDGLB )
                {
                    if ( verbose >= 1 )
                        fprintf( outStream, "envType = rigid, global\n" );
                    hasEnvVertices = 1;
                }
                else
                {
                    if ( verbose >= 1 )
                        fprintf( outStream, "envType = unknown\n" );
                }
            }

            if ( hasEnvVertices)
            {
                int            *numEnvVertices;
                SAA_SubElem    *envVertices = NULL;

                numEnvVertices = (int *)malloc(sizeof(int)*numEnv);

                SAA_envelopeGetNbCtrlVertices( scene, model, numEnv,
                    envelopes, numEnvVertices );

                if ( numEnvVertices != NULL )
                {
                    int totalEnvVertices = 0;
                    int    i,j,k;

                    for( i = 0; i < numEnv; i++ )
                    {
                        totalEnvVertices += numEnvVertices[i];
                        if ( verbose >= 1 )
                            fprintf( outStream, "numEnvVertices[%d] = %d\n", 
                                i, numEnvVertices[i] );
                    }


                    if ( verbose >= 1 )
                        fprintf( outStream, "total env verts = %d\n", 
                            totalEnvVertices );    

                    if ( totalEnvVertices )
                    {
                    envVertices = (SAA_SubElem *)malloc(sizeof(SAA_SubElem)*totalEnvVertices);

                    if ( envVertices != NULL )
                    {

                        SAA_envelopeGetCtrlVertices( scene, model,
                            numEnv, envelopes, numEnvVertices, envVertices);
            
                        // loop through for each envelope
                        for ( i = 0; i < numEnv; i++ )
                        {
                            float *weights = NULL;
                            int       vertArrayOffset = 0;

                            if ( verbose >= 2 )
                                fprintf( outStream, "\nenvelope[%d]:\n", i );

                            weights = (float *)malloc(sizeof(float)*numEnvVertices[i]);

                            if ( weights )
                            {
                            char *envName;
                            int *vpoolMap = NULL;

                            for ( j = 0; j < i; j++ )
                                vertArrayOffset += numEnvVertices[j];

                            if ( verbose >= 1 )
                                fprintf( outStream, 
                                    "envVertArray offset = %d\n", 
                                    vertArrayOffset );

                            // get the weights of the envelope vertices
                            SAA_ctrlVertexGetEnvelopeWeights( 
                                    scene, model, &envelopes[i], 
                                    numEnvVertices[i], 
                                    &envVertices[vertArrayOffset], weights ); 

                            // Get the name of the envelope model
                            if ( use_prefix )
                            {
                                // Get the FULL name of the envelope
                                envName = GetFullName( scene, &envelopes[i] );
                            }
                            else
                            {
                                // Get the name of the envelope
                                envName = GetName( scene, &envelopes[i] );
                            }

                            if ( verbose >= 1 )
                                fprintf( outStream, "envelope name %s\n", envName );

                            // find out if envelope geometry is poly or nurb
                            //SAA_modelGetType( scene, 
                                //FindModelByName( envName, scene, 
                                    //models, numModels ), &type );

                            SAA_modelGetType( scene, &envelopes[i], &type );

                            if ( verbose >= 1 )
                            {
                                fprintf( outStream, "envelope model type ");

                                if ( type == SAA_MSMSH )
                                        fprintf( outStream, "MESH\n" );
                                else if ( type == SAA_MNSRF )
                                        fprintf( outStream, "NURBS\n" );
                                else
                                        fprintf( outStream, "OTHER\n" );
                           }

                            int *envVtxIndices = NULL;
                            envVtxIndices = (int *)malloc(sizeof(int)*numEnvVertices[i]);

                            // Get the envelope vertex indices
                            SAA_ctrlVertexGetIndices( scene, &envelopes[i],                                     numEnvVertices[i], 
                                &envVertices[vertArrayOffset], envVtxIndices );

                            // find out how many vertices the model has
                            int modelNumVert;

                            SAA_modelGetNbVertices( scene, &envelopes[i], &modelNumVert );
                             
                            SAA_DVector *modelVertices = NULL;
                            modelVertices = (SAA_DVector *)malloc(sizeof(SAA_DVector)*modelNumVert);

                            // get the model vertices
                            SAA_modelGetVertices( scene, &envelopes[i],
                                SAA_GEOM_ORIGINAL, 0, modelNumVert, 
                                modelVertices );
                            
                            // create array of global model coords 
                            SAA_DVector *globalModelVertices = NULL;
                            globalModelVertices = (SAA_DVector *)malloc(sizeof(SAA_DVector)*modelNumVert);
                            float        matrix[4][4];

                            // tranform local model vert coords to global

                            // first get the global matrix
                            SAA_modelGetMatrix( scene, &envelopes[i],                                                 SAA_COORDSYS_GLOBAL,  matrix );

                            // populate array of global model verts
                            for ( j = 0; j < modelNumVert; j++ )
                            {
                                _VCT_X_MAT( globalModelVertices[j], 
                                    modelVertices[j], matrix );
                            }

                            // find the egg vertex pool that corresponds
                            // to this envelope model
                            EggVertexPool *envPool = 
                                (EggVertexPool *)(_data.pools.FindName( envName ));
                            // If we are outputting triangles:
                            // create an array that maps from a referenced
                            // vertex in the envelope to a corresponding 
                            // vertex in the egg vertex pool
                            //if ( (type == SAA_MNSRF) && !make_nurbs ) 
                            if ( !make_nurbs || (type == SAA_MSMSH) ) 
                            {
                                vpoolMap = FindClosestTriVert( envPool,
                                    globalModelVertices, modelNumVert );
                            }


                            if ( envPool != NULL )
                            {
    
                            // find the egg joint that corresponds to this model
                            EggJoint *joint = 
                                (EggJoint *)(skeleton->FindDescendent( name ));

                                                                                                                // this doesn't seem to be necessary 4/7/99
                            //EggJoint *parent = (EggJoint *)joint->parent;
                                                                                                                //assert(parent->IsA(NT_EggJoint));

                            // for every envelope vertex 
                            for (j = 0; j < numEnvVertices[i]; j++)
                            {
                                double scaledWeight =  weights[j]/ 100.0f;

                                // make sure its in legal range
                                if (( envVtxIndices[j] < modelNumVert )
                                    && ( envVtxIndices[j] >= 0 ))
                                {
                                  if ( (type == SAA_MNSRF) && make_nurbs ) 
                                  { 
                                    // assign all referenced control vertices
                                    joint->AddVertex( envPool->Vertex(envVtxIndices[j]), scaledWeight );

                                    if ( verbose >= 2 )
                                        fprintf( outStream, 
                                            "%d: adding vref to cv %d with weight %f\n", 
                                            j, envVtxIndices[j], scaledWeight );

                                    envPool->Vertex(envVtxIndices[j])->AddJoint( joint, scaledWeight );
                                    // set flag to show this vertex has
                                    // been assigned
                                    envPool->Vertex(envVtxIndices[j])->multipleJoints = 1;
                                  }
                                  else
                                  {    
                                    //assign all the tri verts associated
                                    // with this control vertex to joint 
                                    for ( k = 0; k < envPool->NumVertices(); k++ )    
                                    {
                                      if ( vpoolMap[k] == envVtxIndices[j] ) 
                                      {

                                        // add each vert in pool to last 
                                        // joint for soft skinning
                                        joint->AddVertex(envPool->Vertex(k), 
                                            scaledWeight);

                                        if ( verbose >= 2 )
                                            fprintf( outStream, 
                                                "%d: adding vref from cv %d to vert %d with weight %f(vpool)\n", 
                                                j, envVtxIndices[j], k, scaledWeight );

                                        envPool->Vertex(k)->AddJoint( joint, scaledWeight );
                                        // set flag to show this vertex has
                                        // been assigned
                                        envPool->Vertex(k)->multipleJoints = 1;
                                      }
                                    }
                                 }
                                }
                                else
                                    if ( verbose >= 2 )
                                        fprintf( outStream, 
                                            "%d: Omitted vref from cv %d with weight %f (out of range 0 to %d )\n",
                                            j, envVtxIndices[j], scaledWeight, modelNumVert );
                                    
                            }

                            }
                            else 
                                if ( verbose >= 2 )
                                    fprintf( outStream, "Couldn't find vpool %s!\n", envName );

                            //free( modelVertices ); 
                            //free( globalModelVertices ); 
                            //free( envVtxIndices );
                            //free( envName );
                            } //if (weights)
                            //free( weights );

                        } // for i

                    } // if (envVertices != NULL)
                    else
                        fprintf( outStream, "Not enough memory for envelope vertices...\n");
                    //free( envVertices );
                    } // if (totalEnvVertices) 
                    else
                        if ( verbose >= 1 )
                            fprintf( outStream, "No envelope vertices present...\n");

                    //free( numEnvVertices );

            } // if (numEnvVertices != NULL)

          } // if (hasEnvVertices)

        } // if (envelopes != NULL)
        else
            fprintf( outStream, "Not enough memory for envelopes...\n" );

        //free( envelopes );

    } //if (numEnv)

    else
        if ( verbose >= 1 )
            fprintf( outStream, "Skeleton member has no envelopes...\n" );
}


////////////////////////////////////////////////////////////////////
//     Function: CleanUpSoftSkin
//       Access: Public 
//  Description: Given a model, make sure all its vertices have been 
//                 soft assigned. If not hard assign to the last
//                 joint we saw.
////////////////////////////////////////////////////////////////////
void soft2egg::
CleanUpSoftSkin( SAA_Scene *scene, SAA_Elem *model, char *name )
{
    static EggJoint *joint;
    SAA_Elem        parent;
    SAA_ModelType     type;
    SAA_Boolean        skel;

    /////////////////////////////////////////////////
    // find out what type of node we're dealing with
    /////////////////////////////////////////////////
    SAA_modelGetType( scene, model, &type );

    char   *parentName;
    int        level;
    SAA_Elem    *searchNode = model;

    if ( verbose >= 1 )
        fprintf( outStream, "\nCleaning up model %s\n", name );

    // this step is weird - I think I want it here but it seems
    // to break some models. Files like props-props_wh_cookietime.3-0 in
    // /ful/rnd/pub/vrml/chip/chips_adventure/char/zone1/rooms/warehouse_final
    // need to do the "if (skel)" bit.

    // am I a skeleton too?
    SAA_modelIsSkeleton( scene, model, &skel );

    // if not look for the last skeleton part
    if ( skel ) 
        parentName = name;
    else do 
    {
        SAA_elementGetHierarchyLevel( scene, searchNode, &level );

        // make sure we don't try to get the root's parent
        if ( level )
        {
            SAA_modelGetParent( scene, searchNode, &parent );

            if ( use_prefix )
            {
                // Get the FULL name of the parent
                parentName = GetFullName( scene, &parent );
            }
            else
            {
                // Get the name of the parent
                parentName = GetName( scene, &parent );
            }

            SAA_modelGetType( scene, &parent, &type );

            SAA_modelIsSkeleton( scene, &parent, &skel );

            if ( verbose >= 1 )
                fprintf( outStream, "model %s, level %d, type %d, skel %d\n",
                    parentName, level, type, skel );

            searchNode = &parent;
        }
        else
        {
            // we reached the root of the tree
            parentName = NULL;
            if ( verbose >= 1 )
                fprintf( outStream, "at root of tree! level %d\n", level );
            break;
        }

    // look until parent is a joint or acts like one
    } while ( !skel && ( strstr( parentName,"joint") == NULL ));

    EggJoint    *thisJoint = NULL;

    if ( parentName != NULL )
    {
        if ( verbose >= 1 )
        {
            fprintf( outStream, "found model parent joint %s\n", parentName);
            fprintf( outStream, "looking for joint %s\n", parentName );
        }
        thisJoint = (EggJoint *)(skeleton->FindDescendent( parentName ));
    }
    else
        if ( verbose >= 1 )
                fprintf( outStream, "Couldn't find parent joint!\n");

    if ( thisJoint != NULL )
    {
        joint = thisJoint;
        if ( verbose >= 1 )
            fprintf( outStream, "setting joint to %s\n", parentName );

        //find the vpool for this model
        EggVertexPool *vPool = 
            (EggVertexPool *)(_data.pools.FindName( name ));

        if (vPool != NULL)
        {
            int i;
            double membership;
            int numVerts = vPool->NumVertices() ;


            if ( verbose >= 1 )
                fprintf( outStream, "found vpool %s w/ %d verts\n", 
                name, numVerts );
        
            for ( i = 0; i < numVerts; i++ )     
            {
                if ( vPool->Vertex(i)->multipleJoints != 1 )
                {
                    if ( verbose >= 1 )
                    {    
                        fprintf( outStream, "vpool %s vert %d", name, i );
                        fprintf( outStream, " not assigned!\n" );
                    }

                    // hard skin this vertex
                    joint->AddVertex( vPool->Vertex(i), 1.0f );
                }
                else
                {    
                    membership = vPool->Vertex(i)->NetMembership();


                    if ( verbose >= 1 )
                    {
                        fprintf( outStream, "vpool %s vert %d", name, 
                            i );
                        fprintf( outStream, " has membership %f\n", 
                            membership );
                    }

                    if ( membership == 0 )
                    {
                        if ( verbose >= 1 )
                            fprintf( outStream, "adding full weight..\n" );

                        // hard skin this vertex
                        joint->AddVertex( vPool->Vertex(i), 1.0f );
                    }
                }
            }
        }
        else
            if ( verbose >= 1 )
                fprintf( outStream, "couldn't find vpool %s\n", name );
    }
    else
    {
        if ( parentName != NULL )
            if ( verbose >= 1 )
                fprintf( outStream, "Couldn't find joint %s\n", parentName );
    }
}

//////////////////////////////////////////////////////////////////////
//     Function: MakeAnimTable
//       Access: Public 
//  Description: Given a scene and a skeleton part ,get all the 
//                   position, rotation, and scale for the skeleton
//                 part for this frame and write them out as Egg
//                 animation tables. 
////////////////////////////////////////////////////////////////////
void soft2egg::
MakeAnimTable( SAA_Scene *scene, SAA_Elem *skeletonPart, char *name )
{    

    if ( skeletonPart != NULL )
    {
        float    i,j,k;
        float    h,p,r;
        float    x,y,z;
        int         size;
        SAA_Boolean globalFlag = FALSE;
        SAA_Boolean bigEndian;

        if ( verbose >= 1 )
            fprintf( outStream, "\n\nanimating child %s\n", name );

        SAA_elementGetUserDataSize( scene, skeletonPart, "GLOBAL", &size );
        
        if ( size != 0 )    
            SAA_elementGetUserData( scene, skeletonPart, "GLOBAL", 
                sizeof( SAA_Boolean), &bigEndian, (void *)&globalFlag );
 
        if ( globalFlag )
        {
            if ( verbose >= 1 )
                fprintf( outStream, " using global matrix\n" );

            //get SAA orientation
            SAA_modelGetRotation( scene, skeletonPart, SAA_COORDSYS_GLOBAL, 
                &p, &h, &r ); 

            //get SAA translation
            SAA_modelGetTranslation( scene, skeletonPart, SAA_COORDSYS_GLOBAL, 
                &x, &y, &z ); 

            //get SAA scaling
            SAA_modelGetScaling( scene, skeletonPart, SAA_COORDSYS_GLOBAL, 
                &i, &j, &k );
        }
        else
        {
            if ( verbose >= 1 )
                fprintf( outStream, "using local matrix\n" );

            //get SAA orientation
            SAA_modelGetRotation( scene, skeletonPart, SAA_COORDSYS_LOCAL, 
                &p, &h, &r ); 

            //get SAA translation
            SAA_modelGetTranslation( scene, skeletonPart, SAA_COORDSYS_LOCAL, 
                &x, &y, &z ); 

            //get SAA scaling
            SAA_modelGetScaling( scene, skeletonPart, SAA_COORDSYS_LOCAL, 
                &i, &j, &k );
        }


        if ( verbose >= 2 )
            fprintf( outStream, "\nanim data: %f %f %f\n\t%f %f %f\n\t%f %f %f\n",
                i, j, k, h, p, r, x, y, z );  

        // find the appropriate anim table for this skeleton part
        AnimGroup     *thisGroup;
        XfmSAnimTable *thisTable;

        //find the anim table associated with this group
        thisGroup = (AnimGroup *)(animRoot->FindDescendent( name ));
        if ( verbose >= 2 )
            fprintf( outStream, "\nlooking for anim group %s\n", name );
        if ( thisGroup != NULL )
        {
            thisTable = (XfmSAnimTable *)(thisGroup->FindDescendent( "xform" ));

            if ( thisTable != NULL )
            {    
                thisTable->sub_tables[0].AddElement( i );
                thisTable->sub_tables[1].AddElement( j ); 
                thisTable->sub_tables[2].AddElement( k ); 
                thisTable->sub_tables[3].AddElement( p ); 
                thisTable->sub_tables[4].AddElement( h ); 
                thisTable->sub_tables[5].AddElement( r ); 
                thisTable->sub_tables[6].AddElement( x ); 
                thisTable->sub_tables[7].AddElement( y ); 
                thisTable->sub_tables[8].AddElement( z ); 
            }
            else
                fprintf( outStream, "Couldn't allocate anim table\n" );
        }
        else
            if ( verbose >= 2 )
                fprintf( outStream, "Couldn't find anim group  %s\n",  name );
    }
    else
    {
        if ( verbose >= 2 )
            fprintf( outStream, "Cannot build anim table - no skeleton\n" );
    }
    
}

////////////////////////////////////////////////////////////////////
//     Function: MakeVertexOffsets
//       Access: Public 
//  Description: Given a scene, a model , the vertices of its original
//                 shape and its name find the difference between the 
//                 geometry of its key shapes and the models original 
//                 geometry and add morph vertices to the egg data to 
//                 reflect these changes.
////////////////////////////////////////////////////////////////////
void soft2egg::
MakeVertexOffsets( SAA_Scene *scene, SAA_Elem *model, SAA_ModelType type,
    int numShapes, int numOrigVert, SAA_DVector *originalVerts, float
    matrix[4][4], char *name )
{
    int i, j;
    int offset;
    int    numCV;
    char *mTableName;
    SAA_DVector *shapeVerts = NULL;
    SAA_DVector *uniqueVerts = NULL;

    if ( (type == SAA_MNSRF) && make_nurbs ) 
        SAA_nurbsSurfaceSetStep( scene, model, nurbs_step, nurbs_step );

    SAA_modelGetNbVertices( scene, model, &numCV );

    // get the shape verts
    uniqueVerts = (SAA_DVector *)malloc(sizeof(SAA_DVector)*numCV);
    SAA_modelGetVertices( scene, model, SAA_GEOM_ORIGINAL, 0,
        numCV, uniqueVerts );

    if ( verbose >= 2 )
        fprintf( outStream, "%d CV's\n", numCV ); 

    if ( verbose >= 2 )
    {    
        for ( i = 0; i < numCV; i++ )
            fprintf( outStream, "uniqueVerts[%d] = %f %f %f %f\n", i, 
                uniqueVerts[i].x, uniqueVerts[i].y, 
                uniqueVerts[i].z,  uniqueVerts[i].w );
    }

    // iterate through for each key shape (except original)
    for ( i = 1; i < numShapes; i++ )
    {
        mTableName = MakeTableName( name, i );

        if ( verbose >= 1 )
        {
            fprintf( outStream, "\nMaking geometry offsets for %s...\n", 
                mTableName );

            if ( (type == SAA_MNSRF) && make_nurbs ) 
                fprintf( outStream, "calculating NURBS morphs...\n" );
            else 
                fprintf( outStream, "calculating triangle morphs...\n" );
        }

        // get the shape verts
        shapeVerts = (SAA_DVector *)malloc(sizeof(SAA_DVector)*numCV);
        SAA_modelGetVertices( scene, model, SAA_GEOM_SHAPE, i+1,
            numCV, shapeVerts );

        if ( verbose >= 2 )
        { 
            for ( j=0; j < numCV; j++ )
            {
                fprintf( outStream, "shapeVerts[%d] = %f %f %f\n", j, 
                    shapeVerts[j].x, shapeVerts[j].y, shapeVerts[j].z );
            }
        }

        // find the appropriate vertex pool
        EggVertexPool *vPool = 
            (EggVertexPool *)(_data.pools.FindName( name ));

        // for every original vertex, compare to the corresponding
        // key shape vertex and see if a vertex offset is needed 
        for ( j=0; j < numOrigVert; j++ )
        {
            double    dx, dy, dz;
        
            if ( (type == SAA_MNSRF) && make_nurbs )
            {
                //dx = shapeVerts[j].x - (originalVerts[j].x/originalVerts[j].w); 
                //dy = shapeVerts[j].y - (originalVerts[j].y/originalVerts[j].w); 
                //dz = shapeVerts[j].z - (originalVerts[j].z/originalVerts[j].w); 
                dx = shapeVerts[j].x - originalVerts[j].x; 
                dy = shapeVerts[j].y - originalVerts[j].y; 
                dz = shapeVerts[j].z - originalVerts[j].z; 
            }
            else 
            {
                // we need to map from original vertices
                // to triangle shape vertices here
                offset = findShapeVert( originalVerts[j], uniqueVerts,
                    numCV ); 

                dx = shapeVerts[offset].x - originalVerts[j].x; 
                dy = shapeVerts[offset].y - originalVerts[j].y; 
                dz = shapeVerts[offset].z - originalVerts[j].z; 
            }

            if ( verbose >= 2 )
            {
                fprintf( outStream, "oVert[%d] = %f %f %f %f\n", j, 
                    originalVerts[j].x, originalVerts[j].y, 
                    originalVerts[j].z,  originalVerts[j].w );

                if ( (type == SAA_MNSRF) && make_nurbs )
                {
                    fprintf( outStream, "global shapeVerts[%d] = %f %f %f %f\n",                        j, shapeVerts[j].x, shapeVerts[j].y, 
                        shapeVerts[j].z, shapeVerts[j].w );
                }
                else
                {
                    fprintf( outStream, 
                        "global shapeVerts[%d] = %f %f %f\n", offset, 
                        shapeVerts[offset].x, 
                        shapeVerts[offset].y, 
                        shapeVerts[offset].z );
                }

                fprintf( outStream, "%d: dx = %f, dy = %f, dz = %f\n", j,
                    dx, dy, dz );
            }

            // if change isn't negligible, make a morph vertex entry 
            double total = fabs(dx)+fabs(dy)+fabs(dz);
            if ( total > 0.00001 )
            {
                if ( vPool != NULL )
                {
                    // create offset
                    EggMorphOffset *dxyz = 
                        new EggMorphOffset( mTableName, dx, dy, dz );

                    EggVertex *eggVert;

                    // get the appropriate egg vertex
                    eggVert = vPool->Vertex(j);    

                    // add the offset to the vertex
                    eggVert->morphs.push_back( *dxyz );
                }
                else
                    fprintf( outStream, "Error: couldn't find vertex pool %s\n", name ); 
                
            } // if total
        } //for j
    } //for i
}


////////////////////////////////////////////////////////////////////
//     Function: MakeMorphTable
//       Access: Public 
//  Description: Given a scene, a model, a name and a frame time,
//                 determine what type of shape interpolation is
//                 used and call the appropriate function to extract
//                 the shape weight info for this frame...
////////////////////////////////////////////////////////////////////
void soft2egg::
MakeMorphTable( SAA_Scene *scene, SAA_Elem *model, SAA_Elem *models,
    int numModels, char *name, float time )
{
    int         numShapes;
    SAA_AnimInterpType    type;

    // Get the number of key shapes
    SAA_modelGetNbShapes( scene, model, &numShapes );

    if ( numShapes > 0 )
    {
        if ( verbose >= 1 )
            fprintf( outStream, "MakeMorphTable: %s: num shapes: %d\n", 
                name, numShapes);

        SAA_modelGetShapeInterpolation( scene, model, &type );

        if ( type == SAA_ANIM_LINEAR || type == SAA_ANIM_CARDINAL )
        {
            MakeLinearMorphTable( scene, model, numShapes, name, time );
        }
        else     // must be weighted...
        {
            // check first for expressions
            MakeExpressionMorphTable( scene, model, models, numModels,
                numShapes, name, time );
        }

    }
    
}


////////////////////////////////////////////////////////////////////
//     Function: MakeLinearMorphTable
//       Access: Public 
//  Description: Given a scene, a model, its name, and the time,
//                 get the shape fcurve for the model and determine
//                 the shape weights for the given time and use them
//                 to populate the morph table.
////////////////////////////////////////////////////////////////////
void soft2egg::
MakeLinearMorphTable( SAA_Scene *scene, SAA_Elem *model, int numShapes,
    char *name, float time )
{    
    int            i;
    SAA_Elem     fcurve;
    float        curveVal;
    SAnimTable *thisTable;
    char          *tableName;

    if ( verbose >= 1 )
        fprintf( outStream, "linear interp, getting fcurve\n" );

    SAA_modelFcurveGetShape( scene, model, &fcurve );

    SAA_fcurveEval( scene, &fcurve, time, &curveVal );    
    
    if ( verbose >= 2 )
        fprintf( outStream, "at time %f, fcurve for %s = %f\n", time,
            name, curveVal );

    float nextVal = 0.0f;

    // populate morph table values for this frame
    for ( i = 1; i < numShapes; i++ )
    {
        // derive table name from the model name
        tableName = MakeTableName( name, i );

        if ( verbose >= 2 )
            fprintf( outStream, "Linear: looking for table '%s'\n", tableName );

        //find the morph table associated with this key shape
        thisTable = (SAnimTable *)(morphRoot->FindDescendent( tableName ));

        if ( thisTable != NULL )
        {    
            if ( i == (int)curveVal )
            {
                if ( curveVal - i == 0 )
                {
                    thisTable->AddElement( 1.0f ); 
                    if ( verbose >= 2 )
                        fprintf( outStream, "adding element 1.0f\n" );
                }
                else
                {
                    thisTable->AddElement( 1.0f - (curveVal - i) );
                    nextVal = curveVal - i;
                    if ( verbose >= 2 )
                        fprintf( outStream, "adding element %f\n",                                                 1.0f - (curveVal - i) );
                }
            }
            else
            {
                if ( nextVal )
                {
                    thisTable->AddElement( nextVal );
                    nextVal = 0.0f;
                    if ( verbose >= 2 )
                        fprintf( outStream, "adding element %f\n", nextVal );
                }
                else
                {
                    thisTable->AddElement( 0.0f );
                    if ( verbose >= 2 )
                        fprintf( outStream, "adding element 0.0f\n" );
                }
            }

            if ( verbose >= 2 )
                fprintf( outStream, " to '%s'\n", tableName );
        }
        else
            fprintf( outStream, "%d: Couldn't find table '%s'\n", 
                i, tableName );
    }    

}

////////////////////////////////////////////////////////////////////
//     Function: MakeWeightedMorphTable
//       Access: Public 
//  Description: Given a scene, a model, a list of all models in the
//                 scene, the number of models in the scece, the number 
//                 of key shapes for this model, the name of the model
//                 and the current time, determine what method of
//                 controlling the shape weights is used and call the
//                 appropriate routine.
////////////////////////////////////////////////////////////////////
void soft2egg::
MakeWeightedMorphTable( SAA_Scene *scene, SAA_Elem *model, SAA_Elem *models, 
    int numModels,  int numShapes, char *name, float time )
{
    SI_Error     result;
    SAA_Elem    *weightCurves;
    float         curveVal;
    SAnimTable *thisTable;
    char          *tableName;

    // allocate array of weight curves (one for each shape)
    weightCurves = ( SAA_Elem *)malloc( sizeof( SAA_Elem ) * numShapes ); 

    result = SAA_modelFcurveGetShapeWeights(
        scene, model, numShapes, weightCurves );

    if ( result == SI_SUCCESS )
    {
        for ( int i = 1; i < numShapes; i++ )
        {
                                                SAA_fcurveEval( scene, &weightCurves[i], time, &curveVal );    

                                                // make sure soft gave us a reasonable number
                                                if (!isNum(curveVal))
                                                        curveVal = 0.0f;

                                                if ( verbose >= 2 )
                                                                fprintf( outStream, "at time %f, weightCurve[%d] for %s = %f\n",                     time, i, name, curveVal );


            // derive table name from the model name
            tableName = MakeTableName( name, i );

                                                // find and populate shape table
            if ( verbose >= 2 )
                fprintf( outStream, "Weight: looking for table '%s'\n", 
                                                                tableName );

            //find the morph table associated with this key shape
            thisTable = (SAnimTable *)(morphRoot->FindDescendent( tableName ));

            if ( thisTable != NULL )
            {    
                thisTable->AddElement( curveVal ); 
                if ( verbose >= 2 )
                    fprintf( outStream, "adding element %f\n", curveVal );
            }
            else
                fprintf( outStream, "%d: Couldn't find table '%s'\n", 
                    i, tableName );
        }
    }
}


////////////////////////////////////////////////////////////////////
//     Function: MakeExpressionMorphTable
//       Access: Public 
//  Description: Given a scene, a model and its number of key shapes
//                 generate a morph table describing transitions btwn
//                 the key shapes by evaluating the positions of the
//                 controlling sliders. 
////////////////////////////////////////////////////////////////////
void soft2egg::
MakeExpressionMorphTable( SAA_Scene *scene, SAA_Elem *model, SAA_Elem *models, 
    int numModels,  int numShapes, char *name, float time )
{    
    int            j;
    SAnimTable *thisTable;
    char          *tableName;
    char          *sliderName;
    char        *track;
    int            numExp;
    SAA_Elem   *expressions;
    float        expVal;
    float        sliderVal;

    // populate morph table values for this frame

    // compose track name
    track = NULL;

    // find how many expressions for this shape
    SAA_elementGetNbExpressions( scene, model, track, FALSE, &numExp );

    if ( verbose >= 2 )
        fprintf( outStream, "%s has %d RHS expressions\n", name, numExp );

    if ( numExp )
    {
        // get the expressions for this shape
        expressions = (SAA_Elem *)malloc(sizeof(SAA_Elem)*numExp);    

        if ( verbose >= 1 )
            fprintf( outStream, "getting %d RHS expressions...\n", numExp );

        result = SAA_elementGetExpressions( scene, model, track, FALSE, 
            numExp, expressions );

        if ( !result )
        {
            for ( j = 1; j < numExp; j++ )
            {
                if ( verbose >= 2 )
                {
                // debug see what we got
                int numvars;
        
                SAA_expressionGetNbVars( scene, &expressions[j], &numvars );

                int *varnamelen;
                int *varstrlen;
                int  expstrlen;

                varnamelen = (int *)malloc(sizeof(int)*numvars);
                varstrlen = (int *)malloc(sizeof(int)*numvars);

                SAA_expressionGetStringLengths( scene, &expressions[j],
                    numvars, varnamelen, varstrlen, &expstrlen );    

                int *varnamesizes;    
                int *varstrsizes;

                varnamesizes = (int *)malloc(sizeof(int)*numvars);
                varstrsizes = (int *)malloc(sizeof(int)*numvars);

                for ( int k = 0; k < numvars; k++ )
                {
                    varnamesizes[k] = varnamelen[k] + 1;
                    varstrsizes[k] = varstrlen[k] + 1;
                }
    
                int expstrsize = expstrlen + 1;

                char **varnames;
                char **varstrs;

                varnames = (char **)malloc(sizeof(char *)*numvars);
                varstrs = (char **)malloc(sizeof(char *)*numvars);

                for ( k = 0; k < numvars; k++ )
                {
                    varnames[k] = (char *)malloc(sizeof(char)*
                        varnamesizes[k]);

                    varstrs[k] = (char *)malloc(sizeof(char)*
                        varstrsizes[k]);
                }
        
                char *expstr = (char *)malloc(sizeof(char)* expstrsize );    

                SAA_expressionGetStrings( scene, &expressions[j], numvars,
                    varnamesizes, varstrsizes, expstrsize, varnames,
                    varstrs, expstr );
                
                if ( verbose >= 2 )
                {
                    fprintf( outStream, "expression = '%s'\n", expstr );
                    fprintf( outStream, "has %d variables\n", numvars );
                }
                } //if verbose
                
                if ( verbose >= 2 )
                    fprintf( outStream, "evaling expression...\n" );

                SAA_expressionEval( scene, &expressions[j], time, &expVal ); 

                if ( verbose >= 2 )
                    fprintf( outStream, "time %f: exp val %f\n", 
                        time, expVal );

                // derive table name from the model name
                tableName = MakeTableName( name, j );

                if ( verbose >= 2 )
                    fprintf( outStream, "Exp: looking for table '%s'\n", 
                        tableName );

                //find the morph table associated with this key shape
                thisTable = (SAnimTable *)
                    (morphRoot->FindDescendent( tableName ));

                if ( thisTable != NULL )
                {    
                    thisTable->AddElement( expVal ); 
                    if ( verbose >= 1 )    
                        fprintf( outStream, "%d: adding element %f to %s\n",
                            j, expVal, tableName );
                    fflush( outStream );
                }
                else
                {
                    fprintf( outStream, "%d: Couldn't find table '%s'", j, 
                            tableName ); 

                    fprintf( outStream, " for value %f\n", expVal );
                }
            }
        }
        else
            fprintf( outStream, "couldn't get expressions!!!\n" );
    }
    else 
        // no expression, use weight curves
        MakeWeightedMorphTable( scene, model, models, numModels, 
            numShapes, name, time );

}


////////////////////////////////////////////////////////////////////
//     Function: MakeTexAnim
//       Access: Public 
//  Description: Given a scene, a POLYGON model, and the name
//                 of the that model, get the u and v offsets for
//                 the current frame. 
////////////////////////////////////////////////////////////////////
void soft2egg::
MakeTexAnim( SAA_Scene *scene, SAA_Elem *model, char *modelName )
{
    if ( verbose >= 1 )
        fprintf( outStream, "\n\nmaking texture animation for %s...\n", 
            modelName );

    // get the color of the surface
    int         numMats;
    pfVec4        Color;
    SAA_Elem    *materials;
    void        *relinfo;

    SAA_modelRelationGetMatNbElements( scene, model, FALSE, &relinfo,
        &numMats ); 

    if ( verbose >= 2 )    
        fprintf( outStream, "surface has %d materials\n", numMats );

    if ( numMats )
    {
        float r,g,b,a;

        materials = (SAA_Elem *)malloc(sizeof(SAA_Elem)*numMats);
        
        SAA_modelRelationGetMatElements( scene, model, relinfo, 
            numMats, materials ); 

        SAA_materialGetDiffuse( scene, &materials[0], &r, &g, &b );
        SAA_materialGetTransparency( scene, &materials[0], &a );
        Color.set( r, g, b, 1.0f - a );

        int numTexLoc = 0;
        int numTexGlb = 0;

        // ASSUME only one texture per material
        SAA_Elem tex;

        // find out how many local textures per surface
        // ASSUME it only has one material
        SAA_materialRelationGetT2DLocNbElements( scene, &materials[0],
            FALSE, &relinfo, &numTexLoc );

        // if present, get local textures
        if ( numTexLoc )
        {
            if ( verbose >= 1 )
                fprintf( outStream, "%s had %d local tex\n", modelName, 
                    numTexLoc );

            // get the referenced texture
            SAA_materialRelationGetT2DLocElements( scene, &materials[0],
                TEX_PER_MAT, &tex ); 

        }
        // if no locals, try to get globals
        else
        {
            SAA_modelRelationGetT2DGlbNbElements( scene, model,
                FALSE, &relinfo, &numTexGlb );

            if ( numTexGlb )
            {
                if ( verbose >= 1 )
                    fprintf( outStream, "%s had %d global tex\n", modelName, numTexGlb );

                // get the referenced texture
                SAA_modelRelationGetT2DGlbElements( scene, 
                    model, TEX_PER_MAT, &tex ); 
            }
        }

        // add tex ref's if we found any textures
        if ( numTexLoc || numTexGlb) 
        {
            char    *fullTexName = NULL;
            char    *texName = NULL;
            char    *uniqueTexName = NULL;
            int      texNameLen;

            // get its name 
            SAA_texture2DGetPicNameLength( scene, &tex, &texNameLen);
            fullTexName = (char *)malloc(sizeof(char)*++texNameLen);
            SAA_texture2DGetPicName( scene, &tex, texNameLen, 
                fullTexName );

            // append unique identifier to texname for
            // this particular object
            uniqueTexName = (char *)malloc(sizeof(char)*
                (strlen(modelName)+strlen(texName)+3) );
            sprintf( uniqueTexName, "%s-%s", modelName, texName );
            if ( verbose >= 2 )
                fprintf( outStream, "referencing tref %s\n", 
                    uniqueTexName );

            float uScale;
            float vScale;
            float uOffset;
            float vOffset;
            SAA_Boolean    uv_swap = FALSE;

            // get texture offset info
            SAA_texture2DGetUScale( scene, &tex, &uScale );
            SAA_texture2DGetVScale( scene, &tex, &vScale );
            SAA_texture2DGetUOffset( scene, &tex, &uOffset );
            SAA_texture2DGetVOffset( scene, &tex, &vOffset );
            SAA_texture2DGetUVSwap( scene, &tex, &uv_swap );


            if ( verbose >= 2 )
            {
                fprintf( outStream, "tex uScale: %f\n", uScale );
                fprintf( outStream, "tex vScale: %f\n", vScale );
                fprintf( outStream, "tex uOffset: %f\n", uOffset );
                fprintf( outStream, "tex vOffset: %f\n", vOffset );
                if ( uv_swap )
                    fprintf( outStream, "nurbTex u & v swapped!\n" );
                else
                    fprintf( outStream, "nurbTex u & v NOT swapped\n" );
            }


            // find the vpool for this model
            EggVertexPool *vPool = 
                (EggVertexPool *)(_data.pools.FindName( modelName ));

            // if we found the pool
            if ( vPool != NULL )
            {
                // generate duv's for model
                float oldOffsets[4];
                double u, v, du, dv;    
                int        size;
                SAA_Boolean bigEndian;

                SAA_elementGetUserDataSize( scene, model, "TEX_OFFSETS",                                 &size );

                if ( size != 0 )    
                {
                    // remember original texture offsets future reference
                    SAA_elementGetUserData( scene, model, "TEX_OFFSETS", 
                        size, &bigEndian, (void *)&oldOffsets );

                    // get the original scales and offsets
                    u = oldOffsets[0];
                    v = oldOffsets[1];

                    du = u - uOffset;
                    dv = v - vOffset;

                    if ( verbose >= 1 )
                    {
                        fprintf( outStream, "original u = %f, v = %f\n",
                            u, v );
                        fprintf( outStream, "u = %f, v = %f\n",
                            uOffset, vOffset );
                        fprintf( outStream, "du = %f, dv = %f\n",
                            du, dv );
                    }

                    strstream uName, vName;

                    // create duv target names
                    uName << modelName << ".u" << ends;
                    vName << modelName << ".v" << ends;

                    // find the appropriate table to store the
                    // duv animation info into
                    SAnimTable *thisTable;

                    //find the duv U table associated with this model
                    thisTable = (SAnimTable *)(morphRoot->FindDescendent( 
                        uName.str() ));

                    if ( thisTable != NULL )
                    {    
                        thisTable->AddElement( du ); 
                        if ( verbose >= 1 )
                            fprintf( outStream, "adding element %f to %s\n", 
                                du, uName.str() );
                    }
                    else
                        fprintf( outStream, "Couldn't find uTable %s\n", 
                            uName.str() );

                    //find the duv V table associated with this model
                    thisTable = (SAnimTable *)(morphRoot->FindDescendent( 
                        vName.str() ));

                    if ( thisTable != NULL )
                    {    
                        thisTable->AddElement( dv ); 
                        if ( verbose >= 1 )
                            fprintf( outStream, "adding element %f to %s\n", 
                                dv, uName.str() );
                    }
                    else
                        fprintf( outStream, "Couldn't find vTable %s\n", 
                            uName.str() );
                }
            }
            else
                if ( verbose >= 2 )
                    fprintf( outStream, "Couldn't find vpool %s\n", modelName );
            
        }

        //free( materials );

    }

}
#endif

////////////////////////////////////////////////////////////////////
//     Function: Main
//       Access: Private 
//  Description: Instantiate converter and process a file
////////////////////////////////////////////////////////////////////
EXPCL_MISC SI_Error soft2egg(int argc, char *argv[]) {
  // pass control to the c++ system
  init_soft2egg(argc, argv);
  return SI_SUCCESS;
}
#ifdef __cplusplus
}
#endif
