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

#include <Python.h>

#include "../include/PyContext.h"
#include "../include/detail/PyCallbackDispatcher.h"

namespace qpy {

int PyCallbackDispatcher::methodIdx_ = 0;

//------------------------------------------------------------------------------
bool PyCallbackDispatcher::Connect( QObject *obj, 
                                    int signalIdx,
                                    const CBackParameterTypes& paramTypes,
                                    PyCBack pyCBack,
                                    PyObject* module ) {
    // check if Python function reference already stored in database;
    // if not create a new 'dynamic method' and map function reference
    // to the newly created method
    int methodIdx = cbackToMethodIndex_.value( pyCBack, -1 );
    if( methodIdx < 0 ) {
        methodIdx = GetMethodIndex();
        cbackToMethodIndex_[ pyCBack ] = methodIdx;
        Py_INCREF( pyCBack );
        pyCBackMethods_[ methodIdx ] = 
            new PyCBackMethod( pc_, module, paramTypes, pyCBack );
}
    // connect signal to method in method array
    return QMetaObject::connect( obj, signalIdx, this, methodIdx + metaObject()->methodCount() );
}
//------------------------------------------------------------------------------
bool PyCallbackDispatcher::Disconnect( QObject *obj, 
                                       int signalIdx,
                                       PyCBack pyCBack ) {
    // iterate over callback methods, each method is associated with
    // one and only one Python function
    int m = 0;
    for( QMap< int, PyCBackMethod* >::iterator i = pyCBackMethods_.begin();
          i != pyCBackMethods_.end(); ++i, ++m ) {
        bool found = false;
        // since we are not actually removing elements from the list but simoly calling
        // PyCBackMethod::DeleteCBack which deletes cback bethod data and sets the cback to null
        // we need to explicitly check if a callback is null
        // NOT REQUIRED ANYMORE SINCE FIXING #24
        if( PyMethod_Check( pyCBack ) ) {
            found = PyMethod_Function( i.value()->CBack() ) == PyMethod_Function( pyCBack );
        } else {
            found = pyCBack == i.value()->CBack();
        } 
        if( found ) {
            Py_XDECREF( i.value()->CBack() );
            i.value()->DeleteCBack();
            pyCBackMethods_.erase( i );
            return QMetaObject::disconnect( obj, signalIdx, this, m + metaObject()->methodCount() ); 
         }
    }   
    return false;
}
//------------------------------------------------------------------------------
int PyCallbackDispatcher::qt_metacall( QMetaObject::Call invoke, MethodId methodIndex, void **arguments ) {
    methodIndex = QObject::qt_metacall( invoke, methodIndex, arguments );
    if( !pyCBackMethods_.contains( methodIndex ) ) return -1;
    if( methodIndex < 0 || invoke != QMetaObject::InvokeMetaMethod ) return methodIndex;
    pyCBackMethods_[ methodIndex ]->Invoke( arguments );
    return -1;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PyCBackMethod::Invoke( void **arguments ) {
    ++arguments; // first parameter is placeholder for return argument! - ignore
    //iterate over arguments and push values on Lua stack
    PyObject* tuple = PyTuple_New( paramTypes_.size() );
    int t = 0;
    for( CBackParameterTypes::const_iterator i = paramTypes_.begin();
         i != paramTypes_.end(); ++i, ++arguments, ++t ) {
        PyObject* obj = 0;
        if( i->IsQObjectPtr() ) {
           obj = pc_->AddObject( reinterpret_cast< QObject* >( *arguments ), pyModule_, pyModule_, 0 );
        } else {
           obj = i->Create( *arguments ); 
        }
        PyTuple_SetItem( tuple, t, obj ); 
    }
    //call Python function
    
    PyObject_CallObject( pyCBack_, tuple );
    Py_DECREF( tuple );
}

}

// Pre-defined types in Qt meta-type environment:
//enum Type {
//        // these are merged with QVariant
//        Void = 0, Bool = 1, Int = 2, UInt = 3, LongLong = 4, ULongLong = 5,
//        Double = 6, QChar = 7, QVariantMap = 8, QVariantList = 9,
//        QString = 10, QStringList = 11, QByteArray = 12,
//        QBitArray = 13, QDate = 14, QTime = 15, QDateTime = 16, QUrl = 17,
//        QLocale = 18, QRect = 19, QRectF = 20, QSize = 21, QSizeF = 22,
//        QLine = 23, QLineF = 24, QPoint = 25, QPointF = 26, QRegExp = 27,
//        QVariantHash = 28, QEasingCurve = 29, LastCoreType = QEasingCurve,
//
//        FirstGuiType = 63 /* QColorGroup */,
//#ifdef QT3_SUPPORT
//        QColorGroup = 63,
//#endif
//        QFont = 64, QPixmap = 65, QBrush = 66, QColor = 67, QPalette = 68,
//        QIcon = 69, QImage = 70, QPolygon = 71, QRegion = 72, QBitmap = 73,
//        QCursor = 74, QSizePolicy = 75, QKeySequence = 76, QPen = 77,
//        QTextLength = 78, QTextFormat = 79, QMatrix = 80, QTransform = 81,
//        QMatrix4x4 = 82, QVector2D = 83, QVector3D = 84, QVector4D = 85,
//        QQuaternion = 86,
//        LastGuiType = QQuaternion,
//
//        FirstCoreExtType = 128 /* VoidStar */,
//        VoidStar = 128, Long = 129, Short = 130, Char = 131, ULong = 132,
//        UShort = 133, UChar = 134, Float = 135, QObjectStar = 136, QWidgetStar = 137,
//        QVariant = 138,
//        LastCoreExtType = QVariant,
//
//// This logic must match the one in qglobal.h
//#if defined(QT_COORD_TYPE)
//        QReal = 0,
//#elif defined(QT_NO_FPU) || defined(QT_ARCH_ARM) || defined(QT_ARCH_WINDOWSCE) || defined(QT_ARCH_SYMBIAN)
//        QReal = Float,
//#else
//        QReal = Double,
//#endif
//
//        User = 256
