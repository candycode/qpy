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

#include <Python.h>
#include <PyContext.h>
#include "QpyTestObject.h"

static PyMethodDef empty_module_methods[] = {
    {NULL}  /* Sentinel */
};
int main( int argc, char** argv ) {
    if( argc != 2 ) {
        std::cout << "Usage: " << argv[ 0 ] 
                  << " <python file>" << std::endl;
        exit( 1 );
    }
    Py_Initialize();
    qpy::PyContext py;
    PyObject* qpyModule = Py_InitModule3( "qpy", py.ModuleFunctions(),
                            "QPy module - where qpy functions reside" );
    py.AddGlobals( qpyModule );
    Py_INCREF( qpyModule );
    PyObject* userModule = Py_InitModule3( "qpy_test", py.ModuleFunctions(),
                            "User module - client code" );
    Py_INCREF( userModule );
    PyObject* mainModule = PyImport_AddModule( "__main__" );
    PyModule_AddObject( mainModule, "qpy", qpyModule ); 
    PyModule_AddObject( mainModule, "qpy_test", userModule ); 
    py.Add< QpyTestObject >( userModule );

    QpyTestObject* to = new QpyTestObject( 71 );
    py.AddObject( to, mainModule, userModule, "myqobj" );

    PyRun_SimpleFile( fopen( argv[ 1 ], "r" ), argv[ 1 ] );
    Py_Finalize();
    delete to;
    return 0;
}
