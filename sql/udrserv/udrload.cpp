/**********************************************************************
// @@@ START COPYRIGHT @@@
//
// (C) Copyright 2001-2014 Hewlett-Packard Development Company, L.P.
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
**********************************************************************/
/* -*-c++-*-
*****************************************************************************
*
* file:         udrload.cpp
* description:  this is the module that processes udr load messages.
*               the tasks for this process are to :
*               . extract sp descriptive attributes from message
*               . extract sp parameter attributes from message
*               . build SPInfo data structure and populate with attributes
*               . return sp id (creation timestamp) to client
*               . handle errors returned from lm.
*               . deal with resource allocation problems of SPInfo data structures
*
* created:      01/01/2001
* language:     c++
*
*
*****************************************************************************
*/

#include "udrextrn.h"
#include "spinfo.h"
#include "udrdefs.h"
#include "udrutil.h"
#include "ComDiags.h"
#include "ComSmallDefs.h"
#include "UdrStreams.h"
#include "UdrExeIpc.h"
#include "LmRoutineC.h"
#include "udrdecs.h"
#include "spinfoCallback.h"

class UdrServerReplyStream;

SPInfo * processLoadParameters(UdrGlobals *UdrGlob,
                               UdrLoadMsg &request,
                               ComDiagsArea &d);
void displayLoadParameters(UdrLoadMsg &request);
void reportLoadResults(UdrGlobals *UdrGlob, SPInfo *sp, LmRoutine *lmr_);
void copyRoutineOptionalData(UdrLoadMsg &request, SPInfo *sp);

