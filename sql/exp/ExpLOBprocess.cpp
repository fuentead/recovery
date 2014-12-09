/**********************************************************************
// @@@ START COPYRIGHT @@@
//
// (C) Copyright 1994-2014 Hewlett-Packard Development Company, L.P.
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
 * File:         ex_lob.C
 * Description:  class to store and retrieve LOB data.
 *               
 *               
 * Created:      10/29/2012
 * Language:     C++
 *
 *
 *
 *
 *****************************************************************************
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <string>
#include <errno.h>
#include <sys/file.h>

#include <iostream>
 
#include <errno.h>
#include <fcntl.h>       // for nonblocking
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <zlib.h>        // ZLIB compression library
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>  // basic socket definitions
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>   // basic system data types
#include <sys/uio.h>
#include <sys/wait.h>

#include <guardian/kphandlz.h>


#include <seabed/ms.h>
#include <seabed/fs.h>
#include <seabed/pctl.h>
#include <seabed/pevents.h>
#include <seabed/fserr.h>

#include "ComRtUtils.h"
//#include "ExeReplInterface.h"
#include "Globals.h"
#include "NAExit.h"
#include "ex_ex.h"                  // ex_assert
#include "SCMVersHelp.h"

#define SQ_USE_LOB_PROCESS 1
#include "ExpLOBaccess.h"
#include "ExpLOBexternal.h"

extern int ms_transid_reg(MS_Mon_Transid_Type);
extern void ms_transid_clear(MS_Mon_Transid_Type);
extern "C" short GETTRANSID(short *transid);
extern "C" short JOINTRANSACTION(Int64 transid);
extern "C" short SUSPENDTRANSACTION(short *transid);
#define TRANSID_IS_VALID(idin) (idin.id[0] != 0)

using namespace std;

// Session State values
enum {
      IDLE_STATE      =  0,
      WAITING         =  1,
      DATA_PENDING    =  2,
      WRITE_PENDING   =  3,
      SEND_PENDING    =  4,
      SENDING_DATA    =  5,
      RQST_PENDING    =  6,
      REPLY_PENDING   =  7,
      WAITING_REPLY   =  8,
      END_SESSION     =  9,
      COMM_RESET      = 10,
      READ_POSTED     = 11,
// TCP/IP State Machine states
      LISTENER_INIT_STATE         = 20,
      LISTENER_SOCKOPT_STATE      = 21,
      LISTENER_ACCEPT_STATE       = 22,
      LISTENER_SHUTDOWN_STATE     = 23,
      LISTENER_CLOSE_STATE        = 24,

      SESSION_INIT_STATE          = 30,
      SESSION_CONNECT_STATE       = 31,
      SESSION_CONNECT_CHECK_STATE = 32,
      SESSION_SOCKOPT1_STATE      = 33,
      SESSION_SOCKOPT2_STATE      = 34,
      SESSION_RECV_STATE          = 35,
      SESSION_SEND_STATE          = 36,
      SESSION_SHUTDOWN_STATE      = 37,
      SESSION_CLOSE_STATE         = 38
};

// AWAITIO classes
enum {Class_0         = 0,
      Class_1         = 1,
      Class_2         = 2,
      Class_3         = 3,
      Class_4         = 4,
      Class_5         = 5,
      Class_6         = 6,
      Class_7         = 7};

void process_msg(BMS_SRE *sre)
{
    // do work here
    return;
}

class rcv_struct
{
public:
  union {
    unsigned char  *databuf;
    Lng32            *datalen;
  };
  Lng32               buflen;
  short             file;
  short             state;
};

#pragma fieldalign platform awaitio_tag_struct
class awaitio_tag_struct
{
public:
  union {
    SB_Tag_Type     Tag;
    struct {
      ULng32  Class:4;          // I/O class
      ULng32  State:12;         // State of this I/O operation.
      ULng32  Index:16;         // Index into control block table.
    };
  };
};

class awaitio_struct
{
public:
  Lng32               Iocount;
  short             Error;
  short             File;
};

enum {IDLE_TIMEOUT    = 30000,      // 5 minute wait before stopping cli process.
      MAX_RETRIES     = 3,          // Maximum number of retries before waiting.
      NUM_OPENS       = 64,         // Initial number of entries in the OCB table.
      RECV_BUFSIZE    = 56 * 1024,  // Size of $RECEIVE I/O buffer.
      RETRY_TIMEOUT   = 3000,       // 30 second wait before retrying.
      SOCK_BUFSIZE    = 56 * 1024,  // Size of socket I/O buffers.
      STARTUP_TIMEOUT = 500};       // CLI Invokers will die if an open message is not
                                    // received before a 5 sec startup timer expires.
rcv_struct          rcv;
// Mutexes and thread variables
pthread_mutex_t     cnfg_mutex;
pthread_mutex_t     g_mutex;
pthread_attr_t      thr_attr;
Lng32                 total_dynamic_memory;
CliGlobals         *cliGlobals    = NULL;
char               *myProgramName = NULL;
xzsys_ddl_smsg_def *sysmsg        = NULL;
FS_Receiveinfo_Type rcvinfo;
ExLobGlobals *lobGlobals = NULL;

//*****************************************************************************
//*****************************************************************************
// LCOV_EXCL_START
static void sigterm_handler(Lng32 signo)
{
    printf("sigterm received\n");
}
// LCOV_EXCL_STOP

void LOB_process_stop(short flag)
{
  if (rcv.state == READ_POSTED)
    BCANCEL(rcv.file);
  else
    NAExit(flag);
}  // BDR_process_stop

//*****************************************************************************

static void *Calloc(size_t memsize)
{
  void     *ptr;
  Lng32       retval;

  retval = pthread_mutex_lock(&g_mutex);
  ex_assert(retval == 0, "Calloc 1");
  ptr = calloc(1, memsize);
  ex_assert(ptr != NULL, "Calloc 2");
  total_dynamic_memory += memsize;
  retval = pthread_mutex_unlock(&g_mutex);
  ex_assert(retval == 0, "Calloc 3");

  return(ptr);
}  // Calloc

void post_receive_read(void)
{
  _bcc_status         cc_status;
  awaitio_tag_struct  tag;

  if (rcv.state == END_SESSION)
    LOB_process_stop(0);

  if (rcv.databuf == NULL)
  {
    rcv.buflen  = RECV_BUFSIZE + 2*sizeof(int);
    rcv.databuf = (unsigned char *)Calloc(rcv.buflen);
    sysmsg = (xzsys_ddl_smsg_def *)&rcv.datalen[0];
  }

  tag.Tag   = 0;
  tag.Class = Class_7;

  cc_status = BREADUPDATEX(rcv.file, (char *)&rcv.datalen[0], RECV_BUFSIZE,
                           NULL, tag.Tag);
  ex_assert(_status_eq(cc_status), "post_receive_read 1");
  rcv.state = READ_POSTED;

  return;
}  // post_receive_read

short process_open(void)
{
  Lng32         retval;
  short       retvals = XZFIL_ERR_OK;

  return retvals;
}  // process_open

//*****************************************************************************
//*****************************************************************************
short process_close(void)
{
  Lng32               retval;
  short             retvals = XZFIL_ERR_OK;

  return retvals;
}  // process_close


void process_mon_msg(MS_Mon_Msg *msg) {
    printf("server received monitor msg, type=%d\n", msg->type);

    switch (msg->type) {
    case MS_MsgType_Change:
        printf("  type=%d, group=%s, key=%s, value=%s\n",
               msg->u.change.type,
               msg->u.change.group,
               msg->u.change.key,
               msg->u.change.value);
        break;
    case MS_MsgType_Close:
        printf("  nid=%d, pid=%d, process=%s, aborted=%d\n",
               msg->u.close.nid,
               msg->u.close.pid,
               msg->u.close.process_name,
               msg->u.close.aborted);
        break;
    case MS_MsgType_Event:
        break;
    case MS_MsgType_NodeDown:
        printf("  nid=%d, node=%s\n",
               msg->u.down.nid,
               msg->u.down.node_name);
        break;
    case MS_MsgType_NodeUp:
        printf("  nid=%d, node=%s\n",
               msg->u.up.nid,
               msg->u.up.node_name);
        break;
    case MS_MsgType_Open:
        printf("  nid=%d, pid=%d, process=%s, death=%d\n",
               msg->u.open.nid,
               msg->u.open.pid,
               msg->u.open.process_name,
               msg->u.open.death_notification);
        break;
    case MS_MsgType_ProcessCreated:
        printf("  nid=%d, pid=%d, tag=0x%llx, process=%s, ferr=%d\n",
               msg->u.process_created.nid,
               msg->u.process_created.pid,
               msg->u.process_created.tag,
               msg->u.process_created.process_name,
               msg->u.process_created.ferr);
        break;
    case MS_MsgType_ProcessDeath:
        printf("  nid=%d, pid=%d, aborted=%d, process=%s\n",
               msg->u.death.nid,
               msg->u.death.pid,
               msg->u.death.aborted,
               msg->u.death.process_name);
        break;
    case MS_MsgType_Service:
        break;
    case MS_MsgType_Shutdown:
        printf("  nid=%d, pid=%d, level=%d\n",
               msg->u.shutdown.nid,
               msg->u.shutdown.pid,
               msg->u.shutdown.level);
        break;
    case MS_MsgType_TmSyncAbort:
    case MS_MsgType_TmSyncCommit:
        break;
    case MS_MsgType_UnsolicitedMessage:
        break;
    default:
        break;
    }
}

Ex_Lob_Error ExLob::append(ExLobRequest *request)
{
    Ex_Lob_Error err; 
    Int64 offset;
    Int64 dummyParam;
    Lng32 handleOutLen;
    Lng32 clierr;

    Int64 size = request->getDesc().getSize();
    offset = request->getDataOffset();

    /* This is done in the master 
    // allocate a new lob desc
    err = allocateDesc(size, dummyParam, offset);

    if (err != LOB_OPER_OK)
      return err;

    request->getDesc().setOffset(offset);
    */

    clierr = SQL_EXEC_LOBcliInterface(request->getHandleIn(), request->getHandleInLen(), 
				      0, 0,
                                      request->getHandleOut(), &handleOutLen,
                                      LOB_CLI_INSERT_APPEND, LOB_CLI_ExecImmed,
                                      &offset, &size,
                                      &dummyParam, &dummyParam, 
				      0,
				      request->getTransId());

    request->setCliError(clierr);
    
    return LOB_OPER_OK;
}

