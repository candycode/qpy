#include <Python.h>
#include <PyContext.h>
#include "QpyTestObject.h"

static PyMethodDef empty_module_methods[] = {
    {NULL}  /* Sentinel */
};
int main( int argc, char** argv ) {

    if( argc != 2 ) {
        std::cout << "Usage: " << argv[ 0 ] << " <python file>" << std::endl;
        exit( 1 );
    }
    
    Py_Initialize();
    qpy::PyContext py;
    PyObject* qpyModule = Py_InitModule3( "qpy", py.ModuleFunctions(), "QPy module - where qpy functions reside" );
    Py_INCREF( qpyModule );
    PyObject* userModule = Py_InitModule3( "qpy_test", py.ModuleFunctions(), "User module - client code" );
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
