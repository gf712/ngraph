# ******************************************************************************
# Copyright 2017-2018 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ******************************************************************************

#
# OpenMP runtime bundled with mklml cannot run with GLIBC==2.5
# For manylinux1, build the OpenMP runtime from LLVM and use it instead.
#

include(ExternalProject)

set(OMPRT_INSTALL_PREFIX ${EXTERNAL_PROJECTS_ROOT}/omprt)

ExternalProject_Add(
    ext_omprt
    PREFIX omprt
    URL http://prereleases.llvm.org/8.0.0/rc5/openmp-8.0.0rc5.src.tar.xz
    URL_HASH SHA1=006e8734f642d831ce591eab3aa8d20c18e24962
    DOWNLOAD_NO_PROGRESS TRUE
    PATCH_COMMAND git apply ${CMAKE_SOURCE_DIR}/cmake/omprt.patch
    CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_INSTALL_PREFIX=${OMPRT_INSTALL_PREFIX}
            -DLIBOMP_OMPT_SUPPORT=OFF
            -DLIBOMP_LIB_NAME=${CMAKE_SHARED_LIBRARY_PREFIX}iomp5
            -DLIBOMP_INSTALL_ALIASES=OFF
    EXCLUDE_FROM_ALL TRUE
)

ExternalProject_Get_Property(ext_omprt INSTALL_DIR)

ExternalProject_Add_Step(
    ext_omprt
    CopyOMPRT
    COMMAND ${CMAKE_COMMAND} -E copy ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}iomp5${CMAKE_SHARED_LIBRARY_SUFFIX} ${NGRAPH_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_SHARED_LIBRARY_PREFIX}iomp5${CMAKE_SHARED_LIBRARY_SUFFIX}
    COMMENT "Copy OpenMP runtime libraries to ngraph build directory."
    DEPENDEES install
    )
