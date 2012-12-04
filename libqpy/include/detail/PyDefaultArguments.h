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
#include <QVariant>

#include "../PyQArgConstructor.h"
#include "../PyArgConstructor.h"

/// QPy namespace
namespace qpy {


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

}
