#pragma once
// QPy - Copyright (c) 2012, Ugo Varetto
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the author nor the
//       names of its contributors may be used to endorse or promote products
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

/// @file
/// @brief Python context manager to add QObject derived types and instancess

#include <Python.h>
#include <structmember.h>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <QObject>
#include <QMetaObject>
#include <QMetaMethod>
#include <QList>
#include <QString>
#include <string>
#include <vector>
#include <QDebug>
#include "detail/PyDefaultArguments.h"
#include "detail/PyArgWrappers.h"
#include "detail/PyCallbackDispatcher.h"


#define PY_CHECK( f ) {if( f != 0 ) throw std::runtime_error( "Python error" );}

namespace qpy {

inline void RaisePyError( const char* errMsg = 0, 
                          PyObject* errType = PyExc_Exception  ) {
    PyErr_SetString( errType, errMsg );
}

///@todo 
/// - search for Q_PROPERTY doc, if available use that for __doc__ attr
/// - add Q_PROPERTY support
/// - add QObject instance addition through PyObject_CallObject, passing
///   the pointer to the pre-existing QObject as a "__QObjectPtr" parameter;
///   setting the __QObjectPtr param will be disabled by default through the
///   default PyQObjectSetter function.
/// - if a toString (case insensitive) method is available, use it for __str__

//------------------------------------------------------------------------------
/// @brief Python-Qt context manager. Helper methods for Qt-Python interaction
///
/// This class is the interface exposed by QPy to client code.
/// Use the provided methods to add QObject-derived type, QObject instances and
/// other types the Python context.
/// PyContext is also used internally by other classes to add QObjects returned
/// by methods or received from signals to the Python context.
class PyContext {
    typedef QList< QArgWrapper > QArgWrappers;
    typedef QList< QByteArray > ArgumentTypes;
    /// @brief Stores information used at method invocation time.
    /// 
    /// When a new QObject is added to the Python context a new Method is created
    /// for each callable method (i.e. slot or Q_INVOKABLE) storing the signature
    /// to be used at invocation time and the QMetaMethod to use for the actual
    /// invocation.
    struct Method {
        //for overloading purposes:
        //QMap< int, tuple< QMetaMethod, QArgWrappers, PyArgWrapper> >
        //select the one matching the number of arguments
        QMetaMethod metaMethod_;
        QArgWrappers argumentWrappers_;
        PyArgWrapper returnWrapper_;
        Method( const QMetaMethod& mm, const QArgWrappers& pw, const PyArgWrapper& rw ) :
            metaMethod_( mm ), argumentWrappers_( pw ), returnWrapper_( rw ) {}
    };
    static const int MAX_GENERIC_ARGS = 10;
public:
    typedef QList< Method > Methods;
public:  //must be public because type access might be needed
         //from PyArguments; declaring some PyArguments as friend
         //won't work when new argument constructors provided by
         //client code need to be registered  
    struct Type {
        const QMetaObject* metaObject; 
        //for overloading purposes:
        //QMap< int, QArgWrappers >
        //currently overloading is done through a run-time search
        QList< QArgWrappers > ctorParams;
        Methods methods;
        PyTypeObject pyType;
        //required to keep char* to be passed around
        std::vector< std::string > pyMethodNames;
        std::vector< PyGetSetDef > pyMethods;
        // need string to pass references to
        // contained c_str
        std::string fullClassName;
        std::string className;
        std::string doc;
        PyObject* pyModule;
        PyContext* pyContext;
    };
    typedef QList< Type > Types;   
    struct PyQObject {
        PyObject_HEAD
        char qobjectTag;
        QObject* obj;
        Type* type;
        PyObject* invoke; //method invocation function
        bool foreignOwned;
        PyObject* pyModule;
    };
public:
    /// Constructor: Create @c qpy module with QPy interface.
    PyContext() {
        InitArgFactory();
    }
    PyTypeObject* AddType( const QMetaObject* mo, 
                           PyObject* module,
                           bool checkConstructor = true,
                           const char* className = 0,
                           const char* doc = 0 ) {
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
        // not required construction; should be run-time configurable
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
                                           GeneratePyArgWrapper( mm.typeName() ) ) );
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

        pt->pyType = CreatePyType( *pt );
        PyType_Ready( &pt->pyType );
        pt->pyType.tp_new = PyQObjectNew;
        PyObject* pyPtr = PyCapsule_New( pt, "qpy type info", 0 );
        assert( pyPtr && "NULL pyPtr" );
        assert( pt->pyType.tp_dict && "NULL dict" );

        PY_CHECK( PyDict_SetItemString( pt->pyType.tp_dict, "__qpy_type_info", pyPtr ) );

