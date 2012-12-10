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

/// @brief Signature mapper to translate Qt methods to Python functions; useful
/// mainly for addressing overloading issues.

#include <QString>

class QMetaObject;

namespace qpy {

struct PyMemberNameMapper {
    virtual void Init( const QMetaObject& mo ) const = 0;
    virtual QString property( const QString& name ) const = 0;
    virtual QString signature( const QString& sig ) const = 0;
    virtual const char* propertyDoc( const QString& name ) const = 0;
    virtual const char* methodDoc( const QString& sig ) const = 0;
    virtual ~PyMemberNameMapper() {} 
};


struct DefaultMemberNameMapper : PyMemberNameMapper {
    void Init( const QMetaObject& mo ) const {}
    QString property( const QString& name ) const { return name; }
    QString signature( const QString& sig ) const {
        QString r = sig;
        r.truncate( r.indexOf( "(" ) );   
        return r;
    };
    const char* propertyDoc( const QString& name ) const { return name.toAscii().constData(); }
    const char* methodDoc( const QString& sig ) const { return sig.toAscii().constData(); }
};


}