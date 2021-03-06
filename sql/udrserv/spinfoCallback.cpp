/**********************************************************************
// @@@ START COPYRIGHT @@@
//
// (C) Copyright 2002-2014 Hewlett-Packard Development Company, L.P.
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
/* -*-C++-*-
 *****************************************************************************
 *
 * File:         spinfocallback.cpp
 * Description:  The SPInfo APIs provided for LmRoutines to call back
 *               SPInfo methods.
 *
 * Created:      
 * Language:     C++
 *
 *****************************************************************************
 */

#include "Platform.h"  // 64-BIT
#include "spinfoCallback.h"
#include "spinfo.h"
#include "udrextrn.h"
#include "exp_expr.h"
#include "sql_buffer.h"
#include "ex_queue.h"
#include "udrutil.h"

extern UdrGlobals *UDR_GLOBALS;
extern Int32 PerformWaitedReplyToClient(UdrGlobals *UdrGlob,
                                UdrServerDataStream &msgStream);
extern void doMessageBox(UdrGlobals *UdrGlob, Int32 trLevel,
                         NABoolean moduleType, const char *moduleName);

extern NABoolean allocateReplyRow(UdrGlobals *UdrGlob,
  SqlBuffer &replyBuffer,       // [IN]  A reply buffer
  queue_index parentIndex,      // [IN]  Identifies the request queue entry
  Int32 replyRowLen,              // [IN]  Length of reply row
  char *&newReplyRow,           // [OUT] The allocated reply row
  ControlInfo *&newControlInfo, // [OUT] The allocated ControlInfo entry
  ex_queue::up_status upStatus  // [IN]  Q_OK_MMORE, Q_NO_DATA, Q_SQLERROR
  );


Int32 sendReqBufferWaitedReply(UdrGlobals *udrGlobals,
                             SPInfo *sp,
                             Int32 tableIndex)
{
  //get datastream
  UdrServerDataStream *dataStream = sp->getDataStream();

  // construct a reply with flags indicating more data required
  // for this table index.
  UdrDataBuffer *reply = new (*dataStream, sp->getReplyBufferSize())
          UdrDataBuffer(sp->getReplyBufferSize(), UdrDataBuffer::UDR_DATA_OUT, NULL);

  //Sql buffer is initilized when created on IPC stream. no need to init again
  //because the buffer state will change from INUSE to EMPTY state. This will
  //cause IPC to not even send the buffer as part of reply.
 
  //Indicate the table index so that client can send a buffer
  //that is applicable to this table.
  reply->setTableIndex(tableIndex);

  //indicate that server expects a buffer from client.
  reply->setSendMoreData(TRUE);

  // set SPInfo state to INVOKED_GETROWS for error checking purposes.
  // Also helps debugging.
  if(sp->getSPInfoState() != SPInfo::INVOKED)
  {
    return SQLUDR_ERROR;
  }
  sp->setSPInfoState(SPInfo::INVOKED_GETROWS);

  //now wait for IO completion. Once IO completes
  //necessary updates to spInfo are already performed.
  Int32 result = PerformWaitedReplyToClient(udrGlobals, *dataStream);

  // reset SpInfo state back to INVOKED state.
  if(sp->getSPInfoState() != SPInfo::INVOKED_GETROWS)
  {
    return SQLUDR_ERROR;
  }
  sp->setSPInfoState(SPInfo::INVOKED);

  return result;
}

