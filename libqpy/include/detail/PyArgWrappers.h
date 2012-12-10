#pragma once
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

/// @file
/// @brief Declarations and definitions of argumant wrappers for conversion
/// bewtween C++/Qt  and Python.
///
/// Each wrapper instance wraps a constructor of a specific type matching
/// the argument in a function/method signature.
#include <Python.h>
#include "../PyQArgConstructor.h"
#include "../PyArgConstructor.h"

namespace qpy {
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
    /// @brief return placeholder for storing Qt return argument
    QGenericReturnArgument Arg() const { return ac_->Argument(); }
    /// Type name.
    QString Type() const { 
        if( ac_ != 0 ) return QMetaType::typeName( ac_->Type() );
        else return QString();
    }
    /// Meta type.
    QMetaType::Type MetaType() const { return ac_->Type(); }
    /// Return true if wrapped type is QObject pointer. Required
    /// to have PyContext add QObject wrappers to the Python interpreter.
    /// Note that it is not enough to check the value returned by MetaType()
    /// because custom registered objects derived from QObject do not have
    /// a QMetaType::QObjectStar type. 
    bool IsQObjectPtr() const { return ac_->IsQObjectPtr(); }
    /// Delete PyArgConstructor instance.
    ~PyArgWrapper() { delete ac_; }
private:
    /// PyArgConstructor instance created at construction time.
    PyArgConstructor* ac_;
};
}