        if( PyModule_AddObject( module, pt->className.c_str(),
                                reinterpret_cast< PyObject* >( &pt->pyType ) ) != 0 ) {
            types_.pop_back();
            throw std::runtime_error( "Cannot add object to module" );
            return 0;
        }
        return &pt->pyType;
    }
    template < typename T > void Add( PyObject* module ) {
        AddType( &T::staticMetaObject, module );    
    }
    PyMethodDef* ModuleFunctions() {
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
    PyObject* AddObject( QObject* qobj, 
                         PyObject* targetModule, // where instance is added 
                         PyObject* typeModule, // where type is defined
                         const char* instanceName,
                         bool pythonOwned = false ) {
        static const bool DO_NOT_CHECK_CONSTRUCTOR = false;
        PyTypeObject* pt = AddType( qobj->metaObject(), typeModule, DO_NOT_CHECK_CONSTRUCTOR );
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
        
        /*if( !pythonOwned ) { //use an object manager class to avoid deriving PyContext from QObject
            foreignObjDB_[ qobj ] = reinterpret_cast< PyObject* >( obj );
            QObject::connect( qobj, SIGNAL( destroyed( QObject* ) ), this, SLOT( ObjectDestroyedSlot( QObject* ) ) );
        }*/
        return reinterpret_cast< PyObject* >( obj );
    }
    /// Register new types by passing the type of QArgConstructor and PyArgConstructor;
    /// this way of registering does not allow to pass actual instances, and does require
    /// support for operator new and delete.
    template < typename QArgConstructorT, typename PyArgConstructorT > 
    bool RegisterType( const QString& typeName, bool overwrite = false ) {
        return RegisterType( typeName, new QArgConstructorT, new PyArgConstructorT, overwrite );
    }
    template < typename QArgConstructorT, typename PyArgConstructorT > 
    bool RegisterType( QMetaType::Type t, bool overwrite = false ) {
        return RegisterType( QMetaType::typeName( t ), 
                             new QArgConstructorT, new PyArgConstructorT, overwrite );
    }
    /// Register new types by passing the type of QArgConstructor and PyArgConstructor;
    /// this way of registering does not allow to pass actual instances, and does require
    /// support for operator new and delete. Also register the 
    template < typename T, typename QArgConstructorT, typename PyArgConstructorT > 
    bool RegisterType( const QString& typeName, bool overwrite = false ) {
        return RegisterType( qRegisterMetaType< T >( typeName.toAscii().constData() ),
                             new QArgConstructorT, new PyArgConstructorT, overwrite );
    }
    void UnRegisterType( const QString& typeName ) {
        if( !argFactory_.contains( typeName ) ) return;    
        argFactory_.erase( argFactory_.find( typeName ) );
    }
    struct TypeConstruction {
        bool qtToPy;
        bool pyToQt;
        QString typeName;
        operator bool() const { return qtToPy || pyToQt; }
        TypeConstruction() : qtToPy( false ), pyToQt( false ) {}
    };
    TypeConstruction RegTypeInfo( const QString& t ) const {
        TypeConstruction tc;
        if( !argFactory_.contains( t ) ) return tc;
        tc.pyToQt = dynamic_cast< const NoQArgConstructor* >( argFactory_[ t ].QArgCtor() ) == 0;
        tc.qtToPy = dynamic_cast< const NoPyArgConstructor* >( argFactory_[ t ].PyArgCtor() )  == 0;    
        tc.typeName = t;
        return tc;
    }
    QList< TypeConstruction > RegisteredTypes() const {
        QList< TypeConstruction > tc;
        for( ArgFactory::const_iterator i = argFactory_.begin(); i != argFactory_.end(); ++i ) {
            tc.push_back( RegTypeInfo( i.key() ) );
        }
        return tc;
    }
private:

    void InitArgFactory() {
        RegisterType< IntQArgConstructor, IntPyArgConstructor >( QMetaType::Int );
        RegisterType< VoidStarQArgConstructor, NoPyArgConstructor >( QMetaType::VoidStar );
        RegisterType< ObjectStarQArgConstructor, ObjectStarPyArgConstructor >( QMetaType::QObjectStar );
        RegisterType< NoQArgConstructor, VoidPyArgConstructor >( QMetaType::Void );
        RegisterType< StringQArgConstructor, StringPyArgConstructor >( QMetaType::QString );
        RegisterType< FloatQArgConstructor, FloatPyArgConstructor >( QMetaType::Float );
        RegisterType< DoubleQArgConstructor, DoublePyArgConstructor >( QMetaType::Double );
    };
    /// @brief Generate QArgWrapper list from parameter type names as
    /// returned by @c QMetaMethod::parameterTypes().
    QArgWrappers GenerateQArgWrappers( const ArgumentTypes& at ) {
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
    /// @brief Create PyArgWrapper instance from type name.
    PyArgWrapper GeneratePyArgWrapper( QString typeName ) {
        typeName = typeName.isEmpty() ? QMetaType::typeName( QMetaType::Void ) : typeName;
        if( !argFactory_.contains( typeName )  
            || dynamic_cast< const NoPyArgConstructor* >( argFactory_[ typeName ].PyArgCtor() ) ) {
                throw std::logic_error( ( "Type " + typeName + " unknown" ).toStdString() );
            return PyArgWrapper();
        } else {
            return PyArgWrapper( argFactory_[ typeName ].MakePyArgConstructor() );
        }
    }    
private:
    class ArgFactoryEntry {
    public:
        ArgFactoryEntry() : qac_( 0 ), pac_( 0 ) {}
        ArgFactoryEntry( const ArgFactoryEntry& fe ) 
            : typeName_( fe.typeName_ ), qac_( 0 ), pac_( 0 ) {
                if( fe.qac_ ) qac_ = fe.qac_->Clone();
                if( fe.pac_ ) pac_ = fe.pac_->Clone();
        }
        ArgFactoryEntry( const QString& tn, QArgConstructor* qac, PyArgConstructor* pac )
            : typeName_( tn ), qac_( qac ), pac_( pac ) {}
        const ArgFactoryEntry& operator=( const ArgFactoryEntry& fe ) {
            typeName_ = fe.typeName_;
            qac_ = fe.qac_ ? fe.qac_->Clone() : 0;
            pac_ = fe.pac_ ? fe.pac_->Clone() : 0;
            return *this;
        }    
        ~ArgFactoryEntry() {
            delete qac_;
            delete pac_;
        }
        const QArgConstructor* QArgCtor() const { return qac_; }
        const PyArgConstructor* PyArgCtor() const { return pac_; }
        QArgConstructor* MakeQArgConstructor() const { return qac_->Clone(); }
        PyArgConstructor* MakePyArgConstructor() const { return pac_->Clone(); }
        const QString& TypeName() const { return typeName_; }
    private:
        QString typeName_;
        QArgConstructor* qac_;
        PyArgConstructor* pac_;
    };
    typedef QMap< QString, ArgFactoryEntry > ArgFactory;
    bool RegisterType( const QString& typeName, QArgConstructor* qac, PyArgConstructor* pac, bool overwrite ) {
        const bool typeExist = argFactory_.contains( typeName );
        if( typeExist && !overwrite ) return false;
        argFactory_[ typeName ] = ArgFactoryEntry( typeName, qac, pac );
        return true; 
    }
    Type* ExistingType( const QMetaObject* mo, PyObject* module ) {
        for( Types::iterator i = types_.begin(); i != types_.end(); ++i ) {
            if( i->metaObject->className() == mo->className() && 
                i->pyModule == module ) return &( *i );
        }
        return 0;    
    }    
private:
    static PyObject* PyQObjectConnect( PyObject* self, PyObject* args, PyObject* kwargs ) {
        PyObject* sourceObject = 0;
        const char* sourceMethod = 0;
        PyObject* sourceMethodObj = 0;
        PyObject* targetFunction = 0;
        PyQObject* srcQObject = 0;
        PyQObject* pyqobj = 0;
        int mi = -1;
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
            PyArg_ParseTuple( args, "OO", &sourceMethod, &targetFunction );
            pyqobj = getterObject_;
            mi = getterMethodId_;
        } else {
            RaisePyError( "3 or 4 arguments required" );
            return 0;
        }             
        if( mi < 0 ) {
            RaisePyError( ( std::string( "Cannot find method" ) 
                          + std::string( sourceMethod ) ).c_str() );
        }
        QMetaMethod mm = pyqobj->type->metaObject->method( mi );
        QList< QByteArray > params = mm.parameterTypes();
        QList< PyArgWrapper > types;
        for( QList< QByteArray >::const_iterator i = params.begin();
             i != params.end(); ++i ) {
            types.push_back( pyqobj->type->pyContext->GeneratePyArgWrapper( i->constData() ) ); 
        
            pyqobj->type->pyContext->dispatcher_.Connect( pyqobj->obj, mi, types, targetFunction,
                                                          pyqobj->type->pyModule );
        }
           
        Py_RETURN_NONE;
    }
    static PyObject* PyQObjectDisconnect( PyObject* self, PyObject* args, PyObject* kwargs ) {
        PyObject* sourceObject = 0;
        const char* sourceMethod = 0;
        PyObject* targetFunction = 0;
        PyQObject* srcQObject = 0;
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
    }
    static PyObject* PyQObjectIsForeignOwned( PyObject* self, PyObject* args, PyObject* kwargs ) {
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
    static PyObject* PyQObjectIsQObject( PyObject* self, PyObject* args, PyObject* kwargs ) {
        PyObject* obj = 0;
        PyArg_ParseTuple( args, "O", &obj );
        if( PyObject_HasAttrString( obj, "__qpy_qobject_tag" ) ) {
            return PyBool_FromLong( 1 ); 
        } else {
            return PyBool_FromLong( 0 );
        }
    }
    static PyObject* PyQObjectAcquire( PyObject* self, PyObject* args, PyObject* kwargs ) {
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
    static PyObject* PyQObjectRelease( PyObject* self, PyObject* args, PyObject* kwargs ) {
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
    static PyObject* PyQObjectPtr( PyObject* self, PyObject* args, PyObject* kwargs ) {
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
    static PyObject* PyQObjectGetter( PyQObject* qobj, void* closure /*method id*/ ) {
        int mid = int( reinterpret_cast< size_t >( closure ) );
        getterObject_ = qobj;
        getterMethodId_ = mid;
        //should indeed have one function per type (var args, no args...) and return the proper one
        Py_INCREF( qobj->invoke );
        return qobj->invoke;
    }
    static int PyQObjectSetter( PyQObject*, PyObject*, void* closure ) {
        PyErr_SetString( PyExc_TypeError, "QPy methods are readonly!" );
        return -1;
    }
    static PyObject* PyQObjectNew( PyTypeObject* type, PyObject*, PyObject* ) {
        PyQObject* self = reinterpret_cast< PyQObject* >( type->tp_alloc( type, 0 ) );
        if( !self ) return 0;
        self->obj = 0;
        self->foreignOwned = false;
        self->pyModule = 0;
        self->type = reinterpret_cast< Type* >(
            PyCapsule_GetPointer( PyDict_GetItemString( type->tp_dict, "__qpy_type_info" ), "qpy type info" ) );
        return reinterpret_cast< PyObject* >( self );
    }
    static PyObject* PyQObjectInvokeMethod( PyQObject* self, PyObject* args ) {
        std::vector< QGenericArgument > ga( MAX_GENERIC_ARGS );
        const int sz = int( PyTuple_Size( args ) );
        const Method& m = self->type->methods[ getterMethodId_ ];
        for( int i = 0; i != sz; ++i ) {
            PyObject* obj = PyTuple_GetItem( args, i );
            ga[ i ] = m.argumentWrappers_[ i ].Arg( obj );
        }
        try {     
            if( m.returnWrapper_.MetaType() == QMetaType::Void ) {
                m.metaMethod_.invoke( self->obj, Qt::DirectConnection, ga[ 0 ], ga[ 1 ], ga[ 2 ], ga[ 3 ],
                          ga[ 4 ], ga[ 5 ], ga[ 6 ], ga[ 7 ], ga[ 8 ], ga[ 9 ] );
                Py_INCREF(Py_None);
                return Py_None;
            } else {  
                if( m.returnWrapper_.IsQObjectPtr() ) {
                    QObject* ptr = 0;
                    m.metaMethod_.invoke( self->obj, Qt::DirectConnection, Q_RETURN_ARG( QObject*, ptr ),
                          ga[ 0 ], ga[ 1 ], ga[ 2 ], ga[ 3 ],
                          ga[ 4 ], ga[ 5 ], ga[ 6 ], ga[ 7 ], ga[ 8 ], ga[ 9 ] );
                    PyQObject* obj = reinterpret_cast< PyQObject* > (
                                         PyObject_CallObject( reinterpret_cast< PyObject* >( &(self->type->pyType) ), 0 ) );
                    obj->foreignOwned = false;
                    obj->obj = ptr;
                    return reinterpret_cast< PyObject* >( obj );
                } else {
                     m.metaMethod_.invoke( self->obj, Qt::DirectConnection, m.returnWrapper_.Arg(),
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

    static int PyQObjectInit( PyQObject* self, PyObject* args, PyObject* kwds ) {
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

    static void PyQObjectDealloc( PyQObject* self ) {
        if( !self->foreignOwned ) self->obj->deleteLater();
        Py_XDECREF( self->invoke );
        self->ob_type->tp_free( ( PyObject*) self );
    }

private:
    PyTypeObject CreatePyType( const Type& type ) {
        static PyMemberDef members[] = { { const_cast< char* >( "__qpy_qobject_tag" ), T_BOOL, 
                               offsetof( PyQObject, qobjectTag ), 0, const_cast< char* >( "Identifies object as QPy QObject wrapper" ) },
                               { 0 } };
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
private:
    /// @brief QObject-Method database: Each QObject is stored together with the list
    /// of associated method signatures
    Types types_;
    PyCallbackDispatcher dispatcher_;
    ArgFactory argFactory_;
    static PyQObject* getterObject_; //thread_local if needed
    static int getterMethodId_;      //thread_local if needed
};

}
