/* -*-C++-*-
 *****************************************************************************
 *
 * File:         ComMisc.cpp
 * Description:  Miscellaneous global functions
 *
 *
 * Created:      11/07/09
 * Language:     C++
 *
 *
// @@@ START COPYRIGHT @@@
//
// (C) Copyright 2010-2014 Hewlett-Packard Development Company, L.P.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
// @@@ END COPYRIGHT @@@
 *
 *
 *****************************************************************************
 */

#include "Platform.h"

#include "ComOperators.h"
#include "ComASSERT.h"
#include "ComMisc.h"
#include "ComDistribution.h" // enumToLiteral, literalToEnum, literalAndEnumStruct

// define the enum-to-literal function
#define ComDefXLateE2L(E2L,eType,array) void E2L (const eType e, NAString &l) \
{ NABoolean found; char lit[100]; \
  enumToLiteral (array, occurs(array), e, lit, found); \
  ComASSERT(found); l = lit; }

// define the literal-to-enum function
#define ComDefXLateL2E(L2E,eType,array) eType L2E(const char * l) \
{ NABoolean found; \
  eType result = (eType) literalToEnum (array, occurs(array), l, found); \
  ComASSERT(found); \
  return result; }

// Define both
#define ComDefXLateFuncs(L2E,E2L,eType,array) ComDefXLateL2E(L2E,eType,array);ComDefXLateE2L(E2L,eType,array)

// -----------------------------------------------------------------------
// ComUudfParamKind translation
// -----------------------------------------------------------------------


