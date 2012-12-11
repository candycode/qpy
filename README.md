QPy
====

QPy is a binding between Python and Qt(tested with versions 
4.7 and 4.8).

The main use of QPy is to make a pre-existing application or library
accessible from Python, possibly by adding a set of QObject wrappers
to make non Qt objects or non-invocable methods available in 
the Python interpreter.  

License & Copyright
-------------------

QPy is Copyright (c) 2012 by Ugo Varetto and distributed under the terms of the
BSD three-clause license (i.e. do what you want with it, just please do add 
copyright information).
Complete license information is included in the file 'LICENSE', part of this 
source code distribution.


Features
--------

- add QObject-derived types into the Python interpreter; instances of such types
  are created through the QObject constructors declared as Q_INVOKABLE
- add QObjects to the Python interpreter: QObject instances are wrapped with Python class
  instances; invokable methods are automatically added as methods to the class and
  properties as object attributes;
- specify the subset of methods and properties to be added to the Python class;
- invoke QObject methods exposed as signals, slots or through the Q_INVOKABLE
  macro;
- connect QObject signals to Python callback functions;
- connect QObject signals to QObject methods;
- connect QObject signals to Python methods
- optionally have Python destroy the added QObjects when instances are garbage
  collected;
- register user-defined types at run-time
- implement a custom member name mapper to e.g. address overloading issues or
  change names to match PEP-8  


Usage
-----


###C++

1 - Create and instance of `qpy::PyContext`.

```c++
#include <PyContext.h>
...
qpy::PyContext py;
```

2 - Create a Python module to access the QPy functions and add the QPy functions to it:

```c++
PyObject* qpyModule = Py_InitModule3( "qpy", py.ModuleFunctions(),;
                                      "QPy module - where qpy functions reside" );
py.AddGlobals( qpyModule ); //optional; adds __version__ info

```

3 - Create a new Python module where QObjects wrapper are added:

```c++
PyObject* userModule = Py_InitModule3( "qpy_user", py.ModuleFunctions(),
                                       "User module - client code" );
```

4.1 - Add types into Python module through the `qpy::PyContext::Add< Type >` method:
```c++
py.Add< QpyTestObject >( userModule );

```

4.2 - Add QObjects into a Python module through the `qpy::PyContext::AddObject` method.
```c++
 QpyTestObject* to = new QpyTestObject( 71 );
    py.AddObject( to, mainModule, userModule, "myqobj" );
```

5 - Execute Python code.


###Python

* Access the functions in the `qpy` module.
* Directly access object instances passed from C++
* Use QObject constructors to create new instances of QObject-derived types.

```python
import qpy
import qpy_user

qobj = qpy_user.QpyTestObject(3)
qobj.Print()
print( qobj.GetValue() )

myqobj.Print()
```

Connect signals to Python methods and functions or to another QObject's methods.

```python
class AClass(object):
  def cback(self, v):
    print("AClass.cback: Got {0}".format(v))

aclass = AClass()

qpy.connect(qobj.aSignal, aclass.cback)

qobj2 = qpy_user.QpyTestObject()

qpy.connect(qobj.aSignal, qobj2.catchSignal)
qpy.connect(qobj, "anotherSignal(QString)", qobj2, "catchSignal(QString)")

```

Build
-----

A CMake configuration file is provided to build the library and test code.
Since however there are no pre-compilation configuration steps you can very
easily copy and paste the source code directly into any project.

When building with the default CMake-generated Makefile the output is found
under the ```<build dir>/libqpy``` directory.

To install library, include, test driver and test scripts simply set the
installation directory(default ```/usr/local```) and run ```make install```.

Being a binding between Qt and Python the only dependencies are Python and a Qt 
distribution.
You should be able to build QPy on any platform that works with Qt >= 4.7 
and Python >= 2.6.
I am personally using QPy on the following platforms (64bit versions only):

- Windows 7
- MacOS X Snow Leopard to Mountain Lion
- Ubuntu Linux 11.x and 12.x

with Qt 4.8 and Python 2.7.


Supported types
---------------

The supported pre-registered types are:

- QObject pointer
- double
- int
- float
- QString

New types will be added. Since however type registration is dynamic
it is very easy to add new types in user code without rebuilding the library
through the `qpy::PyContext::Register*` methods.


Limitations
-----------

The main limitation is that currently overloaded methods are not supported,
i.e. QPy is not able to resolve a call to two methods like:

- MyObject::method( int );
- MyObject::method( double );

This is due to the fact that when invoking a C++ method from a dynamic language
it is not possible to automatically resolve the parameter types.

Some of this will be, to some extent, fixed in the future since it is possible
to check types from the Python C api.

One option is to use a custom Qt->Python member mapper to decide how to translate
overloaded methods to Python functions(as done in QLua).

Signal to slot binding does work without any type resolution issue if and only
if method signatures are specified as strings.

Todo
----

- example of how to create a Python extension wrapping QObjects
- docs: how to register new types
- wrap QObject::tr()
- add additional pre-registered types
- add support for `numpy`  