void processALoadMessage(UdrGlobals *UdrGlob,
                         UdrServerReplyStream &msgStream,
                         UdrLoadMsg &request,
                         IpcEnvironment &env)
{
  const char *moduleName = "processALoadMessage";
  char errorText[MAXERRTEXT];

  ComDiagsArea *diags = ComDiagsArea::allocate(UdrGlob->getIpcHeap());

  doMessageBox(UdrGlob, TRACE_SHOW_DIALOGS,
               UdrGlob->showLoad_, moduleName);

  NABoolean showLoadLogic = (UdrGlob->verbose_ &&
                             UdrGlob->traceLevel_ >= TRACE_IPMS &&
                             UdrGlob->showLoad_);

  if (showLoadLogic)
  {
    ServerDebug("[UdrServ (%s)] Processing load request", moduleName);
  }

  // UDR_LOAD message always comes with transaction and they are out
  // side Enter Tx and Exit Tx pair. Make sure we are under correct
  // transaction.
  msgStream.activateCurrentMsgTransaction();

  //
  // Check to see if the incoming UDR handle has already been seen
  //
  NABoolean handleAlreadyExists = FALSE;
  SPInfo *sp = UdrGlob->getSPList()->spFind(request.getHandle());
  if (sp)
  {
    handleAlreadyExists = TRUE;
    if (showLoadLogic)
    {
      ServerDebug("    Duplicate handle arrived");
      ServerDebug("    SPInfoState is %s", sp->getSPInfoStateString());
    }

    if (sp->getSPInfoState() != SPInfo::UNLOADING)
    {
      //
      // An SPInfo exists but it is not one of the token instances to
      // represent an out-of-sequence LOAD/UNLOAD. This is an internal
      // error. Something has been botched in the message protocol.
      //
      char buf[100];
      convertInt64ToAscii(request.getHandle(), buf);
      *diags << DgSqlCode(-UDR_ERR_DUPLICATE_LOADS);
      *diags << DgString0(buf);
    }
    else
    {
      // The LOAD/UNLOAD requests for this handle arrived
      // out-of-sequence. Nothing to do at this point. An empty reply
      // will be generated later in this function.
    }
  }

  if (!handleAlreadyExists)
  {
    if (!UdrHandleIsValid(request.getHandle()))
    {
      *diags << DgSqlCode(-UDR_ERR_MISSING_UDRHANDLE);
      *diags << DgString0("Load Message");
    }
    else
    {
      //
      // First process the metadata in the LOAD requests and then
      // contact Language Manager to load the SP.
      //
      sp = processLoadParameters(UdrGlob, request, *diags);
      
      if (showLoadLogic)
      {
        ServerDebug("[UdrServ (%s)]  About to call LM::getRoutine",
                    moduleName);
      }
      
      if (sp == NULL)
      {
        *diags << DgSqlCode(-UDR_ERR_UNABLE_TO_ALLOCATE_MEMORY);
        *diags << DgString0("SPInfo");
      }
      else
      {
        UdrGlob->setCurrSP(sp);
        LmRoutine *lmRoutine;
        LmResult lmResult;
        LmLanguageManager *lm =
          UdrGlob->getOrCreateLM(lmResult, sp->getLanguage(), diags);
        
        if (lm)
        {
          lmResult = lm->getRoutine(sp->getNumParameters(),
                                    sp->getLmParameters(),
                                    sp->getNumTables(),
                                    sp->getLmTables(),
                                    sp->getReturnValue(),
				    sp->getParamStyle(),
				    sp->getTransactionAttrs(),
                                    sp->getSQLAccessMode(),
                                    sp->getParentQid(),
                                    sp->getRequestRowSize(),
                                    sp->getReplyRowSize(),
                                    sp->getSqlName(),
                                    sp->getExternalName(),
                                    sp->getRoutineSig(),
                                    sp->getContainerName(),
                                    sp->getExternalPathName(),
                                    sp->getLibrarySqlName(),
		                    UdrGlob->getCurrentUserName(),
	                            UdrGlob->getSessionUserName(),
	                            sp->getExternalSecurity(),
	                            sp->getRoutineOwnerId(),
                                    &lmRoutine,
                                    (LmHandle)&SpInfoGetNextRow,
                                    (LmHandle)&SpInfoEmitRow,
                                    sp->getMaxNumResultSets(),
                                    diags);
        }
        
        if (lmResult == LM_OK)
        {
          if (lmRoutine == NULL)
          {
            *diags << DgSqlCode(-UDR_ERR_MISSING_LMROUTINE);
            *diags << DgString0("error: returned a null LM handle");
            *diags << DgInt1((Int32)0);
          }
          else
          {
            sp->setLMHandle(lmRoutine);

	    // Retrieve any optional data from UdrLoadMsg.
	    copyRoutineOptionalData(request, sp);

            reportLoadResults(UdrGlob, sp, lmRoutine);
            sp->setSPInfoState(SPInfo::LOADED);
          }

        } // lmResult == LM_OK

        if (showLoadLogic)
        {
          if (lmResult == LM_OK)
          {
            sprintf(errorText,
                    "[UdrServ (%.30s)]  LM::getRoutine was successful.",
                    moduleName);
          }
          else
          {
            sprintf(errorText,
                    "[UdrServ (%.30s)]  LM::getRoutine resulted in error.",
                    moduleName);
          }
          ServerDebug(errorText);
          doMessageBox(UdrGlob, TRACE_SHOW_DIALOGS,
                       UdrGlob->showMain_, errorText);
        }

        if (sp && !(sp->isLoaded()))
        {
          sp->setSPInfoState(SPInfo::LOAD_FAILED);
        }

      } // if (sp == NULL) else ...
    } // if (handle is not valid) else ...
  } // !handleAlreadyExists

  // build a reply and send it
  msgStream.clearAllObjects();

  UdrLoadReply *reply = new (UdrGlob->getIpcHeap())
    UdrLoadReply(UdrGlob->getIpcHeap());

  if (reply == NULL)
  {  // no reply buffer build...
    controlErrorReply(UdrGlob, msgStream, UDR_ERR_MESSAGE_PROCESSING,
                      INVOKE_ERR_NO_REPLY_BUFFER, NULL);
    return;
  }

  // Only return a valid UDR Handle if diagnostics are not present and
  // no LM errors occurred. We also return a valid handle if this LOAD
  // arrived out-of-sequence after the UNLOAD and no diagnostics have
  // been generated yet.
  if (diags && diags->getNumber() > 0)
  {
    reply->setHandle(INVALID_UDR_HANDLE);
  }
  else if (sp)
  {
    if (sp->isLoaded() || handleAlreadyExists)
    {
      reply->setHandle(sp->getUdrHandle());
    }
    else
    {
      reply->setHandle(INVALID_UDR_HANDLE);
    }
  }

  msgStream << *reply;

  if (diags && diags->getNumber() > 0)
  {
    msgStream << *diags;
    UdrGlob->numErrUDR_++;
    UdrGlob->numErrSP_++;
    UdrGlob->numErrLoadSP_++;
    if (showLoadLogic)
      dumpDiagnostics(diags, 2);
  }

  if (showLoadLogic)
  {
    ServerDebug("[UdrServ (%s)] About to send LOAD reply", moduleName);
  }

#ifdef NA_DEBUG_C_RUNTIME
  if (UdrGlob && UdrGlob->getJavaLM())
  {
    sleepIfPropertySet(*(UdrGlob->getJavaLM()),
                       "MXUDR_LOAD_DELAY", diags);
  }
#endif // NA_DEBUG_C_RUNTIME

  sendControlReply(UdrGlob, msgStream, sp);

  if (diags)
  {
    diags->decrRefCount();
  }

  reply->decrRefCount();

} // processALoadMessage

