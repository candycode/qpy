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
/// @brief Declarations and definitions of PyObject <--> QVariant converters

#include <QVariant>
#include <QString>
#include "../PyQVariantToPyObject.h"
#include "../PyObjectToQVariant.h"

namespace qpy {

struct IntQVariantToPyObject : QVariantToPyObject {
    IntQVariantToPyObject( bool f ) : QVariantToPyObject( f ) {}
    PyObject* Create( const QVariant& v ) const {
        return PyInt_FromLong( v.toInt() );
    }
};
struct DoubleQVariantToPyObject : QVariantToPyObject {
    DoubleQVariantToPyObject( bool f ) : QVariantToPyObject( f ) {}
    PyObject* Create( const QVariant& v ) const {
        return PyFloat_FromDouble( v.toDouble() );
    }
};
struct StringQVariantToPyObject : QVariantToPyObject {
    StringQVariantToPyObject( bool f ) : QVariantToPyObject( f ) {}
    PyObject* Create( const QVariant& v ) const {
        return PyString_FromString( qPrintable( v.toString() ) );
    } 
};


struct IntPyObjectToQVariant : PyObjectToQVariant {
    IntPyObjectToQVariant( bool f ) : PyObjectToQVariant( f ) {}
    QVariant Create( PyObject* obj ) const {
        return QVariant( int( PyInt_AsLong( obj ) ) );
    }
};
struct DoublePyObjectToQVariant : PyObjectToQVariant {
    DoublePyObjectToQVariant( bool f ) : PyObjectToQVariant( f ) {}
    QVariant Create( PyObject* obj ) const {
        return QVariant( PyFloat_AsDouble( obj ) );
    }
};
struct StringPyObjectToQVariant : PyObjectToQVariant {
    StringPyObjectToQVariant( bool f ) : PyObjectToQVariant( f ) {}
    QVariant Create( PyObject* obj ) const {
        return QVariant( PyString_AsString( obj ) );
    }
};

}
