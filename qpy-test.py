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

