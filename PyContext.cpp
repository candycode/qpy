#include "PyContext.h"

namespace qpy {
    PyContext::PyQObject* PyContext::getterObject_ = 0;
    int PyContext::getterMethodId_ = -1;
}