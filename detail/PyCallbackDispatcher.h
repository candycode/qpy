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

///@file
///@brief Qt signals to Python callback functions.  
#include <Python.h>
#include <QObject>
#include <QMap>
#include <QList>

namespace qpy {

class PyContext;

typedef QList< PyArgWrapper > CBackParameterTypes;

//------------------------------------------------------------------------------
/// @brief C++ method abstraction: Qt signals are connected to instances of this
/// class which invokes associated Python functions through the @c Invoke method.
///
/// At signal connection time a signal is connected to a dynamically created
/// instance of this class which stores internally a reference to the Python
/// function to invoke. 
class PyCBackMethod {
public:
    /// @brief Constructor
    /// @param pc PyContext
    /// @param p signal signature: This information is used to translate
    ///          the parameter values received from the signal (as an array of void*)
    ///          into Python values
    /// @param pyCBack reference to Python function to invoke
    PyCBackMethod( PyContext* pc, PyObject* pm, const CBackParameterTypes& p, PyObject* pyCBack ) 
        : pc_( pc ), pyModule_( pm ), paramTypes_( p ), pyCBack_( pyCBack ) {}
    /// @brief Called by QObject::qt_metacall as part of a signal-method invocation. 
    ///
    /// Iterates over the list of arguments and parameter types in parallel and
    /// for each argument uses the corresponding parameter wrapper to 
    /// create PyObjects from Qt types.
    /// Values which are of QObject* type are automatically translated to PyQObject
    /// QObjects added to the Python contexts are not owned by Python.
    void Invoke( void **arguments );
    /// Return associated reference to Python function
    PyObject* CBack() const{ return pyCBack_; }
    /// This is required to release the bound function when disconnect is invoked
    /// since methods are never removed from the list to maintain consistency
    /// between the method id and the position in the list itself
    void DeleteCBack() { 
        Py_XDECREF( pyCBack_ );
        pyCBack_ = 0;
    } 
private:
    /// PyContext instance
    PyContext* pc_;
    /// Signature of associated signal
    CBackParameterTypes paramTypes_;
    /// Reference to Python function to invoke
    PyObject* pyCBack_;
    /// Python module
    PyObject* pyModule_;
};


typedef PyObject* PyCBack;
typedef int MethodId;

//------------------------------------------------------------------------------
/// @brief Manages Python function invocation through Qt signals. And connection
/// of Qt signals to Python functions or QObject methods.
///
/// Offers methods to connect Qt signals emitted from QObjects to Python functions
/// or other QObject methods.
/// Whenever a new signal -> Python connection is requested a new proxy method is
/// generated and the signal is routed to the new method which in turn takes
/// care of invoking the Python function.
/// Note that when disconnecting a signal the associated method is not currently
/// removed from the method array because signals are connected to methods through
/// the method's position in the method array, thus removing a method from the array
/// invalidates all the signal to method connections for which the method index
/// is greater than the one of the removed method.
class PyCallbackDispatcher : public QObject {
public:
    /// Standard QObject constructor
    PyCallbackDispatcher( QObject* parent = 0 ) 
        : QObject( parent ), pc_( 0 ) {}
    /// Constructor, bind dispatcher to Lua context
    PyCallbackDispatcher( PyContext* pc, PyObject* pm, QObject* parent = 0 ) 
        : pc_( pc ), QObject( parent ) {}
    /// Overridden method: This is what makes it possible to bind a signal
    /// to a Python function through the index of a proxy method.
    int qt_metacall( QMetaObject::Call c, int id, void **arguments ); 
    /// Connect signal to Python function
    /// @param module Python module; used in case new QObjects need to 
    ///        be added as result of triggered signals
    /// @param obj source QObject
    /// @param signalIdx signal index
    /// @param paramTypes signal signature
    /// @param pyCBack reference to Python target function
    bool Connect( QObject *obj, 
                  int signalIdx,
                  const CBackParameterTypes& paramTypes,
                  PyCBack pyCBack,
                  PyObject* module );
    /// Disconnect signal from Python function
    /// @param obj source QObject
    /// @param signalIdx signal index
    /// @param pyCBack Python function
    bool Disconnect( QObject *obj, 
                     int signalIdx,
                     PyCBack pyCBack );
    /// Set Python context
    void SetPyContext( PyContext* pc ) { pc_ = pc; };
    /// Destructor: Clear method database
    virtual ~PyCallbackDispatcher() {
        for( QMap< int, PyCBackMethod* >::iterator i = pyCBackMethods_.begin();
             i != pyCBackMethods_.end(); ++i ) {
            Py_XDECREF( i.value()->CBack() );
            delete *i;
        }
    }
private:
    int GetMethodIndex() const {
        return methodIdx_++;
        //return int( pyCBackMethods_.size() );
    }    
private:
    /// Python context
    PyContext* pc_;
    /// Methods
    //QList< PyCBackMethod* > pyCBackMethods_;
    QMap< int, PyCBackMethod* > pyCBackMethods_;
    /// Map Lua reference to method index in luaCBackMethods_ list
    QMap< PyCBack, MethodId > cbackToMethodIndex_;

    static int methodIdx_; //thread_local
   
};
}
