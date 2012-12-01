import qpy
import qpy_test
obj = qpy_test.QpyTestObject()
def cback(v):
    print("Got {0}".format(v))
class AClass(object):
    def cback(self, v):
        print("AClass.cback: got {0}".format(v))
obj2 = qpy_test.QpyTestObject();
aclass = AClass()

qpy.connect(obj.aSignal, cback)
qpy.connect(obj.aSignal, aclass.cback)
qpy.connect(obj.anotherSignal, obj2.catchAnotherSignal)

obj.aSignal(123)
obj.anotherSignal('123')

qpy.disconnect(obj.aSignal, cback)
qpy.disconnect(obj.aSignal, aclass.cback)
qpy.disconnect(obj.anotherSignal, obj2.catchAnotherSignal)

obj.aSignal(123)
obj.anotherSignal("123")