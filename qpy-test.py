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

myqobj.Print()

print(qpy.is_qobject(myqobj))

print(qpy.is_foreign_owned(myqobj))

print(qpy.is_foreign_owned(to))

def cback(v):
	print("Got {0}".format(v))

qpy.connect(to, 'aSignal(int)', cback)

to.aSignal(131)

class AClass(object):
 	def cback(self, v):
 		print("AClass.cback: Got {0}".format(v))

aclass = AClass()

assert aclass

qpy.connect(to, 'aSignal(int)', aclass.cback)

to.aSignal(321)

print('done!')





