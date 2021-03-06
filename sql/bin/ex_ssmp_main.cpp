/**********************************************************************
// @@@ START COPYRIGHT @@@
//
// (C) Copyright 2006-2014 Hewlett-Packard Development Company, L.P.
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
* File:         ex_ssmp_main.cpp
* Description:  This is the main program for SSMP. SQL stats merge process.
*
* Created:      05/08/2006
* Language:     C++
*
*****************************************************************************
*/
#include "Platform.h"
#ifdef _DEBUG
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <errno.h>
#include "ExCextdecs.h"
#include "ex_ex.h"
#include "Ipc.h"
#include "Globals.h"
#include "SqlStats.h"
#include "memorymonitor.h"
#include "ssmpipc.h"
#include "rts_msg.h"
#include "PortProcessCalls.h"
#include "seabed/ms.h"
#include "seabed/fs.h"
extern void my_mpi_fclose();
#include "SCMVersHelp.h"
DEFINE_DOVERS(mxssmp)

void  runServer(Int32 argc, char **argv);

void processAccumulatedStatsReq(SsmpNewIncomingConnectionStream *ssmpMsgStream, SsmpGlobals *ssmpGlobals);

Int32 main(Int32 argc, char **argv)
{
  dovers(argc, argv);
  try {
    file_init(&argc, &argv);
    file_mon_process_startup(true);
  }
  catch (SB_Fatal_Excep &e) {
    SQLMXLoggingArea::logExecRtInfo(__FILE__, __LINE__, e.what(), 0);
    exit(1);
  }

  atexit(my_mpi_fclose);

  // Synchronize C and C++ output streams
  ios::sync_with_stdio();
#ifdef _DEBUG
  // Redirect stdout and stderr to files named in environment
  // variables
  const char *stdOutFile = getenv("SQ_SSCP_STDOUT");
  const char *stdErrFile = getenv("SQ_SSCP_STDERR");
  Int32 fdOut = -1;
  Int32 fdErr = -1;

  if (stdOutFile && stdOutFile[0])
  {
    fdOut = open(stdOutFile,
                 O_WRONLY | O_APPEND | O_CREAT | O_SYNC,
                 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fdOut >= 0)
    {
      fprintf(stderr, "[Redirecting MXUDR stdout to %s]\n", stdOutFile);
      fflush(stderr);
      dup2(fdOut, fileno(stdout));
    }
    else
    {
      fprintf(stderr, "*** WARNING: could not open %s for redirection: %s.\n",
              stdOutFile, strerror(errno));
    }
  }

  if (stdErrFile && stdErrFile[0])
  {
    fdErr = open(stdErrFile,
                 O_WRONLY | O_APPEND | O_CREAT | O_SYNC,
                 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fdErr >= 0)
    {
      fprintf(stderr, "[Redirecting MXUDR stderr to %s]\n", stdErrFile);
      fflush(stderr);
      dup2(fdErr, fileno(stderr));
    }
    else
    {
      fprintf(stderr, "*** WARNING: could not open %s for redirection: %s.\n",
              stdErrFile, strerror(errno));
    }
  }

  runServer(argc, argv);

  if (fdOut >= 0)
  {
    close(fdOut);
  }
  if (fdErr >= 0)
  {
    close(fdErr);
  }
#else
  runServer(argc, argv);
#endif
  return 0;
}