Ex_Lob_Error ExLob::update(ExLobRequest *request)
{
    Ex_Lob_Error err; 
    Int64 offset;
    Int64 dummyParam;
    Lng32 handleOutLen;
    Lng32 clierr;

    Int64 size = request->getDesc().getSize();
    offset = request->getDataOffset();
    /* sss this is done in the master
    // allocate a new lob desc
    err = allocateDesc(size, dummyParam, offset);

    if (err != LOB_OPER_OK)
      return err;

    request->getDesc().setOffset(offset);
    */
    clierr = SQL_EXEC_LOBcliInterface(request->getHandleIn(), request->getHandleInLen(), 
				      0, 0,
                                      request->getHandleOut(), &handleOutLen,
                                      LOB_CLI_UPDATE_UNIQUE, LOB_CLI_ExecImmed,
                                      &offset, &size,
                                      &dummyParam, &dummyParam, 
				      0,
				      request->getTransId());

    request->setCliError(clierr);
    
    return LOB_OPER_OK;
}

Ex_Lob_Error ExLob::putDesc(ExLobDesc &desc, Int64 descNum) 
{
  /*  Ex_Lob_Error err = LOB_OPER_OK;

    Lng32 offset = sizeof(ExLobDescHeader) + sizeof(ExLobDesc) * descNum;
    if (pwrite(fdDesc_, &desc, sizeof(ExLobDesc), offset) == -1) {
       return LOB_DESC_WRITE_ERROR;
    }
  */
    return LOB_OPER_OK;
}

