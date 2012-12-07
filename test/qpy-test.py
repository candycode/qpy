# QPy - Copyright (c) 2012,2013 Ugo Varetto
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the author and copyright holder nor the
#       names of contributors to the project may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL UGO VARETTO BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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





