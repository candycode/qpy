#include "PyContext.h"



namespace qpy {

PyContext::ConnectList PyContext::endpoints_;
int PyContext::getterMethodId_ = -1;
bool PyContext::signal_ = false;

//----------------------------------------------------------------------------
PyTypeObject* PyContext::AddType( const QMetaObject* mo, 
                           		  PyObject* module,
                                  bool checkConstructor,
                                  const char* className,
                                  const char* doc ) {
    Type* pet = ExistingType( mo, module );
    if( pet ) return &pet->pyType;
  
    Type t;
    t.metaObject = mo;
    types_.push_back( t );
    Type* pt = &types_.back();
    pt->pyContext = this;
    pt->className = className ? className : mo->className();
    assert( PyModule_GetName( module ) );
    pt->fullClassName = std::string( PyModule_GetName( module ) ) + "." + pt->className;
    if( doc) pt->doc = doc;
    pt->pyModule = module;
    // do we need this ? objects might be wrapping a pre-existing QObject
    // not requiring construction; should be run-time configurable
    if( checkConstructor && mo->constructorCount() < 1 ) {
        throw std::logic_error( "No constructor available" );
        return 0; // in case exceptions not enabled
    }
    for( int i = 0; i != mo->constructorCount(); ++i ) {
        QMetaMethod mm = mo->constructor( i );
        pt->ctorParams.push_back( GenerateQArgWrappers( mm.parameterTypes() ) );
                                 
    }
    for( int i = 0; i != mo->methodCount(); ++i ) {
        QMetaMethod mm = mo->method( i );
        pt->methods.push_back( Method( mm,
                                       GenerateQArgWrappers( mm.parameterTypes() ),
                                       GeneratePyArgWrapper( mm.typeName() ),
                                       mo ) );
        QString sig = mm.signature();
        sig.truncate( sig.indexOf( "(" ) );
        pt->pyMethodNames.push_back( sig.toStdString() );
       
        PyGetSetDef gs = { const_cast< char* >( pt->pyMethodNames.back().c_str() ),
                           reinterpret_cast< getter >( PyQObjectGetter ),
                           reinterpret_cast< setter >( PyQObjectSetter ),
                           const_cast< char* >( "QPy method invocation function" ),
                           reinterpret_cast< void* >( i ) };
        pt->pyMethods.push_back( gs );                                                        

    }
    //add sentinel!
    PyGetSetDef gsd = { 0, 0, 0, 0, 0};
    pt->pyMethods.push_back( gsd );

    pt->pyType = CreatePyType( *pt );
    PyType_Ready( &pt->pyType );
    pt->pyType.tp_new = PyQObjectNew;
    PyObject* pyPtr = PyCapsule_New( pt, "qpy type info", 0 );
    assert( pyPtr && "NULL pyPtr" );
    assert( pt->pyType.tp_dict && "NULL dict" );

    PY_CHECK( PyDict_SetItemString( pt->pyType.tp_dict, "__qpy_type_info", pyPtr ) );

    Py_INCREF( reinterpret_cast< PyObject* >( &pt->pyType ) );
    if( PyModule_AddObject( module, pt->className.c_str(),
                            reinterpret_cast< PyObject* >( &pt->pyType ) ) != 0 ) {
        types_.pop_back();
        throw std::runtime_error( "Cannot add object to module" );
        return 0;
    }
    return &pt->pyType;
}

//----------------------------------------------------------------------------
PyMethodDef* PyContext::ModuleFunctions() {
    static PyMethodDef functions[] = {
        { "acquire", reinterpret_cast< PyCFunction >( PyQObjectAcquire ), METH_VARARGS,
          "Acquire ownership of QObject derived object; "
          "garbage collected by Python" },
        { "release", reinterpret_cast< PyCFunction >( PyQObjectRelease ), METH_VARARGS,
          "Release ownership of QObject derived object; "
          "will *not* be garbage collected by Python" },
        { "is_qobject", reinterpret_cast< PyCFunction >( PyQObjectIsQObject ), METH_VARARGS,
          "Checks if object is a QObject" },
        { "is_foreign_owned", reinterpret_cast< PyCFunction >( PyQObjectIsForeignOwned ), METH_VARARGS,
          "Checks if QObject is foreign owned.\nForeign owned objects shall not be garbge collected by Python" },
        { "connect", reinterpret_cast< PyCFunction >( PyQObjectConnect ), METH_VARARGS,
          "Connect Qt signal to Python function or method" },
        { "disconnect", reinterpret_cast< PyCFunction >( PyQObjectDisconnect ), METH_VARARGS,
          "Disconnect Qt signal from Python function or method" },
        { "qobject_ptr", reinterpret_cast< PyCFunction >( PyQObjectPtr ), METH_VARARGS,
          "Return pointer to embedded QObject" },           
        {0}
                                    };
    return functions;
}  

//----------------------------------------------------------------------------
 PyObject* PyContext::AddObject( QObject* qobj, 
                                 PyObject* targetModule, // where instance is added 
                                 PyObject* typeModule, // where type is defined
                                 const char* instanceName,
                                 bool pythonOwned ) {
    static const bool CHECK_CONSTRUCTOR_OPTION = false;
    PyTypeObject* pt = AddType( qobj->metaObject(), typeModule, CHECK_CONSTRUCTOR_OPTION );
    assert( pt );
    PyQObject* obj = reinterpret_cast< PyQObject* >( 
                            PyObject_CallObject( reinterpret_cast< PyObject* >( pt ), 0  ) );
    obj->foreignOwned = !pythonOwned;
    obj->obj = qobj;
    obj->pyModule = targetModule;
    // this method is might be called also to wrap QObject* returned by methods; in this case
    // it should not add the object explicitly into the module
    if( instanceName ) {
        PyModule_AddObject( targetModule, instanceName, reinterpret_cast< PyObject* >( obj ) );
    }
    
    return reinterpret_cast< PyObject* >( obj );
}

//----------------------------------------------------------------------------
void PyContext::InitArgFactory() {
    RegisterType< IntQArgConstructor, IntPyArgConstructor >( QMetaType::Int );
    RegisterType< VoidStarQArgConstructor, NoPyArgConstructor >( QMetaType::VoidStar );
    RegisterType< ObjectStarQArgConstructor, ObjectStarPyArgConstructor >( QMetaType::QObjectStar );
    RegisterType< NoQArgConstructor, VoidPyArgConstructor >( QMetaType::Void );
    RegisterType< StringQArgConstructor, StringPyArgConstructor >( QMetaType::QString );
    RegisterType< FloatQArgConstructor, FloatPyArgConstructor >( QMetaType::Float );
    RegisterType< DoubleQArgConstructor, DoublePyArgConstructor >( QMetaType::Double );
};

//----------------------------------------------------------------------------   
PyContext::QArgWrappers PyContext::GenerateQArgWrappers( const ArgumentTypes& at ) {
    QArgWrappers aw;
    ///@warning moc *always* adds a QObject* to any constructor!!!
    for( ArgumentTypes::const_iterator i = at.begin(); i != at.end(); ++i ) {
        if( !argFactory_.contains( *i ) 
            || dynamic_cast< const NoQArgConstructor* >( argFactory_[ *i ].QArgCtor() ) ) {
            throw std::logic_error( ( "Type " + QString( *i ) + " unknown" ).toStdString() );
        } else {
            aw.push_back( QArgWrapper( argFactory_[ *i ].MakeQArgConstructor() ) );
        }
    }
    return aw;
}   

//----------------------------------------------------------------------------    
PyArgWrapper PyContext::GeneratePyArgWrapper( QString typeName ) {
    typeName = typeName.isEmpty() ? QMetaType::typeName( QMetaType::Void ) : typeName;
    if( !argFactory_.contains( typeName )  
        || dynamic_cast< const NoPyArgConstructor* >( argFactory_[ typeName ].PyArgCtor() ) ) {
            throw std::logic_error( ( "Type " + typeName + " unknown" ).toStdString() );
        return PyArgWrapper();
    } else {
        return PyArgWrapper( argFactory_[ typeName ].MakePyArgConstructor() );
    }
}    

//============================================================================
// Python interface
//============================================================================

//----------------------------------------------------------------------------
PyObject* PyContext::PyQObjectConnect( PyObject* self, PyObject* args, PyObject* kwargs ) {
    signal_ = false;
    PyObject* sourceObject = 0;
    const char* sourceMethod = 0;
    const char* targetMethod = 0;
    PyObject* targetFunction = 0;
    PyQObject* pyqobj = 0;
    PyObject* targetObject = 0;
    PyQObject* pyqobjTarget = 0;
    int miTarget = -1;
    int mi = -1;
    bool qtobjects = false;
    struct Clear{
        ~Clear() { endpoints_.clear(); }
    } CLEAR_ENDPOINTS;
    if( PyTuple_Size( args ) == 3 ) {
        PyArg_ParseTuple( args, "OsO", &sourceObject, &sourceMethod, &targetFunction );
        if( PyObject_HasAttrString( sourceObject, "__qpy_qobject_tag" ) ) {
            pyqobj = reinterpret_cast< PyQObject* >( sourceObject );       
            mi = pyqobj->type->metaObject->indexOfMethod( sourceMethod ); 
        } else {
            RaisePyError( "Not a PyQObject" );
            return 0;
        }       
    } else if( PyTuple_Size( args ) == 2 ) {
        PyObject* sourceMethodFunction = 0;
        PyArg_ParseTuple( args, "OO", &sourceMethodFunction, &targetFunction );
        if( endpoints_.size() > 1 ) {
            pyqobjTarget = endpoints_.back().pyqobj;
            miTarget = endpoints_.back().methodId;
            endpoints_.pop_back();
            qtobjects = true;
        }
        pyqobj = endpoints_.back().pyqobj;
        mi = endpoints_.back().methodId;
        endpoints_.pop_back();
    } else if( PyTuple_Size( args ) == 4 ) {
        PyArg_ParseTuple( args, "OsOs", &sourceObject, &sourceMethod, &targetObject, &targetMethod );
        if( PyObject_HasAttrString( sourceObject, "__qpy_qobject_tag" ) ) {
            pyqobj = reinterpret_cast< PyQObject* >( sourceObject );       
            mi = pyqobj->type->metaObject->indexOfMethod( sourceMethod ); 
        } else {
            RaisePyError( "Not a PyQObject" );
            return 0;
        }
        if( PyObject_HasAttrString( targetObject, "__qpy_qobject_tag" ) ) {
            pyqobjTarget = reinterpret_cast< PyQObject* >( targetObject );       
            miTarget = pyqobj->type->metaObject->indexOfMethod( targetMethod ); 
        } else {
            RaisePyError( "Not a PyQObject" );
            return 0;
        }
        qtobjects = true;             
    } else {
        RaisePyError( "3 or 4 arguments required" );
        return 0;
    }             
    if( mi < 0 ) {
        RaisePyError( ( std::string( "Cannot find method" ) 
                      + std::string( sourceMethod ) ).c_str() );
        return 0;
    }
    if( qtobjects ) {
        QMetaObject::connect( pyqobj->obj, mi , pyqobjTarget->obj, miTarget );
    } else {
        QMetaMethod mm = pyqobj->type->metaObject->method( mi );
        QList< QByteArray > params = mm.parameterTypes();
        QList< PyArgWrapper > types;
        for( QList< QByteArray >::const_iterator i = params.begin();
             i != params.end(); ++i ) {
            types.push_back( pyqobj->type->pyContext->GeneratePyArgWrapper( i->constData() ) ); 
        }
        pyqobj->type->pyContext->dispatcher_.Connect( pyqobj->obj, mi, types, targetFunction,
                                                      pyqobj->type->pyModule );  
    }
       
    Py_RETURN_NONE;
}

//----------------------------------------------------------------------------
PyObject* PyContext::PyQObjectDisconnect( PyObject* self, PyObject* args, PyObject* kwargs ) {
    signal_ = false;
    PyObject* sourceObject = 0;
    const char* sourceMethod = 0;
    PyObject* targetFunction = 0;
    PyQObject* srcQObject = 0;
    struct Clear{
        ~Clear() { endpoints_.clear(); }
    } CLEAR_ENDPOINTS;
    if( PyTuple_Size( args ) == 3 ) {
        PyArg_ParseTuple( args, "OsO", &sourceObject, &sourceMethod, &targetFunction );
        if( PyObject_HasAttrString( sourceObject, "__qpy_qobject_tag" ) ) {
            PyQObject* pyqobj = reinterpret_cast< PyQObject* >( sourceObject );
            const int mi = pyqobj->type->metaObject->indexOfMethod( sourceMethod );
            if( mi < 0 ) {
                RaisePyError( ( std::string( "Cannot find method" ) 
                            + std::string( sourceMethod ) ).c_str() );
            }
            pyqobj->type->pyContext->dispatcher_.Disconnect( pyqobj->obj, mi, targetFunction );
            Py_RETURN_NONE;
        } else {
            RaisePyError( "Not a PyQObject", PyExc_TypeError );
            return 0;
        }
    } else if( PyTuple_Size( args ) == 2 ) {
        PyQObject* pyqobj = 0;
        PyQObject* pyqobjTarget = 0;
        bool qtobjects = false;
        int mi = -1;
        int miTarget = -1;
        if( endpoints_.size() > 1 ) {
            pyqobjTarget = endpoints_.back().pyqobj;
            miTarget = endpoints_.back().methodId;
            endpoints_.pop_back();
            qtobjects = true;
        }
        pyqobj = endpoints_.back().pyqobj;
        mi = endpoints_.back().methodId;
        endpoints_.pop_back();
        if( qtobjects ) {
            QMetaObject::disconnect( pyqobj->obj, mi, pyqobjTarget->obj, miTarget );
        } else {
            PyObject* dummy = 0;
            PyArg_ParseTuple( args, "OO", &dummy, &targetFunction );    
            pyqobj->type->pyContext->dispatcher_.Disconnect( pyqobj->obj, mi, targetFunction );
        }
        Py_RETURN_NONE;
    } else if( PyTuple_Size( args ) == 4 ) {
        PyObject* sourceObject = 0;
        const char* sourceMethod = 0;
        PyObject* targetObject = 0;
        const char* targetMethod = 0;
        PyQObject* pyqobj = 0;
        PyQObject* pyqobjTarget = 0;
        int mi = -1;
        int miTarget = -1;
        PyArg_ParseTuple( args, "OsOs", &sourceObject, &sourceMethod, &targetObject, &targetMethod );
        if( PyObject_HasAttrString( sourceObject, "__qpy_qobject_tag" ) ) {
            pyqobj = reinterpret_cast< PyQObject* >( sourceObject );       
            mi = pyqobj->type->metaObject->indexOfMethod( sourceMethod ); 
        } else {
            RaisePyError( "Not a PyQObject" );
            return 0;
        }
        if( PyObject_HasAttrString( targetObject, "__qpy_qobject_tag" ) ) {
            pyqobjTarget = reinterpret_cast< PyQObject* >( targetObject );       
            miTarget = pyqobj->type->metaObject->indexOfMethod( targetMethod ); 
        } else {
            RaisePyError( "Not a PyQObject" );
            return 0;
        }
        QMetaObject::disconnect( pyqobj->obj, mi, pyqobjTarget->obj, miTarget );
        Py_RETURN_NONE;             
    } else {
        RaisePyError( "3 or 4 arguments required" );
        return 0;
    }        
}

//----------------------------------------------------------------------------
PyObject* PyContext::PyQObjectIsForeignOwned( PyObject* self, PyObject* args, PyObject* kwargs ) {
    PyObject* obj = 0;
    PyArg_ParseTuple( args, "O", &obj );
    if( PyObject_HasAttrString( obj, "__qpy_qobject_tag" ) ) {
        PyQObject* pyqobj = reinterpret_cast< PyQObject* >( obj );
        return PyBool_FromLong( int( pyqobj->foreignOwned ) );
    } else {
        RaisePyError( "Not a PyQObject", PyExc_TypeError );
        return 0;
    }
}

//----------------------------------------------------------------------------
PyObject* PyContext::PyQObjectIsQObject( PyObject* self, PyObject* args, PyObject* kwargs ) {
    PyObject* obj = 0;
    PyArg_ParseTuple( args, "O", &obj );
    if( PyObject_HasAttrString( obj, "__qpy_qobject_tag" ) ) {
        return PyBool_FromLong( 1 ); 
    } else {
        return PyBool_FromLong( 0 );
    }
}

//----------------------------------------------------------------------------
PyObject* PyContext::PyQObjectAcquire( PyObject* self, PyObject* args, PyObject* kwargs ) {
    PyObject* obj = 0;
    PyArg_ParseTuple( args, "O", &obj );
    if( PyObject_HasAttrString( obj, "__qpy_qobject_tag" ) ) {
        PyQObject* pyqobj = reinterpret_cast< PyQObject* >( obj );
        pyqobj->foreignOwned = false;
        Py_RETURN_NONE;
    } else {
       RaisePyError( "Not a PyQObject", PyExc_TypeError );
       return 0;
    }         
}

//----------------------------------------------------------------------------
PyObject* PyContext::PyQObjectRelease( PyObject* self, PyObject* args, PyObject* kwargs ) {
    PyObject* obj = 0;
    PyArg_ParseTuple( args, "O", &obj );
    if( PyObject_HasAttrString( obj, "__qpy_qobject_tag" ) ) {
        PyQObject* pyqobj = reinterpret_cast< PyQObject* >( obj );
        pyqobj->foreignOwned = true;
        Py_RETURN_NONE;
    } else {
       RaisePyError( "Not a PyQObject", PyExc_TypeError );
       return 0;
    }         
}

//----------------------------------------------------------------------------
PyObject* PyContext::PyQObjectPtr( PyObject* self, PyObject* args, PyObject* kwargs ) {
    PyObject* obj = 0;
    PyArg_ParseTuple( args, "O", &obj );
    if( PyObject_HasAttrString( obj, "__qpy_qobject_tag" ) ) {
        PyQObject* pyqobj = reinterpret_cast< PyQObject* >( obj );
        return PyLong_FromVoidPtr( pyqobj->obj );
    } else {
       RaisePyError( "Not a PyQObject", PyExc_TypeError );
       return 0;
    }         
}

//----------------------------------------------------------------------------
PyObject* PyContext::PyQObjectGetter( PyQObject* qobj, void* closure /*method id*/ ) {
    int mid = int( reinterpret_cast< size_t >( closure ) );
    if( qobj->obj->metaObject()->method( mid ).methodType() == QMetaMethod::Signal ) {
        signal_ = true;
    }
    if( signal_ ) endpoints_.push_back( ConnectEntry( qobj, mid ) );
    getterMethodId_ = mid;
    //should indeed have one function per type (var args, no args...) and return the proper one
    Py_INCREF( qobj->invoke );
    return qobj->invoke;
}

//----------------------------------------------------------------------------
int PyContext::PyQObjectSetter( PyQObject*, PyObject*, void* closure ) {
    PyErr_SetString( PyExc_TypeError, "QPy methods are readonly!" );
    return -1;
}

//----------------------------------------------------------------------------
PyObject* PyContext::PyQObjectNew( PyTypeObject* type, PyObject*, PyObject* ) {
    PyQObject* self = reinterpret_cast< PyQObject* >( type->tp_alloc( type, 0 ) );
    if( !self ) return 0;
    self->obj = 0;
    self->foreignOwned = false;
    self->pyModule = 0;
    self->type = reinterpret_cast< Type* >(
        PyCapsule_GetPointer( PyDict_GetItemString( type->tp_dict, "__qpy_type_info" ), "qpy type info" ) );
    return reinterpret_cast< PyObject* >( self );
}

//----------------------------------------------------------------------------
PyObject* PyContext::PyQObjectInvokeMethod( PyQObject* self, PyObject* args ) {
    signal_ = false;
    endpoints_.clear(); // cheap, usually emtpy or with a single element, in case
                        // a signal is invoke explicitly
    std::vector< QGenericArgument > ga( MAX_GENERIC_ARGS );
    const int sz = int( PyTuple_Size( args ) );
    const Method& m = self->type->methods[ getterMethodId_ ];
    if( sz > m.argumentWrappers_.size() ) {
        RaisePyError( qPrintable(QString( "Method %1::%2 requires %3 arguments, %4 provided" )
                      .arg( m.metaObject_->className() )
                      .arg( m.metaMethod_.signature() )
                      .arg( m.argumentWrappers_.size() )
                      .arg( sz ) ) );
        return 0;
    }
    for( int i = 0; i != sz; ++i ) {
        PyObject* obj = PyTuple_GetItem( args, i );
        ga[ i ] = m.argumentWrappers_[ i ].Arg( obj );
    }
    try {     
        if( m.returnWrapper_.MetaType() == QMetaType::Void ) {
            m.metaMethod_.invoke( self->obj, Qt::AutoConnection, ga[ 0 ], ga[ 1 ], ga[ 2 ], ga[ 3 ],
                      ga[ 4 ], ga[ 5 ], ga[ 6 ], ga[ 7 ], ga[ 8 ], ga[ 9 ] );
            Py_INCREF(Py_None);
            return Py_None;
        } else {  
            if( m.returnWrapper_.IsQObjectPtr() ) {
                QObject* ptr = 0;
                m.metaMethod_.invoke( self->obj, Qt::AutoConnection, Q_RETURN_ARG( QObject*, ptr ),
                      ga[ 0 ], ga[ 1 ], ga[ 2 ], ga[ 3 ],
                      ga[ 4 ], ga[ 5 ], ga[ 6 ], ga[ 7 ], ga[ 8 ], ga[ 9 ] );
                PyQObject* obj = reinterpret_cast< PyQObject* > (
                                     PyObject_CallObject( reinterpret_cast< PyObject* >( &(self->type->pyType) ), 0 ) );
                obj->foreignOwned = false;
                obj->obj = ptr;
                return reinterpret_cast< PyObject* >( obj );
            } else {
                 m.metaMethod_.invoke( self->obj, Qt::AutoConnection, m.returnWrapper_.Arg(),
                      ga[ 0 ], ga[ 1 ], ga[ 2 ], ga[ 3 ],
                      ga[ 4 ], ga[ 5 ], ga[ 6 ], ga[ 7 ], ga[ 8 ], ga[ 9 ] );
                return m.returnWrapper_.Create();                     
            }
        }
    } catch( const std::exception& e ) {
        RaisePyError( e.what() );
        return 0;        
    } catch( ... ) {
        RaisePyError( "Exception raised" );
        return 0;
    }       
}   

//----------------------------------------------------------------------------
int PyContext::PyQObjectInit( PyQObject* self, PyObject* args, PyObject* kwds ) {
    if( !self->foreignOwned ) {
        std::vector< QGenericArgument > ga( MAX_GENERIC_ARGS );
        const int sz = int( PyTuple_Size( args ) );
        const QArgWrappers* pArgWrappers = 0;
        for( int i = 0; i != self->type->ctorParams.size(); ++i ) {
            if( self->type->ctorParams[ i ].size() == sz ) {
                pArgWrappers = &self->type->ctorParams[ i ];
                break;
            }
        }
        if( !pArgWrappers ) {
            RaisePyError( "Cannot find constructor" );
            return -1;
        }
        const QArgWrappers& argWrappers = *pArgWrappers;
        /// @todo implement overloading based on the number of arguments
        for( int i = 0; i != sz; ++i ) {
             PyObject* obj = PyTuple_GetItem( args, i );
            ///@todo
            /// if( !self->ctorParams[ i ].CheckType( obj ) ) {
            ///     PyErr_SetString( PyExc_TypeError, "QPy: Wrong type passed to constructor" );
            ///     return -1;   
            /// }
            ga[ i ] = argWrappers[ i ].Arg( reinterpret_cast< PyObject* >( obj ) );
        }
        self->obj = self->type->metaObject->newInstance( ga[ 0 ], ga[ 1 ], ga[ 2 ], ga[ 3 ],
                                                         ga[ 4 ], ga[ 5 ], ga[ 6 ], ga[ 7 ],
                                                         ga[ 8 ], ga[ 9 ] );
    }

    static PyMethodDef md;
    md.ml_name = "__qpy_invoke_method";
    md.ml_meth = reinterpret_cast< PyCFunction >( PyQObjectInvokeMethod );
    md.ml_flags = METH_VARARGS;
    md.ml_doc = "Invoke QObject method";
    self->invoke = PyCFunction_NewEx( &md, reinterpret_cast< PyObject* >( self ),
                                      PyString_FromString( md.ml_name ) );   
    return 0;
}

//----------------------------------------------------------------------------
void PyContext::PyQObjectDealloc( PyQObject* self ) {
    if( !self->foreignOwned ) self->obj->deleteLater();
    Py_XDECREF( self->invoke );
    self->ob_type->tp_free( ( PyObject*) self );
}

//----------------------------------------------------------------------------
PyTypeObject PyContext::CreatePyType( const Type& type ) {
    static PyMemberDef members[] = { { const_cast< char* >( "__qpy_qobject_tag" ), T_BOOL, 
                           offsetof( PyQObject, qobjectTag ), 0, const_cast< char* >( "Identifies object as QPy QObject wrapper" ) },
                           { 0, 0, 0, 0, 0 } };                
    PyTypeObject t = {
        PyObject_HEAD_INIT(NULL)
        0,                         /*ob_size*/
        const_cast< char* >( type.fullClassName.c_str() ),             /*tp_name*/
        sizeof(PyQObject),             /*tp_basicsize*/
        0,                         /*tp_itemsize*/
        (destructor) PyQObjectDealloc, /*tp_dealloc*/
        0,                         /*tp_print*/
        0,                         /*tp_getattr*/
        0,                         /*tp_setattr*/
        0,                         /*tp_compare*/
        0,                         /*tp_repr*/
        0,                         /*tp_as_number*/
        0,                         /*tp_as_sequence*/
        0,                         /*tp_as_mapping*/
        0,                         /*tp_hash */
        0,                         /*tp_call*/
        0,                         /*tp_str*/
        0,                         /*tp_getattro*/
        0,                         /*tp_setattro*/
        0,                         /*tp_as_buffer*/
        Py_TPFLAGS_DEFAULT, /*tp_flags*/
        type.doc.c_str(),           /* tp_doc */
        0,                     /* tp_traverse */
        0,                     /* tp_clear */
        0,                     /* tp_richcompare */
        0,                     /* tp_weaklistoffset */
        0,                     /* tp_iter */
        0,                     /* tp_iternext */
        0,             /* tp_methods */
        members,             /* tp_members */
        const_cast< PyGetSetDef* >( &type.pyMethods[ 0 ] ),                         /* tp_getset */
        0,                         /* tp_base */
        0,                         /* tp_dict */
        0,                         /* tp_descr_get */
        0,                         /* tp_descr_set */
        0,                         /* tp_dictoffset */
        (initproc)PyQObjectInit,      /* tp_init */
        0,                         /* tp_alloc */
        PyQObjectNew,                 /* tp_new */
    };
    return t;
}


}