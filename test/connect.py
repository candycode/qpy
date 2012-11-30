import qpy
obj = qpy.QpyTestObject()
def cback(v):
    print("Got {0}".format(v))
qpy.connect(obj.anotherSignal, cback)
obj.anotherSignal("123")
class C(object):
    def cback(self, v):
        print("Got {0}".format(v))
c = C()
qpy.connect(obj.aSignal, c.cback)
obj.aSignal(123)