SPInfo *processLoadParameters(UdrGlobals *UdrGlob,
                              UdrLoadMsg &request,
                              ComDiagsArea &d)
{
  const char *moduleName = "processLoadParameters";
  Lng32 oldDiags = 0;
  Lng32 newDiags = 0;
  
  doMessageBox(UdrGlob, TRACE_SHOW_DIALOGS,
               UdrGlob->showLoad_, moduleName);
  // check for test mode processing...
  
  SPInfo *sp = NULL;
  
  // process message parameters...
  
  if (UdrGlob->traceLevel_ >= TRACE_DETAILS &&
      UdrGlob->showLoad_)
  {
    displayLoadParameters(request);
  }
  
  doMessageBox(UdrGlob, TRACE_SHOW_DIALOGS,
               UdrGlob->showLoad_, "create SPInfo");
  
  oldDiags = d.getNumber();

  sp = new (UdrGlob->getUdrHeap()) SPInfo(UdrGlob, UdrGlob->getUdrHeap(),
                                          request.getHandle(),
                                          (char *)request.getSqlName(),
                                          (char *)request.getRoutineName(),
                                          (char *)request.getSignature(),
                                          (char *)request.getContainerName(),
                                          (char *)request.getExternalPath(),
                                          (char *)request.getLibrarySqlName(),
                                          request.getNumParameters(),
                                          request.getNumInValues(),
                                          request.getNumOutValues(),
                                          request.getMaxResultSets(),
                                          request.getTransactionAttrs(),
                                          request.getSqlAccessMode(),
                                          request.getLanguage(),
                                          request.getParamStyle(),
                                          request.isIsolate(),
                                          request.isCallOnNull(),
                                          request.isExtraCall(),
                                          request.isDeterministic(),
                                          request.getExternalSecurity(),
                                          request.getRoutineOwnerId(),
                                          request.getInBufferSize(),
                                          request.getOutBufferSize(),
                                          request.getInputRowSize(),
                                          request.getOutputRowSize(),
                                          /* ComDiagsArea */ d,
                                          (char *)request.getParentQid());
  if (sp == NULL)
  {  // memory problem
    UdrGlob->getSPList()->releaseOldestSPJ(/* ComDiagsArea */ d);
    sp = new (UdrGlob->getUdrHeap())
      SPInfo(UdrGlob, UdrGlob->getUdrHeap(),
             request.getHandle(),
             (char *)request.getSqlName(),
             (char *)request.getRoutineName(),
             (char *)request.getSignature(),
             (char *)request.getContainerName(),
             (char *)request.getExternalPath(),
             (char *)request.getLibrarySqlName(),
             request.getNumParameters(),
             request.getNumInValues(),
             request.getNumOutValues(),
             request.getMaxResultSets(),
             request.getTransactionAttrs(),
             request.getSqlAccessMode(),
	     request.getLanguage(),
	     request.getParamStyle(),
             request.isIsolate(),
             request.isCallOnNull(),
             request.isExtraCall(),
             request.isDeterministic(),
             request.getExternalSecurity(),
             request.getRoutineOwnerId(),
             request.getInBufferSize(),
             request.getOutBufferSize(),
             request.getInputRowSize(),
             request.getOutputRowSize(),
             /* ComDiagsArea */ d,
             (char *)request.getParentQid());
    if (sp == NULL)
    {
      return NULL;
    }
  }
  
  newDiags = d.getNumber();
  
  if (oldDiags != newDiags)
  {
    // diagnostics generated in ctor for SPInfo
    // something bad has happened. Bail out.
    delete sp;
    return NULL;
  }
  
  // Process IN/INOUT parameters
  ComUInt32 i;
  for (i = 0; i < sp->getNumInParameters(); i++)
  {
    UdrParameterInfo &pi = (UdrParameterInfo &)request.getInParam(i);

    // Copy information from the UdrParameterInfo into the spinfo. The
    // spinfo will initialize an LmParameter instance for this
    // parameter.
    sp->setInParam(i, pi);
  }
  
  // Process OUT/INOUT parameters
  for (ComUInt32 j = 0; j < sp->getNumOutParameters(); j++)
  {
    UdrParameterInfo &po = (UdrParameterInfo &)request.getOutParam(j);

    // Copy information from the UdrParameterInfo into the spinfo. The
    // spinfo will initialize an LmParameter instance for this
    // parameter. Note for INOUT parameters, the LmParameter will not
    // be completely re-initialized. It was partially initialized in
    // the loop above for input parameters. This setOutParam call
    // completes the LmParameter initialization.
    sp->setOutParam(j, po);
  }

  // Process table input info if any
  if(sp->getParamStyle() == COM_STYLE_TM)
  {
    sp->setNumTableInfo(request.getNumInputTables());
    sp->setTableInputInfo(request.getInputTables()); 
  }

  if (UdrGlob->verbose_ &&
      UdrGlob->traceLevel_ >= TRACE_DETAILS &&
      UdrGlob->showLoad_)
  {
    sp->displaySPInfo(2);
  }
  
  if (UdrGlob->verbose_ &&
      UdrGlob->traceLevel_ >= TRACE_DATA_AREAS &&
      UdrGlob->showLoad_)
  {
    ServerDebug("  LmParameter array:");
    for (ComUInt32 i = 0; i < sp->getNumParameters(); i++)
    {
      dumpLmParameter(sp->getLmParameter(i), i, "    ");
    }
  }

  if (UdrGlob->verbose_ &&
      UdrGlob->traceLevel_ >= TRACE_DATA_AREAS &&
      UdrGlob->showLoad_)
    ServerDebug("");
  
  return sp;
} // processLoadParameters

