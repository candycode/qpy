#include <Python.h>
#include "PyContext.h"
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
    PyObject* m = Py_InitModule3( "qpy", py.ModuleFunctions(), "Example QPy module" );
    PyObject* mainModule = PyImport_AddModule( "__main__" );
    PyModule_AddObject( mainModule, "qpy", m ); 
    py.Add< QpyTestObject >( m );

    QpyTestObject* to = new QpyTestObject( 71 );
    py.AddObject( to, mainModule, m, "myqobj" );

    PyRun_SimpleFile( fopen( argv[ 1 ], "r" ), argv[ 1 ] );
    Py_Finalize();   
    return 0;
}