Int32 SpInfoGetNextRow(char            *rowData,           
                      Int32             tableIndex,           
                      SQLUDR_Q_STATE  *queue_state        
                      )
{
  
  UdrGlobals *udrGlobals = UDR_GLOBALS;


  const char *moduleName = "SPInfo::SpInfoGetNextRow";

  doMessageBox(udrGlobals,
               TRACE_SHOW_DIALOGS,
               udrGlobals->showInvoke_,
               moduleName);

  // Assumption here that there is always one SP when table mapping 
  // UDR is used.
  SPInfo *sp = udrGlobals->getCurrSP();
  if(!sp)
  {
// LCOV_EXCL_START
    *queue_state = SQLUDR_Q_CANCEL;
    return SQLUDR_ERROR;
// LCOV_EXCL_STOP
  }

  // Access  SQL buffer that corresponds to table index 
  SqlBuffer *getSqlBuf = sp->getReqSqlBuffer(tableIndex);
  if (getSqlBuf == NULL)
  {
    //Perform a waited reply to client requesting for more data,
    Int32 status = sendReqBufferWaitedReply(udrGlobals, sp, tableIndex);
    if(status == SQLUDR_ERROR)
    {
      *queue_state = SQLUDR_Q_CANCEL;
      return SQLUDR_ERROR;
    }

    //expect sql buffer to be populated in spinfo now. So get it.
    getSqlBuf = sp->getReqSqlBuffer(tableIndex);

    if (getSqlBuf == NULL)
    {
      //Something is wrong this time.
      *queue_state = SQLUDR_Q_CANCEL;
      return SQLUDR_ERROR;
    }
  }

  down_state downState;
  tupp requestRow;
  NABoolean endOfData = getSqlBuf->moveOutSendOrReplyData(
      TRUE,          // [IN] sending? (vs. replying)
      &downState,    // [OUT] queue state
      requestRow,    // [OUT] new data tupp_descriptor
      NULL,          // [OUT] new ControlInfo area
      NULL,          // [OUT] new diags tupp_descriptor
      NULL);         // [OUT] new stats area

  if (endOfData) // indicates no more tupples in the buffer.
  {
    //Check if the client already indicated that the current buffer
    //was the last buffer. If it is last buffer, return.
    if(sp->isLastReqSqlBuffer(tableIndex))
    {
      *queue_state = SQLUDR_Q_EOD;
      return SQLUDR_SUCCESS;
    }

    //As a safety measure, memset the sql buffer since we expect new
    //data in sql buffer after performing waited send below.
    memset(getSqlBuf,'\0', sp->getRequestBufferSize());

    //Perform a waited reply to client requesting for more data,    
    Int32 status = sendReqBufferWaitedReply(udrGlobals, sp, tableIndex);
    if(status == SQLUDR_ERROR)
    {
      *queue_state = SQLUDR_Q_CANCEL;
      return SQLUDR_ERROR;
    }

    // Now get the new buffer.
    getSqlBuf = sp->getReqSqlBuffer(tableIndex);
    if (getSqlBuf == NULL)
    {
      *queue_state = SQLUDR_Q_CANCEL;
      return SQLUDR_ERROR;
    }

    //Extract the row again.
    endOfData = getSqlBuf->moveOutSendOrReplyData(
      TRUE,          // [IN] sending? (vs. replying)
      &downState,    // [OUT] queue state
      requestRow,    // [OUT] new data tupp_descriptor
      NULL,          // [OUT] new ControlInfo area
      NULL,          // [OUT] new diags tupp_descriptor
      NULL);  

    if (endOfData)
    {

      // It is possible for client to return a empty sql buffer and
      // and indicate that it is the last buffer. In this case, return
      // EOD. before that check for any last set of rows if any.
      if(sp->isLastReqSqlBuffer(tableIndex))
      {
        *queue_state = SQLUDR_Q_EOD;
        return SQLUDR_SUCCESS;
      }

      //we just got the buffer, it cannot be empty unless 
      //it is last buffer indication.
      *queue_state = SQLUDR_Q_CANCEL;
      return SQLUDR_ERROR;
    }
  }

  memset(rowData, '\0', sp->getInputRowLength(tableIndex));
  memcpy(rowData, requestRow.getDataPointer(), sp->getInputRowLength(tableIndex));  
  
  *queue_state = SQLUDR_Q_MORE;
  return SQLUDR_SUCCESS;

}

