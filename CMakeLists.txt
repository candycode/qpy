# QPy - Copyright (c) 2012, Ugo Varetto
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the author nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL UGO VARETTO BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
cmake_minimum_required(VERSION 2.8)

project(qpy)

#Qt
find_package(Qt4 REQUIRED QtCore)
include(${QT_USE_FILE})
#Python
find_package(PythonLibs)

include_directories( ${PYTHON_INCLUDE_DIRS} ${QT_INCLUDES} )

#test app
set( MOC_HEADERS TestObject.h QpyTestObject.h )
QT4_WRAP_CPP( MOC_SRCS ${MOC_HEADERS} )
add_executable( qpy-test qpy-test.cpp ${MOC_SRCS} ${MOC_HEADERS} )
#add_executable( qscratch qscratch.cpp ${MOC_SRCS} ${MOC_HEADERS} )
#add_executable( qpytest qpy-test.cpp ${MOC_SRCS} ${MOC_HEADERS} )
#target_link_libraries( qscratch ${PYTHON_LIBRARIES} ${QT_LIBRARIES} ) 
target_link_libraries( qpy-test ${PYTHON_LIBRARIES} ${QT_LIBRARIES} ) 
