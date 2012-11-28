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
/// @brief Declarations and definitions of constructors for creating C++ values
/// from PyObjects.

#include <Python.h>
#include <stdexcept>
#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include <QString>
#include <QList>
#include <QGenericArgument>
#include <QGenericReturnArgument>
#include <QVector>

/// QPy namespace
namespace qpy {

//------------------------------------------------------------------------------
/// @brief Interface for constructor objects which generate C++ values from
/// PyObjects. 
///
/// There shall be exactly one and only one constructor per C++ type.
/// The QPy run-time (indirectly) invokes the QArgConstructor::Create() 
/// method whenever the invocation of a method of a QObject derived 
/// class instance is requested from Python code. 
struct QArgConstructor {
    /// Create a QGenericArgument from Python values.
    virtual QGenericArgument Create( PyObject* ) const = 0;
    /// Virtual destructor.
    virtual ~QArgConstructor() {}
    /// Create a new instance of the current class.
    virtual QArgConstructor* Clone() const = 0;

};
/// Useful to specify that Python -> Qt constructor not available
/// when registering new types through PyContext
struct NoQArgConstructor : QArgConstructor {
    /// Create a QGenericArgument from Python values.
    virtual QGenericArgument Create( PyObject* ) const {
        throw std::logic_error( "QArgConstructor not available" );
        return QGenericArgument();
    }
    /// Create a new instance of the current class.
    virtual QArgConstructor* Clone() const { return new NoQArgConstructor( *this ); }

};
typedef NoQArgConstructor NO_QT_ARG;
/// QArgConstructor implementation for @c integer type.
class IntQArgConstructor : public QArgConstructor {
public:
    /// @brief create an @c integer value from a PyObject then create 
    /// QGenericArgument referencing the data member.
    /// @param pyobj pointer to PyObject
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance
    QGenericArgument Create( PyObject* pyobj ) const {
        i_ = PyInt_AsLong( pyobj );
        return Q_ARG( int, i_ );
    }
    /// Make copy through copy constructor.
    IntQArgConstructor* Clone() const {
        return new IntQArgConstructor( *this );
    }
private:
    /// Storage for value read from Python.
    mutable int i_;
};
/// QArgConstructor implementation for @c QString type.
class StringQArgConstructor : public QArgConstructor {
public:
    /// @brief create a @c QString value from a PyObject then create 
    /// QGenericArgument referencing the data member.
    /// @param pyobj pointer to PyObject
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance
    QGenericArgument Create( PyObject* pyobj ) const {
        s_ = PyString_AsString( pyobj );
        return Q_ARG( QString, s_ );
    }
    /// Make copy through copy constructor.
    StringQArgConstructor* Clone() const {
        return new StringQArgConstructor( *this );
    }
private:
    /// Storage for value read from Python.
    mutable QString s_;
};
/// QArgConstructor implementation for @c QString type.
class DoubleQArgConstructor : public QArgConstructor {
public:
    /// @brief create a @c double value from a PyObject then create 
    /// QGenericArgument referencing the data member.
    /// @param pyobj pointer to PyObject
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance
    QGenericArgument Create( PyObject* pyobj ) const {
        d_ = PyFloat_AsDouble( pyobj );
        return Q_ARG( double, d_ );
    }
    /// Make copy through copy constructor.
    DoubleQArgConstructor* Clone() const {
        return new DoubleQArgConstructor( *this );
    }
private:
    /// Storage for value read from Python.
    mutable double d_;
};
/// QArgConstructor implementation for @c QString type.
class FloatQArgConstructor : public QArgConstructor {
public:
    /// @brief create a @c float value from a PyObject then create 
    /// QGenericArgument referencing the data member.
    /// @param pyobj pointer to PyObject
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance
    QGenericArgument Create( PyObject* pyobj ) const {
        f_ = float( PyFloat_AsDouble( pyobj ) );
        return Q_ARG( float, f_ );
    }
    /// Make copy through copy constructor.
    FloatQArgConstructor* Clone() const {
        return new FloatQArgConstructor( *this );
    }
private:
    /// Storage for value read from Python.
    mutable float f_;
};
/// QArgConstructor implementation for @c integer type.
class VoidStarQArgConstructor : public QArgConstructor {
public:
    /// @brief create an @c integer value from a PyObject then create 
    /// QGenericArgument referencing the data member.
    /// @param pyobj pointer to PyObject
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance
    QGenericArgument Create( PyObject* pyobj ) const {
        i_ = PyLong_AsVoidPtr( pyobj );
        return Q_ARG( void*, i_ );
    }
    /// Make copy through copy constructor.
    VoidStarQArgConstructor* Clone() const {
        return new VoidStarQArgConstructor( *this );
    }
private:
    /// Storage for value read from Python.
    mutable void* i_;
};
/// QArgConstructor implementation for @c integer type.
class ObjectStarQArgConstructor : public QArgConstructor {
public:
    /// @brief create an @c integer value from a PyObject then create 
    /// QGenericArgument referencing the data member.
    /// @param pyobj pointer to PyObject
    /// @return QGenericArgument instance whose @c data field points
    ///         to a private data member of this class' instance
    QGenericArgument Create( PyObject* pyobj ) const;

