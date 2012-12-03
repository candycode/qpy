import qpy
import qpy_test
obj = qpy_test.QpyTestObject()

print(obj.copyString('a string'))
print(obj.copyInt(123))
print(round(obj.copyFloat(1.23),2))
print(round(obj.copyDouble(12.3),2))
