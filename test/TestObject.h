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

#include <iostream>

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include <QList>
#include <QVector>

class TestObject : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE TestObject( QObject* parent = 0 ) : QObject( parent ) {}
    Q_INVOKABLE TestObject( const QString& astring, QObject* parent ) : QObject( parent ), text_( astring ) {}
    Q_INVOKABLE TestObject( const TestObject& other ) : text_( other.text_ ) {}
public slots:
    const QString& getText() const { return text_; }
    void method( const QString& msg ) {
        std::cout << msg.toStdString() << std::endl;
    }
    void emitSignal( const QString& msg ) { 
        std::cout << "emitting signal aSignal(" << msg.toStdString() << ")" << std::endl;
        emit aSignal( msg );
    }
    void aSlot( const QString& msg ) { 
        std::cout << "aSlot() called with data: " << msg.toStdString() << std::endl; 
    }
    QString copyString( const QString& s ) { return s; }
    QVariantMap copyVariantMap( const QVariantMap& vm ) { return vm; }
    QVariantList copyVariantList( const QVariantList& vl ) { return vl; }
    QObject* createObject() { 
        TestObject* mo = new TestObject; 
        mo->setObjectName( "New Object" );
        return mo; //WARNING: NOT DESTROYED WHEN GARBAGE COLLECTED 
                   //SINCE DEFAULT IS 'QOBJ_NO_DELETE'
    }
    QList< float > copyFloatList( const QList< float >& l ) { return l; }
    QVector< float > copyFloatVector( const QVector< float >& v ) { return v; }
    QList< short > copyShortList( const QList< short >& l ) { return l; }
    QVector< short > copyShortVector( const QVector< short >& v ) { return v; }
signals:
    void aSignal(const QString&);
private:
    QString text_;
};
