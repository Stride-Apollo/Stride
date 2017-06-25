#############################################################################
#  This file is part of the Stride software.
#  It is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or any
#  later version.
#  The software is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  You should have received a copy of the GNU General Public License,
#  along with the software. If not, see <http://www.gnu.org/licenses/>.
#  see http://www.gnu.org/licenses/.
#
#  Copyright 2016, Willem L, Kuylen E & Broeckhove J
#############################################################################
#
#   Meta makefile calls cmake to do the heavy lifting. It first
#   includes the file MakeLocalConfig (if it exists) for local
#   configuration. If no such file exists the defaults below apply.
#   This file is tracked by the mercurial repository so do not
#   change this for personal customization. That should be done in
#   the file MakeLocalConfig.
#
#############################################################################

#============================================================================
#   Load MakeLocalConfig (if it exists) to override defaults below
#============================================================================
MakeLocalConfig = $(wildcard MakeLocalConfig)
ifeq ($(MakeLocalConfig),MakeLocalConfig)
	include MakeLocalConfig
endif


#============================================================================
#	Determine Visualization install path
#============================================================================
PATH_INSTALL_VIS = build/installed/vis

ifneq ($(CMAKE_INSTALL_PREFIX),)
	PATH_INSTALL_VIS = $(CMAKE_INSTALL_PREFIX)/vis
endif

#============================================================================
# 	CMake command
#============================================================================
ifeq ($(CMAKE),)
	CMAKE = cmake
endif
ifeq ($(CMAKE_GENERATOR),)
	CMAKE_GENERATOR = "Unix Makefiles"
endif

#============================================================================
#   MACRO definitions to pass on to cmake
#============================================================================
ifneq ($(CMAKE_GENERATOR),)
	CMAKE_ARGS += -DCMAKE_GENERATOR=$(CMAKE_GENERATOR)
endif
ifneq ($(CMAKE_BUILD_TYPE),)
	CMAKE_ARGS += -DCMAKE_BUILD_TYPE:STRING=$(CMAKE_BUILD_TYPE)
endif
ifneq ($(CMAKE_INSTALL_PREFIX),)
	CMAKE_ARGS += -DCMAKE_INSTALL_PREFIX:PATH=$(CMAKE_INSTALL_PREFIX)
endif
ifneq ($(CMAKE_C_COMPILER),)
	CMAKE_ARGS += -DCMAKE_C_COMPILER:FILEPATH=$(CMAKE_C_COMPILER)
endif
ifneq ($(CMAKE_CXX_COMPILER),)
	CMAKE_ARGS += -DCMAKE_CXX_COMPILER:FILEPATH=$(CMAKE_CXX_COMPILER)
endif
ifneq ($(CMAKE_CXX_FLAGS),)
	CMAKE_ARGS += -DCMAKE_CXX_FLAGS:STRING=$(CMAKE_CXX_FLAGS)
endif
ifneq ($(CMAKE_CXX_FLAGS_DEBUG),)
	CMAKE_ARGS += -DCMAKE_CXX_FLAGS_DEBUG:STRING=$(CMAKE_CXX_FLAGS_DEBUG)
endif
ifneq ($(STRIDE_BOOST_ROOT),)
	CMAKE_ARGS += -DSTRIDE_BOOST_ROOT:PATH=$(STRIDE_BOOST_ROOT)
endif
ifneq ($(STRIDE_INCLUDE_DOC),)
	CMAKE_ARGS += -DSTRIDE_INCLUDE_DOC:BOOL=$(STRIDE_INCLUDE_DOC)
endif
ifneq ($(STRIDE_FORCE_NO_OPENMP),)
	CMAKE_ARGS += -DSTRIDE_FORCE_NO_OPENMP:BOOL=${STRIDE_FORCE_NO_OPENMP}
endif
ifneq ($(STRIDE_FORCE_NO_HDF5),)
	CMAKE_ARGS += -DSTRIDE_FORCE_NO_HDF5:BOOL=${STRIDE_FORCE_NO_HDF5}
endif
ifneq ($(STRIDE_FORCE_NO_MPI),)
	CMAKE_ARGS += -DSTRIDE_FORCE_NO_MPI:BOOL=${STRIDE_FORCE_NO_MPI}
