"""Internal support module for Android builds."""

import xml.etree.ElementTree as ET

from ._proto.targeting_pb2 import Abi
from ._proto.config_pb2 import BundleConfig # pylint: disable=unused-import
from ._proto.files_pb2 import NativeLibraries # pylint: disable=unused-import
from ._proto.Resources_pb2 import ResourceTable # pylint: disable=unused-import
from ._proto.Resources_pb2 import XmlNode


AbiAlias = Abi.AbiAlias


def str_resource(id):
    def compile(attrib, manifest):
        attrib.resource_id = id
    return compile


def int_resource(id):
    def compile(attrib, manifest):
        attrib.resource_id = id
        if attrib.value.startswith('0x') or attrib.value.startswith('0X'):
            attrib.compiled_item.prim.int_hexadecimal_value = int(attrib.value, 16)
        else:
            attrib.compiled_item.prim.int_decimal_value = int(attrib.value)
    return compile


def bool_resource(id):
    def compile(attrib, manifest):
        attrib.resource_id = id
        attrib.compiled_item.prim.boolean_value = {
            'true': True, '1': True, 'false': False, '0': False
        }[attrib.value]
    return compile


def enum_resource(id, *values):
    def compile(attrib, manifest):
        attrib.resource_id = id
        attrib.compiled_item.prim.int_decimal_value = values.index(attrib.value)
    return compile


def flag_resource(id, **values):
    def compile(attrib, manifest):
        attrib.resource_id = id
        bitmask = 0
        flags = attrib.value.split('|')
        for flag in flags:
            bitmask |= values[flag]
        attrib.compiled_item.prim.int_hexadecimal_value = bitmask
    return compile


def ref_resource(id):
    def compile(attrib, manifest):
        assert attrib.value[0] == '@'
        ref_type, ref_name = attrib.value[1:].split('/')
        attrib.resource_id = id
        attrib.compiled_item.ref.name = ref_type + '/' + ref_name

        if ref_type == 'android:style':
            attrib.compiled_item.ref.id = ANDROID_STYLES[ref_name]
        elif ':' not in ref_type:
            attrib.compiled_item.ref.id = manifest.register_resource(ref_type, ref_name)
        else:
            print(f'Warning: unhandled AndroidManifest.xml reference "{attrib.value}"')
    return compile