// LCOV_EXCL_START
void displayLoadParameters(UdrLoadMsg &request)
{
  ServerDebug("");
  ServerDebug("UDR load parameters:");
  ServerDebug("  UDR handle          : " INT64_SPEC , request.getHandle());
  ServerDebug("  sql name            : %s", request.getSqlName());
  ServerDebug("  routine name        : %s", request.getRoutineName());
  ServerDebug("  routine signature   : %s", request.getSignature());
  ServerDebug("  container name      : %s", request.getContainerName());
  ServerDebug("  external path name  : %s", request.getExternalPath());
  ServerDebug("  library SQL name    : %s", request.getLibrarySqlName());
  ServerDebug("  parent QID          : %s", request.getParentQid());
  
  const char *transString;
  switch (request.getTransactionAttrs() )
  {
    case COM_NO_TRANSACTION_REQUIRED:
      transString = "NO TRANSACTION REQUIRED";
      break;
    case COM_TRANSACTION_REQUIRED:
      transString = "TRANSACTION REQUIRED";
      break;    
    default:
      transString = "*** UNKNOWN ***";
      break;
  }
  ServerDebug("  transaction required     : %s", transString);

  const char *modeString;
  switch (request.getSqlAccessMode() )
  {
    case COM_NO_SQL:
      modeString = "NO SQL";
      break;
    case COM_CONTAINS_SQL:
      modeString = "CONTAINS SQL";
      break;
    case COM_READS_SQL:
      modeString = "READS SQL DATA";
      break;
    case COM_MODIFIES_SQL:
      modeString = "MODIFIES SQL DATA";
      break;
    default:
      modeString = "*** UNKNOWN ***";
      break;
  }
  ServerDebug("  sql access mode     : %s", modeString);

  const char *language;
  switch (request.getLanguage())
  {
    case COM_LANGUAGE_JAVA:
      language = "Java";
      break;

    case COM_LANGUAGE_C:
      language = "C";
      break;

    case COM_LANGUAGE_SQL:
      language = "SQL";
      break;

    default:
      language = "*** UNKNOWN ***";
      break;
  }
  ServerDebug("  language            : %s", language);

  const char *externalSecurity;
  switch (request.getExternalSecurity())
  {
    case COM_ROUTINE_EXTERNAL_SECURITY_INVOKER:
      externalSecurity = "EXTERNAL SECURITY INVOKER";
      break;

    case COM_ROUTINE_EXTERNAL_SECURITY_DEFINER:
      externalSecurity = "EXTERNAL SECURITY DEFINER";
      break;

    default:
      externalSecurity = "*** UNKNOWN ***";
      break;
  }
  ServerDebug("  externalSecurity    : %s", externalSecurity);
  ServerDebug("  routine Owner Id    : %d",
              (Int32) request.getRoutineOwnerId());

  ServerDebug("  max result sets     : %d",
              (Int32) request.getMaxResultSets());
  ServerDebug("  num params          : %d",
              (Int32) request.getNumParameters());
  ServerDebug("  num in values       : %d",
              (Int32) request.getNumInValues());
  ServerDebug("  num out values      : %d",
              (Int32) request.getNumOutValues());
  ServerDebug("  udr flags           : %d",
              (Int32) request.getUdrFlags());

  if (request.isIsolate())
    ServerDebug("    isolate           : TRUE");
  else
    ServerDebug("    isolate           : FALSE");

  if (request.isCallOnNull())
    ServerDebug("    call on NULL      : TRUE");
  else
    ServerDebug("    call on NULL      : FALSE");

  if (request.isExtraCall())
    ServerDebug("    extra call        : TRUE");
  else
    ServerDebug("    extra call        : FALSE");

  if (request.isDeterministic())
    ServerDebug("    deterministic     : TRUE");
  else
    ServerDebug("    deterministic     : FALSE");

  ServerDebug("");
  
} // displayLoadParameters
// LCOV_EXCL_STOP