Int32 sendEmitWaitedReply(UdrGlobals *udrGlobals,
                        SPInfo *sp,
                        SqlBuffer *emitSqlBuffer)
{
  const char *moduleName = "sendEmitWaitedReply";

  NABoolean traceInvokeDataAreas = false;
// LCOV_EXCL_START
  if (udrGlobals->verbose_ && udrGlobals->showInvoke_ &&
      udrGlobals->traceLevel_ >= TRACE_DATA_AREAS)
    traceInvokeDataAreas = true;

  if (traceInvokeDataAreas)
  {
    ServerDebug("");
    ServerDebug("[UdrServ (%s)] EMIT SQL Buffer", moduleName);

    displaySqlBuffer(emitSqlBuffer, sp->getReplyBufferSize());
  }
// LCOV_EXCL_STOP

  //get datastream
  UdrServerDataStream *dataStream = sp->getDataStream();

  // construct a reply with flags indicating continue request
  // needed.
  UdrDataBuffer *reply = new (*dataStream, sp->getReplyBufferSize())
        UdrDataBuffer(sp->getReplyBufferSize(), UdrDataBuffer::UDR_DATA_OUT, NULL);

  // Before copying the sqlBuffer from current heap to the one in IPC stream, 
  // pack the buffer.
  emitSqlBuffer->drivePack();

  // memcpy the emitSqlBuffer into reply sqlBuffer that just got created
  // inside UdrDataBuffer. We could avoid this memcpy. Not sure how? PV
  memcpy(reply->getSqlBuffer(), emitSqlBuffer, sp->getReplyBufferSize()); 

  // indicate to client to send a continue request.
  reply->setSendMoreData(FALSE);

  // set SPInfo state to INVOKED_EMITROWS for error checking purposes.
  // Also helps debugging.
  if(sp->getSPInfoState() != SPInfo::INVOKED)
  {
    return SQLUDR_ERROR;
  }
  sp->setSPInfoState(SPInfo::INVOKED_EMITROWS);

  //now wait for IO completion. Once IO completes
  //necessary updates to spInfo are already performed.
  Int32 result = PerformWaitedReplyToClient(udrGlobals, *dataStream);

  // reset SpInfo state back to INVOKED state.
  if(sp->getSPInfoState() != SPInfo::INVOKED_EMITROWS)
  {
    return SQLUDR_ERROR;
  }
  sp->setSPInfoState(SPInfo::INVOKED);

  return result;
}


