import qpy
import qpy_test
to = qpy_test.QpyTestObject(3)
to.Print()
print( to.GetValue() )

to.SetValue(4)
to.Print()
print( to.GetValue() )
to.SetDefaultValue()
to.Print()

to2 = to.Self()
to2.Print()
to2.SetValue(5)
to2.Print()

print(to2.copyString("a string"))
print(round(to2.copyFloat(1.2),2))
print(round(to2.copyDouble(1.3),2))

print("PROPERTY: {0}".format(to2.value))
to2.value = 234
print("PROPERTY: {0}".format(to2.value))


myqobj.Print()

print(qpy.is_qobject(myqobj))

print(qpy.is_foreign_owned(myqobj))

print(qpy.is_foreign_owned(to))

def cback(v):
	print("Got {0}".format(v))

print("CONNECT")

qpy.connect(to, 'aSignal(int)', cback)

to.aSignal(131)

class AClass(object):
 	def cback(self, v):
 		print("AClass.cback: Got {0}".format(v))

aclass = AClass()
aclass2 = AClass()

qpy.connect(to.aSignal, aclass2.cback)
qpy.connect(to, 'aSignal(int)', aclass.cback)

to.aSignal(321)

print("DISCONNECT")

qpy.disconnect(to, 'aSignal(int)', aclass.cback)
qpy.disconnect(to.aSignal, aclass2.cback)
qpy.disconnect(to, 'aSignal(int)', cback)

to.aSignal(123)

print('done!')