# See data/res/values/public.xml
ANDROID_STYLES = {
    'Animation': 0x01030000,
    'Animation.Activity': 0x01030001,
    'Animation.Dialog': 0x01030002,
    'Animation.Translucent': 0x01030003,
    'Animation.Toast': 0x01030004,
    'Theme': 0x01030005,
    'Theme.NoTitleBar': 0x01030006,
    'Theme.NoTitleBar.Fullscreen': 0x01030007,
    'Theme.Black': 0x01030008,
    'Theme.Black.NoTitleBar': 0x01030009,
    'Theme.Black.NoTitleBar.Fullscreen': 0x0103000a,
    'Theme.Dialog': 0x0103000b,
    'Theme.Light': 0x0103000c,
    'Theme.Light.NoTitleBar': 0x0103000d,
    'Theme.Light.NoTitleBar.Fullscreen': 0x0103000e,
    'Theme.Translucent': 0x0103000f,
    'Theme.Translucent.NoTitleBar': 0x01030010,
    'Theme.Translucent.NoTitleBar.Fullscreen': 0x01030011,
    'Widget': 0x01030012,
    'Widget.AbsListView': 0x01030013,
    'Widget.Button': 0x01030014,
    'Widget.Button.Inset': 0x01030015,
    'Widget.Button.Small': 0x01030016,
    'Widget.Button.Toggle': 0x01030017,
    'Widget.CompoundButton': 0x01030018,
    'Widget.CompoundButton.CheckBox': 0x01030019,
    'Widget.CompoundButton.RadioButton': 0x0103001a,
    'Widget.CompoundButton.Star': 0x0103001b,
    'Widget.ProgressBar': 0x0103001c,
    'Widget.ProgressBar.Large': 0x0103001d,
    'Widget.ProgressBar.Small': 0x0103001e,
    'Widget.ProgressBar.Horizontal': 0x0103001f,
    'Widget.SeekBar': 0x01030020,
    'Widget.RatingBar': 0x01030021,
    'Widget.TextView': 0x01030022,
    'Widget.EditText': 0x01030023,
    'Widget.ExpandableListView': 0x01030024,
    'Widget.ImageWell': 0x01030025,
    'Widget.ImageButton': 0x01030026,
    'Widget.AutoCompleteTextView': 0x01030027,
    'Widget.Spinner': 0x01030028,
    'Widget.TextView.PopupMenu': 0x01030029,
    'Widget.TextView.SpinnerItem': 0x0103002a,
    'Widget.DropDownItem': 0x0103002b,
    'Widget.DropDownItem.Spinner': 0x0103002c,
    'Widget.ScrollView': 0x0103002d,
    'Widget.ListView': 0x0103002e,
    'Widget.ListView.White': 0x0103002f,
    'Widget.ListView.DropDown': 0x01030030,
    'Widget.ListView.Menu': 0x01030031,
    'Widget.GridView': 0x01030032,
    'Widget.WebView': 0x01030033,
    'Widget.TabWidget': 0x01030034,
    'Widget.Gallery': 0x01030035,
    'Widget.PopupWindow': 0x01030036,
    'MediaButton': 0x01030037,
    'MediaButton.Previous': 0x01030038,
    'MediaButton.Next': 0x01030039,
    'MediaButton.Play': 0x0103003a,
    'MediaButton.Ffwd': 0x0103003b,
    'MediaButton.Rew': 0x0103003c,
    'MediaButton.Pause': 0x0103003d,
    'TextAppearance': 0x0103003e,
    'TextAppearance.Inverse': 0x0103003f,
    'TextAppearance.Theme': 0x01030040,
    'TextAppearance.DialogWindowTitle': 0x01030041,
    'TextAppearance.Large': 0x01030042,
    'TextAppearance.Large.Inverse': 0x01030043,
    'TextAppearance.Medium': 0x01030044,
    'TextAppearance.Medium.Inverse': 0x01030045,
    'TextAppearance.Small': 0x01030046,
    'TextAppearance.Small.Inverse': 0x01030047,
    'TextAppearance.Theme.Dialog': 0x01030048,
    'TextAppearance.Widget': 0x01030049,
    'TextAppearance.Widget.Button': 0x0103004a,
    'TextAppearance.Widget.IconMenu.Item': 0x0103004b,
    'TextAppearance.Widget.EditText': 0x0103004c,
    'TextAppearance.Widget.TabWidget': 0x0103004d,
    'TextAppearance.Widget.TextView': 0x0103004e,
    'TextAppearance.Widget.TextView.PopupMenu': 0x0103004f,
    'TextAppearance.Widget.DropDownHint': 0x01030050,
    'TextAppearance.Widget.DropDownItem': 0x01030051,
    'TextAppearance.Widget.TextView.SpinnerItem': 0x01030052,
    'TextAppearance.WindowTitle': 0x01030053,
}


