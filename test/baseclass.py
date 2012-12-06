import qpy
import qpy_test

class QPyDerived(qpy_test.QpyTestObject):
    def __init__(self, v):
        qpy_test.QpyTestObject.__init__(self, v)


def cback(v):
    print('Got {0}'.format(v))

d = QPyDerived(4)
d.Print()
qpy.connect(d.aSignal, cback)

d.aSignal(123)

qpy.disconnect(d.aSignal, cback)

d.aSignal(321)    