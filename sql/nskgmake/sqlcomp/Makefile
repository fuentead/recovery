#######################################################################
# @@@ START COPYRIGHT @@@
#
# (C) Copyright 1995-2014 Hewlett-Packard Development Company, L.P.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
# @@@ END COPYRIGHT @@@
#######################################################################

CPPSRC := CmpDescribe.cpp \
	CmpMain.cpp \
	CmpSeabaseDDLcommon.cpp \
	CmpSeabaseDDLtable.cpp \
	CmpSeabaseDDLindex.cpp \
	CmpSeabaseDDLview.cpp \
        CmpSeabaseDDLauth.cpp \
	CmpSeabaseDDLroutine.cpp \
	CmpSeabaseDDLupgrade.cpp \
        PrivMgrCommands.cpp \
        PrivMgrMD.cpp \
        PrivMgrMDTable.cpp \
        PrivMgrPrivileges.cpp \
        PrivMgrDesc.cpp \
        PrivMgrComponents.cpp \
        PrivMgrComponentOperations.cpp \
        PrivMgrComponentPrivileges.cpp \
        PrivMgrObjects.cpp \
        PrivMgrRoles.cpp \
	DefaultValidator.cpp \
	nadefaults.cpp \
	NewDel.cpp \
	parser.cpp \
	CmpISPInterface.cpp \
	QCache.cpp \
	vers_libsqlcomp.cpp

DEFS += -DTRAF_SOFTWARE_VERS_MAJOR=$(TRAFODION_VER_MAJOR)  -DTRAF_SOFTWARE_VERS_MINOR=$(TRAFODION_VER_MINOR) -DTRAF_SOFTWARE_VERS_UPDATE=$(TRAFODION_VER_UPDATE)

# Define the correct compilation and linking flags depending on whether
# this is a debug or release build.

ifeq ($(FLAVOR),debug)
   # for coverage checking support
   ifeq ($(SQ_COVERAGE_OPTIMIZER),1)
      CXXFLAGS += --coverage
   endif
   ifeq ($(SQ_COVERAGE_REFRESH),1)
      CXXFLAGS += --coverage
   endif
endif
#DEP_LIBS := tdm_sqlcli
#GUI Debugger
  CXXFLAGS += -DNA_DEBUG_GUI