Ex_Lob_Error ExLob::getDesc(ExLobRequest *request) 
{
   Ex_Lob_Error err;
   Lng32 clierr;
   Int64 dummyParam;
   Lng32 handleOutLen = 0;
   Lng32 blackBoxLen = 0;
   Int64 offset;
   Int64 size;

   clierr = SQL_EXEC_LOBcliInterface(request->getHandleIn(), request->getHandleInLen(), 
				     request->getBlackBox(), &blackBoxLen,
                                     request->getHandleOut(), &handleOutLen,
                                     LOB_CLI_SELECT_UNIQUE, LOB_CLI_ExecImmed,
                                     &offset, &size,
                                     &dummyParam, &dummyParam, 
				     0,
				     request->getTransId());

   request->setHandleOutLen(handleOutLen);
   request->setBlackBoxLen(blackBoxLen);
   request->getDesc().setOffset(offset);
   request->getDesc().setSize(size);
   request->setCliError(clierr);

   return LOB_OPER_OK;
}

Ex_Lob_Error ExLob::delDesc(ExLobRequest *request)
{ 
    Ex_Lob_Error err; 
    Int64 offset;
    Int64 dummyParam;
    Lng32 handleOutLen;
    Lng32 clierr;

    clierr = SQL_EXEC_LOBcliInterface(request->getHandleIn(), request->getHandleInLen(), 
				      0, 0,
                                      (char *)&dummyParam, (Lng32 *)&dummyParam,
                                      LOB_CLI_DELETE, LOB_CLI_ExecImmed,
                                      &dummyParam, &dummyParam,
                                      &dummyParam, &dummyParam, 
				      0,
				      request->getTransId());

    request->setCliError(clierr);
    
    return LOB_OPER_OK;
}