Int32 SpInfoEmitRow  (char            *rowData,           
                     Int32             tableIndex,         
                     SQLUDR_Q_STATE  *queue_state        
                    )
{
  UdrGlobals *udrGlobals = UDR_GLOBALS;
  
  const char *moduleName = "SPInfo::SpInfoEmitRow";

  doMessageBox(udrGlobals,
               TRACE_SHOW_DIALOGS,
               udrGlobals->showInvoke_,
               moduleName);

  // Assumption here that there is always one SP when table mapping 
  // UDR is used.
  SPInfo *sp = udrGlobals->getCurrSP();
  if(!sp)
// LCOV_EXCL_START
  {
    *queue_state = SQLUDR_Q_CANCEL;
    return SQLUDR_ERROR;
  }
// LCOV_EXCL_STOP


  // Access emit SQL buffer that corresponds to table index 
  SqlBuffer *emitSqlBuffer = sp->getEmitSqlBuffer(tableIndex);

  // if SQLUDR_Q_EOD is received as the very first emit, there
  // is nothing much to do. We could avoid allocating emitSqlbuffer.
  if((emitSqlBuffer == NULL ) && (*queue_state == SQLUDR_Q_EOD))
      return SQLUDR_SUCCESS;

  // If SQLUDR_Q_EOD is received and emitSqlBuffer is partially filled,
  // then send the emitSqlBUffer to client in awaited call. Note that
  // there is no need to insert a Q_NO_DATA tupple in the buffer.
  // Q_NO_DATA tupple is set in the final reply from workTM().
  if((emitSqlBuffer != NULL ) && (*queue_state == SQLUDR_Q_EOD) &&
     (emitSqlBuffer->getTotalTuppDescs() > 0))
  {
    Int32 status = sendEmitWaitedReply(udrGlobals, sp, emitSqlBuffer);

    if(status == SQLUDR_ERROR)
    {
      *queue_state = SQLUDR_Q_CANCEL;
      return SQLUDR_ERROR;
    }

    return SQLUDR_SUCCESS;
  }

  if(*queue_state == SQLUDR_Q_EOD)
  {
    return SQLUDR_SUCCESS;
  }

  // If we reach here means, atleast one row is available to be emitted.
  if(emitSqlBuffer == NULL)
  {
    //allocate one and set it back in SPInfo.
    NAHeap *udrHeap = udrGlobals->getUdrHeap();
    emitSqlBuffer = 
      (SqlBuffer*)udrHeap->allocateMemory(sp->getReplyBufferSize());

    if (emitSqlBuffer == NULL)
    {
      *queue_state = SQLUDR_Q_CANCEL;
      return SQLUDR_ERROR;
    }

    emitSqlBuffer->driveInit(sp->getReplyBufferSize(), FALSE, SqlBuffer::NORMAL_);
    emitSqlBuffer->bufferInUse();
    sp->setEmitSqlBuffer(emitSqlBuffer, tableIndex);
  }

  //Allocate a row inside replyBuffer.
  char *replyData = NULL;
  ControlInfo *replyControlInfo = NULL;
  NABoolean replyRowAllocated = 
    allocateReplyRow(udrGlobals,
                     *emitSqlBuffer,       // [IN]  A reply buffer
                     sp->getParentIndex(), // [IN]  Identifies the request queue entry
                     sp->getReplyRowSize(),// [IN]  Length of reply row
                     replyData,            // [OUT] The allocated reply row
                     replyControlInfo,     // [OUT] The allocated ControlInfo entry
                     ex_queue::Q_OK_MMORE  // [IN]  Q_OK_MMORE, Q_NO_DATA, Q_SQLERROR
                    );

  if (!replyRowAllocated)
  {
    // Since buffer is full send this buffer off to client and then continue.
    Int32 status = sendEmitWaitedReply(udrGlobals, sp, emitSqlBuffer);

    if(status == SQLUDR_ERROR)
    {
      *queue_state = SQLUDR_Q_CANCEL;
      return SQLUDR_ERROR;
    }

    // Now that we got continue message back from client, lets continue
    // filling up emitSqlbuffer again.
    emitSqlBuffer->driveInit(sp->getReplyBufferSize(), FALSE, SqlBuffer::NORMAL_);

    replyRowAllocated = 
    allocateReplyRow(udrGlobals,
                     *emitSqlBuffer,       // [IN]  A reply buffer
                     sp->getParentIndex(), // [IN]  Identifies the request queue entry
                     sp->getReplyRowSize(),// [IN]  Length of reply row
                     replyData,            // [OUT] The allocated reply row
                     replyControlInfo,     // [OUT] The allocated ControlInfo entry
                     ex_queue::Q_OK_MMORE  // [IN]  Q_OK_MMORE, Q_NO_DATA, Q_SQLERROR
                    );

    if(!replyRowAllocated)
    {
      *queue_state = SQLUDR_Q_CANCEL;
      return SQLUDR_ERROR;
    }
  }
  memcpy(replyData, rowData, sp->getReplyRowSize());

  /*
  This piece of code emits every token, for testing purposes.
  // Since buffer is full send this buffer off to client and then continue.
  int status = sendEmitWaitedReply(udrGlobals, sp, emitSqlBuffer);

  if(status == SQLUDR_ERROR)
  {
    *queue_state = SQLUDR_Q_CANCEL;
    return SQLUDR_ERROR;
  }

  // Now that we got continue message back from client, lets continue
  // filling up emitSqlbuffer again.
  emitSqlBuffer->driveInit(sp->getReplyBufferSize(), FALSE, SqlBuffer::NORMAL_);
  */
  return SQLUDR_SUCCESS;

}

