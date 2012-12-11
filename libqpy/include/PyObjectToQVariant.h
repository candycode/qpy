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
/// @brief Declaration of PyObject to QVariant converter

#include <Python.h>

#include <QVariant>

namespace qpy {

/// @brief Abstract base class for constructing
/// Python to Qt data converters.
///
/// Client code is required to specify if instances
/// are owned by the client code itself or if ownership
/// is to be transferred to the QPy run-time.
struct PyObjectToQVariant {
	/// Constructor
	/// @param fo Set to true if foreign owned, i.e. if it is
	/// destroyed by client code
    PyObjectToQVariant( bool fo ) : foreignOwned_( fo ) {}
    /// Return ownership
    bool ForeignOwned() const { return foreignOwned_; }
    /// Create QVariant from PyObject
    virtual QVariant Create( PyObject* ) const = 0;
    /// Virtual destructor
    virtual ~PyObjectToQVariant() {}
 private:
    bool foreignOwned_;    
};


}