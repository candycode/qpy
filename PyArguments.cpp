#include "PyArguments.h"
#include "PyContext.h"


namespace qpy {

QGenericArgument ObjectStarQArgConstructor::Create( PyObject* pyobj ) const {
    obj_ = reinterpret_cast< PyContext::PyQObject* >( pyobj )->obj;
    return Q_ARG( QObject*, obj_ );
}    	
	
}