endif
ifneq ($(STRIDE_VERBOSE_TESTING),)
	CMAKE_ARGS += -DSTRIDE_VERBOSE_TESTING:BOOL=$(STRIDE_VERBOSE_TESTING)
endif
ifneq ($(STRIDE_UNIPAR),)
	CMAKE_ARGS += -DSTRIDE_UNIPAR:BOOL=$(STRIDE_UNIPAR)
endif
ifeq ($(BUILD_DIR),)
	BUILD_DIR = ./build
endif

#============================================================================
#   Targets
#============================================================================
.PHONY: help configure bootstrap all build_all build_main build_test
.PHONY: install install_all install_main install_test package
.PHONY: test installcheck distclean remove_build clean_all
.PHONY: build_vis install_vis

help:
	@ $(CMAKE) -E echo " "
	@ $(CMAKE) -E echo " Read INSTALL.txt in this directory for a brief overview."
	@ $(CMAKE) -E echo "    "
	@ $(CMAKE) -E echo " Current macro values are (cmake will use an appropriate"
	@ $(CMAKE) -E echo " default for any macro that has not been set):"
	@ $(CMAKE) -E echo "   STRIDE_BOOST_ROOT          : " $(STRIDE_BOOST_ROOT)
	@ $(CMAKE) -E echo "   STRIDE_INCLUDE_DOC         : " $(STRIDE_INCLUDE_DOC)
	@ $(CMAKE) -E echo "   STRIDE_UNIPAR              : " $(STRIDE_UNIPAR)
	@ $(CMAKE) -E echo "   STRIDE_FORCE_NO_HDF5       : " $(STRIDE_FORCE_NO_HDF5)
	@ $(CMAKE) -E echo "   STRIDE_VERBOSE_TESTING     : " $(STRIDE_VERBOSE_TESTING)
	@ $(CMAKE) -E echo "   BUILD_DIR                  : " $(BUILD_DIR)
	@ $(CMAKE) -E echo " "
	@ $(CMAKE) -E echo "   CMAKE_GENERATOR            : " $(CMAKE_GENERATOR)
	@ $(CMAKE) -E echo "   CMAKE_C_COMPILER           : " $(CMAKE_C_COMPILER)
	@ $(CMAKE) -E echo "   CMAKE_CXX_COMPILER         : " $(CMAKE_CXX_COMPILER)
	@ $(CMAKE) -E echo "   CMAKE_CXX_FLAGS            : " $(CMAKE_CXX_FLAGS)
	@ $(CMAKE) -E echo "   CMAKE_BUILD_TYPE           : " $(CMAKE_BUILD_TYPE)
	@ $(CMAKE) -E echo "   CMAKE_INSTALL_PREFIX       : " $(CMAKE_INSTALL_PREFIX)
	@ $(CMAKE) -E echo " "

configure:
	$(CMAKE) -E make_directory $(BUILD_DIR)
	$(CMAKE) -E chdir $(BUILD_DIR) $(CMAKE) $(CMAKE_ARGS) ../src

all: configure
	$(MAKE) -j $(PARALLEL_MAKE) -C $(BUILD_DIR) all

install: configure
	$(MAKE) -j $(PARALLEL_MAKE) -C $(BUILD_DIR) --no-print-directory install

install_main: configure
	$(MAKE) -j $(PARALLEL_MAKE) -C $(BUILD_DIR)/main --no-print-directory install

install_test: install_main
	$(MAKE) -j $(PARALLEL_MAKE) -C $(BUILD_DIR)/test --no-print-directory install

distclean clean:
	$(CMAKE) -E remove_directory $(BUILD_DIR)

test: install install_test
	cd $(BUILD_DIR)/installed && ./bin/gtester

build_vis:
	@ cd src/main/electron && npm run-script package


install_vis: build_vis
	@ mkdir -p $(PATH_INSTALL_VIS)
	@ cp -r build/main/electron/vis*/* $(PATH_INSTALL_VIS)/

clean_all: distclean
	git clean -df
	git ls-files --others | egrep '\.c\?make$$' | xargs -r rm
	git ls-files --others | egrep 'CMakeCache' | xargs -r rm
	git ls-files --others | egrep 'Makefile$$' | xargs -r rm
	git ls-files --others | egrep 'CMakeFiles' | xargs -r rm


#############################################################################
