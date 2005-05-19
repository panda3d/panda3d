--------------------------------------------------------------------
-- Egg Model Importer
--------------------------------------------------------------------

utility EggModelImporter "Egg Model Importer" silentErrors:false
(
  button EggBrowse "Egg" tooltip:"Browse for the Egg file" width:30 align:#left across:2 height:20 offset:[-5,0]
  edittext EggName width:120 offset:[-45,0] height:20 align:#left
  spinner Scale "Scale %" range:[1,100000,100]
  button ImportButton "Import" width:80 height:30

  struct EggJoint(index,id,xv,yv,zv,pos,endpos,zaxis,thick,parent,node,anyvertex=false,children=#())

  local whiteSpace = " \t\r\n"
  local y_up
  local threshold
  local file
  local token
  local suppress
  local vertexPos
  local vertexUV
  local vertexColor
  local vertexWeights
  local vertexBones
  local polygonVerts
  local polygonMatId
  local textureId
  local textureFile
  local vertexPoolId
  local vertexPoolOffs
  local groupStack
  local jointStack
  local joints
  local theMesh
  local theSkin
  local theMaterial
  local fileSize

  fn findTexture id =
  (
    local index = findItem textureId id
    if (index == 0) then (
      append textureId id
      append textureFile id
      index = textureId.count
    )
    return index
  )

  fn findVertexPool id =
  (
    local index = findItem vertexPoolId id
    if (index == 0) then (
      local msg = "Cannot locate vertex pool " + id
      abortParse msg
      return 0
    )
    return (vertexPoolOffs[index])
  )

  fn abortParse msg =
  (
    if (suppress == false) then (
      local pos = "unknown"
      if (file != undefined) then (
        pos = filePos file
        pos = pos as string
      )
      local fmsg = msg + " (filepos=" + pos + ")"
      messageBox fmsg
      suppress = true
      if (file != undefined) then close file
      token = undefined
      file = undefined
    )
  )

  fn readToken =
  (
    if (suppress) then (
      token = undefined
      return undefined
    )
    local res = token
    while true do (
      c = " "
      while (eof(file)==false) and (findString whiteSpace c != undefined) do
        c = readChar file
      if (findString whiteSpace c != undefined) then (
        token = undefined
      ) else if (eof(file)) then (
        token = c
      ) else if (c=="/") then (
        c = readChar file
        if (c=="/") then (
          if (eof(file) == false) then
            readDelimitedString file "\n"
          continue
        ) else (
          token = "/" + c
          if (eof(file) == false) then
            token += readDelimitedString file whiteSpace
        )
      ) else if (c=="<") then (
        token = readDelimitedString file ">"
        token = "<" + token + ">"
      ) else if (c == "\"") then (
        token = readDelimitedString file c
      ) else (
        token = c + readDelimitedString file whiteSpace
      )
      local pos = filePos file
      if (pos > threshold) or (eof(file)) then (
        threshold = pos + 50000
        local percent = (pos * 100) / fileSize
        progressUpdate percent
      )
      return res
    )
  )

  fn plainToken =
  (
    return (token != undefined) and (token[1] != "<") and (token != "{") and (token != "}")
  )

  fn tagToken =
  (
    return (token != undefined) and (token[1] == "<")
  )

  fn readSpecificToken id =
  (
    local res = readToken()
    if (res != id) then (
      local expect = "Expected '" + id + "'"
      abortParse expect
    )
  )

  fn readPlainToken =
  (
    if (plainToken() == false) then
      abortParse "expected a plain token"
    return readToken()
  )
 
  fn readFloatToken =
  (
    local res = readToken()
    try (
      return (res as float)
    ) catch (
      abortParse "expected a floating point number"
      return 0.0
    )
  )

  fn readIntegerToken =
  (
    local res = readToken()
    try (
      return (res as integer)
    ) catch (
      abortParse "expected an integer number"
      return 0
    )
  )

  fn readSectionIgnore =
  (
    if (tagToken() == false) then abortParse "Expected a tag-token"
    local tok = readToken()
    while plainToken() do readToken()
    readSpecificToken "{"
    while plainToken() do readToken()
    while tagToken() do readSectionIgnore()
    readSpecificToken "}"
  )

  fn readSectionVertex ibase =
  (
    readSpecificToken "<Vertex>"
    local vtxId = readIntegerToken()
    local nextId = (vertexPos.count - ibase) + 1
    if (vtxId != nextId) then
      return abortParse "vertex sequence numbers out of order"
    readSpecificToken "{"
    local x = readFloatToken()
    local y = readFloatToken()
    local z = readFloatToken()
    local u = 1.0
    local v = 1.0
    local r = 1.0
    local g = 1.0
    local b = 1.0
    local a = 1.0
    while tagToken() do case token of (
      "<UV>" : (
        readToken()
        readSpecificToken "{"
        u = readFloatToken()
        v = readFloatToken()
        readSpecificToken "}"
      )
      "<RGBA>" : (
        readToken()
        readSpecificToken "{"
        r = readFloatToken()
        g = readFloatToken()
        b = readFloatToken()
        a = readFloatToken()
        readSpecificToken "}"
      )
      default : readSectionIgnore()
    )
    readSpecificToken "}"
    local weights = #()
    local bones = #()
    if (jointStack.count > 1) then (
      append weights 1.0
      append bones jointStack[1].index
      jointStack[1].anyvertex = true
    )
    append vertexPos [x,y,z]
    append vertexUV  [u,v,0]
    append vertexColor (color r g b a)
    append vertexWeights weights
    append vertexBones bones
  )

  fn readSectionPolygon =
  (
    readSpecificToken "<Polygon>"
    readSpecificToken "{"
    local tref = undefined
    local vpool = undefined
    local vlist = #()
    while tagToken() do case token of (
      "<TRef>" : (
        readToken()
        readSpecificToken "{"
        tref = readToken()
        readSpecificToken "}"
      )
      "<VertexRef>" : (
        readToken()
        readSpecificToken "{"
        while (plainToken()) do (
          local vid = readIntegerToken()
          append vlist vid
        )
        readSpecificToken "<Ref>"
        readSpecificToken "{"
        vpool = readPlainToken()
        readSpecificToken "}"
        readSpecificToken "}"
      )
      default : readSectionIgnore()
    )
    readSpecificToken "}"
    local matid = findTexture tref
    local offs = findVertexPool vpool
    local vcount = vlist.count
    if (vcount >= 3) then (
      local v1 = vlist[1]+offs
      for last = 3 to vcount do (
        local v2 = vlist[last-1]+offs
        local v3 = vlist[last]+offs
        append polygonVerts [v1,v2,v3]
        append polygonMatId matid
      )
    )
  )

  fn readSectionVertexPool =
  (
    readSpecificToken "<VertexPool>"
    local poolId = readPlainToken()
    local poolOffs = vertexPos.count
    append vertexPoolId poolId
    append vertexPoolOffs poolOffs
    readSpecificToken "{"
    while token == "<Vertex>" do readSectionVertex poolOffs
    readSpecificToken "}"
  )

  fn readSectionJointMatrix4 =
  (
    readSpecificToken "<Matrix4>"
    readSpecificToken "{"
          
    local A = readFloatToken()
    local B = readFloatToken()
    local C = readFloatToken()
    local D = readFloatToken()
    
    local E = readFloatToken()
    local F = readFloatToken()
    local G = readFloatToken()
    local H = readFloatToken()
    
    local I = readFloatToken()
    local J = readFloatToken()
    local K = readFloatToken()
    local L = readFloatToken()
    
    local M = readFloatToken()
    local N = readFloatToken()
    local O = readFloatToken()
    local P = readFloatToken()
    
    readSpecificToken "}"

    local joint = jointStack[1]
    local xv  = joint.xv*A + joint.yv*B + joint.zv*C
    local yv  = joint.xv*E + joint.yv*F + joint.zv*G
    local zv  = joint.xv*I + joint.yv*J + joint.zv*K
    local pos = joint.xv*M + joint.yv*N + joint.zv*O + joint.pos
    joint.xv = xv
    joint.yv = yv
    joint.zv = zv
    joint.pos = pos
  )

  fn readSectionJointTranslate =
  (
    readSpecificToken "<Translate>"
    readSpecificToken "{"
    local M = readFloatToken()
    local N = readFloatToken()
    local O = readFloatToken()
    readSpecificToken "}"

    local joint = jointStack[1]
    local pos = joint.xv*M + joint.yv*N + joint.zv*O + joint.pos
    joint.pos = pos
  )

  fn readSectionJointTransform =
  (
    readSpecificToken "<Transform>"
    readSpecificToken "{"
    while tagToken() do case token of (
      "<Matrix4>":   readSectionJointMatrix4()
      "<Translate>": readSectionJointTranslate()
      default:       readSectionIgnore()
    )
    readSpecificToken "}"
  )

  fn readSectionVertexWeight =
  (
    local indices = #()
    readSpecificToken "<VertexRef>"
    readSpecificToken "{"
    while (plainToken()) do (
      local index = readIntegerToken()
      append indices index
    )
    readSpecificToken "<Scalar>"
    readSpecificToken "membership"
    readSpecificToken "{"
    local strength = readFloatToken()
    readSpecificToken "}"
    readSpecificToken "<Ref>"
    readSpecificToken "{"
    local pool = readPlainToken()
    readSpecificToken "}"
    readSpecificToken "}"
    local joint = jointStack[1].index
    local offs = findVertexPool pool
    for index in indices do (
      append vertexWeights[index+offs] strength
      append vertexBones[index+offs] joint
      jointStack[1].anyvertex = true
    )
  )

  fn readSectionGroup =
  (
    local kind = readToken()
    local id = readPlainToken()
    if (kind == "<Joint>") then (
      local parent = jointStack[1]
      local index = joints.count + 1
      local joint = EggJoint index:index id:id xv:parent.xv yv:parent.yv zv:parent.zv pos:parent.pos parent:parent
      append parent.children index
      insertItem joint jointStack 1
      append joints joint
    ) else (
      insertItem id groupStack 1
    )
    readSpecificToken "{"
    while tagToken() do case token of (
    "<Transform>"  : readSectionJointTransform()
    "<Group>"      : readSectionGroup()
    "<Joint>"      : readSectionGroup()
    "<VertexPool>" : readSectionVertexPool()
    "<VertexRef>"  : readSectionVertexWeight()
    "<Polygon>"    : readSectionPolygon()
    default        : readSectionIgnore()
    )
    readSpecificToken "}"
    if (kind == "<Joint>") then (
      deleteItem jointStack 1
    ) else (
      deleteItem groupStack 1
    )
  )

  fn readSectionTexture =
  (
    readSpecificToken "<Texture>"
    local texid = readPlainToken()
    readSpecificToken "{"
    local texfn = readPlainToken()
    while tagToken() do readSectionIgnore()
    readSpecificToken "}"
    textureFile[findTexture texid] = texfn
  )

  fn readSectionCoordinateSystem =
  (
    readSpecificToken "<CoordinateSystem>"
    readSpecificToken "{"
    local sys = readPlainToken()
    readSpecificToken "}"
    if (sys == "Y-Up") then y_up = true
  )

  fn chooseBestChild joint dir =
  (
    if (length dir < 0.001) then return 0
    dir = normalize dir
    local firstbest = -1000
    local firstchild = -1000
    local firstpos = joint.pos
    local secondbest = 0
    for chindex in joint.children do (
      local child = joints[chindex]
      local tryfwd = child.pos - joint.pos
      if (child.pos != firstpos) and (length tryfwd > 0.001) then (
        local trydir = normalize tryfwd
        local quality = dot trydir dir
        if (quality > firstbest) then (
          secondbest = firstbest
          firstbest = quality
          firstpos = child.pos
          firstchild = child
        ) else if (quality > secondbest) then (
          secondbest = quality
        )
      )
    )
    if (firstbest > secondbest + 0.1) then
      return firstchild
    return 0
  )

  fn chooseEndPos joint thickness =
  (
    local fwd = (joint.pos - joint.parent.pos)
    if (length fwd < 0.001) then
      fwd = joint.parent.endpos - joint.parent.pos
    fwd = normalize fwd
    local child = chooseBestChild joint fwd
    if (child == 0) then (
      joint.endpos = fwd * thickness * 0.8 + joint.pos
      joint.thick = thickness * 0.8
    ) else (
      joint.endpos = child.pos
      joint.thick = length (child.pos - joint.pos)
      if (joint.thick > thickness) then joint.thick = thickness
    )
    orient = normalize (joint.endpos - joint.pos)
    local altaxis = cross orient [0,-1,0]
    if (length altaxis < 0.001) then altaxis = cross orient [0,0,1]
    local zaxis = cross altaxis orient
    joint.zaxis = normalize zaxis
  )

  fn rotateFromYUp =
  (
    if (not y_up) then return 0
    local pos
    for i = 1 to vertexPos.count do (
      pos = vertexPos[i]
      vertexPos[i] = [pos.x, -pos.z, pos.y]
    )
    for i = 1 to joints.count do (
      pos = joints[i].pos
      joints[i].pos = [pos.x, -pos.z, pos.y]
      pos = joints[i].xv
      joints[i].xv = [pos.x, -pos.z, pos.y]
      pos = joints[i].yv
      joints[i].yv = [pos.x, -pos.z, pos.y]
      pos = joints[i].zv
      joints[i].zv = [pos.x, -pos.z, pos.y]
    )
  )

  fn adjustScale =
  (
    if (Scale.value == 100.0) then return 0
    local s = Scale.value * 0.01
    for i = 1 to vertexPos.count do (
      vertexPos[i] = vertexPos[i]*s
    )
    for i = 1 to joints.count do
      joints[i].pos = joints[i].pos*s
  )    

  fn buildMesh =
  (
    if (vertexPos.count == 0) then return 0
    if (polygonVerts.count == 0) then return 0
    theMesh = mesh vertices:vertexPos faces:polygonVerts materialIDs:polygonMatId tverts:vertexUV
    buildTVFaces theMesh
    for f = 1 to theMesh.numfaces do 
      setTVFace theMesh f polygonVerts[f]
  )

  fn buildSkeleton =
  (
    if (joints.count == 0) then return 0
    local zlo = joints[1].pos.z
    local zhi = joints[1].pos.z
    for i = 1 to joints.count do (
      local z = joints[i].pos.z
      if (z < zlo) then zlo = z
      if (z > zhi) then zhi = z
    )
    local thickness = (zhi - zlo) * 0.025
    for i = 1 to joints.count do (
      local joint = joints[i]
      chooseEndPos joint thickness
      local bone = BoneSys.createBone joint.pos joint.endpos joint.zaxis
      joint.node = bone
      bone.width = joint.thick
      bone.height = joint.thick
      bone.name = joint.id
      bone.parent = joint.parent.node
    )
  )

  fn buildSkin =
  (
    if (theMesh == undefined) then return 0
    if (joints.count == 0) then return 0
    setCommandPanelTaskMode mode:#modify
    select theMesh
    theSkin = Skin()
    addModifier theMesh theSkin
    for joint in joints do
      skinOps.addBone theSkin joint.node 1
    update theMesh
    modPanel.setCurrentObject theSkin
    local nv = skinOps.getNumberVertices theSkin
    format "NV=%\n" nv
    for i = 1 to vertexWeights.count do (
      local weights = vertexWeights[i]
      local bones = vertexBones[i]
      skinOps.replaceVertexWeights theSkin i bones weights
      format "weights for vertex % are % %\n" i bones weights
    )
    for i = joints.count to 1 by -1 do (
      if (joints[i].anyvertex == false) then (
        skinOps.removeBone theSkin i
      )
    )
  )

  fn buildMaterial = 
  (
    if (textureId.count == 0) then return 0
    local dir = getFilenamePath EggName.text
    theMaterial = multiMaterial numsubs:textureId.count
    for i = 1 to textureId.count do (
      local id = textureId[i]
      local ppath = textureFile[i]
      local pieces = filterString ppath "/"
      local shortname = pieces[pieces.count]
      format "Loading material: %" ppath 
      local subMat = standardmaterial name:shortname
      subMat.diffusemap = Bitmaptexture name:shortname
      subMat.diffusemap.filename = dir + "\\" + shortname
      theMaterial.materiallist[i] = subMat
      showtexturemap subMat subMat.diffusemap true
    )
    theMesh.material = theMaterial
  )

  on EggBrowse pressed do
  (
    local myTypes = "Panda3D Egg (*.egg)|*.egg|All Files (*.*)|*.*|"
    local fname = getOpenFileName caption:"Select Panda3D Egg" types:myTypes
    if fname != undefined then EggName.text = fname
  )

  on ImportButton pressed do
  (
    y_up = false
    threshold = 0
    token = undefined
    file = undefined
    suppress = false
    textureId = #()
    textureFile = #()
    vertexPos = #()
    vertexUV = #()
    vertexColor = #()
    vertexWeights = #()
    vertexBones = #()
    polygonVerts = #()
    polygonMatId = #()
    vertexPoolId = #()
    vertexPoolOffs = #()
    groupStack = #()
    jointStack = #()
    joints = #()
    theMesh = undefined
    theSkin = undefined
    
    local root = EggJoint id:"ROOT" xv:[1,0,0] yv:[0,1,0] zv:[0,0,1] pos:[0,0,0] endpos:[0,0,1] zaxis:[0,-1,0]
    insertItem root jointStack 1

    file = openFile EggName.text
    if (file == undefined) then (
      messageBox "Cannot open egg file"
      return 0
    )

    seek file #eof
    fileSize = filePos file
    seek file 0
    progressStart "Importing..."
    readToken()
    while tagToken() do case token of (
      "<CoordinateSystem>": readSectionCoordinateSystem()
      "<Texture>"         : readSectionTexture()
      "<Group>"           : readSectionGroup()
      default             : readSectionIgnore()
    )
    progressEnd()
    if (token != undefined) then
      abortParse "Unexpected junk in egg file"
    if (file != undefined) then close file
    if (suppress) then return 0

    rotateFromYUp()
    adjustScale()
    buildMesh()
    buildSkeleton()
    buildSkin()
    buildMaterial()
  )
)

