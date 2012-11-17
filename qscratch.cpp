#include <iostream>
#include <QString>
#include <QMetaObject>
#include <QMetaType>
#include <QObject>
#include <QMetaMethod>

#include "TestObject.h"


int main( int , char** ) {
#if 0 
    qRegisterMetaType< TestObject >( "TestObject" );
    TestObject* to = reinterpret_cast< TestObject* >( QMetaType::construct( QMetaType::type( "TestObject" ), 0 ) );
    std::cout << qPrintable( to->getText() ) << std::endl;
    std::cout << TestObject::staticMetaObject.constructorCount() << std::endl;
    const int methodCount = to->metaObject()->methodCount();
    const QMetaObject* mo = to->metaObject();
    for( int i = 0; i != methodCount; ++i ) {
        QMetaMethod mm = mo->method( i );
        std::cout << mm.signature() << " type: " << mm.methodType() << std::endl;
    }        
#endif

#if 1
    const QMetaObject* mo = &TestObject::staticMetaObject;
    TestObject* to = qobject_cast< TestObject* >( mo->newInstance( Q_ARG( QString, "constructed!" ), Q_ARG( QObject*, 0 )  ) ) ;
    std::cout << qPrintable( to->getText() ) << std::endl;
    std::cout << qPrintable( mo->method(4).signature() ) << std::endl;
#endif
#if 0
    std::cout << "# of constructors: " << mo.constructorCount() << std::endl;
    const QString text( "ciao " );
    QObject* nullObj = 0;
    QObject* pObj = mo.newInstance( Q_ARG( QString, text ),
                                    Q_ARG( QObject*, nullObj ) );
    std::cout << pObj << std::endl; 
    TestObject* to = qobject_cast< TestObject* >( pObj );
    std::cout << qPrintable( to->getText() ) << std::endl;
#endif

    return 0;
}


