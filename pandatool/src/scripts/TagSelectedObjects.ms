--created by Andrew Gartner andrewgartner@gmail.com
--PandaSE team Spring semester 2010
--Carnegie Mellon Entertainment Technology Center
--TagSelectedObjects.ms
--creates a CF and CS type for any selected object
--in a max scene in order for the egger to apply
--a collision tag of that type based on the script's rollouts

(
global TagSelectedObjects
try(destroyDialog TagSelectedObjects)catch()
rollout TagSelectedObjects "Tag Selected Objects"
(
--key = #("Test","Test2")
--val = #("Test")
dropdownlist dlist_CStype "Collision Solid Type" items:#("plane","polyset","polygon","sphere","invsphere","tube","floormesh")
dropdownlist dlist_CFtype "Collision Flag Type" items:#("descend","keep","event","solid","center","intangible","level","turnstile")
button 	btn_tag "Tag Objects" width:140 height:30
button btn_remTag "Remove Tag" width:140 height:30


fn tagObjects =
(
theObjs = for obj in geometry collect obj
for obj in theObjs do
(

	key = dlist_CStype.selected 
	val = dlist_CFtype.selected
	print key
	print val
	setUserProp obj key 1
	setUserProp obj val 1
	--obj.wirecolor = gray
	
)--for
)--fn

fn removeTags =
(
theObjs = for obj in geometry collect obj
for obj in theObjs do
(
	Cs_type = #("plane","polyset","polygon","sphere","invsphere","tube","floormesh")
	Cf_type = #("descend","keep","event","solid","center","intangible","level","turnstile")
	for cs_type in Cs_type do
	(
		key = cs_type as string
		if getUserProp obj key != undefined do
			setUserProp obj key 0
	)
	for cf_type in Cf_type do
	(
		key2 = cf_type as string
		if getUserProp obj key2 != undefined do
			setUserProp obj key2 0
	)
)
)
on btn_tag pressed do tagObjects()
on btn_remTag pressed do removeTags()
)--rollout
createDialog TagSelectedObjects
)--globals