/*Ex_Lob_Error ExLob::purgeLob()
{
    Ex_Lob_Error err = LOB_OPER_OK;

    close(fdDesc_);
    fdDesc_ = -1;

    if (remove(lobDescFile_) != 0) {
      return LOB_DESC_FILE_DELETE_ERROR;
    }

    return LOB_OPER_OK;
    } */

Ex_Lob_Error ExLob::purgeLob()
{
    Ex_Lob_Error err = LOB_OPER_OK;

    hdfsCloseFile(fs_,fdDesc_);
    fdDesc_ = NULL;

    if (hdfsDelete(fs_,lobDescFile_,0) != 0) {
      return LOB_DESC_FILE_DELETE_ERROR;
    }

    return LOB_OPER_OK;
}

/*Ex_Lob_Error ExLob::allocateDesc(ULng32 size, Int64 &descNum, Int64 &dataOffset)
{
    Ex_Lob_Error err = LOB_OPER_OK;


    err = lockDesc();
    if (err != LOB_OPER_OK)
      return err;
    ExLobDescHeader header;

	 if (pread(fdDesc_, &header, sizeof(ExLobDescHeader), 0) == -1) {
	   flock(fdDesc_, LOCK_UN);
	   return LOB_DESC_HEADER_READ_ERROR;
	 }

    if (header.getAvailSize() >= size) {
      descNum = header.getFreeDesc(); 
      dataOffset = header.getDataOffset();
      header.incFreeDesc();
      header.decAvailSize(size);
      header.incDataOffset(size);

      if (pwrite(fdDesc_, &header, sizeof(ExLobDescHeader), 0) == -1) {
        err = LOB_DESC_HEADER_WRITE_ERROR;
      }

    }
    else {
      err = LOB_DATA_FILE_FULL_ERROR;
    }
   

    ExLobDesc desc(dataOffset, size, descNum);
    if (pwrite(fdDesc_, &desc, sizeof(ExLobDesc), 
        sizeof(ExLobDescHeader) + sizeof(ExLobDesc) * descNum) == -1) {
      return LOB_DESC_WRITE_ERROR;
    }

    err = unlockDesc();
    if (err != LOB_OPER_OK)
      return err;

    return err;
} */

   

Ex_Lob_Error ExLob::insertDesc(ExLobRequest *request) 
{
   Ex_Lob_Error err;
   Lng32 clierr;
   Int64 dummyParam;
   Lng32 handleOutLen = 0;
   Lng32 blackBoxLen = 0;
   Int64 offset = request->getDataOffset();
   Int64 size = request->getDesc().getSize();

   blackBoxLen = request->getBlackBoxLen();
   clierr = SQL_EXEC_LOBcliInterface(request->getHandleIn(), request->getHandleInLen(), 
				     request->getBlackBox(), &blackBoxLen,
                                     request->getHandleOut(), &handleOutLen,
                                     LOB_CLI_INSERT, LOB_CLI_ExecImmed,
                                     &offset, &size,
                                     &dummyParam, &dummyParam, 
				     0,
				     request->getTransId());

   request->setHandleOutLen(handleOutLen);
   request->setBlackBoxLen(blackBoxLen);
   request->setCliError(clierr);

   return LOB_OPER_OK;
}

