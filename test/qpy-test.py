import qpy
to = qpy.QpyTestObject(3)
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
print(to2.copyFloat(1.2))
print(to2.copyDouble(1.3))

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

#qpy.connect(to.aSignal, aclass2.cback)
qpy.connect(to, 'aSignal(int)', aclass.cback)

to.aSignal(321)

qpy.connect(to.anotherSignal, to2.catchSignal)

print("DISCONNECT")

qpy.disconnect(to, 'aSignal(int)', aclass.cback)
qpy.disconnect(to, 'aSignal(int)', cback)

to.aSignal(123)


to.anotherSignal('Emitted')

print('done!')





