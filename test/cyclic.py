from qpy_test import QpyTestObject

class D(QpyTestObject):
    def __init__(self):
        QpyTestObject.__init__(self)
        self.container = None

obj1 = D()
qobjects = [obj1]
obj1.container = qobjects

del obj1

print('OK')