Ex_Lob_Error ExLob::selectCursorDesc(ExLobRequest *request) 
{
   Ex_Lob_Error err;
   Int64 dummyParam;
   void *cliInterface = NULL;
   cursor_t cursor;
   int clierr;

   clierr = SQL_EXEC_LOBcliInterface(request->getHandleIn(), request->getHandleInLen(),
				     0, 0,
                                     (char *)&dummyParam, (Lng32 *)&dummyParam,
                                     LOB_CLI_SELECT_CURSOR, LOB_CLI_ExecImmed,
                                     &dummyParam, &dummyParam,
                                     &dummyParam, &dummyParam, 
				     &cliInterface,
				     request->getTransId());

   cursor.cliInterface_ = cliInterface;
   // these are only used in library, but initialize them here anyway.
   cursor.bytesRead_ = -1;
   cursor.descOffset_ = -1;
   cursor.descSize_ = -1;
   cursor.eod_ = false; 

   lobCursors_it it = lobCursors_.find(string(request->getHandleIn(), request->getHandleInLen()));

   if (it == lobCursors_.end())
   {
      lobCursors_.insert(pair<string, cursor_t>
                        (string(request->getHandleIn(), request->getHandleInLen()), cursor));                         
   }
   else
   {
      it->second = cursor;
   }

   request->setCliError(clierr);

   return LOB_OPER_OK;
}

Ex_Lob_Error ExLob::fetchCursorDesc(ExLobRequest *request) 
{
   Ex_Lob_Error err;
   Int64 dummyParam;
   Int64 offset = -1;
   Int64 size = -1;
   int clierr;

   lobCursors_it it = lobCursors_.find(string(request->getHandleIn(), request->getHandleInLen()));

   if (it == lobCursors_.end())
   {
      return LOB_CURSOR_NOT_OPEN;                         
   }

   void *cliInterface = it->second.cliInterface_;

   clierr = SQL_EXEC_LOBcliInterface(request->getHandleIn(), request->getHandleInLen(), 
				     0, 0,
                                     (char *)&dummyParam, (Lng32 *)&dummyParam,
                                     LOB_CLI_SELECT_FETCH, LOB_CLI_ExecImmed,
                                     &offset, &size,
                                     &dummyParam, &dummyParam, 
				     &cliInterface,
				     request->getTransId());

   request->getDesc().setOffset(offset);
   request->getDesc().setSize(size);
   request->setCliError(clierr);

   return LOB_OPER_OK;
}

Ex_Lob_Error ExLob::closeCursorDesc(ExLobRequest *request)
{
   Ex_Lob_Error err;
   Int64 dummyParam;
   int clierr;

   lobCursors_it it = lobCursors_.find(string(request->getHandleIn(), request->getHandleInLen()));

   if (it == lobCursors_.end())
   {
      return LOB_CURSOR_NOT_OPEN;                         
   }

   void *cliInterface = it->second.cliInterface_;

   clierr = SQL_EXEC_LOBcliInterface(request->getHandleIn(), request->getHandleInLen(), 
				     NULL, NULL,
                                     (char *)&dummyParam, (Lng32 *)&dummyParam,
                                     LOB_CLI_SELECT_CLOSE, LOB_CLI_ExecImmed,
                                     &dummyParam, &dummyParam,
                                     &dummyParam, &dummyParam, 
				     &cliInterface,
				     request->getTransId());

   // do not update cliErr in request because the fetch would have updated it. 

   // remove cursor from the map.
   err = closeCursor(request->getHandleIn(), request->getHandleInLen());

   return LOB_OPER_OK;
}