void runServer(Int32 argc, char **argv)
{
  jmp_buf ssmpJmpBuf;
  Int32 shmId;
  StatsGlobals *statsGlobals = (StatsGlobals *)shareStatsSegment(shmId);
  Int32 r = 0;
  while (statsGlobals == NULL && ++r < 10)
    { // try 9 more times if the shared segement is not available
      DELAY(100);  // delay for 1 sec.
      statsGlobals = (StatsGlobals *)shareStatsSegment(shmId);
    }
  if (statsGlobals == NULL)
  {
     char tmbuf[64];
     time_t now;
     struct tm *nowtm;
     now = time(NULL);
     nowtm = localtime(&now);
     strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S ", nowtm);

     cout << tmbuf << "SSCP didn't create/initialize the RMS shared segment"
             << ", SSMP exited\n";
     NAExit(0);
  }

  CliGlobals *cliGlobals = CliGlobals::createCliGlobals(FALSE);
  statsGlobals->setSsmpPid(cliGlobals->myPin());
  statsGlobals->setSsmpTimestamp(cliGlobals->myStartTime());
  short error = statsGlobals->openStatsSemaphore(
                                    statsGlobals->ssmpProcSemId_);
  ex_assert(error == 0, "Error in opening the semaphore");

  cliGlobals->setStatsGlobals(statsGlobals);
  cliGlobals->setSharedMemId(shmId);

  // Handle possibility that the previous instance of MXSSMP died
  // while holding the stats semaphore.  This code has been covered in
  // a manual unit test, but it is not possible to cover this easily in
  // an automated test.
  // LCOV_EXCL_START
  if (statsGlobals->semPid_ != -1)
  {
    NAProcessHandle prevSsmpPhandle((SB_Phandle_Type *)
                            &statsGlobals->ssmpProcHandle_);
    prevSsmpPhandle.decompose();
    if (statsGlobals->semPid_ == prevSsmpPhandle.getPin())
    {
      NAProcessHandle myPhandle;
      myPhandle.getmine();
      myPhandle.decompose();
      short savedPriority, savedStopMode;
      short error =
           statsGlobals->releaseAndGetStatsSemaphore(
                     statsGlobals->ssmpProcSemId_,
                     (pid_t) myPhandle.getPin(),
                     (pid_t) prevSsmpPhandle.getPin(),
                     savedPriority, savedStopMode,
                     FALSE /*shouldTimeout*/);
      ex_assert(error == 0,
               "releaseAndGetStatsSemaphore() returned error");

      statsGlobals->releaseStatsSemaphore(
                     statsGlobals->ssmpProcSemId_,
                     (pid_t) myPhandle.getPin(),
                     savedPriority, savedStopMode);
    }
    else
    {
      // Handle possibility that the process which last got the semaphore
      // is no longer executing.  Normally, the cleanup would have
      // happened when MXSSMP processes a ZSYS_VAL_SMSG_PROCDEATH message,
      // but it could be that this was sent between instances of MXSSMP.
      NAProcessHandle myPhandle;
      myPhandle.getmine();
      myPhandle.decompose();
      char processName[50];
      Int32 loopCnt = 0;
      while (statsGlobals->semPid_ != -1)
      {
        pid_t tempPid = statsGlobals->semPid_;
        Int32 ln_error = msg_mon_get_process_name(myPhandle.getCpu(),
                                          statsGlobals->semPid_, processName);
        ex_assert(ln_error != XZFIL_ERR_INVALIDSTATE,
          "msg_mon_get_process_name shouldn't be called after "
          "msg_mon_process_shutdown");
        if (ln_error == XZFIL_ERR_NOSUCHDEV)
          {
            statsGlobals->seabedError_ = ln_error;
            statsGlobals->cleanup_SQL(tempPid, myPhandle.getPin());
          }
        else if (ln_error == XZFIL_ERR_OK)
          break;
        else
          {
            // Let this process stop - maybe the next MXSSMP will
            // have better luck....
            DELAY(300);
            if (++loopCnt >= 3)
              ex_assert(ln_error == XZFIL_ERR_OK,
                        "Too many errors from msg_mon_get_process_name")
          }
      }
    }
  }
  // LCOV_EXCL_STOP

#ifdef SQ_NEW_PHANDLE
  XPROCESSHANDLE_GETMINE_(&statsGlobals->ssmpProcHandle_);
#else
  XPROCESSHANDLE_GETMINE_(statsGlobals->ssmpProcHandle_);
#endif

  NAHeap *ssmpHeap = cliGlobals->getExecutorMemory();
  cliGlobals->setJmpBufPtr(&ssmpJmpBuf);
  if (setjmp(ssmpJmpBuf))
    NAExit(1); // Abend

  IpcEnvironment       *ssmpIpcEnv = new (ssmpHeap) IpcEnvironment(ssmpHeap,
            cliGlobals->getEventConsumed(), FALSE, IPC_SQLSSMP_SERVER,
             FALSE, TRUE);

  SsmpGlobals *ssmpGlobals = new (ssmpHeap) SsmpGlobals(ssmpHeap, ssmpIpcEnv,
                                                        statsGlobals);

  // Currently open $RECEIVE with 2048
  SsmpGuaReceiveControlConnection *cc =
	 new (ssmpHeap) SsmpGuaReceiveControlConnection(ssmpIpcEnv,
					  ssmpGlobals,
					  2048);

  ssmpIpcEnv->setControlConnection(cc);

  while (TRUE)
  {

/*
 * Until ssmp starts receiving messages, disable this check.
 * We need ssmp to wake up periodically to perform garbage collection.
 *
    // wait for the first open message to come in
    while (cc->getConnection() == NULL)
      cc->wait(IpcInfiniteTimeout);

    // start the first receive operation
#ifdef _DEBUG_RTS
    cerr << "No. of Requesters-1 "  << cc->getNumRequestors() << " \n";
#endif
    while (cc->getNumRequestors() > 0)
    for (;;)
    {
      ssmpGlobals->work();
    }
  }
*/
    // wait for system messages only until ssmp starts receiving msgs.
    cc->wait(300);
    // go do GC.
    ssmpGlobals->work();
  }
}