void reportLoadResults(UdrGlobals *UdrGlob, SPInfo *sp, LmRoutine *lmr_)
{
  const char *moduleName = "reportLoadResults";
  
  doMessageBox(UdrGlob, TRACE_SHOW_DIALOGS,
               UdrGlob->showLoad_, moduleName);
  
  if (UdrGlob->verbose_ &&
      UdrGlob->traceLevel_ >= TRACE_DETAILS &&
      UdrGlob->showLoad_ )
  {
    ServerDebug("");
    ServerDebug("[UdrServ (%s)] LOAD message results:", moduleName);
    ServerDebug("  LOAD Udr Handle     : " INT64_SPEC ,
                (Int64) sp->getUdrHandle());
    ServerDebug("  LM result parameter : %d", (Int32) sp->getNumParameters());
    if (sp->getReturnValue() != NULL)
    {
      dumpLmParameter(*sp->getReturnValue(), 0, "    ");
    }
    if (lmr_)
    {
      ServerDebug("  LM routine          : ");
      dumpBuffer((unsigned char *)lmr_, sizeof(LmRoutine));
    }
    ServerDebug("");
  }

} // reportLoadResults

// Utility function to push optional data buffers from UdrLoadMsg
// into LM.
void copyRoutineOptionalData(UdrLoadMsg &request, SPInfo *sp)
{
  if (sp->getLanguage() != COM_LANGUAGE_C)
    return;
  
  ComUInt32 numBufs = request.getNumOptionalDataBufs();
  LmRoutineC *routine = (LmRoutineC *) sp->getLMHandle();
  routine->setNumPassThroughInputs(numBufs);
  
  for (ComUInt32 i = 0; i < numBufs; i++)
  {
    // Process each optional data buffer. Each buffer is a 4-byte
    // length field followed by data.
    ComUInt32 buflen = 0;
    char *dataBuf = request.getOptionalDataBuf(i);
    memcpy(&buflen, dataBuf, 4);
    if (buflen > 0)
      routine->setPassThroughInput(i, dataBuf + 4, buflen);
    else
      routine->setPassThroughInput(i, (void *)"", 0);
  }
}