void processRequest(ExLobRequest *request)
{
     Ex_Lob_Error err = LOB_OPER_OK;
     Int64 descNum;
     Int64 dataOffset;
     Int64 operLen;
     ExLobDescHeader descHeader;
     ExLob *lobPtr;
     ExLobDesc desc;
     ExLobDesc *descPtr;
     Lng32 clierr;\
     Lng32 handleOutLen;
     Int64 size;
    
     err = lobGlobals->getLobPtr(request->getDescFileName(), lobPtr);
     if (err != LOB_OPER_OK)
       {
	 request->setError(LOB_INIT_ERROR);	   
	 return ;
       }
     if (!lobGlobals->isCliInitialized())
       {
	 Lng32 clierr = SQL_EXEC_LOBcliInterface(0, 0,
					    0, 0,
					    0, 0,
					    LOB_CLI_INIT, LOB_CLI_ExecImmed,
					    0, 0,
					    0, 0,
					    0,
					    0);
	 if (clierr < 0)
	   {
	     request->setError(LOB_INIT_ERROR);	   
	     return ;
	   }
	 lobGlobals->setCliInitialized();
       }
     switch(request->getType())
     {
       case Lob_Req_Allocate_Desc:
	 /* sss no need for this since it's done in master
	   err = lobPtr->allocateDesc(request->getDesc().getSize(), descNum, dataOffset);

	 request->setDescNumOut(descNum);
         request->setDataOffset(dataOffset);
	 */
         err = lobPtr->insertDesc(request);
         break;

       case Lob_Req_Append:
         err = lobPtr->append(request);
         break;

       case Lob_Req_Select_Cursor:
         err = lobPtr->selectCursorDesc(request);
         break;

       case Lob_Req_Fetch_Cursor:
         err = lobPtr->fetchCursorDesc(request);
         if (request->getCliError() == 100) {
            lobPtr->closeCursorDesc(request);
         }
         break;

       case Lob_Req_Update:
         err = lobPtr->update(request);
         break;

       case Lob_Req_Get_Desc:
         err = lobPtr->getDesc(request);
         break;   

       case Lob_Req_Put_Desc:
         err = lobPtr->putDesc(request->getDesc(), request->getDescNumIn());
         break;   

       case Lob_Req_Del_Desc:
         err = lobPtr->delDesc(request);
         break;

       case Lob_Req_Purge_Desc:
         /* no op ..it is dropped in mastererr = lobPtr->purgeLob();
         if (err == LOB_OPER_OK) {
            err = lobGlobals->delLobPtr(request->getDescFileName());
	    } */
         break;

       case Lob_Req_Print:
         lobPtr->print();
         break; 

       default:
         err = LOB_REQUEST_UNDEFINED_ERROR;
         printf("bad request = %d\n", request->getType());
         break;
     };

     request->setError(err);

     request->log();

     return;
}

void ExLobRequest::log()
{
    printf("req num = %ld\n", getReqNum());
    printf("req type = %d\n", getType());
    printf("trans id = %ld\n", getTransId());
    printf("file = %s\n", getDescFileName());
    printf("size = %d\n", getDesc().getSize());
    printf("offset = %d\n", getDesc().getOffset());
    printf("lob err = %d\n", getError());
    printf("cli err = %d\n", getCliError());
    printf("\n");
}

void receive_message(ExLobRequest *request)
{
   Int64 transId;
   int err;
   int cliRC = GETTRANSID((short *)&transId);
   printf("transid before setting = %ld\n", transId);

   if (TRANSID_IS_VALID(request->getTransIdBig())) {
      err = ms_transid_reg(request->getTransIdBig());
      printf("transid reg err = %d\n", err);
   } else if (request->getTransId()) {
     err = JOINTRANSACTION(request->getTransId());
     printf("join txn err = %d\n", err);
   }

   cliRC = GETTRANSID((short *)&transId);
   printf("transid after setting = %ld\n", transId);

   processRequest(request);

   if (TRANSID_IS_VALID(request->getTransIdBig())) {
     ms_transid_clear(request->getTransIdBig());
   } else if (request->getTransId()) {
     transId = request->getTransId();
     SUSPENDTRANSACTION((short*)&transId);
   }
   
   return;
}  // receive_message

