// DOXYGEN FRONT PAGE

// QPy - Copyright (c) 2012,2013 Ugo Varetto
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the author and copyright holder nor the
//       names of contributors to the project may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL UGO VARETTO BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**\mainpage QPy

\warning IN PROGRESS...actual c++ doxygen comments still missing for the most part

QPy is a Qt <--> Python run-time binding framework.

QObjects are exposed to Python through Python wrapper objects.
Properties are mapped to Python class instance attributes; signals, slots and
\c Q_INVOKABLE methods to Python class instance methods. 

It is possible to add both types and pre-instantiated objects to the Python
interpreter; in case of types new instances are created from within the Python
interpreter by calling object constructors matching their C++ invokable counterparts.
All the QObject constructors declared as Q_INVOKABLE are automatically accessible
from Python.

\section Usage

The main usage for QPy is:
- to expose C++ code to Python through QObject-derived types
- connect Qt signals to Python functions and methods

The public interface is defined in the qpy::PyContext class. 
All it's needed to access QPy functionality is to create an instance
of \c PyContext and use its methods to add types and objects to Python
modules.

You make QPy functions such as \c connect and \c disconnect available from
Python code by explicitly initializing a module with the function table returned
by qpy::PyContext::ModuleFunctions() method.

You add new types and instances by invoking qpy::PyContext::Add<Type> and
PyContext::AddObject methods.

\subsection Initialization

Create:

-# an instance of \c PyContext
-# a Python module to host the QPy functions and initialize the module
  with the function table returned by \PyContext::ModuleFunctions() 
-# a separate Python module to host user-defined objects and types 

Add both modules to Python's __main__ module.

\code
Py_Initialize();
qpy::PyContext py;
PyObject* qpyModule = Py_InitModule3( "qpy", py.ModuleFunctions(),
                                "QPy module - where qpy functions reside" );
py.AddGlobals( qpyModule ); // optional, adds '__version__' info
Py_INCREF( qpyModule );
PyObject* userModule = Py_InitModule3( "qpy_test", py.ModuleFunctions(),
                                         "User module - client code" );
Py_INCREF( userModule );
PyObject* mainModule = PyImport_AddModule( "__main__" );
\endcode

Add types and pre-existing instances to the Python module

\code
py.Add< QpyTestObject >( userModule );

QpyTestObject* to = new QpyTestObject( 71 );
py.AddObject( to, // QObject-derived instance
              mainModule, // location where instance is added
              userModule, // location where class type is defined
              "myqobj" // global instance name
            );
\endcode

\subsection MethodInvocation Method invocation

QObject-derived constructors and methods are readily available to Python code.

\code
import qpy
import qpy_test

to = qpy_test.QpyTestObject(3)
to.Print()

myqobj.SetValue(123)
myqobj.Print()
\endcode

\subsection PropertyAccess Property Access

\c Q_PROPERTIES are as well available, maintaining the read/write
permissions as defined in the \c Q_PROPERTY declaration.

\code
to.value = 234
print("PROPERTY: {0}".format(to.value))
\endcode 

\subsection Signals

Qt signals can be connected to:
- Qt slots
- Python methods
- Python functions

To connect/disconnect signals you use the \c connect/\c disconnect functions
in the QPy Python module(e.g. 'qpy') where the QPy functions are added.

Two syntax formats supported:

Explicit signature:
\code
def cback(v):
    print('Got {received}'.format(received=v))
to2 = qpy_test.QpyTestObject(123)
qpy.connect(to, "aSignal(int)", to2, "catchSignal(int)")
qpy.connect(to, "aSignal(int)", cback)
\endcode

Method name:
\code
qpy.connect(to.aSignal, to2.catchSignal)
qpy.connect(to.aSignal, cback)
\endcode

\subsection Types

Data are marshalled between Python and Qt with specializations of the
following abstract classes:
- method invocation:
  - \c PyArgConstructor: C++/Qt to Python type conversion
  - \c PyQArgConstructor: Python to C++/Qt type conversion
- property access:
  - \c PyQArgConstructor: Python to QVariant conversion
  - \c PyArgConstructor: QVariant to Python conversion

passed to the PyContext::Register* methods.

*/


