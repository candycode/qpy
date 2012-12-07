# QPy - Copyright (c) 2012,2013 Ugo Varetto
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the author and copyright holder nor the
#       names of contributors to the project may be used to endorse or promote products
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

#look for a tag...
exec_program(
  "git"
  ${CMAKE_CURRENT_SOURCE_DIR}
  ARGS "describe"
  OUTPUT_VARIABLE GIT_TAG )

#...if not found get the SHA1 hash instead
if( ${GIT_TAG} MATCHES "fatal:.*" )
exec_program(
  "git"
  ${CMAKE_CURRENT_SOURCE_DIR}
  ARGS "rev-parse HEAD"
  OUTPUT_VARIABLE GIT_VERSION )
else()
  set( GIT_VERSION ${GIT_TAG} )
endif()
add_definitions( -DQPY_GIT_VERSION="${GIT_VERSION}")

include_directories( ${PYTHON_INCLUDE_DIRS} ${QT_INCLUDES} )

set( HEADERS include/PyContext.h include/PyArgConstructor.h include/PyQArgConstructor.h
     include/PyObjectToQVariant.h include/PyQVariantToPyObject.h )

set( DETAIL_HEADERS include/detail/PyArgWrappers.h include/detail/PyDefaultArguments.h
	 include/detail/PyCallbackDispatcher.h include/detail/PyQVariantDefault.h )

set( SRC src/PyDefaultArguments.cpp src/PyCallbackDispatcher.cpp src/PyContext.cpp )
add_library( qpy ${HEADERS} ${DETAIL_HEADERS} ${SRC} )
target_link_libraries( qpy ${PYTHON_LIBRARIES} ${QT_LIBRARIES} ) 
