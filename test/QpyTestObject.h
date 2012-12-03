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
#include <iostream>
#include <QObject>
class QpyTestObject : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE QpyTestObject() : QObject( 0 ) {}
    Q_INVOKABLE QpyTestObject( int value ) : QObject( 0 ), value_( value ) {}
public slots:
    QString copyString( const QString& s ) { return s; }
    float copyFloat( float f ) { return f; }
    double copyDouble( double d ) { return d; }
    int copyInt( int i ) { return i; }
    int GetValue() const { return value_; }
    void SetValue( int v ) { value_ = v; }
    void SetDefaultValue() { value_ = 0; }
    void Print() const { std::cout << "Value = " << value_ << std::endl; } 
    void catchSignal( int s ) {
        std::cout << "Caught signal " << s << std::endl;
    }
    QObject* Self() { return this; }
    void catchAnotherSignal( QString msg ) { 
        std::cout << "Caught another signal " << msg.toStdString() << std::endl;
    }
signals:
    void aSignal( int );
    void anotherSignal( QString );
private:
    int value_;
};