# See data/res/values/public.xml, attrs.xml and especially attrs_manifest.xml
ANDROID_ATTRIBUTES = {
    'allowBackup': bool_resource(0x1010280),
    'allowClearUserData': bool_resource(0x1010005),
    'allowParallelSyncs': bool_resource(0x1010332),
    'allowSingleTap': bool_resource(0x1010259),
    'allowTaskReparenting': bool_resource(0x1010204),
    'alwaysRetainTaskState': bool_resource(0x1010203),
    'appCategory': enum_resource(0x01010545, "game", "audio", "video", "image", "social", "news", "maps", "productivity", "accessibility"),
    'clearTaskOnLaunch': bool_resource(0x1010015),
    'configChanges': flag_resource(0x0101001f, mcc=0x0001, mnc=0x0002, locale=0x0004, touchscreen=0x0008, keyboard=0x0010, keyboardHidden=0x0020, navigation=0x0040, orientation=0x0080, screenLayout=0x0100, uiMode=0x0200, screenSize=0x0400, smallestScreenSize=0x0800, layoutDirection=0x2000, colorMode=0x4000, grammaticalGender=0x8000, fontScale=0x40000000, fontWeightAdjustment=0x10000000),
    'debuggable': bool_resource(0x0101000f),
    'documentLaunchMode': enum_resource(0x1010445, "none", "intoExisting", "always", "never"),
    'enabled': bool_resource(0x101000e),
    'excludeFromRecents': bool_resource(0x1010017),
    'exported': bool_resource(0x1010010),
    'extractNativeLibs': bool_resource(0x10104ea),
    'finishOnTaskLaunch': bool_resource(0x1010014),
    'fullBackupContent': bool_resource(0x10104eb),
    'glEsVersion': int_resource(0x1010281),
    'hardwareAccelerated': bool_resource(0x10102d3),
    'hasCode': bool_resource(0x101000c),
    'host': str_resource(0x1010028),
    'icon': ref_resource(0x1010002),
    'immersive': bool_resource(0x10102c0),
    'installLocation': enum_resource(0x10102b7, "auto", "internalOnly", "preferExternal"),
    'isGame': bool_resource(0x010103f4),
    'label': str_resource(0x01010001),
    'launchMode': enum_resource(0x101001d, "standard", "singleTop", "singleTask", "singleInstance"),
    'maxSdkVersion': int_resource(0x1010271),
    'mimeType': str_resource(0x1010026),
    'minSdkVersion': int_resource(0x101020c),
    'multiprocess': bool_resource(0x1010013),
    'name': str_resource(0x1010003),
    'noHistory': bool_resource(0x101022d),
    'pathPattern': str_resource(0x101002c),
    'preferMinimalPostProcessing': bool_resource(0x101060c),
    'required': bool_resource(0x101028e),
    'resizeableActivity': bool_resource(0x10104f6),
    'scheme': str_resource(0x1010027),
    'screenOrientation': enum_resource(0x101001e, 'landscape', 'portrait', 'user', 'behind', 'sensor', 'nosensor', 'sensorLandscape', 'sensorPortrait', 'reverseLandscape', 'reversePortrait', 'fullSensor', 'userLandscape', 'userPortrait', 'fullUser', 'locked'),
    'stateNotNeeded': bool_resource(0x1010016),
    'supportsRtl': bool_resource(0x010103af),
    'supportsUploading': bool_resource(0x101029b),
    'targetSandboxVersion': int_resource(0x101054c),
    'targetSdkVersion': int_resource(0x1010270),
    'theme': ref_resource(0x01010000),
    'value': str_resource(0x1010024),
    'versionCode': int_resource(0x101021b),
    'versionName': str_resource(0x101021c),
}


class AndroidManifest:
    def __init__(self):
        super().__init__()
        self._stack = []
        self.root = XmlNode()
        self.resource_types = []
        self.resources = {}

    def parse_xml(self, data):
        parser = ET.XMLParser(target=self)
        parser.feed(data)
        parser.close()

    def start_ns(self, prefix, uri):
        decl = self.root.element.namespace_declaration.add()
        decl.prefix = prefix
        decl.uri = uri

    def start(self, tag, attribs):
        if not self._stack:
            node = self.root
        else:
            node = self._stack[-1].child.add()

        element = node.element
        element.name = tag

        self._stack.append(element)

        for key, value in attribs.items():
            attrib = element.attribute.add()
            attrib.value = value

            if key.startswith('{'):
                attrib.namespace_uri, key = key[1:].split('}', 1)
                res_compile = ANDROID_ATTRIBUTES.get(key, None)
                if not res_compile:
                    print(f'Warning: unhandled AndroidManifest.xml attribute "{key}"')
            else:
                res_compile = None

            attrib.name = key

            if res_compile:
                res_compile(attrib, self)

    def end(self, tag):
        self._stack.pop()

    def register_resource(self, type, name):
        if type not in self.resource_types:
            self.resource_types.append(type)
            type_id = len(self.resource_types)
            self.resources[type] = []
        else:
            type_id = self.resource_types.index(type) + 1

        resources = self.resources[type]
        if name in resources:
            entry_id = resources.index(name)
        else:
            entry_id = len(resources)
            resources.append(name)

        id = (0x7f << 24) | (type_id << 16) | (entry_id)
        return id

    def dumps(self):
        return self.root.SerializeToString()
