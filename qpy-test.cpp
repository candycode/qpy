#include <Python.h>
#include "PyContext.h"
#include "QpyTestObject.h"

static PyMethodDef empty_module_methods[] = {
    {NULL}  /* Sentinel */
};
int main( int argc, char** argv ) {
    
    Py_Initialize();
    PyObject* m = Py_InitModule3( "qpy", empty_module_methods, "Example QPy module" );
    PyObject* mainModule = PyImport_AddModule( "__main__" );
    PyModule_AddObject( mainModule, "qpy", m ); 
    qpy::PyContext py;
    py.Add< QpyTestObject >( m );

    QpyTestObject* to = new QpyTestObject( 71 );
    py.AddObject( to, mainModule, m, "myqobj" );

    PyRun_SimpleFile( fopen( argv[ 1 ], "r" ), argv[ 1 ] );
    Py_Finalize();   
    return 0;
}