    /// Make copy through copy constructor.
    ObjectStarQArgConstructor* Clone() const {
        return new ObjectStarQArgConstructor( *this );
    }
private:
    /// Storage for value read from Python.
    mutable QObject* obj_;
};

//------------------------------------------------------------------------------
/// @brief Abstract base class for return constructors which create PyObjects
/// from C++ values. 
class PyArgConstructor {
public:
    /// @brief Create PyObject from value returned from QObject method.
    virtual PyObject* Create() const = 0;
    /// @brief Create PyObject from parameter passed to Python callback
    /// when signal triggered
    virtual PyObject* Create( void* ) const = 0;
    /// Virtual destructor.
    virtual ~PyArgConstructor() {}
    /// Return copy of object.
    virtual PyArgConstructor* Clone() const = 0;
    /// Return type of constructed data.
    virtual QMetaType::Type Type() const = 0;
    /// @brief Return QGenericReturnArguments holding a reference to the
    /// memory location where the returned value is stored.
    QGenericReturnArgument Argument() const { return ga_; }
    /// @brief Return @c true if type is a pointer to a QObject-derived object.
    ///
    /// This is required to have the QPy run-time add the passed QObject into
    /// the Python context. The other option is to have PyArgConstructors::Create
    /// receive a reference to a PyContext which introduces a two-way
    /// dependency between PyArgConstructor and PyContext.
    virtual bool IsQObjectPtr() const { return false; }
protected:
    /// Creates return argument of the proper type.
    template < typename T > void SetArg( T& arg ) {
        ga_ = QReturnArgument< T >( QMetaType::typeName( Type() ), arg );
    }
private:    
    /// Placeholder for returned data. 
    QGenericReturnArgument ga_; // not private, breaks encapsulation
};
/// Use this type to specify that no Qt -> Python conversion exist for
/// a type when registering types through PyContext
class NoPyArgConstructor : public PyArgConstructor {
public:
    PyObject* Create( void*  ) const {
        Py_RETURN_NONE;
    }
    PyObject* Create() const {
        Py_RETURN_NONE;
    }
    NoPyArgConstructor* Clone() const {
        return new NoPyArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Void; }
};
typedef NoPyArgConstructor NO_PY_ARG;
/// PyArgConstructor implementation for @c integer type
class IntPyArgConstructor : public PyArgConstructor {
public:
    IntPyArgConstructor() {
        SetArg( i_ );
    }
    IntPyArgConstructor( const IntPyArgConstructor& other ) : i_( other.i_ ) {
        SetArg( i_ );
    }
    PyObject* Create( void* p ) const {
        int i = *reinterpret_cast< int* >( p );
        return PyInt_FromLong( i );
    }
    PyObject* Create() const {
        return PyInt_FromLong( i_ );
    }
    IntPyArgConstructor* Clone() const {
        return new IntPyArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Int; }
private:
    int i_; 
};
/// PyArgConstructor implementation for @c QString type
class StringPyArgConstructor : public PyArgConstructor {
public:
    StringPyArgConstructor() {
        SetArg( s_ );
    }
    StringPyArgConstructor( const StringPyArgConstructor& other ) : s_( other.s_ ) {
        SetArg( s_ );
    }
    PyObject* Create( void* p ) const {
        QString s = *reinterpret_cast< QString* >( p );
        return PyString_FromString( s.toAscii().constData() );
    }
    PyObject* Create() const {
        return PyString_FromString( s_.toAscii().constData() );
    }
    StringPyArgConstructor* Clone() const {
        return new StringPyArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::QString; }
private:
    QString s_; 
};
/// PyArgConstructor implementation for @c double type
class DoublePyArgConstructor : public PyArgConstructor {
public:
    DoublePyArgConstructor() {
        SetArg( d_ );
    }
    DoublePyArgConstructor( const DoublePyArgConstructor& other ) : d_( other.d_ ) {
        SetArg( d_ );
    }
    PyObject* Create( void* p ) const {
        double d = *reinterpret_cast< double* >( p );
        return PyFloat_FromDouble( d );
    }
    PyObject* Create() const {
        return PyFloat_FromDouble( d_ );
    }
    DoublePyArgConstructor* Clone() const {
        return new DoublePyArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Double; }
private:
    double d_; 
};
/// PyArgConstructor implementation for @c float type
class FloatPyArgConstructor : public PyArgConstructor {
public:
    FloatPyArgConstructor() {
        SetArg( f_ );
    }
    FloatPyArgConstructor( const FloatPyArgConstructor& other ) : f_( other.f_ ) {
        SetArg( f_ );
    }
    PyObject* Create( void* p ) const {
        float f = *reinterpret_cast< float* >( p );
        return PyFloat_FromDouble( f );
    }
    PyObject* Create() const {
        return PyFloat_FromDouble( f_ );
    }
    FloatPyArgConstructor* Clone() const {
        return new FloatPyArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Float; }
private:
    float f_; 
};

/// PyArgConstructor implementation for @c void type
class VoidPyArgConstructor : public PyArgConstructor {
public:
    PyObject* Create() const { 
        Py_RETURN_NONE;
    } 
    PyObject* Create( void* ) const { return 0; }
    VoidPyArgConstructor* Clone() const {
        return new VoidPyArgConstructor( *this );
    }
    QMetaType::Type Type() const { return QMetaType::Void; }
};

/// PyArgConstructor implementation for @c QObject* type
class ObjectStarPyArgConstructor : public PyArgConstructor {
public:
    ObjectStarPyArgConstructor() {
        SetArg( obj_ );   
    }
    ObjectStarPyArgConstructor( const ObjectStarPyArgConstructor& other ) : obj_( other.obj_ ) {
        SetArg( obj_ );   
    }
    PyObject* Create() const {
        Py_RETURN_NONE;
    }
    PyObject* Create( void* ) const {
        Py_RETURN_NONE;
    }
    ObjectStarPyArgConstructor* Clone() const {
        return new ObjectStarPyArgConstructor( *this );
    }
    bool IsQObjectPtr() const { return true; }
    QMetaType::Type Type() const { return QMetaType::QObjectStar; }
private:
    QObject* obj_;
};



//------------------------------------------------------------------------------
/// @brief Wrapper for parameters in a QObject method invocation.
///
/// Whenever a new QObject is added to the Python context, the signature of each
/// method is translated to an index and a list of QArgWrapper objects 
/// stored inside a PyContext instance.
/// At invocation time the proper method is invoked through a call to
/// @c QMetaMethod::invoke passing the arguments returned by the QArgWrapper::Arg
/// method invoked on each parameter in the argument list.
/// QArgWrapper stores an instance of QArgConstructor used to create a
/// QGenericArgument from PyObjects.
class QArgWrapper {
public:
    /// @brief Default constructor.
    QArgWrapper( QArgConstructor* ac = 0 ) : ac_( ac ) {

    }
    /// Copy constructor: Clone QArgConstructor instance.
    QArgWrapper( const QArgWrapper& other ) : ac_( 0 ) {
        if( other.ac_ ) ac_ = other.ac_->Clone();
    }
    /// @brief Return QGenericArgument instance created from PyObjects
    ///
    /// Internally it calls QArgConstructor::Create to generate QGenericArguments from
    /// a PyObject pointer.
    QGenericArgument Arg( PyObject* pobj ) const {
        return ac_ ? ac_->Create( pobj ) : QGenericArgument();
    }
    /// @brief Destructor; delete QArgConstructor instance.
    ~QArgWrapper() { delete ac_; }
private:
    /// Instance of QArgConstructor created from type information at construction
    /// time.
    QArgConstructor* ac_;    
};


/// @brief Wrapper for objects returned from QObject method invocations or passed
/// to Python callbacks in response to emitted signals.
///
/// This class translates C++ values to PyObjects and is used to both return
/// values from method invocations and translate the parameters received from
/// a signal to PyObject values whenever a Python callback invocation is triggered by
/// an emitted signal.
class PyArgWrapper {
public:
    ///@brief Default constructor.
    PyArgWrapper( PyArgConstructor* pac = 0 ) : ac_( pac ) {}
    ///@brief Copy constructor: Clones the internal Return constructor instance.
    PyArgWrapper( const PyArgWrapper& other ) : ac_( 0 ) {
        if( other.ac_ ) ac_ = other.ac_->Clone();
    }
    /// @brief return values stored in the inner PyArgConstructor.
    ///
    /// This is the method invoked to return values from a QObject method invocation.
    PyObject* Create() const {
        return ac_->Create();
    }
    /// @brief return value converted from void* .
    ///
    /// This is the method invoked when a Python function is called as the result
    /// of a triggered signal.
    PyObject* Create( void* p ) const {
        return ac_->Create( p );
    }
    /// @brief retunr placeholder for storing Qt return argument
    QGenericReturnArgument Arg() const { return ac_->Argument(); }
    /// Type name.
    QString Type() const { 
        if( ac_ != 0 ) return QMetaType::typeName( ac_->Type() );
        else return QString();
    }
    /// Meta type.
    QMetaType::Type MetaType() const { return ac_->Type(); }
    /// Return true if wrapped type is QObject pointer.
    bool IsQObjectPtr() const { return ac_->IsQObjectPtr(); }
    /// Delete LArgConstructor instance.
    ~PyArgWrapper() { delete ac_; }
private:
    /// PyArgConstructor instance created at construction time.
    PyArgConstructor* ac_;
};

}