Ex_Lob_Error ExLobGlobals::initialize()
{
    lobMap_ = (lobMap_t *) new lobMap_t;

    if (lobMap_ == NULL)
      return LOB_INIT_ERROR;

    return LOB_OPER_OK;
}

/*Ex_Lob_Error ExLob::initialize(char *lobFile)
{
    mode_t filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    Lng32 openFlags = O_RDWR;

    strcpy(lobDescFile_, lobFile);

    fdDesc_ = open(lobDescFile_, openFlags);
    if (fdDesc_ == -1) {
      return LOB_DESC_FILE_OPEN_ERROR;
    }

    return LOB_OPER_OK;
    }*/

Ex_Lob_Error ExLob::initialize(char *lobDescFile)
{
  // Made this obsolete

    return LOB_OPER_OK;
}

Ex_Lob_Error ExLobGlobals::getLobPtr(char *lobName, ExLob *& lobPtr)
{
    Ex_Lob_Error err;
    lobMap_t *lobMap = NULL;
    lobMap_it it;

    lobMap = lobGlobals->getLobMap();
    it = lobMap->find(string(lobName));

    if (it == lobMap->end())
    {
        lobPtr = new ExLob();
        if (lobPtr == NULL) 
          return LOB_ALLOC_ERROR;

        lobMap->insert(pair<string, ExLob*>(string(lobName), lobPtr));
    }
    else
    {
        lobPtr = it->second;
    }

    return LOB_OPER_OK;
}

Ex_Lob_Error ExLobGlobals::delLobPtr(char *lobName)
{
    Ex_Lob_Error err;
    lobMap_t *lobMap = NULL;
    lobMap_it it;

    lobMap = lobGlobals->getLobMap();
    it = lobMap->find(string(lobName));
    if (it != lobMap->end()) {
       ExLob *lobPtr = it->second;
       delete lobPtr;
       lobMap->erase(it);
    }

    return LOB_OPER_OK;
}
/*
Ex_Lob_Error ExLob::lockDesc()
{

    // get exclusive lock because read and write has to be atomic
    if (flock(fdDesc_, LOCK_EX) == -1) {
      return LOB_DESC_FILE_LOCK_ERROR;
    }
  
      return LOB_OPER_OK;
}

Ex_Lob_Error ExLob::unlockDesc()
{
    // release the lock

    if (flock(fdDesc_, LOCK_UN) == -1) {
      return LOB_DESC_FILE_LOCK_ERROR;
    }

      return LOB_OPER_OK;
}
*/


Ex_Lob_Error ExLobDescHeader::print(char *descFileName)
{
  printf("Header: \n");
  printf("freeDesc = %d\n", freeDesc_);
  printf("dataOffset = %d\n", dataOffset_);
  printf("avail Size = %d\n", availSize_);
  return LOB_OPER_OK;
}

/*Ex_Lob_Error ExLob::print() 
{
  // get exclusive lock because read and write has to be atomic
  if (flock(fdDesc_, LOCK_EX) == -1) {
    return LOB_DESC_FILE_LOCK_ERROR;
  }

  ExLobDescHeader header;
  if (pread(fdDesc_, &header, sizeof(ExLobDescHeader), 0) == -1) {
    return LOB_DESC_HEADER_READ_ERROR;
  }

  header.print(NULL);

  printf("Descriptors:\n");

  printf("dNum size state tail prev next nextfree  offset\n");
  ExLobDesc desc;
  for (Lng32 numDescs = 0; numDescs < header.getFreeDesc(); numDescs++) {
    if (pread(fdDesc_, &desc, sizeof(ExLobDesc), 
               sizeof(ExLobDescHeader) + sizeof(ExLobDesc) * numDescs) == -1) {
      flock(fdDesc_, LOCK_UN);
      return LOB_DESC_READ_ERROR;
    }
    printf("%4d ", numDescs);
    desc.print();
  }

  flock(fdDesc_, LOCK_UN);
  return LOB_OPER_OK;
}
*/

