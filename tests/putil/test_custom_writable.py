from panda3d.core import TypeRegistry, TypedWritable
from panda3d.core import DatagramBuffer, BamReader, BamWriter
import sys


class CustomObject(TypedWritable):
    def __init__(self):
        self.field = 0

    def get_class_type():
        registry = TypeRegistry.ptr()
        handle = registry.register_dynamic_type("CustomObject")
        registry.record_derivation(handle, TypedWritable)
        return handle

    def write_datagram(self, writer, dg):
        dg.add_uint8(self.field)

    def fillin(self, scan, reader):
        self.field = scan.get_uint8()

    @staticmethod
    def make_from_bam(scan, reader):
        obj = CustomObject()
        obj.fillin(scan, reader)
        return obj


BamReader.register_factory(CustomObject.get_class_type(), CustomObject.make_from_bam)


def test_typed_writable_subclass():
    obj = CustomObject()
    obj.field = 123
    assert obj.get_type() == CustomObject.get_class_type()
    assert obj.type == CustomObject.get_class_type()

    buf = DatagramBuffer()

    writer = BamWriter(buf)
    writer.init()
    writer.write_object(obj)
    del writer

    reader = BamReader(buf)
    reader.init()
    obj = reader.read_object()
    assert sys.getrefcount(obj) == 3
    reader.resolve()
    del reader
    assert sys.getrefcount(obj) == 2

    assert obj.field == 123
