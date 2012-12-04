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
#include <QSet>
#include <string>
#include <vector>
#include <QDebug>
#include "detail/PyDefaultArguments.h"
#include "detail/PyArgWrappers.h"
#include "detail/PyCallbackDispatcher.h"
#include "detail/PyQVariantDefault.h"


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
        const QMetaObject* metaObject_;
        Method( const QMetaMethod& mm,
                const QArgWrappers& pw,
                const PyArgWrapper& rw,
                const QMetaObject* mo ) :
            metaMethod_( mm ), argumentWrappers_( pw ),
            returnWrapper_( rw ), metaObject_( mo ) {}
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
        InitQVariantPyObjectMaps();
    }
    ~PyContext() {
        for( QVariantToPyObjectMap::iterator i = qvariantToPyObject_.begin();
             i != qvariantToPyObject_.end(); ++i ) {
            if( !i.value()->ForeignOwned() ) delete i.value();
        }
        for( PyObjectToQVariantMap::iterator i = pyObjectToQVariant_.begin();
             i != pyObjectToQVariant_.end(); ++i ) {
            if( !i.value()->ForeignOwned() ) delete i.value();
        }      
    }
    PyTypeObject* AddType( const QMetaObject* mo, 
                           PyObject* module,
                           bool checkConstructor = true,
                           const QSet< QString >& selectedMembers = QSet< QString >(),
                           const char* className = 0,
                           const char* doc = 0 );
    template < typename T > void Add( PyObject* module ) {
        AddType( &T::staticMetaObject, module );    
    }
    PyMethodDef* ModuleFunctions();
    PyObject* AddObject( QObject* qobj, 
                         PyObject* targetModule, // where instance is added 
                         PyObject* typeModule, // where type is defined
                         const char* instanceName,
                         bool pythonOwned = false,
                         const QSet< QString >& selectedMembers = QSet< QString >() );
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
    void RegisterQVariantToPyObject( QVariant::Type t,  QVariantToPyObject* qp ) {
        if( qvariantToPyObject_.contains( t ) ) {
            if( !qvariantToPyObject_[ t ]->ForeignOwned() ) delete qvariantToPyObject_[ t ];
        }
        qvariantToPyObject_[ t ] = qp;
    }
    template < typename T >
    void RegisterQVariantToPyObject( QVariantToPyObject* qp ) {
        const int id = qRegisterMetaType< T >();
        RegisterQVariantToPyObject( id, qp );
        return id;
    }
    void RegisterPyObjectToQVariant( QVariant::Type t,  PyObjectToQVariant* qp ) {
        if( pyObjectToQVariant_.contains( t ) ) {
            if( !pyObjectToQVariant_[ t ]->ForeignOwned() ) delete pyObjectToQVariant_[ t ];
        }
        pyObjectToQVariant_[ t ] = qp;
    }
    template < typename T >
    int RegisterPyObjectToQVariant( PyObjectToQVariant* qp ) {
        const int id = qRegisterMetaType< T >();
        RegisterPyObjectToQVariant( qMetaTypeId< T >, qp );
        return id;
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
    /// @brief Add default types to context
    void InitArgFactory();
    /// @brief Add QVariant <--> Python converters for default types
    void InitQVariantPyObjectMaps();
    /// @brief Generate QArgWrapper list from parameter type names as
    /// returned by @c QMetaMethod::parameterTypes().
    QArgWrappers GenerateQArgWrappers( const ArgumentTypes& at );
    /// @brief Create PyArgWrapper instance from type name.
    PyArgWrapper GeneratePyArgWrapper( QString typeName );
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
    typedef QMap< QVariant::Type, QVariantToPyObject* > QVariantToPyObjectMap;
    typedef QMap< QVariant::Type, PyObjectToQVariant* > PyObjectToQVariantMap;    
private:
    static PyObject* PyQObjectConnect( PyObject* self, PyObject* args, PyObject* kwargs );
    static PyObject* PyQObjectDisconnect( PyObject* self, PyObject* args, PyObject* kwargs );
    static PyObject* PyQObjectIsForeignOwned( PyObject* self, PyObject* args, PyObject* kwargs );
    static PyObject* PyQObjectIsQObject( PyObject* self, PyObject* args, PyObject* kwargs );
    static PyObject* PyQObjectAcquire( PyObject* self, PyObject* args, PyObject* kwargs );
    static PyObject* PyQObjectRelease( PyObject* self, PyObject* args, PyObject* kwargs );
    static PyObject* PyQObjectPtr( PyObject* self, PyObject* args, PyObject* kwargs );
    static PyObject* PyQObjectGetter( PyQObject* qobj, void* closure /*method id*/ );
    static int PyQObjectSetter( PyQObject*, PyObject*, void* closure );
    static PyObject* PyQObjectNew( PyTypeObject* type, PyObject*, PyObject* );
    static PyObject* PyQObjectInvokeMethod( PyQObject* self, PyObject* args );
    static int PyQObjectInit( PyQObject* self, PyObject* args, PyObject* kwds );
    static void PyQObjectDealloc( PyQObject* self );
private:
    PyTypeObject CreatePyType( const Type& type );
    struct ConnectEntry {
        PyQObject* pyqobj;
        int methodId;
        ConnectEntry() : pyqobj( 0 ), methodId( -1 ) {}
        ConnectEntry( PyQObject* p, int m ) : pyqobj( p ), methodId( m ) {}
    };
    typedef QList< ConnectEntry > ConnectList;
private:
    /// @brief QObject-Method database: Each QObject is stored together with the list
    /// of associated method signatures
    Types types_;
    PyCallbackDispatcher dispatcher_;
    ArgFactory argFactory_;
    QVariantToPyObjectMap qvariantToPyObject_;
    PyObjectToQVariantMap pyObjectToQVariant_;
    static ConnectList endpoints_; //thread_local if needed
    static int getterMethodId_;
    static bool signal_;
};

}