Ex_Lob_Error ExLob::print() 
{
  // get exclusive lock because read and write has to be atomic
  Int32 retval = 0;
  //get a mutex to prevent multiple updates
  retval = pthread_mutex_lock(&g_mutex);    
  if (retval != 0 )
    return LOB_ALLOC_ERROR;
  long openFlags = O_RDONLY ;
  ExLobDescHeader header;
  if (hdfsPread(fs_,fdDesc_,0, &header, sizeof(ExLobDescHeader)) <= 0) {
    return LOB_DESC_HEADER_READ_ERROR;
  }

  header.print(NULL);

  printf("Descriptors:\n");

  printf("dNum size state tail prev next nextfree  offset\n");
  ExLobDesc desc;
  for (Lng32 numDescs = 0; numDescs < header.getFreeDesc(); numDescs++) {
    if (hdfsPread(fs_,fdDesc_, sizeof(ExLobDescHeader) + sizeof(ExLobDesc) * numDescs, &desc, sizeof(ExLobDesc) ) <= 0 ) {
       pthread_mutex_unlock(&g_mutex);
      return LOB_DESC_READ_ERROR;
    }
    printf("%4d ", numDescs);
    desc.print();
  }

  
  pthread_mutex_unlock(&g_mutex);
  return LOB_OPER_OK;
}


Lng32 main(Lng32 argc, char *argv[])
{
    Lng32             lv_event_len;
    Lng32             lv_error;
    Lng32             lv_ret;
    BMS_SRE         lv_sre;
    Lng32                 retval;
    _bcc_status         cc_status;
    awaitio_tag_struct  awaitTag;
    awaitio_struct      awaitIo;

    // Register sigterm_handler as our signal handler for SIGTERM.
    if (signal(SIGTERM, sigterm_handler) == SIG_ERR)
    {
      // LCOV_EXCL_START
      cout << "*** Cannot handle SIGTERM ***" << endl;
      exit(1);
      // LCOV_EXCL_STOP
    }

    retval = pthread_mutex_init(&cnfg_mutex, NULL);
    ex_assert(retval == 0, "main 1");

    // seaquest related stuff
    retval = msg_init_attach(&argc, &argv, true, (char *)"");
    if (retval)
      printf("msg_init_attach returned: %d\n", retval);  // LCOV_EXCL_LINE
    // sq_fs_dllmain();
    msg_mon_process_startup(true);
    msg_mon_enable_mon_messages(1);
    
    retval = atexit((void(*)(void))msg_mon_process_shutdown);
    if (retval != 0)
    {
      // LCOV_EXCL_START
      cout << "*** atexit failed with error " << retval << " ***" << endl;
      exit(1);
      // LCOV_EXCL_STOP
    }

    // initialize lob globals
    lobGlobals = new ExLobGlobals();
    if (lobGlobals == NULL) 
      return -1;

    retval = lobGlobals->initialize();
    if (retval != LOB_OPER_OK)
      return -1;

    /* Lng32 clierr = SQL_EXEC_LOBcliInterface(0, 0,
					    0, 0,
					    0, 0,
					    LOB_CLI_INIT, LOB_CLI_ExecImmed,
					    0, 0,
					    0, 0,
					    0,
					    0);
    if (clierr < 0)
      return -1; */

    bool done = false;
    BMS_SRE sre;
    Lng32 len;
    char recv_buffer[BUFSIZ];
    Lng32 err;

    printf("lob process initialialized. Ready for requests\n");
    while(!done)
    {
       do {
          XWAIT(LREQ, -1);
          err = BMSG_LISTEN_((short *) &sre, 0, 0);
       } 
       while (err == BSRETYPE_NOWORK); 

       err = BMSG_READDATA_(sre.sre_msgId, recv_buffer, BUFSIZ);

       if ((sre.sre_flags & XSRE_MON)) {
          MS_Mon_Msg *msg = (MS_Mon_Msg *)recv_buffer;
          process_mon_msg(msg);
          if (msg->type == MS_MsgType_Shutdown)
             done = true;
          len = 0;
       } else {
          receive_message((ExLobRequest *)recv_buffer);  
          len = sizeof(ExLobRequest);
       }

       BMSG_REPLY_(sre.sre_msgId,
                   NULL,
                   0,
                   recv_buffer,
                   len,
                   0,
                   NULL);
    }

    msg_mon_process_shutdown();
    return 0;
}







