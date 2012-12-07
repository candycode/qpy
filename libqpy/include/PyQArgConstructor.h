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
/// @brief Declaration constructors for creating C++ values from PyObjects.

#include <Python.h>
#include <stdexcept>

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
}