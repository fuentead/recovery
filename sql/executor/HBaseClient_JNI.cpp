// **********************************************************************
// @@@ START COPYRIGHT @@@
//
// (C) Copyright 2013-2014 Hewlett-Packard Development Company, L.P.
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
// **********************************************************************

#include "Context.h"
#include "Globals.h"
#include "HBaseClient_JNI.h"
#include "HdfsLogger.h"
#include <signal.h>
#include "pthread.h"
// ===========================================================================
// ===== Class ByteArrayList
// ===========================================================================

JavaMethodInit* ByteArrayList::JavaMethods_ = NULL;
jclass ByteArrayList::javaClass_ = 0;
bool ByteArrayList::javaMethodsInitialized_ = false;
pthread_mutex_t ByteArrayList::javaMethodsInitMutex_ = PTHREAD_MUTEX_INITIALIZER;

static const char* const balErrorEnumStr[] = 
{
  "JNI NewStringUTF() in add() for writing."  // BAL_ERROR_ADD_PARAM
 ,"Java exception in add() for writing."      // BAL_ERROR_ADD_EXCEPTION
 ,"Java exception in get() for reading."      // BAL_ERROR_GET_EXCEPTION
};

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
char* ByteArrayList::getErrorText(BAL_RetCode errEnum)
{
  if (errEnum < (BAL_RetCode)JOI_LAST)
    return JavaObjectInterface::getErrorText((JOI_RetCode)errEnum);
  else    
    return (char*)balErrorEnumStr[errEnum-BAL_FIRST-1];
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
ByteArrayList::~ByteArrayList()
{
//  HdfsLogger::log(CAT_JNI_TOP, LL_DEBUG, "ByteArrayList destructor called.");
}
 
//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
BAL_RetCode ByteArrayList::init()
{
  static char className[]="org/trafodion/sql/HBaseAccess/ByteArrayList";
  BAL_RetCode rc;
  
  if (isInitialized())
    return BAL_OK;
    
  if (javaMethodsInitialized_)
    return (BAL_RetCode)JavaObjectInterface::init(className, javaClass_, JavaMethods_, (Int32)JM_LAST, javaMethodsInitialized_);
  else
  {
    pthread_mutex_lock(&javaMethodsInitMutex_);
    if (javaMethodsInitialized_)
    {
      pthread_mutex_unlock(&javaMethodsInitMutex_);
      return (BAL_RetCode)JavaObjectInterface::init(className, javaClass_, JavaMethods_, (Int32)JM_LAST, javaMethodsInitialized_);
    }
    JavaMethods_ = new JavaMethodInit[JM_LAST];
    
    JavaMethods_[JM_CTOR      ].jm_name      = "<init>";
    JavaMethods_[JM_CTOR      ].jm_signature = "()V";
    JavaMethods_[JM_ADD       ].jm_name      = "addElement";
    JavaMethods_[JM_ADD       ].jm_signature = "([B)V";
    JavaMethods_[JM_GET       ].jm_name      = "getElement";
    JavaMethods_[JM_GET       ].jm_signature = "(I)[B";
    JavaMethods_[JM_GETSIZE   ].jm_name      = "getSize";
    JavaMethods_[JM_GETSIZE   ].jm_signature = "()I";
    JavaMethods_[JM_GETENTRYSIZE].jm_name      = "getEntrySize";
    JavaMethods_[JM_GETENTRYSIZE].jm_signature = "(I)I";
    JavaMethods_[JM_GETENTRY  ].jm_name      = "getEntry";
    JavaMethods_[JM_GETENTRY].jm_signature = "(I)[B";
    
    rc = (BAL_RetCode)JavaObjectInterface::init(className, javaClass_, JavaMethods_, (Int32)JM_LAST, javaMethodsInitialized_);
    javaMethodsInitialized_ = TRUE;
    pthread_mutex_unlock(&javaMethodsInitMutex_);
  }
  return rc;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
BAL_RetCode ByteArrayList::add(const Text& t)
{
//  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "ByteArrayList::add(%s) called.", t.data());

  int len = t.size();
  jbyteArray jba_t = jenv_->NewByteArray(len);
  if (jba_t == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(BAL_ERROR_ADD_PARAM));
    return BAL_ERROR_ADD_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_t, 0, len, (const jbyte*)t.data());

  // void add(byte[]);
  jenv_->CallVoidMethod(javaObj_, JavaMethods_[JM_ADD].methodID, jba_t);
  jenv_->DeleteLocalRef(jba_t);  
  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    return BAL_ERROR_ADD_EXCEPTION;
  }

  return BAL_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
BAL_RetCode ByteArrayList::add(const TextVec& vec)
{
  for (std::vector<Text>::const_iterator it = vec.begin() ; it != vec.end(); ++it)
  {
    Text str(*it);
    add(str);
  }

  return BAL_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
BAL_RetCode ByteArrayList::addElement(const char* data, int keyLength)
{
  Text str(data, keyLength);
  add(str);
  return BAL_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
Text* ByteArrayList::get(Int32 i)
{
  // byte[] get(i);
  jbyteArray jba_val = static_cast<jbyteArray>(jenv_->CallObjectMethod(javaObj_, JavaMethods_[JM_GET].methodID, i));
  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    return NULL;
  }

  if (jba_val == NULL)
    return NULL;

  jbyte* p_val = jenv_->GetByteArrayElements(jba_val, 0);
  int len = jenv_->GetArrayLength(jba_val);
  Text* val = new (heap_) Text((char*)p_val, len); 
  jenv_->ReleaseByteArrayElements(jba_val, p_val, JNI_ABORT);
  jenv_->DeleteLocalRef(jba_val);  

  return val;
}

Int32 ByteArrayList::getSize()
{
  Int32 len = jenv_->CallIntMethod(javaObj_, JavaMethods_[JM_GETSIZE].methodID);
  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    return 0;
  }
  return len;
}

Int32 ByteArrayList::getEntrySize(Int32 i)
{
  jint jidx = i;  

  Int32 len = 
    jenv_->CallIntMethod(javaObj_, JavaMethods_[JM_GETENTRYSIZE].methodID, jidx);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    return 0;
  }
  return len;
}


char* ByteArrayList::getEntry(Int32 i, char* buf, Int32 bufLen, Int32& datalen)
{
  datalen = 0;

  jint jidx = i;  

  jbyteArray jBuffer = static_cast<jbyteArray>
      (jenv_->CallObjectMethod(javaObj_, JavaMethods_[JM_GETENTRY].methodID, jidx));

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    return NULL;
  }

  if (jBuffer != NULL) {

    datalen = jenv_->GetArrayLength(jBuffer);

    if (datalen > bufLen)
      // call setJniErrorStr?
      return NULL;

    jenv_->GetByteArrayRegion(jBuffer, 0, datalen, (jbyte*)buf);

    jenv_->DeleteLocalRef(jBuffer);
  }

  return buf;
}
// ===========================================================================
// ===== Class HBaseClient_JNI
// ===========================================================================

JavaMethodInit* HBaseClient_JNI::JavaMethods_ = NULL;
jclass HBaseClient_JNI::javaClass_ = 0;
bool HBaseClient_JNI::javaMethodsInitialized_ = false;
pthread_mutex_t HBaseClient_JNI::javaMethodsInitMutex_ = PTHREAD_MUTEX_INITIALIZER;

// Keep in sync with HBC_RetCode enum.
static const char* const hbcErrorEnumStr[] = 
{
  "Preparing parameters for initConnection()."
 ,"Java exception in initConnection()."
 ,"Java exception in cleanup()."
 ,"Java exception in getHTableClient()."
 ,"Java exception in releaseHTableClient()."
 ,"Preparing parameters for create()."
 ,"Java exception in create()."
 ,"Preparing parameters for drop()."
 ,"Java exception in drop()."
 // No corresponding HBC_RetCode values for next 2 at present.
 //,"Preparing parameters for dropAll()."
 //,"Java exception in dropAll()."
 ,"Preparing parameters for exists()."
 ,"Java exception in exists()."
 ,"Preparing parameters for flushAll()."
 ,"Java exception in flushAll()." 
 ,"Preparing parameters for grant()."
 ,"Java exception in grant()."
 ,"Preparing parameters for revoke()."
 ,"Java exception in revoke()."
 ,"Error in Thread Create"
 ,"Error in Thread Req Alloc"
 ,"Error in Thread SIGMAS"
 ,"Error in Attach JVM"
 ,"Java exception in getHBulkLoadClient()."
 ,"Preparing parameters for estimateRowCount()."
 ,"Java exception in estimateRowCount()."
 ,"Java exception in releaseHBulkLoadClient()."
};

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
// private default constructor
HBaseClient_JNI::HBaseClient_JNI(NAHeap *heap, int debugPort, int debugTimeout)
                 :  JavaObjectInterface(heap, debugPort, debugTimeout)
                   ,isConnected_(FALSE)
{
  for (int i=0; i<NUM_HBASE_WORKER_THREADS; i++) {
    threadID_[i] = NULL;
  }
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
char* HBaseClient_JNI::getErrorText(HBC_RetCode errEnum)
{
  if (errEnum < (HBC_RetCode)JOI_LAST)
    return JavaObjectInterface::getErrorText((JOI_RetCode)errEnum);
  else    
    return (char*)hbcErrorEnumStr[errEnum-HBC_FIRST-1];
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HBaseClient_JNI* HBaseClient_JNI::getInstance(int debugPort, int debugTimeout)
{
   ContextCli *currContext = GetCliGlobals()->currContext();
   HBaseClient_JNI *hbaseClient_JNI = currContext->getHBaseClient();
   if (hbaseClient_JNI == NULL)
   {
     NAHeap *heap = currContext->exHeap();
    
     hbaseClient_JNI  = new (heap) HBaseClient_JNI(heap,
                   debugPort, debugTimeout);
     currContext->setHbaseClient(hbaseClient_JNI);
   }
   return hbaseClient_JNI;
}

void HBaseClient_JNI::deleteInstance()
{
   ContextCli *currContext = GetCliGlobals()->currContext();
   HBaseClient_JNI *hbaseClient_JNI = currContext->getHBaseClient();
   if (hbaseClient_JNI != NULL)
   {
      NAHeap *heap = currContext->exHeap();
      NADELETE(hbaseClient_JNI, HBaseClient_JNI, heap);
      currContext->setHbaseClient(NULL);
   }
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HBaseClient_JNI::~HBaseClient_JNI()
{
  //HdfsLogger::log(CAT_JNI_TOP, LL_DEBUG, "HBaseClient_JNI destructor called.");
  
  // worker threads need to go away and be joined. 
  if (threadID_[0])
  {
    // tell the worker threads to go away
    for (int i=0; i<NUM_HBASE_WORKER_THREADS; i++) {
      enqueueShutdownRequest(); 
    }

    // wait for worker threads to exit and join
    for (int i=0; i<NUM_HBASE_WORKER_THREADS; i++) {
      pthread_join(threadID_[i], NULL);
    }

    pthread_mutex_destroy(&mutex_);
    pthread_cond_destroy(&workBell_);
  }
  // Clean the Java Side
  cleanup();
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HBC_RetCode HBaseClient_JNI::init()
{
  static char className[]="org/trafodion/sql/HBaseAccess/HBaseClient";
  HBC_RetCode rc;
  
  if (isInitialized())
    return HBC_OK;
  
  if (javaMethodsInitialized_)
    return (HBC_RetCode)JavaObjectInterface::init(className, javaClass_, JavaMethods_, (Int32)JM_LAST, javaMethodsInitialized_);
  else
  {
    pthread_mutex_lock(&javaMethodsInitMutex_);
    if (javaMethodsInitialized_)
    {
      pthread_mutex_unlock(&javaMethodsInitMutex_);
      return (HBC_RetCode)JavaObjectInterface::init(className, javaClass_, JavaMethods_, (Int32)JM_LAST, javaMethodsInitialized_);
    }
    JavaMethods_ = new JavaMethodInit[JM_LAST];
    
    JavaMethods_[JM_CTOR       ].jm_name      = "<init>";
    JavaMethods_[JM_CTOR       ].jm_signature = "()V";
    JavaMethods_[JM_GET_ERROR  ].jm_name      = "getLastError";
    JavaMethods_[JM_GET_ERROR  ].jm_signature = "()Ljava/lang/String;";
    JavaMethods_[JM_INIT       ].jm_name      = "init";
    JavaMethods_[JM_INIT       ].jm_signature = "(Ljava/lang/String;Ljava/lang/String;)Z";
    JavaMethods_[JM_CLEANUP    ].jm_name      = "cleanup";
    JavaMethods_[JM_CLEANUP    ].jm_signature = "()Z";
    JavaMethods_[JM_GET_HTC    ].jm_name      = "getHTableClient";
    JavaMethods_[JM_GET_HTC    ].jm_signature = "(Ljava/lang/String;Z)Lorg/trafodion/sql/HBaseAccess/HTableClient;";
    JavaMethods_[JM_REL_HTC    ].jm_name      = "releaseHTableClient";
    JavaMethods_[JM_REL_HTC    ].jm_signature = "(Lorg/trafodion/sql/HBaseAccess/HTableClient;)V";
    JavaMethods_[JM_CREATE     ].jm_name      = "create";
    JavaMethods_[JM_CREATE     ].jm_signature = "(Ljava/lang/String;[Ljava/lang/Object;)Z";
    JavaMethods_[JM_CREATEK    ].jm_name      = "createk";
    JavaMethods_[JM_CREATEK    ].jm_signature = "(Ljava/lang/String;[Ljava/lang/Object;[Ljava/lang/Object;)Z";
    JavaMethods_[JM_DROP       ].jm_name      = "drop";
    JavaMethods_[JM_DROP       ].jm_signature = "(Ljava/lang/String;)Z";
    JavaMethods_[JM_DROP_ALL       ].jm_name      = "dropAll";
    JavaMethods_[JM_DROP_ALL       ].jm_signature = "(Ljava/lang/String;)Z";
    JavaMethods_[JM_COPY       ].jm_name      = "copy";
    JavaMethods_[JM_COPY       ].jm_signature = "(Ljava/lang/String;Ljava/lang/String;)Z";
    JavaMethods_[JM_EXISTS     ].jm_name      = "exists";
    JavaMethods_[JM_EXISTS     ].jm_signature = "(Ljava/lang/String;)Z";
    JavaMethods_[JM_GRANT      ].jm_name      = "grant";
    JavaMethods_[JM_GRANT      ].jm_signature = "([B[B[Ljava/lang/Object;)Z";
    JavaMethods_[JM_REVOKE     ].jm_name      = "revoke";
    JavaMethods_[JM_REVOKE     ].jm_signature = "([B[B[Ljava/lang/Object;)Z";
    JavaMethods_[JM_FLUSHALL   ].jm_name      = "flushAllTables";
    JavaMethods_[JM_FLUSHALL   ].jm_signature = "()Z";
    JavaMethods_[JM_GET_HBLC   ].jm_name      = "getHBulkLoadClient";
    JavaMethods_[JM_GET_HBLC   ].jm_signature = "()Lorg/trafodion/sql/HBaseAccess/HBulkLoadClient;";
    JavaMethods_[JM_EST_RC     ].jm_name      = "estimateRowCount";
    JavaMethods_[JM_EST_RC     ].jm_signature = "(Ljava/lang/String;II[J)Z";
    JavaMethods_[JM_REL_HBLC   ].jm_name      = "releaseHBulkLoadClient";
    JavaMethods_[JM_REL_HBLC   ].jm_signature = "(Lorg/trafodion/sql/HBaseAccess/HBulkLoadClient;)V";
   
    rc = (HBC_RetCode)JavaObjectInterface::init(className, javaClass_, JavaMethods_, (Int32)JM_LAST, javaMethodsInitialized_);
    javaMethodsInitialized_ = TRUE;
    pthread_mutex_unlock(&javaMethodsInitMutex_);
  }
  return rc;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
NAString HBaseClient_JNI::getLastJavaError()
{
  return JavaObjectInterface::getLastJavaError(JavaMethods_[JM_GET_ERROR].methodID);
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HBC_RetCode HBaseClient_JNI::initConnection(const char* zkServers, const char* zkPort)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBaseClient_JNI::initConnection(%s, %s) called.", zkServers, zkPort);

  jstring js_zkServers = jenv_->NewStringUTF(zkServers);
  if (js_zkServers == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_INIT_PARAM));
    return HBC_ERROR_INIT_PARAM;
  }
  jstring js_zkPort = jenv_->NewStringUTF(zkPort);
  if (js_zkPort == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_INIT_PARAM));
    return HBC_ERROR_INIT_PARAM;
  }

  // boolean init(java.lang.String, java.lang.String); 
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_INIT].methodID, js_zkServers, js_zkPort);

  jenv_->DeleteLocalRef(js_zkServers);  
  jenv_->DeleteLocalRef(js_zkPort);  

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::initConnection()", getLastError());
    return HBC_ERROR_INIT_EXCEPTION;
  }

  if (jresult == false) 
  {
    logError(CAT_HBASE, "HBaseClient_JNI::initConnection()", getLastError());
    return HBC_ERROR_INIT_EXCEPTION;
  }

  isConnected_ = TRUE;
  return HBC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HBC_RetCode HBaseClient_JNI::cleanup()
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBaseClient_JNI::cleanup() called.");
 
  if (! (isInitialized_ && isConnected_))
     return HBC_OK;

  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HBC_ERROR_INIT_PARAM;
  // boolean cleanup();
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_CLEANUP].methodID);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::cleanup()", getLastError());
    return HBC_ERROR_CLEANUP_EXCEPTION;
  }

  if (jresult == false) 
  {
    logError(CAT_HBASE, "HBaseClient_JNI::cleanup()", getLastError());
    return HBC_ERROR_CLEANUP_EXCEPTION;
  }

  return HBC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HTableClient_JNI* HBaseClient_JNI::getHTableClient(NAHeap *heap, const char* tableName, bool useTRex, ExHbaseAccessStats *hbs)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBaseClient_JNI::getHTableClient(%s) called.", tableName);

  if (javaObj_ == NULL || (!isInitialized()))
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_GET_HTC_EXCEPTION));
    return NULL;
  }

  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return NULL;

  jstring js_tblName = jenv_->NewStringUTF(tableName);
  if (js_tblName == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_GET_HTC_EXCEPTION));
    return NULL;
  }

  jobject j_htc = jenv_->CallObjectMethod(javaObj_, JavaMethods_[JM_GET_HTC].methodID, js_tblName, (jboolean)useTRex);

  jenv_->DeleteLocalRef(js_tblName); 

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::getHTableClient()", getLastError());
    return NULL;
  }

  if (j_htc == NULL) 
  {
    logError(CAT_HBASE, "HBaseClient_JNI::getHTableClient()", getLastError());
    return NULL;
  }
  HTableClient_JNI *htc = new (heap) HTableClient_JNI(heap, j_htc);
  jenv_->DeleteLocalRef(j_htc);
  if (htc->init() != HTC_OK)
  {
     NADELETE(htc, HTableClient_JNI, heap);
     return NULL;
  }
  htc->setTableName(tableName);
  if (htc->setJniObject() != HTC_OK)
     return NULL; 
  htc->setHbaseStats(hbs);
  return htc;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HBC_RetCode HBaseClient_JNI::releaseHTableClient(HTableClient_JNI* htc)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBaseClient_JNI::releaseHTableClient() called.");

  jobject j_htc = htc->getJavaObject();

  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HBC_ERROR_INIT_PARAM;
    
  jenv_->CallVoidMethod(javaObj_, JavaMethods_[JM_REL_HTC].methodID, j_htc);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::releaseHTableClient()", getLastError());
    return HBC_ERROR_REL_HTC_EXCEPTION;
  }
  NADELETE(htc, HTableClient_JNI, htc->getHeap()); 

  return HBC_OK;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
HBulkLoadClient_JNI* HBaseClient_JNI::getHBulkLoadClient(NAHeap *heap)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBaseClient_JNI::getHBulkLoadClient() called.");
  if (javaObj_ == NULL || (!isInitialized()))
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_GET_HBLC_EXCEPTION));
    return NULL;
  }

  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return NULL;

  jobject j_hblc = jenv_->CallObjectMethod(javaObj_, JavaMethods_[JM_GET_HBLC].methodID);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::getHBulkLoadClient()", getLastError());
    return NULL;
  }

  if (j_hblc == NULL)
  {
    logError(CAT_HBASE, "HBaseClient_JNI::getHBulkLoadClient()", getLastError());
    return NULL;
  }
  HBulkLoadClient_JNI *hblc = new (heap) HBulkLoadClient_JNI(heap, j_hblc);
  jenv_->DeleteLocalRef(j_hblc);
  if ( hblc->init()!= HBLC_OK)
  {
     NADELETE(hblc, HBulkLoadClient_JNI, heap);
     return NULL; 
  }
  return hblc;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HBC_RetCode HBaseClient_JNI::releaseHBulkLoadClient(HBulkLoadClient_JNI* hblc)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBaseClient_JNI::releaseHBulkLOadClient() called.");

  jobject j_hblc = hblc->getJavaObject();

  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HBC_ERROR_INIT_PARAM;

  jenv_->CallVoidMethod(javaObj_, JavaMethods_[JM_REL_HBLC].methodID, j_hblc);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::releaseHBulkLOadClient()", getLastError());
    return HBC_ERROR_REL_HBLC_EXCEPTION;
  }
  NADELETE(hblc, HBulkLoadClient_JNI, hblc->getHeap());

  return HBC_OK;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
HBC_RetCode HBaseClient_JNI::create(const char* fileName, HBASE_NAMELIST& colFamilies)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBaseClient_JNI::create(%s) called.", fileName);
  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HBC_ERROR_INIT_PARAM;
  jstring js_fileName = jenv_->NewStringUTF(fileName);
  if (js_fileName == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_CREATE_PARAM));
    return HBC_ERROR_CREATE_PARAM;
  }
  jobjectArray j_fams = convertToStringObjectArray(colFamilies);
  if (j_fams == NULL)
  {
     getExceptionDetails();
     logError(CAT_HBASE, __FILE__, __LINE__);
     logError(CAT_HBASE, "HBaseClient_JNI::create()", getLastError());
     jenv_->DeleteLocalRef(js_fileName); 
     return HBC_ERROR_CREATE_PARAM;
  }
    
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, 
        JavaMethods_[JM_CREATE].methodID, js_fileName, j_fams);

  jenv_->DeleteLocalRef(js_fileName); 
  jenv_->DeleteLocalRef(j_fams);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::create()", getLastError());
    return HBC_ERROR_CREATE_EXCEPTION;
  }

  if (jresult == false) 
  {
    logError(CAT_HBASE, "HBaseClient_JNI::create()", getLastError());
    return HBC_ERROR_CREATE_EXCEPTION;
  }

  return HBC_OK;
}


//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HBC_RetCode HBaseClient_JNI::create(const char* fileName, 
                                    NAText* createOptionsArray,
                                    int numSplits, int keyLength,
                                    const char ** splitValues)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBaseClient_JNI::create(%s) called.", fileName);
  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HBC_ERROR_INIT_PARAM;
  jstring js_fileName = jenv_->NewStringUTF(fileName);
  if (js_fileName == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_CREATE_PARAM));
    return HBC_ERROR_CREATE_PARAM;
  }
  jobjectArray j_opts = convertToStringObjectArray(createOptionsArray, 
                   HBASE_MAX_OPTIONS);
  if (j_opts == NULL)
  {
     getExceptionDetails();
     logError(CAT_HBASE, __FILE__, __LINE__);
     logError(CAT_HBASE, "HBaseClient_JNI::create()", getLastError());
     jenv_->DeleteLocalRef(js_fileName); 
     return HBC_ERROR_CREATE_PARAM;
  }

  jobjectArray j_keys = NULL;    
  if (numSplits > 0)
  {
     j_keys = convertToByteArrayObjectArray(splitValues, numSplits, keyLength);
     if (j_keys == NULL)
     {
        getExceptionDetails();
        logError(CAT_HBASE, __FILE__, __LINE__);
        logError(CAT_HBASE, "HBaseClient_JNI::create()", getLastError());
        jenv_->DeleteLocalRef(js_fileName); 
        jenv_->DeleteLocalRef(j_opts);
        return HBC_ERROR_CREATE_PARAM;
     }
  }
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, 
          JavaMethods_[JM_CREATEK].methodID, js_fileName, j_opts, j_keys);

  jenv_->DeleteLocalRef(js_fileName); 
  jenv_->DeleteLocalRef(j_opts);
  if (j_keys != NULL)
     jenv_->DeleteLocalRef(j_keys);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::create()", getLastError());
    return HBC_ERROR_CREATE_EXCEPTION;
  }

  if (jresult == false) 
  {
    logError(CAT_HBASE, "HBaseClient_JNI::create()", getLastError());
    return HBC_ERROR_CREATE_EXCEPTION;
  }

  return HBC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HBaseClientRequest::HBaseClientRequest(NAHeap *heap, HBaseClientReqType reqType)
                    :  heap_(heap)
                      ,reqType_(reqType)
                      ,fileName_(NULL)
{
}

HBaseClientRequest::~HBaseClientRequest()
{
  if (fileName_) {
    NADELETEBASIC(fileName_, heap_);
  }
}

void HBaseClientRequest::setFileName(const char *fileName)
{
  int len = strlen(fileName);
  fileName_ = new (heap_) char[len + 1];
  strcpy(fileName_, fileName);
}

HBC_RetCode HBaseClient_JNI::enqueueRequest(HBaseClientRequest *request)
{
  pthread_mutex_lock( &mutex_ );
  reqQueue_.push_back(request);
  pthread_cond_signal(&workBell_);
  pthread_mutex_unlock( &mutex_ );

  return HBC_OK;
}

HBC_RetCode HBaseClient_JNI::enqueueDropRequest(const char* fileName)
{
  HBaseClientRequest *request = new (heap_) HBaseClientRequest(heap_, HBC_Req_Drop);

  if (!request) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_THREAD_REQ_ALLOC));
    return HBC_ERROR_THREAD_REQ_ALLOC; 
  }

  request->setFileName(fileName);

  enqueueRequest(request);

  return HBC_OK;
}

HBC_RetCode HBaseClient_JNI::enqueueShutdownRequest()
{
  HBaseClientRequest *request = new (heap_) HBaseClientRequest(heap_, HBC_Req_Shutdown);

  if (!request) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_THREAD_REQ_ALLOC));
    return HBC_ERROR_THREAD_REQ_ALLOC;
  }

  enqueueRequest(request);

  return HBC_OK;
}

HBaseClientRequest* HBaseClient_JNI::getHBaseRequest() 
{
  HBaseClientRequest *request;
  reqList_t::iterator it;

  pthread_mutex_lock( &mutex_ );
  it = reqQueue_.begin();

  request = NULL;

  while (request == NULL)
  {
    if (it != reqQueue_.end())
    {
      request = *it;
      it = reqQueue_.erase(it);
    } else {
      pthread_cond_wait(&workBell_, &mutex_);
      it = reqQueue_.begin();
    }
  }

  pthread_mutex_unlock( &mutex_ );

  return request;
}

HBC_RetCode HBaseClient_JNI::performRequest(HBaseClientRequest *request, JNIEnv* jenv)
{
  switch (request->reqType_)
  {
    case HBC_Req_Drop :
	  drop(request->fileName_, jenv);
	  break;
	default :
	  break;
  }

  return HBC_OK;
}

HBC_RetCode HBaseClient_JNI::doWorkInThread() 
{
  int rc;

  HBaseClientRequest *request;

  // mask all signals
  sigset_t mask;
  sigfillset(&mask);
  rc = pthread_sigmask(SIG_BLOCK, &mask, NULL);
  if (rc)
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_THREAD_SIGMASK));
    return HBC_ERROR_THREAD_SIGMASK;
  }

  JNIEnv* jenv; // thread local
  jint result = jvm_->GetEnv((void**) &jenv, JNI_VERSION_1_6);
  switch (result)
  {
    case JNI_OK:
	  break;

	case JNI_EDETACHED:
	  result = jvm_->AttachCurrentThread((void**) &jenv, NULL);
          if (result != JNI_OK)
          {
             GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_ATTACH_JVM));
	     return HBC_ERROR_ATTACH_JVM;
          }
      break;

	default: 
	  break;
  }

  // enter processing zone
  for (;;)
  {
    request = getHBaseRequest();

	if (request->isShutDown()) {
	  // wake up another thread as this wakeup could have consumed
	  // multiple workBell_ rings.
	  pthread_cond_signal(&workBell_);
	  break;
	} else {
      performRequest(request, jenv);
	  NADELETE(request, HBaseClientRequest, request->getHeap());
	}
  }

  jvm_->DetachCurrentThread();

  pthread_exit(0);

  return HBC_OK;
}

static void *workerThreadMain_JNI(void *arg)
{
  // parameter passed to the thread is an instance of the HBaseClient_JNI object
  HBaseClient_JNI *client = (HBaseClient_JNI *)arg;

  client->doWorkInThread();

  return NULL;
}

HBC_RetCode HBaseClient_JNI::startWorkerThreads()
{
  pthread_mutexattr_t mutexAttr;
  pthread_mutexattr_init( &mutexAttr );
  pthread_mutex_init( &mutex_, &mutexAttr );
  pthread_cond_init( &workBell_, NULL );

  int rc;
  for (int i=0; i<NUM_HBASE_WORKER_THREADS; i++) {
    rc = pthread_create(&threadID_[i], NULL, workerThreadMain_JNI, this);
    if (rc != 0)
    {
       GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_THREAD_CREATE));
       return HBC_ERROR_THREAD_CREATE;
    }
  }

  return HBC_OK;
}


//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HBC_RetCode HBaseClient_JNI::drop(const char* fileName, bool async)
{
  if (async) {
    if (!threadID_[0]) {
	  startWorkerThreads();
	}
    enqueueDropRequest(fileName);
  } else {
    return drop(fileName, jenv_); // not in worker thread
  }

  return HBC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HBC_RetCode HBaseClient_JNI::flushAllTablesStatic()
{
  return GetCliGlobals()->currContext()->getHBaseClient()->flushAllTables();
}

HBC_RetCode HBaseClient_JNI::flushAllTables()
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBaseClient_JNI::flushAllTablescalled.");
  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HBC_ERROR_INIT_PARAM;

  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_FLUSHALL].methodID);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::flushAllTables()", getLastError());
    return HBC_ERROR_FLUSHALL_EXCEPTION;
  }

  if (jresult == false) 
  {
    logError(CAT_HBASE, "HBaseClient_JNI::flushAllTables()", getLastError());
    return HBC_ERROR_FLUSHALL_EXCEPTION;
  }

  return HBC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HBC_RetCode HBaseClient_JNI::drop(const char* fileName, JNIEnv* jenv)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBaseClient_JNI::drop(%s) called.", fileName);
  jstring js_fileName = jenv->NewStringUTF(fileName);
  if (js_fileName == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_DROP_PARAM));
    return HBC_ERROR_DROP_PARAM;
  }

  // boolean drop(java.lang.String);
  jboolean jresult = jenv->CallBooleanMethod(javaObj_, JavaMethods_[JM_DROP].methodID, js_fileName);

  jenv->DeleteLocalRef(js_fileName);  

  if (jenv->ExceptionCheck())
  {
    getExceptionDetails(jenv);
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::drop()", getLastError());
    return HBC_ERROR_DROP_EXCEPTION;
  }

  if (jresult == false) 
  {
    logError(CAT_HBASE, "HBaseClient_JNI::drop()", getLastJavaError());
    return HBC_ERROR_DROP_EXCEPTION;
  }

  return HBC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HBC_RetCode HBaseClient_JNI::dropAll(const char* pattern, bool async)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBaseClient_JNI::dropAll(%s) called.", pattern);

  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HBC_ERROR_INIT_PARAM;
  if (async) {
    // not supported yet.
    return HBC_ERROR_DROP_EXCEPTION;
  }

  jstring js_pattern = jenv_->NewStringUTF(pattern);
  if (js_pattern == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_DROP_PARAM));
    return HBC_ERROR_DROP_PARAM;
  }

  // boolean drop(java.lang.String);
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_DROP_ALL].methodID, js_pattern);

  jenv_->DeleteLocalRef(js_pattern);  

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails(jenv_);
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::dropAll()", getLastError());
    return HBC_ERROR_DROP_EXCEPTION;
  }

  if (jresult == false) 
  {
    logError(CAT_HBASE, "HBaseClient_JNI::dropAll()", getLastJavaError());
    return HBC_ERROR_DROP_EXCEPTION;
  }

  return HBC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HBC_RetCode HBaseClient_JNI::copy(const char* currTblName, const char* oldTblName)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBaseClient_JNI::copy(%s,%s) called.", currTblName, oldTblName);
  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HBC_ERROR_INIT_PARAM;

  jstring js_currTblName = jenv_->NewStringUTF(currTblName);
  if (js_currTblName == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_DROP_PARAM));
    return HBC_ERROR_DROP_PARAM;
  }

  jstring js_oldTblName = jenv_->NewStringUTF(oldTblName);
  if (js_oldTblName == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_DROP_PARAM));
    return HBC_ERROR_DROP_PARAM;
  }

  // boolean drop(java.lang.String);
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_COPY].methodID, js_currTblName, js_oldTblName);

  jenv_->DeleteLocalRef(js_currTblName);  

  jenv_->DeleteLocalRef(js_oldTblName);  

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails(jenv_);
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::copy()", getLastError());
    return HBC_ERROR_DROP_EXCEPTION;
  }

  if (jresult == false) 
  {
    logError(CAT_HBASE, "HBaseClient_JNI::copy()", getLastError());
    return HBC_ERROR_DROP_EXCEPTION;
  }

  return HBC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HBC_RetCode HBaseClient_JNI::exists(const char* fileName)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBaseClient_JNI::exists(%s) called.", fileName);
  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HBC_ERROR_INIT_PARAM;
  jstring js_fileName = jenv_->NewStringUTF(fileName);
  if (js_fileName == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_EXISTS_PARAM));
    return HBC_ERROR_EXISTS_PARAM;
  }

  // boolean exists(java.lang.String);
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_EXISTS].methodID, js_fileName);

  jenv_->DeleteLocalRef(js_fileName);  

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::exists()", getLastError());
    return HBC_ERROR_EXISTS_EXCEPTION;
  }

  if (jresult == false) 
     return HBC_DONE;  // Table does not exist

  return HBC_OK;  // Table exists.
}


//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HBC_RetCode HBaseClient_JNI::grant(const Text& user, const Text& tblName, const TextVec& actions)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBaseClient_JNI::grant(%s, %s, %s) called.", user.data(), tblName.data(), actions.data());
  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HBC_ERROR_INIT_PARAM;

  int len = user.size();
  jbyteArray jba_user = jenv_->NewByteArray(len);
  if (jba_user == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_GRANT_PARAM));
    return HBC_ERROR_GRANT_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_user, 0, len, (const jbyte*)user.data());

  len = tblName.size();
  jbyteArray jba_tblName = jenv_->NewByteArray(len);
  if (jba_tblName == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_GRANT_PARAM));
    jenv_->DeleteLocalRef(jba_user);  
    return HBC_ERROR_GRANT_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_tblName, 0, len, (const jbyte*)tblName.data());

  jobjectArray j_actionCodes = NULL;
  if (!actions.empty())
  {
    HdfsLogger::log(CAT_HBASE, LL_DEBUG, "  Adding %d actions.", actions.size());
    j_actionCodes = convertToStringObjectArray(actions);
    if (j_actionCodes == NULL)
    {
       getExceptionDetails();
       logError(CAT_HBASE, __FILE__, __LINE__);
       logError(CAT_HBASE, "HBaseClient_JNI::grant()", getLastError());
       jenv_->DeleteLocalRef(jba_user);  
       jenv_->DeleteLocalRef(jba_tblName);  
       return HBC_ERROR_GRANT_PARAM;
    }
  }
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, 
       JavaMethods_[JM_GRANT].methodID, jba_user, jba_tblName, j_actionCodes);

  jenv_->DeleteLocalRef(jba_user);  
  jenv_->DeleteLocalRef(jba_tblName);  
  if (j_actionCodes != NULL)
     jenv_->DeleteLocalRef(j_actionCodes);  


  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::grant()", getLastError());
    return HBC_ERROR_GRANT_EXCEPTION;
  }

  if (jresult == false) 
  {
    logError(CAT_HBASE, "HBaseClient_JNI::grant()", getLastError());
    return HBC_ERROR_GRANT_EXCEPTION;
  }

  return HBC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Estimate row count for tblName by adding the entry counts from the trailer
// block of each HFile for the table, and dividing by the number of columns.
//////////////////////////////////////////////////////////////////////////////
HBC_RetCode HBaseClient_JNI::estimateRowCount(const char* tblName,
                                              Int32 partialRowSize,
                                              Int32 numCols,
                                              Int64& rowCount)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBaseClient_JNI::estimateRowCount(%s) called.", tblName);
  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HBC_ERROR_INIT_PARAM;
  jstring js_tblName = jenv_->NewStringUTF(tblName);
  if (js_tblName == NULL)
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_ROWCOUNT_EST_PARAM));
    return HBC_ERROR_ROWCOUNT_EST_PARAM;
  }

  jint jPartialRowSize = partialRowSize;
  jint jNumCols = numCols;
  jlongArray jRowCount = jenv_->NewLongArray(1);
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_EST_RC].methodID,
                                              js_tblName, jPartialRowSize,
                                              jNumCols, jRowCount);
  jboolean isCopy;
  jlong* arrayElems = jenv_->GetLongArrayElements(jRowCount, &isCopy);
  rowCount = *arrayElems;
  if (isCopy == JNI_TRUE)
    jenv_->ReleaseLongArrayElements(jRowCount, arrayElems, JNI_ABORT);

  jenv_->DeleteLocalRef(js_tblName);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::estimateRowCount()", getLastError());
    return HBC_ERROR_ROWCOUNT_EST_EXCEPTION;
  }

  if (jresult == false)
  {
    logError(CAT_HBASE, "HBaseClient_JNI::estimateRowCount()", getLastError());
    return HBC_ERROR_ROWCOUNT_EST_EXCEPTION;
  }

  return HBC_OK;  // Table exists.
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
JavaMethodInit* HBulkLoadClient_JNI::JavaMethods_ = NULL;
jclass HBulkLoadClient_JNI::javaClass_ = 0;
bool HBulkLoadClient_JNI::javaMethodsInitialized_ = false;
pthread_mutex_t HBulkLoadClient_JNI::javaMethodsInitMutex_ = PTHREAD_MUTEX_INITIALIZER;


static const char* const hblcErrorEnumStr[] = ///need to update content
{
    "preparing parameters for init."
   ,"java exception in init."
   ,"java exception in cleanup."
   ,"java exception in close"
   ,"java exception in create_hfile()."
   ,"java exception in create_hfile()."
   ,"preparing parameters for add_to_hfile()."
   ,"java exception in add_to_hfile()."
   ,"preparing parameters for hblc_error_close_hfile()."
   ,"java exception in close_hfile()."
   ,"java exception in do_bulkload()."
   ,"java exception in do_bulkload()."
   ,"preparing parameters for bulkload_cleanup()."
   ,"java exception in bulkload_cleanup()."
   ,"preparing parameters for init_hblc()."
   ,"java exception in init_hblc()."
};
HBLC_RetCode HBulkLoadClient_JNI::init()
{
  static char className[]="org/trafodion/sql/HBaseAccess/HBulkLoadClient";
  HBLC_RetCode rc;

  if (isInitialized())
    return HBLC_OK;

  if (javaMethodsInitialized_)
    return (HBLC_RetCode)JavaObjectInterface::init(className, javaClass_, JavaMethods_, (Int32)JM_LAST, javaMethodsInitialized_);
  else
  {
    pthread_mutex_lock(&javaMethodsInitMutex_);
    if (javaMethodsInitialized_)
    {
      pthread_mutex_unlock(&javaMethodsInitMutex_);
      return (HBLC_RetCode)JavaObjectInterface::init(className, javaClass_, JavaMethods_, (Int32)JM_LAST, javaMethodsInitialized_);
    }
    JavaMethods_ = new JavaMethodInit[JM_LAST];

    JavaMethods_[JM_CTOR       ].jm_name      = "<init>";
    JavaMethods_[JM_CTOR       ].jm_signature = "()V";
    JavaMethods_[JM_GET_ERROR  ].jm_name      = "getLastError";
    JavaMethods_[JM_GET_ERROR  ].jm_signature = "()Ljava/lang/String;";
    JavaMethods_[JM_INIT_HFILE_PARAMS     ].jm_name      = "initHFileParams";
    JavaMethods_[JM_INIT_HFILE_PARAMS     ].jm_signature = "(Ljava/lang/String;Ljava/lang/String;JLjava/lang/String;)Z";
    JavaMethods_[JM_CLOSE_HFILE      ].jm_name      = "closeHFile";
    JavaMethods_[JM_CLOSE_HFILE      ].jm_signature = "()Z";
    JavaMethods_[JM_DO_BULK_LOAD     ].jm_name      = "doBulkLoad";
    JavaMethods_[JM_DO_BULK_LOAD     ].jm_signature = "(Ljava/lang/String;Ljava/lang/String;ZZ)Z";
    JavaMethods_[JM_BULK_LOAD_CLEANUP].jm_name      = "bulkLoadCleanup";
    JavaMethods_[JM_BULK_LOAD_CLEANUP].jm_signature = "(Ljava/lang/String;)Z";
    JavaMethods_[JM_ADD_TO_HFILE_DB  ].jm_name      = "addToHFile";
    JavaMethods_[JM_ADD_TO_HFILE_DB  ].jm_signature = "(SLjava/lang/Object;Ljava/lang/Object;)Z";

    rc = (HBLC_RetCode)JavaObjectInterface::init(className, javaClass_, JavaMethods_, (Int32)JM_LAST, javaMethodsInitialized_);
    javaMethodsInitialized_ = TRUE;
    pthread_mutex_unlock(&javaMethodsInitMutex_);
  }
  return rc;
}

char* HBulkLoadClient_JNI::getErrorText(HBLC_RetCode errEnum)
{
  if (errEnum < (HBLC_RetCode)JOI_LAST)
    return JavaObjectInterface::getErrorText((JOI_RetCode)errEnum);
  else
    return (char*)hblcErrorEnumStr[errEnum-HBLC_FIRST-1];
}

HBLC_RetCode HBulkLoadClient_JNI::initHFileParams(
                        const HbaseStr &tblName,
                        const Text& hFileLoc,
                        const Text& hfileName,
                        Int64 maxHFileSize)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBulkLoadClient_JNI::initHFileParams(%s, %s, %s, %ld) called.", hFileLoc.data(), hfileName.data(), tblName.val,maxHFileSize);

  jstring js_hFileLoc = jenv_->NewStringUTF(hFileLoc.c_str());
   if (js_hFileLoc == NULL)
   {
     GetCliGlobals()->setJniErrorStr(getErrorText(HBLC_ERROR_CREATE_HFILE_PARAM));
     return HBLC_ERROR_CREATE_HFILE_PARAM;
   }
  jstring js_hfileName = jenv_->NewStringUTF(hfileName.c_str());
   if (js_hfileName == NULL)
   {
     GetCliGlobals()->setJniErrorStr(getErrorText(HBLC_ERROR_CREATE_HFILE_PARAM));
     return HBLC_ERROR_CREATE_HFILE_PARAM;
   }
   jstring js_tabName = jenv_->NewStringUTF(tblName.val);
    if (js_tabName == NULL)
    {
      GetCliGlobals()->setJniErrorStr(getErrorText(HBLC_ERROR_CREATE_HFILE_PARAM));
      return HBLC_ERROR_CREATE_HFILE_PARAM;
    }
  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBulkLoadClient_JNI::initHFileParams() => before calling Java.", getLastError());
    return HBLC_ERROR_CREATE_HFILE_EXCEPTION;
  }

  jlong j_maxSize = maxHFileSize;

  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_INIT_HFILE_PARAMS].methodID, js_hFileLoc, js_hfileName,j_maxSize,js_tabName);

  jenv_->DeleteLocalRef(js_hFileLoc);
  jenv_->DeleteLocalRef(js_hfileName);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBulkLoadClient_JNI::initHFileParams()", getLastError());
    return HBLC_ERROR_CREATE_HFILE_EXCEPTION;
  }

  if (jresult == false)
  {
    logError(CAT_HBASE, "HBulkLoadClient_JNI::initHFileParams()", getLastError());
    return HBLC_ERROR_CREATE_HFILE_EXCEPTION;
  }

  return HBLC_OK;
}

HBLC_RetCode HBulkLoadClient_JNI::addToHFile( short rowIDLen, HbaseStr &rowIDs,
            HbaseStr &rows, ExHbaseAccessStats *hbs)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBulkLoadClient_JNI::addToHFile called.");

  jobject jRowIDs = jenv_->NewDirectByteBuffer(rowIDs.val, rowIDs.len);
  if (jRowIDs == NULL)
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBLC_ERROR_ADD_TO_HFILE_EXCEPTION));
    return HBLC_ERROR_ADD_TO_HFILE_EXCEPTION;
  }

  jobject jRows = jenv_->NewDirectByteBuffer(rows.val, rows.len);
  if (jRows == NULL)
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBLC_ERROR_ADD_TO_HFILE_EXCEPTION));
    return HBLC_ERROR_ADD_TO_HFILE_EXCEPTION;
  }

  jshort j_rowIDLen = rowIDLen;

  if (hbs)
    hbs->getTimer().start();
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, 
            JavaMethods_[JM_ADD_TO_HFILE_DB].methodID, 
            j_rowIDLen, jRowIDs, jRows);
  if (hbs)
  {
    hbs->getTimer().stop();
    hbs->incHbaseCalls();
  }
  jenv_->DeleteLocalRef(jRowIDs);
  jenv_->DeleteLocalRef(jRows);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBulkLoadClient_JNI::addToHFile()", getLastError());
    return HBLC_ERROR_ADD_TO_HFILE_EXCEPTION;
  }

  if (jresult == false)
  {
    logError(CAT_HBASE, "HBulkLoadClient_JNI::addToHFile()", getLastError());
    return HBLC_ERROR_ADD_TO_HFILE_EXCEPTION;
  }

  return HBLC_OK;
}

HBLC_RetCode HBulkLoadClient_JNI::closeHFile(
                        const HbaseStr &tblName)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBulkLoadClient_JNI::closeHFile(%s) called.", tblName.val);

  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_CLOSE_HFILE].methodID);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBulkLoadClient_JNI::closeHFile()", getLastError());
    return HBLC_ERROR_CLOSE_HFILE_EXCEPTION;
  }

  if (jresult == false)
  {
    logError(CAT_HBASE, "HBulkLoadClient_JNI::closeHFile()", getLastError());
    return HBLC_ERROR_CLOSE_HFILE_EXCEPTION;
  }

  return HBLC_OK;
}


HBLC_RetCode HBulkLoadClient_JNI::doBulkLoad(
                             const HbaseStr &tblName,
                             const Text& prepLocation,
                             const Text& tableName,
                             NABoolean quasiSecure,
                             NABoolean snapshot)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBulkLoadClient_JNI::doBulkLoad(%s, %s, %s) called.", tblName.val, prepLocation.data(), tableName.data());

  jstring js_PrepLocation = jenv_->NewStringUTF(prepLocation.c_str());
   if (js_PrepLocation == NULL)
   {
     GetCliGlobals()->setJniErrorStr(getErrorText(HBLC_ERROR_DO_BULKLOAD_PARAM));
     return HBLC_ERROR_DO_BULKLOAD_PARAM;
   }
  jstring js_TableName = jenv_->NewStringUTF(tableName.c_str());
   if (js_TableName == NULL)
   {
     GetCliGlobals()->setJniErrorStr(getErrorText(HBLC_ERROR_DO_BULKLOAD_PARAM));
     return HBLC_ERROR_DO_BULKLOAD_PARAM;
   }

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBulkLoadClient_JNI::doBulkLoad() => before calling Java.", getLastError());
    return HBLC_ERROR_DO_BULKLOAD_EXCEPTION;
  }

  jboolean j_quasiSecure = quasiSecure;

  jboolean j_snapshot = snapshot;
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_DO_BULK_LOAD].methodID, js_PrepLocation, js_TableName, j_quasiSecure, j_snapshot);

  jenv_->DeleteLocalRef(js_PrepLocation);
  jenv_->DeleteLocalRef(js_TableName);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBulkLoadClient_JNI::doBulkLoad()", getLastError());
    return HBLC_ERROR_DO_BULKLOAD_EXCEPTION;
  }

  if (jresult == false)
  {
    logError(CAT_HBASE, "HBaseClient_JNI::doBulkLoad()", getLastError());
    return HBLC_ERROR_DO_BULKLOAD_EXCEPTION;
  }

  return HBLC_OK;
}

HBLC_RetCode HBulkLoadClient_JNI::bulkLoadCleanup(
                             const HbaseStr &tblName,
                             const Text& location)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBulkLoadClient_JNI::bulkLoadCleanup(%s, %s) called.", tblName.val, location.data());

  jstring js_location = jenv_->NewStringUTF(location.c_str());
   if (js_location == NULL)
   {
     GetCliGlobals()->setJniErrorStr(getErrorText(HBLC_ERROR_BULKLOAD_CLEANUP_PARAM));
     return HBLC_ERROR_BULKLOAD_CLEANUP_PARAM;
   }


  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBulkLoadClient_JNI::bulkLoadCleanup() => before calling Java.", getLastError());
    return HBLC_ERROR_BULKLOAD_CLEANUP_PARAM;
  }
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_BULK_LOAD_CLEANUP].methodID, js_location);

  jenv_->DeleteLocalRef(js_location);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBulkLoadClient_JNI::bulkLoadCleanup()", getLastError());
    return HBLC_ERROR_BULKLOAD_CLEANUP_PARAM;
  }

  if (jresult == false)
  {
    logError(CAT_HBASE, "HBulkLoadClient_JNI::bulkLoadCleanup()", getLastError());
    return HBLC_ERROR_BULKLOAD_CLEANUP_PARAM;
  }

  return HBLC_OK;
}

HBulkLoadClient_JNI::~HBulkLoadClient_JNI()
{
  //HdfsLogger::log(CAT_JNI_TOP, LL_DEBUG, "HBulkLoadClient_JNI destructor called.");
}

NAString HBulkLoadClient_JNI::getLastJavaError()
{
  return JavaObjectInterface::getLastJavaError(JavaMethods_[JM_GET_ERROR].methodID);
}




////////////////////////////////////////////////////////////////////
HBC_RetCode HBaseClient_JNI::revoke(const Text& user, const Text& tblName, const TextVec& actions)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBaseClient_JNI::revoke(%s, %s, %s) called.", user.data(), tblName.data(), actions.data());
  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HBC_ERROR_INIT_PARAM;

  int len = user.size();
  jbyteArray jba_user = jenv_->NewByteArray(len);
  if (jba_user == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_REVOKE_PARAM));
    return HBC_ERROR_REVOKE_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_user, 0, len, (const jbyte*)user.data());

  len = tblName.size();
  jbyteArray jba_tblName = jenv_->NewByteArray(len);
  if (jba_tblName == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HBC_ERROR_REVOKE_PARAM));
    jenv_->DeleteLocalRef(jba_user);  
    return HBC_ERROR_REVOKE_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_tblName, 0, len, (const jbyte*)tblName.data());

  jobjectArray j_actionCodes = NULL;
  if (!actions.empty())
  {
    HdfsLogger::log(CAT_HBASE, LL_DEBUG, "  Adding %d actions.", actions.size());
    j_actionCodes = convertToStringObjectArray(actions);
    if (j_actionCodes == NULL)
    {
       getExceptionDetails();
       logError(CAT_HBASE, __FILE__, __LINE__);
       logError(CAT_HBASE, "HBaseClient_JNI::revoke()", getLastError());
       jenv_->DeleteLocalRef(jba_user);  
       jenv_->DeleteLocalRef(jba_tblName);  
       return HBC_ERROR_REVOKE_PARAM;
    }
  }
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, 
       JavaMethods_[JM_REVOKE].methodID, jba_user, jba_tblName, j_actionCodes);

  jenv_->DeleteLocalRef(jba_user);  
  jenv_->DeleteLocalRef(jba_tblName);  
   if (j_actionCodes != NULL)
      jenv_->DeleteLocalRef(j_actionCodes);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::revoke()", getLastError());
    return HBC_ERROR_REVOKE_EXCEPTION;
  }

  if (jresult == false) 
  {
    logError(CAT_HBASE, "HBaseClient_JNI::revoke()", getLastError());
    return HBC_ERROR_REVOKE_EXCEPTION;
  }

  return HBC_OK;
}


////////////////////////////////////////////////////////////////////
void HBaseClient_JNI::logIt(const char* str)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, str);
}


// ===========================================================================
// ===== Class HTableClient
// ===========================================================================

JavaMethodInit* HTableClient_JNI::JavaMethods_ = NULL;
jclass HTableClient_JNI::javaClass_ = 0;
bool HTableClient_JNI::javaMethodsInitialized_ = false;
pthread_mutex_t HTableClient_JNI::javaMethodsInitMutex_ = PTHREAD_MUTEX_INITIALIZER;

static const char* const htcErrorEnumStr[] = 
{
  "Preparing parameters for initConnection()."
 ,"Java exception in initConnection()."
 ,"Java exception in setTransactionID()."
 ,"Java exception in cleanup()."
 ,"Java exception in close()."
 ,"Preparing parameters for scanOpen()."
 ,"Java exception in scanOpen()."
 ,"Java exception in fetchRows()."
 ,"Java exception in scanClose()."
 ,"Preparing parameters for getRowOpen()."
 ,"Java exception in getRowOpen()."
 ,"Preparing parameters for getRowsOpen()."
 ,"Java exception in getRowsOpen()."
 ,"Java exception in getClose()."
 ,"Preparing parameters for deleteRow()."
 ,"Java exception in deleteRow()."
 ,"Preparing parameters for deleteRows()."
 ,"Java exception in deleteRows()."
 ,"Preparing parameters for checkAndDeleteRow()."
 ,"Java exception in checkAndDeleteRow()."
 ,"Row deleted in checkAndDeleteRow()."
 ,"Preparing parameters for insertRow()."
 ,"Java exception in insertRow()."
 ,"Preparing parameters for insertRows()."
 ,"Java exception in insertRows()."
 ,"Preparing parameters for checkAndInsertRow()."
 ,"Java exception in checkAndInsertRow()."
 ,"Dup RowId in checkAndInsertRow()."
 ,"Transactions not supported yet in checkAndInsertRow()"
 ,"Preparing parameters for checkAndUpdateRow()."
 ,"Java exception in checkAndUpdateRow()."
 ,"Dup RowId in checkAndUpdateRow()."
 ,"Transactions not supported yet in checkAndUpdateRow()"
 ,"Preparing parameters for create()."
 ,"Java exception in create()."
 ,"Preparing parameters for drop()."
 ,"Java exception in drop()."
 ,"Preparing parameters for exists()."
 ,"Java exception in exists()."
 ,"Preparing parameters for coProcAggr()."
 ,"Java exception in coProcAggr()."
 ,"Preparing parameters for grant()."
 ,"Java exception in grant()."
 ,"Preparing parameters for revoke()."
 ,"Java exception in revoke()."
 ,"Java exception in getendkeys()."
 ,"Java exception in getHTableName()."
 ,"Java exception in flush()."
 ,"Java exception in getColName()."
 ,"Java exception in getColValue()."
 ,"Java exception in setJniObject()."
 ,"Java exception in getRowID()."
 ,"Java exception in nextCell()."
};

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
char* HTableClient_JNI::getErrorText(HTC_RetCode errEnum)
{
  if (errEnum < (HTC_RetCode)JOI_LAST)
    return JavaObjectInterface::getErrorText((JOI_RetCode)errEnum);
  else    
    return (char*)htcErrorEnumStr[errEnum-HTC_FIRST-1];
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HTableClient_JNI::~HTableClient_JNI()
{
  //HdfsLogger::log(CAT_JNI_TOP, LL_DEBUG, "HTableClient destructor called.");
  cleanupResultInfo();
  if (tableName_ != NULL)
  {
     NADELETEBASIC(tableName_, heap_);
  }
  if (colNameAllocLen_ != 0)
     NADELETEBASIC(colName_, heap_);
  if (numCellsAllocated_ > 0)
  {
      NADELETEBASIC(p_kvValLen_, heap_);
      NADELETEBASIC(p_kvValOffset_, heap_);
      NADELETEBASIC(p_kvFamLen_, heap_);
      NADELETEBASIC(p_kvFamOffset_, heap_);
      NADELETEBASIC(p_kvQualLen_, heap_);
      NADELETEBASIC(p_kvQualOffset_, heap_);
      NADELETEBASIC(p_timestamp_, heap_);
      numCellsAllocated_ = 0;
  }
}
 
//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HTC_RetCode HTableClient_JNI::init()
{
  static char className[]="org/trafodion/sql/HBaseAccess/HTableClient";
  HTC_RetCode rc;
  
  if (isInitialized())
    return HTC_OK;
  
  if (javaMethodsInitialized_)
    return (HTC_RetCode)JavaObjectInterface::init(className, javaClass_, JavaMethods_, (Int32)JM_LAST, javaMethodsInitialized_);
  else
  {
    pthread_mutex_lock(&javaMethodsInitMutex_);
    if (javaMethodsInitialized_)
    {
      pthread_mutex_unlock(&javaMethodsInitMutex_);
      return (HTC_RetCode)JavaObjectInterface::init(className, javaClass_, JavaMethods_, (Int32)JM_LAST, javaMethodsInitialized_);
    }
    JavaMethods_ = new JavaMethodInit[JM_LAST];
    
    JavaMethods_[JM_CTOR       ].jm_name      = "<init>";
    JavaMethods_[JM_CTOR       ].jm_signature = "()V";
    JavaMethods_[JM_GET_ERROR  ].jm_name      = "getLastError";
    JavaMethods_[JM_GET_ERROR  ].jm_signature = "()Ljava/lang/String;";
    JavaMethods_[JM_SCAN_OPEN  ].jm_name      = "startScan";
    JavaMethods_[JM_SCAN_OPEN  ].jm_signature = "(J[B[B[Ljava/lang/Object;JZI[Ljava/lang/Object;[Ljava/lang/Object;[Ljava/lang/Object;FZ)Z";
    JavaMethods_[JM_GET_OPEN   ].jm_name      = "startGet";
    JavaMethods_[JM_GET_OPEN   ].jm_signature = "(J[B[Ljava/lang/Object;J)Z";
    JavaMethods_[JM_GETS_OPEN  ].jm_name      = "startGet";
    JavaMethods_[JM_GETS_OPEN  ].jm_signature = "(J[Ljava/lang/Object;[Ljava/lang/Object;J)Z";
    JavaMethods_[JM_DELETE     ].jm_name      = "deleteRow";
    JavaMethods_[JM_DELETE     ].jm_signature = "(J[B[Ljava/lang/Object;J)Z";
    JavaMethods_[JM_CHECKANDDELETE     ].jm_name      = "checkAndDeleteRow";
    JavaMethods_[JM_CHECKANDDELETE     ].jm_signature = "(J[B[B[BJ)Z";
    JavaMethods_[JM_CHECKANDUPDATE     ].jm_name      = "checkAndUpdateRow";
    JavaMethods_[JM_CHECKANDUPDATE     ].jm_signature = "(J[BLjava/lang/Object;[B[BJ)Z";
    JavaMethods_[JM_COPROC_AGGR     ].jm_name      = "coProcAggr";
    JavaMethods_[JM_COPROC_AGGR     ].jm_signature = "(JI[B[B[B[BZI)[B";
    JavaMethods_[JM_GET_NAME   ].jm_name      = "getTableName";
    JavaMethods_[JM_GET_NAME   ].jm_signature = "()Ljava/lang/String;";
    JavaMethods_[JM_GET_HTNAME ].jm_name      = "getTableName";
    JavaMethods_[JM_GET_HTNAME ].jm_signature = "()Ljava/lang/String;";
    JavaMethods_[JM_GETENDKEYS ].jm_name      = "getEndKeys";
    JavaMethods_[JM_GETENDKEYS ].jm_signature = "()Lorg/trafodion/sql/HBaseAccess/ByteArrayList;";
    JavaMethods_[JM_FLUSHT     ].jm_name      = "flush";
    JavaMethods_[JM_FLUSHT     ].jm_signature = "()Z";
    JavaMethods_[JM_SET_WB_SIZE ].jm_name      = "setWriteBufferSize";
    JavaMethods_[JM_SET_WB_SIZE ].jm_signature = "(J)Z";
    JavaMethods_[JM_SET_WRITE_TO_WAL ].jm_name      = "setWriteToWAL";
    JavaMethods_[JM_SET_WRITE_TO_WAL ].jm_signature = "(Z)Z";
    JavaMethods_[JM_DIRECT_INSERT ].jm_name      = "insertRow";
    JavaMethods_[JM_DIRECT_INSERT ].jm_signature = "(J[BLjava/lang/Object;J)Z";
    JavaMethods_[JM_DIRECT_CHECKANDINSERT     ].jm_name      = "checkAndInsertRow";
    JavaMethods_[JM_DIRECT_CHECKANDINSERT     ].jm_signature = "(J[BLjava/lang/Object;J)Z";
    JavaMethods_[JM_DIRECT_INSERT_ROWS ].jm_name      = "putRows";
    JavaMethods_[JM_DIRECT_INSERT_ROWS ].jm_signature = "(JSLjava/lang/Object;Ljava/lang/Object;JZ)Z";
    JavaMethods_[JM_DIRECT_DELETE_ROWS ].jm_name      = "deleteRows";
    JavaMethods_[JM_DIRECT_DELETE_ROWS ].jm_signature = "(JSLjava/lang/Object;J)Z";
    JavaMethods_[JM_FETCH_ROWS ].jm_name      = "fetchRows";
    JavaMethods_[JM_FETCH_ROWS ].jm_signature = "()I";
    JavaMethods_[JM_SET_JNIOBJECT].jm_name      = "setJniObject";
    JavaMethods_[JM_SET_JNIOBJECT].jm_signature = "(J)V";
   
    rc = (HTC_RetCode)JavaObjectInterface::init(className, javaClass_, JavaMethods_, (Int32)JM_LAST, javaMethodsInitialized_);
    javaMethodsInitialized_ = TRUE;
    pthread_mutex_unlock(&javaMethodsInitMutex_);
  }
  return rc;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
NAString HTableClient_JNI::getLastJavaError()
{
  return JavaObjectInterface::getLastJavaError(JavaMethods_[JM_GET_ERROR].methodID);
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HTC_RetCode HTableClient_JNI::startScan(Int64 transID, const Text& startRowID, 
   const Text& stopRowID, const TextVec& cols, Int64 timestamp, 
   bool cacheBlocks, Lng32 numCacheRows, NABoolean preFetch,
					const TextVec *inColNamesToFilter, 
					const TextVec *inCompareOpList,
					const TextVec *inColValuesToCompare,
					Float32 samplePercent)

{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HTableClient_JNI::startScan() called.");
  int len = startRowID.size();
  jbyteArray jba_startRowID = jenv_->NewByteArray(len);
  if (jba_startRowID == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_SCANOPEN_PARAM));
    return HTC_ERROR_SCANOPEN_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_startRowID, 0, len, (const jbyte*)startRowID.data());

  len = stopRowID.size();
  jbyteArray jba_stopRowID = jenv_->NewByteArray(len);
  if (jba_stopRowID == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_SCANOPEN_PARAM));
    jenv_->DeleteLocalRef(jba_startRowID);
    return HTC_ERROR_SCANOPEN_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_stopRowID, 0, len, (const jbyte*)stopRowID.data());

  jobjectArray j_cols = NULL;
  if (!cols.empty())
  {
    j_cols = convertToByteArrayObjectArray(cols);
    if (j_cols == NULL)
    {
       getExceptionDetails();
       logError(CAT_HBASE, __FILE__, __LINE__);
       logError(CAT_HBASE, "HTableClient_JNI::startScan()", getLastError());
       jenv_->DeleteLocalRef(jba_startRowID);
       jenv_->DeleteLocalRef(jba_stopRowID);
       return HTC_ERROR_SCANOPEN_PARAM;
    }
    numColsInScan_ = cols.size();
  }
  else
     numColsInScan_ = 0;
  jlong j_tid = transID;  
  jlong j_ts = timestamp;

  jboolean j_cb = cacheBlocks;
  jboolean j_preFetch = preFetch;
  jint j_ncr = numCacheRows;
  numReqRows_ = numCacheRows;
  currentRowNum_ = -1;
  currentRowCellNum_ = -1;
  
  jobjectArray j_colnamestofilter = NULL;
  if ((inColNamesToFilter) && (!inColNamesToFilter->empty()))
  {
    j_colnamestofilter = convertToByteArrayObjectArray(*inColNamesToFilter);
    if (j_colnamestofilter == NULL)
    {
       getExceptionDetails();
       logError(CAT_HBASE, __FILE__, __LINE__);
       logError(CAT_HBASE, "HTableClient_JNI::startScan()", getLastError());
       jenv_->DeleteLocalRef(jba_startRowID);
       jenv_->DeleteLocalRef(jba_stopRowID);
       if (j_cols != NULL)
           jenv_->DeleteLocalRef(j_cols);
       return HTC_ERROR_SCANOPEN_PARAM;
    }
  }

  jobjectArray j_compareoplist = NULL;
  if ((inCompareOpList) && (! inCompareOpList->empty()))
  {
     j_compareoplist = convertToByteArrayObjectArray(*inCompareOpList);
     if (j_compareoplist == NULL)
     {
        getExceptionDetails();
        logError(CAT_HBASE, __FILE__, __LINE__);
        logError(CAT_HBASE, "HTableClient_JNI::startScan()", getLastError());
        jenv_->DeleteLocalRef(jba_startRowID);
        jenv_->DeleteLocalRef(jba_stopRowID);
        if (j_cols != NULL)
           jenv_->DeleteLocalRef(j_cols);
        if (j_colnamestofilter != NULL)
           jenv_->DeleteLocalRef(j_colnamestofilter);

        return HTC_ERROR_SCANOPEN_PARAM;
     }
  }

  jobjectArray j_colvaluestocompare = NULL;
  if ((inColValuesToCompare) && (!inColValuesToCompare->empty()))
  {
     j_colvaluestocompare = convertToByteArrayObjectArray(*inColValuesToCompare);
     if (j_colvaluestocompare == NULL)
     {
        getExceptionDetails();
        logError(CAT_HBASE, __FILE__, __LINE__);
        logError(CAT_HBASE, "HTableClient_JNI::startScan()", getLastError());
        jenv_->DeleteLocalRef(jba_startRowID);
        jenv_->DeleteLocalRef(jba_stopRowID);
        if (j_cols != NULL)
           jenv_->DeleteLocalRef(j_cols);
        if (j_colnamestofilter != NULL)
           jenv_->DeleteLocalRef(j_colnamestofilter);
        if (j_compareoplist != NULL)
           jenv_->DeleteLocalRef(j_compareoplist);
        return HTC_ERROR_SCANOPEN_PARAM;
     }
  }

  jfloat j_smplPct = samplePercent;

  if (hbs_)
    hbs_->getTimer().start();
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, 
            JavaMethods_[JM_SCAN_OPEN].methodID, 
            j_tid, jba_startRowID, jba_stopRowID, j_cols, j_ts, j_cb, j_ncr,
            j_colnamestofilter, j_compareoplist, j_colvaluestocompare, 
            j_smplPct, j_preFetch);
  if (hbs_)
    {
      hbs_->getTimer().stop();
      hbs_->incHbaseCalls();
    }

  jenv_->DeleteLocalRef(jba_startRowID);  
  jenv_->DeleteLocalRef(jba_stopRowID);  
  if (j_cols != NULL)
     jenv_->DeleteLocalRef(j_cols);
  if (j_colnamestofilter != NULL)
     jenv_->DeleteLocalRef(j_colnamestofilter);
  if (j_compareoplist != NULL)
     jenv_->DeleteLocalRef(j_compareoplist);
  if (j_colvaluestocompare != NULL)
     jenv_->DeleteLocalRef(j_colvaluestocompare);
  

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HTableClient_JNI::scanOpen()", getLastError());
    return HTC_ERROR_SCANOPEN_EXCEPTION;
  }

  if (jresult == false) 
  {
    logError(CAT_HBASE, "HTableClient_JNI::scanOpen()", getLastError());
    return HTC_ERROR_SCANOPEN_EXCEPTION;
  }
  fetchMode_ = SCAN_FETCH;
  return HTC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HTC_RetCode HTableClient_JNI::startGet(Int64 transID, const Text& rowID, 
      const TextVec& cols, Int64 timestamp)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HTableClient_JNI::startGet(%s) called.", rowID.data());
  int len = rowID.size();
  jbyteArray jba_rowID = jenv_->NewByteArray(len);
  if (jba_rowID == NULL) 
  {
     GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_GETROWOPEN_PARAM));
     return HTC_ERROR_GETROWOPEN_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_rowID, 0, len, (const jbyte*)rowID.data());
  jobjectArray j_cols = NULL;
  if (!cols.empty())
  {
     j_cols = convertToByteArrayObjectArray(cols);
     if (j_cols == NULL)
     {
        getExceptionDetails();
        logError(CAT_HBASE, __FILE__, __LINE__);
        logError(CAT_HBASE, "HTableClient_JNI::startGet()", getLastError());
        jenv_->DeleteLocalRef(jba_rowID);
        return HTC_ERROR_GETROWOPEN_PARAM;
     }  
     numColsInScan_ = cols.size();
  }
  else
     numColsInScan_ = 0;
  numReqRows_ = 1;
  jlong j_tid = transID;  
  jlong j_ts = timestamp;
  
  if (hbs_)
    hbs_->getTimer().start();
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, 
            JavaMethods_[JM_GET_OPEN].methodID, j_tid, jba_rowID, 
            j_cols, j_ts);
  if (hbs_)
    {
      hbs_->getTimer().stop();
      hbs_->incHbaseCalls();
    }

  jenv_->DeleteLocalRef(jba_rowID);  
  if (j_cols != NULL)
     jenv_->DeleteLocalRef(j_cols);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HTableClient_JNI::getRowOpen()", getLastError());
    return HTC_ERROR_GETROWOPEN_EXCEPTION;
  }

  fetchMode_ = GET_ROW;
  if (jresult == false) 
     numRowsReturned_ = -1;
  else
     numRowsReturned_ = 0;
  return HTC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HTC_RetCode HTableClient_JNI::startGets(Int64 transID, const TextVec& rowIDs, 
	const TextVec& cols, Int64 timestamp)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HTableClient_JNI::startGet(multi-row) called.");
  jobjectArray j_cols = NULL;
  if (!cols.empty())
  {
     j_cols = convertToByteArrayObjectArray(cols);
     if (j_cols == NULL)
     {
        getExceptionDetails();
        logError(CAT_HBASE, __FILE__, __LINE__);
        logError(CAT_HBASE, "HTableClient_JNI::startGets()", getLastError());
        return HTC_ERROR_GETROWSOPEN_PARAM;
     }
     numColsInScan_ = cols.size();
  }  
  else
     numColsInScan_ = 0;
  jobjectArray j_rows = convertToByteArrayObjectArray(rowIDs);
  if (j_rows == NULL)
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HTableClient_JNI::startGets()", getLastError());
    if (j_cols != NULL)
        jenv_->DeleteLocalRef(j_cols);
    return HTC_ERROR_GETROWSOPEN_PARAM;
  }  
  numReqRows_ = rowIDs.size();
  jlong j_tid = transID;  
  jlong j_ts = timestamp;
  
  if (hbs_)
    hbs_->getTimer().start();
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, 
      JavaMethods_[JM_GETS_OPEN].methodID, j_tid, j_rows, j_cols, j_ts);
  if (hbs_)
    {
      hbs_->getTimer().stop();
      hbs_->incHbaseCalls();
    }

  jenv_->DeleteLocalRef(j_rows);
  if (j_cols != NULL)
      jenv_->DeleteLocalRef(j_cols);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HTableClient_JNI::getRowsOpen()", getLastError());
    return HTC_ERROR_GETROWSOPEN_EXCEPTION;
  }

  if (jresult == false) 
  {

    logError(CAT_HBASE, "HTableClient_JNI::getRowsOpen()", getLastError());
    return HTC_ERROR_GETROWSOPEN_EXCEPTION;
  }
  fetchMode_ = BATCH_GET;
  return HTC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HTC_RetCode HTableClient_JNI::deleteRow(Int64 transID, HbaseStr &rowID, const TextVec& cols, Int64 timestamp)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HTableClient_JNI::deleteRow(%ld, %s) called.", transID, rowID.val);
  jbyteArray jba_rowID = jenv_->NewByteArray(rowID.len);
  if (jba_rowID == NULL) 
  {
     GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_DELETEROW_PARAM));
     return HTC_ERROR_DELETEROW_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_rowID, 0, rowID.len, (const jbyte*)rowID.val);
  jobjectArray j_cols = NULL;
  if (!cols.empty())
  {
     j_cols = convertToByteArrayObjectArray(cols);
     if (j_cols == NULL)
     {
        getExceptionDetails();
        logError(CAT_HBASE, __FILE__, __LINE__);
        logError(CAT_HBASE, "HTableClient_JNI::deleteRow()", getLastError());
        jenv_->DeleteLocalRef(jba_rowID);  
        return HTC_ERROR_DELETEROW_PARAM;
     }
  }  
  jlong j_tid = transID;  
  jlong j_ts = timestamp;
  if (hbs_)
    hbs_->getTimer().start();
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, 
          JavaMethods_[JM_DELETE].methodID, j_tid, jba_rowID, j_cols, j_ts);
  if (hbs_)
    {
      hbs_->getTimer().stop();
      hbs_->incHbaseCalls();
    }

  jenv_->DeleteLocalRef(jba_rowID);  
  if (j_cols != NULL)
     jenv_->DeleteLocalRef(j_cols);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HTableClient_JNI::deleteRow()", getLastError());
    return HTC_ERROR_DELETEROW_EXCEPTION;
  }

  if (jresult == false) 
  {
    logError(CAT_HBASE, "HTableClient_JNI::deleteRow()", getLastError());
    return HTC_ERROR_DELETEROW_EXCEPTION;
  }

  if (hbs_)
    hbs_->incBytesRead(rowID.len);

  return HTC_OK;
}
//
//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HTC_RetCode HTableClient_JNI::deleteRows(Int64 transID, short rowIDLen, HbaseStr &rowIDs, Int64 timestamp)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HTableClient_JNI::deleteRows() called.");

  jobject jRowIDs = jenv_->NewDirectByteBuffer(rowIDs.val, rowIDs.len);
  if (jRowIDs == NULL)
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_DELETEROWS_PARAM));
    return HTC_ERROR_DELETEROWS_PARAM;
  }
  jshort j_rowIDLen = rowIDLen;
  jlong j_tid = transID;  
  jlong j_ts = timestamp;

  if (hbs_)
    hbs_->getTimer().start();
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_DIRECT_DELETE_ROWS].methodID, j_tid, j_rowIDLen, jRowIDs, j_ts);
  if (hbs_)
    {
      hbs_->getTimer().stop();
      hbs_->incHbaseCalls();
    }

  jenv_->DeleteLocalRef(jRowIDs);  

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HTableClient_JNI::deleteRows()", getLastError());
    return HTC_ERROR_DELETEROWS_EXCEPTION;
  }

  if (jresult == false) 
  {
    logError(CAT_HBASE, "HTableClient_JNI::deleteRows()", getLastError());
    return HTC_ERROR_DELETEROWS_EXCEPTION;
  }

  if (hbs_)
    hbs_->incBytesRead(rowIDs.len);

  return HTC_OK;
}


//////////////////////////////////////////////////////////////////////////////
// 


//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HTC_RetCode HTableClient_JNI::checkAndDeleteRow(Int64 transID, HbaseStr &rowID,
	    const Text &columnToCheck, const Text &colValToCheck,
	    Int64 timestamp)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HTableClient_JNI::checkAndDeleteRow(%s, %s, %s) called.", rowID.val, columnToCheck.data(), colValToCheck.data());
  jbyteArray jba_rowID = jenv_->NewByteArray(rowID.len);
  if (jba_rowID == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_CHECKANDDELETEROW_PARAM));
    return HTC_ERROR_CHECKANDDELETEROW_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_rowID, 0, rowID.len, (const jbyte*)rowID.val);

  int len = columnToCheck.size();
  jbyteArray jba_columntocheck = jenv_->NewByteArray(len);
  if (jba_columntocheck == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_CHECKANDDELETEROW_PARAM));
    return HTC_ERROR_CHECKANDDELETEROW_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_columntocheck, 0, len, (const jbyte*)columnToCheck.data());

  len = colValToCheck.size();
  jbyteArray jba_colvaltocheck = jenv_->NewByteArray(len);
  if (jba_colvaltocheck == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_CHECKANDDELETEROW_PARAM));
    return HTC_ERROR_CHECKANDDELETEROW_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_colvaltocheck, 0, len, 
			    (const jbyte*)colValToCheck.data());
 
  jlong j_tid = transID;  
  jlong j_ts = timestamp;

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HTableClient_JNI::checkAndDeleteRow() => before calling Java.", getLastError());
    return HTC_ERROR_CHECKANDDELETEROW_EXCEPTION;
  }

  // public boolean checkAndDeleteRow(long, byte[], byte[], long);
  if (hbs_)
    hbs_->getTimer().start();
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_CHECKANDDELETE].methodID, j_tid, jba_rowID, jba_columntocheck, jba_colvaltocheck, j_ts);
  if (hbs_)
    {
      hbs_->getTimer().stop();
      hbs_->incHbaseCalls();
    }

  jenv_->DeleteLocalRef(jba_rowID);  
  jenv_->DeleteLocalRef(jba_columntocheck);  
  jenv_->DeleteLocalRef(jba_colvaltocheck);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HTableClient_JNI::checkAndDeleteRow()", getLastError());
    return HTC_ERROR_CHECKANDDELETEROW_EXCEPTION;
  }

  if (jresult == false) 
  {
     GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_CHECKANDDELETE_ROW_NOTFOUND));
     return HTC_ERROR_CHECKANDDELETE_ROW_NOTFOUND;
  }
    
  if (hbs_)
    hbs_->incBytesRead(rowID.len);

  return HTC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HTC_RetCode HTableClient_JNI::insertRow(Int64 transID, HbaseStr &rowID, 
     HbaseStr &row, Int64 timestamp)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HTableClient_JNI::insertRow(%s) direct called.", rowID.val);

  jbyteArray jba_rowID = jenv_->NewByteArray(rowID.len);
  if (jba_rowID == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_INSERTROW_PARAM));
    return HTC_ERROR_INSERTROW_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_rowID, 0, rowID.len, (const jbyte*)rowID.val);

  jobject jDirectBuffer = jenv_->NewDirectByteBuffer(row.val, row.len);
  if (jDirectBuffer == NULL)
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_INSERTROW_PARAM));
    return HTC_ERROR_INSERTROW_PARAM;
  }

  jlong j_tid = transID;  
  jlong j_ts = timestamp;

  if (hbs_)
    hbs_->getTimer().start();
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_DIRECT_INSERT].methodID, j_tid, jba_rowID, jDirectBuffer, j_ts);
  if (hbs_)
    {
      hbs_->getTimer().stop();
      hbs_->incHbaseCalls();
    }

  jenv_->DeleteLocalRef(jba_rowID);  
  jenv_->DeleteLocalRef(jDirectBuffer);  

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HTableClient_JNI::insertRow()", getLastError());
    return HTC_ERROR_INSERTROW_EXCEPTION;
  }

  if (jresult == false) 
  {
    logError(CAT_HBASE, "HTableClient_JNI::insertRow()", getLastError());
    return HTC_ERROR_INSERTROW_EXCEPTION;
  }

  if (hbs_)
    hbs_->incBytesRead(rowID.len + row.len);

  return HTC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HTC_RetCode HTableClient_JNI::insertRows(Int64 transID, short rowIDLen, HbaseStr &rowIDs, 
      HbaseStr &rows, Int64 timestamp, bool autoFlush)
{
  jobject jRowIDs = jenv_->NewDirectByteBuffer(rowIDs.val, rowIDs.len);
  if (jRowIDs == NULL)
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_INSERTROWS_PARAM));
    return HTC_ERROR_INSERTROWS_PARAM;
  }
  
  jobject jRows = jenv_->NewDirectByteBuffer(rows.val, rows.len);
  if (jRows == NULL)
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_INSERTROWS_PARAM));
    return HTC_ERROR_INSERTROWS_PARAM;
  }
  jlong j_tid = transID;  
  jlong j_ts = timestamp;
  jshort j_rowIDLen = rowIDLen;
  jboolean j_af = autoFlush;
 
  if (hbs_)
    hbs_->getTimer().start();
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_DIRECT_INSERT_ROWS].methodID, j_tid, j_rowIDLen, jRowIDs, jRows, j_ts, j_af);
  if (hbs_)
    {
      hbs_->getTimer().stop();
      hbs_->incHbaseCalls();
    }

  jenv_->DeleteLocalRef(jRowIDs);  
  jenv_->DeleteLocalRef(jRows);  

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HTableClient_JNI::insertRows()", getLastError());
    return HTC_ERROR_INSERTROWS_EXCEPTION;
  }

  if (jresult == false) 
  {
    logError(CAT_HBASE, "HTableClient_JNI::insertRows()", getLastError());
    return HTC_ERROR_INSERTROWS_EXCEPTION;
  }

  if (hbs_)
    hbs_->incBytesRead(rowIDs.len + rows.len);

  return HTC_OK;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
HTC_RetCode HTableClient_JNI::setWriteBufferSize(Int64 size)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HTableClient_JNI::setWriteBufferSize() called.");



  jlong j_size = size;

  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_SET_WB_SIZE].methodID, j_size);


  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HTableClient_JNI::setWriteBufferSize()", getLastError());
    return HTC_ERROR_INSERTROWS_EXCEPTION;// need to change exception
  }

  if (jresult == false)
  {
    logError(CAT_HBASE, "HTableClient_JNI::setWriteBufferSize()", getLastError());
    return HTC_ERROR_INSERTROWS_EXCEPTION;// need to change exception
  }

  return HTC_OK;
}

HTC_RetCode HTableClient_JNI::setWriteToWAL(bool WAL)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HTableClient_JNI::setWriteToWAL() called.");

  jboolean j_WAL = WAL;

    jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_SET_WRITE_TO_WAL].methodID, j_WAL);


  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HTableClient_JNI::setWriteToWAL()", getLastError());
    return HTC_ERROR_INSERTROWS_EXCEPTION;// need to change exception
  }

  if (jresult == false)
  {
    logError(CAT_HBASE, "HTableClient_JNI::setWriteToWAL()", getLastError());
    return HTC_ERROR_INSERTROWS_EXCEPTION;// need to change exception??????????????????
  }

  return HTC_OK;
}
//
//////////////////////////////////////////////////////////////////////////////
//   3-way return value!!!
//////////////////////////////////////////////////////////////////////////////
HTC_RetCode HTableClient_JNI::checkAndInsertRow(Int64 transID, HbaseStr &rowID,
HbaseStr &row, Int64 timestamp)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HTableClient_JNI::checkAndInsertRow(%s) called.", rowID.val);
  jbyteArray jba_rowID = jenv_->NewByteArray(rowID.len);
  if (jba_rowID == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_CHECKANDINSERTROW_PARAM));
    return HTC_ERROR_CHECKANDINSERTROW_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_rowID, 0, rowID.len, (const jbyte*)rowID.val);

  jobject jDirectBuffer = jenv_->NewDirectByteBuffer(row.val, row.len);
  if (jDirectBuffer == NULL)
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_CHECKANDINSERTROW_PARAM));
    return HTC_ERROR_CHECKANDINSERTROW_PARAM;
  }

  jlong j_tid = transID;  
  jlong j_ts = timestamp;

  if (hbs_)
    hbs_->getTimer().start();
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_DIRECT_CHECKANDINSERT].methodID, j_tid, jba_rowID, jDirectBuffer, j_ts);
  if (hbs_)
    {
      hbs_->getTimer().stop();
      hbs_->incHbaseCalls();
    }

  jenv_->DeleteLocalRef(jba_rowID);  
  jenv_->DeleteLocalRef(jDirectBuffer);
  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HTableClient_JNI::checkAndInsertRow()", getLastError());
    return HTC_ERROR_CHECKANDINSERTROW_EXCEPTION;
  }

  if (jresult == false) 
     return HTC_ERROR_CHECKANDINSERT_DUP_ROWID;

  if (hbs_)
    hbs_->incBytesRead(rowID.len + row.len);

  return HTC_OK;
}

HTC_RetCode HTableClient_JNI::checkAndUpdateRow(Int64 transID, HbaseStr &rowID,
            HbaseStr &row,
	    const Text &columnToCheck, const Text &colValToCheck, 
            Int64 timestamp)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HTableClient_JNI::checkAndUpdateRow(%s) called.", rowID.val);
  jbyteArray jba_rowID = jenv_->NewByteArray(rowID.len);
  if (jba_rowID == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_CHECKANDUPDATEROW_PARAM));
    return HTC_ERROR_CHECKANDUPDATEROW_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_rowID, 0, rowID.len, (const jbyte*)rowID.val);

  jobject jDirectBuffer = jenv_->NewDirectByteBuffer(row.val, row.len);
  if (jDirectBuffer == NULL)
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_CHECKANDUPDATEROW_PARAM));
    return HTC_ERROR_CHECKANDUPDATEROW_PARAM;
  }
  
  int len = columnToCheck.size();
  jbyteArray jba_columntocheck = jenv_->NewByteArray(len);
  if (jba_columntocheck == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_CHECKANDUPDATEROW_PARAM));
    return HTC_ERROR_CHECKANDUPDATEROW_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_columntocheck, 0, len, 
			    (const jbyte*)columnToCheck.data());
 
  len = colValToCheck.size();
  jbyteArray jba_colvaltocheck = jenv_->NewByteArray(len);
  if (jba_colvaltocheck == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_CHECKANDUPDATEROW_PARAM));
    return HTC_ERROR_CHECKANDUPDATEROW_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_colvaltocheck, 0, len, 
			    (const jbyte*)colValToCheck.data());
 
  jlong j_tid = transID;  
  jlong j_ts = timestamp;

  if (hbs_)
    hbs_->getTimer().start();
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, 
                JavaMethods_[JM_CHECKANDUPDATE].methodID, 
                j_tid, jba_rowID, jDirectBuffer, 
	        jba_columntocheck, jba_colvaltocheck, j_ts);
  if (hbs_)
    {
      hbs_->getTimer().stop();
      hbs_->incHbaseCalls();
    }

  jenv_->DeleteLocalRef(jba_rowID);  
  jenv_->DeleteLocalRef(jba_columntocheck);  
  jenv_->DeleteLocalRef(jba_colvaltocheck);
  jenv_->DeleteLocalRef(jDirectBuffer);
  
  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HTableClient_JNI::checkAndUpdateRow()", getLastError());
    return HTC_ERROR_CHECKANDUPDATEROW_EXCEPTION;
  }

  if (jresult == false) 
  {
     GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_CHECKANDUPDATE_ROW_NOTFOUND));
     return HTC_ERROR_CHECKANDUPDATE_ROW_NOTFOUND;
  }

  if (hbs_)
    hbs_->incBytesRead(rowID.len + row.len);

  return HTC_OK;
}
//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
const char *HTableClient_JNI::getTableName()
{
  return tableName_;
}
//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
std::string* HTableClient_JNI::getHTableName()
{
  jstring js_name = (jstring)jenv_->CallObjectMethod(javaObj_, JavaMethods_[JM_GET_HTNAME].methodID);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HTableClient_JNI::getHTableName()", getLastError());
    return NULL;
  }
 
  if (js_name == NULL)
    return NULL;
    
  const char* char_result = jenv_->GetStringUTFChars(js_name, 0);
  std::string* tableName = new (heap_) std::string(char_result);
  jenv_->ReleaseStringUTFChars(js_name, char_result);
  jenv_->DeleteLocalRef(js_name);  
  
  return tableName;
}

HTC_RetCode HTableClient_JNI::coProcAggr(Int64 transID, 
					 int aggrType, // 0:count, 1:min, 2:max, 3:sum, 4:avg
					 const Text& startRowID, 
					 const Text& stopRowID, 
					 const Text &colFamily,
					 const Text &colName,
					 const NABoolean cacheBlocks,
					 const Lng32 numCacheRows,
					 Text &aggrVal) // returned value
{

  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HTableClient_JNI::coProcAggr called.");

  int len = 0;

  len = startRowID.size();
  jbyteArray jba_startrowid = jenv_->NewByteArray(len);
  if (jba_startrowid == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_COPROC_AGGR_PARAM));
    return HTC_ERROR_COPROC_AGGR_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_startrowid, 0, len, 
			    (const jbyte*)startRowID.data());

  len = stopRowID.size();
  jbyteArray jba_stoprowid = jenv_->NewByteArray(len);
  if (jba_stoprowid == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_COPROC_AGGR_PARAM));
    return HTC_ERROR_COPROC_AGGR_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_stoprowid, 0, len, 
			    (const jbyte*)stopRowID.data());
 
  len = colFamily.size();
  jbyteArray jba_colfamily = jenv_->NewByteArray(len);
  if (jba_colfamily == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_COPROC_AGGR_PARAM));
    return HTC_ERROR_COPROC_AGGR_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_colfamily, 0, len, 
			    (const jbyte*)colFamily.data());
 
  len = colName.size();
  jbyteArray jba_colname = jenv_->NewByteArray(len);
  if (jba_colname == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_COPROC_AGGR_PARAM));
    return HTC_ERROR_COPROC_AGGR_PARAM;
  }
  jenv_->SetByteArrayRegion(jba_colname, 0, len, 
			    (const jbyte*)colName.data());

  jlong j_tid = transID;  
  jint j_aggrtype = aggrType;

  jboolean j_cb = cacheBlocks;
  jint j_ncr = numCacheRows;

  if (hbs_)
    hbs_->getTimer().start();
  jarray jresult = (jarray)jenv_->CallObjectMethod(javaObj_, 
              JavaMethods_[JM_COPROC_AGGR].methodID, j_tid, 
              j_aggrtype, jba_startrowid, jba_stoprowid, jba_colfamily, 
              jba_colname, j_cb, j_ncr);
  if (hbs_)
    {
      hbs_->getTimer().stop();
      hbs_->incHbaseCalls();
    }

  jenv_->DeleteLocalRef(jba_startrowid);  
  jenv_->DeleteLocalRef(jba_stoprowid);  
  jenv_->DeleteLocalRef(jba_colfamily);
  jenv_->DeleteLocalRef(jba_colname);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HTableClient_JNI::coProcAggr()", getLastError());
    return HTC_ERROR_COPROC_AGGR_EXCEPTION;
  }

  Text *val = NULL;
  if (jresult != NULL)
  {
     jbyte *result = jenv_->GetByteArrayElements((jbyteArray)jresult, NULL);
     int len = jenv_->GetArrayLength(jresult);
     val = new (heap_) Text((char *)result, len);
     jenv_->ReleaseByteArrayElements((jbyteArray)jresult, result, JNI_ABORT);
     jenv_->DeleteLocalRef(jresult);

  }
  if (val == NULL)
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HTC_ERROR_COPROC_AGGR_PARAM));
    return HTC_ERROR_COPROC_AGGR_PARAM;
  }  
  aggrVal = *val;

  return HTC_OK;
}

ByteArrayList* HTableClient_JNI::getEndKeys()
{
  jobject jByteArrayList = 
     jenv_->CallObjectMethod(javaObj_, JavaMethods_[JM_GETENDKEYS].methodID);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    return NULL;
  }

  if (jByteArrayList == NULL)
    return NULL;

  ByteArrayList* endKeys = new (heap_) ByteArrayList(heap_, jByteArrayList);
  jenv_->DeleteLocalRef(jByteArrayList);
  if (endKeys->init() != BAL_OK)
  {
     NADELETE(endKeys, ByteArrayList, heap_);
     return NULL;
  }
  return endKeys;

}

HTC_RetCode HTableClient_JNI::flushTable()
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HBaseClient_JNI::flushTable() called.");

  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_FLUSHT].methodID);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::flushTable()", getLastError());
    return HTC_ERROR_FLUSHTABLE_EXCEPTION;
  }

  if (jresult == false) 
  {
    logError(CAT_HBASE, "HBaseClient_JNI::flushTable()", getLastError());
    return HTC_ERROR_FLUSHTABLE_EXCEPTION;
  }

  return HTC_OK;
}

// ===========================================================================
// ===== Class HiveClient_JNI
// ===========================================================================

JavaMethodInit* HiveClient_JNI::JavaMethods_ = NULL;
jclass HiveClient_JNI::javaClass_ = 0;
bool HiveClient_JNI::javaMethodsInitialized_ = false;
pthread_mutex_t HiveClient_JNI::javaMethodsInitMutex_ = PTHREAD_MUTEX_INITIALIZER;

static const char* const hvcErrorEnumStr[] = 
{
 "Preparing parameters for initConnection()."
 ,"Java exception in initConnection()."
 ,"Java exception in close()."
 ,"Preparing parameters for exists()."
 ,"Java exception in exists()."
 ,"Preparing parameters for getHiveTableStr()."
 ,"Java exception in getHiveTableStr()."
 ,"Preparing parameters for getRedefTime()."
 ,"Java exception in getRedefTime()."
 ,"Java exception in getAllSchemas()."
 ,"Preparing parameters for getAllTables()."
 ,"Java exception in getAllTables()."
};



//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
char* HiveClient_JNI::getErrorText(HVC_RetCode errEnum)
{
  if (errEnum < (HVC_RetCode)JOI_LAST)
    return JavaObjectInterface::getErrorText((JOI_RetCode)errEnum);
  else    
    return (char*)hvcErrorEnumStr[errEnum-HVC_FIRST-1];
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HiveClient_JNI* HiveClient_JNI::getInstance()
{
    ContextCli *currContext = GetCliGlobals()->currContext();
    HiveClient_JNI *hiveClient_JNI = currContext->getHiveClient();
    if (hiveClient_JNI == NULL)
    { 
       NAHeap *heap = currContext->exHeap();
       hiveClient_JNI = new (heap) HiveClient_JNI(heap);
       currContext->setHiveClient(hiveClient_JNI);
  }
  return hiveClient_JNI;
}

void HiveClient_JNI::deleteInstance()
{
  ContextCli *currContext = GetCliGlobals()->currContext();
  HiveClient_JNI *hiveClient_JNI = currContext->getHiveClient();
  if (hiveClient_JNI != NULL)
  {
     NAHeap *heap = currContext->exHeap();
     NADELETE(hiveClient_JNI, HiveClient_JNI, heap);
     currContext->setHiveClient(NULL);
  }
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HiveClient_JNI::~HiveClient_JNI()
{
  if (isConnected_)
    close(); // error handling?
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HVC_RetCode HiveClient_JNI::init()
{
  static char className[]="org/trafodion/sql/HBaseAccess/HiveClient";
  HVC_RetCode rc;
  
  if (isInitialized())
    return HVC_OK;
  
  if (javaMethodsInitialized_)
    return (HVC_RetCode)JavaObjectInterface::init(className, javaClass_, JavaMethods_, (Int32)JM_LAST, javaMethodsInitialized_);
  else
  {
    pthread_mutex_lock(&javaMethodsInitMutex_);
    if (javaMethodsInitialized_)
    {
      pthread_mutex_unlock(&javaMethodsInitMutex_);
      return (HVC_RetCode)JavaObjectInterface::init(className, javaClass_, JavaMethods_, (Int32)JM_LAST, javaMethodsInitialized_);
    }
    JavaMethods_ = new JavaMethodInit[JM_LAST];
    
    JavaMethods_[JM_CTOR       ].jm_name      = "<init>";
    JavaMethods_[JM_CTOR       ].jm_signature = "()V";
    JavaMethods_[JM_GET_ERROR  ].jm_name      = "getLastError";
    JavaMethods_[JM_GET_ERROR  ].jm_signature = "()Ljava/lang/String;";
    JavaMethods_[JM_INIT       ].jm_name      = "init";
    JavaMethods_[JM_INIT       ].jm_signature = "(Ljava/lang/String;)Z";
    JavaMethods_[JM_CLOSE      ].jm_name      = "close";
    JavaMethods_[JM_CLOSE      ].jm_signature = "()Z";
    JavaMethods_[JM_EXISTS     ].jm_name      = "exists";
    JavaMethods_[JM_EXISTS     ].jm_signature = "(Ljava/lang/String;Ljava/lang/String;)Z";
    JavaMethods_[JM_GET_HVT    ].jm_name      = "getHiveTableString";
    JavaMethods_[JM_GET_HVT    ].jm_signature = "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;";
    JavaMethods_[JM_GET_RDT    ].jm_name      = "getRedefTime";
    JavaMethods_[JM_GET_RDT    ].jm_signature = "(Ljava/lang/String;Ljava/lang/String;)J";
    JavaMethods_[JM_GET_ASH     ].jm_name      = "getAllSchemas";
    JavaMethods_[JM_GET_ASH     ].jm_signature = "()[Ljava/lang/Object;";
    JavaMethods_[JM_GET_ATL    ].jm_name      = "getAllTables";
    JavaMethods_[JM_GET_ATL    ].jm_signature = "(Ljava/lang/String;)[Ljava/lang/Object;";
   
    rc = (HVC_RetCode)JavaObjectInterface::init(className, javaClass_, JavaMethods_, (Int32)JM_LAST, javaMethodsInitialized_);
    javaMethodsInitialized_ = TRUE;
    pthread_mutex_unlock(&javaMethodsInitMutex_);
  }
  return rc;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
NAString HiveClient_JNI::getLastJavaError()
{
  return JavaObjectInterface::getLastJavaError(JavaMethods_[JM_GET_ERROR].methodID);
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HVC_RetCode HiveClient_JNI::initConnection(const char* metastoreURI)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HiveClient_JNI::initConnection(%s) called.", metastoreURI);

  jstring js_metastoreURI = jenv_->NewStringUTF(metastoreURI);
  if (js_metastoreURI == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HVC_ERROR_INIT_PARAM));
    return HVC_ERROR_INIT_PARAM;
  }
  

  // boolean init(java.lang.String); 
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_INIT].methodID, js_metastoreURI);

  jenv_->DeleteLocalRef(js_metastoreURI);   

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HiveClient_JNI::initConnection()", getLastError());
    return HVC_ERROR_INIT_EXCEPTION;
  }

  if (jresult == false) 
  {
    logError(CAT_HBASE, "HiveClient_JNI::initConnection()", getLastError());
    return HVC_ERROR_INIT_EXCEPTION;
  }

  isConnected_ = TRUE;
  return HVC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HVC_RetCode HiveClient_JNI::close()
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HiveClient_JNI::close() called.");

  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HVC_ERROR_INIT_PARAM;
  // boolean close();
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_CLOSE].methodID);
  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HiveClient_JNI::close()", getLastError());
    return HVC_ERROR_CLOSE_EXCEPTION;
  }
  
  if (jresult == false) 
  {
    logError(CAT_HBASE, "HiveClient_JNI::close()", getLastError());
    return HVC_ERROR_CLOSE_EXCEPTION;
  }

  return HVC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HVC_RetCode HiveClient_JNI::exists(const char* schName, const char* tabName)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HiveClient_JNI::exists(%s, %s) called.", schName, tabName);
  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HVC_ERROR_INIT_PARAM;
  jstring js_schName = jenv_->NewStringUTF(schName);
  if (js_schName == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HVC_ERROR_EXISTS_PARAM));
    return HVC_ERROR_EXISTS_PARAM;
  }
  jstring js_tabName = jenv_->NewStringUTF(tabName);
  if (js_tabName == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HVC_ERROR_EXISTS_PARAM));
    return HVC_ERROR_EXISTS_PARAM;
  }

  // boolean exists(java.lang.String, java.lang.String);
  jboolean jresult = jenv_->CallBooleanMethod(javaObj_, JavaMethods_[JM_EXISTS].methodID, js_schName, js_tabName);

  jenv_->DeleteLocalRef(js_schName);
  jenv_->DeleteLocalRef(js_tabName);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HiveClient_JNI::exists()", getLastError());
    return HVC_ERROR_EXISTS_EXCEPTION;
  }

  if (jresult == false) 
     return HVC_DONE;  // Table does not exist

  return HVC_OK;  // Table exists.
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HVC_RetCode HiveClient_JNI::getHiveTableStr(const char* schName, 
                                            const char* tabName, 
                                            Text& hiveTblStr)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "Enter HiveClient_JNI::getHiveTableStr(%s, %s, %s).", schName, tabName, hiveTblStr.data());
  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HVC_ERROR_INIT_PARAM;
  jstring js_schName = jenv_->NewStringUTF(schName);
  if (js_schName == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HVC_ERROR_GET_HVT_PARAM));
    return HVC_ERROR_GET_HVT_PARAM;
  }
  jstring js_tabName = jenv_->NewStringUTF(tabName);
  if (js_tabName == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HVC_ERROR_GET_HVT_PARAM));
    return HVC_ERROR_GET_HVT_PARAM;
  }

  // java.lang.String getHiveTableString(java.lang.String, java.lang.String);
  jstring jresult = (jstring)jenv_->CallObjectMethod(javaObj_, 
                                            JavaMethods_[JM_GET_HVT].methodID, 
                                            js_schName, js_tabName);

  jenv_->DeleteLocalRef(js_schName);
  jenv_->DeleteLocalRef(js_tabName);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HiveClient_JNI::getHiveTableStr()", getLastError());
    return HVC_ERROR_GET_HVT_EXCEPTION;
  }
 
  if (jresult == NULL)
     return HVC_DONE;
  if (jenv_->GetStringLength(jresult) <= 0)
  { 
     jenv_->DeleteLocalRef(jresult);
     return HVC_DONE; // Table does not exist
  }
    
  // Not using UFTchars and NAWString for now.
  const char* char_result = jenv_->GetStringUTFChars(jresult, 0);
  hiveTblStr += char_result ; // deep copy. hiveTblStr is assumed to be empty.
  jenv_->ReleaseStringUTFChars(jresult, char_result);
  jenv_->DeleteLocalRef(jresult);  

  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "Exit HiveClient_JNI::getHiveTableStr(%s, %s, %s).", schName, tabName, hiveTblStr.data());
  return HVC_OK;  // Table exists.
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////  
HVC_RetCode HiveClient_JNI::getRedefTime(const char* schName, 
                                         const char* tabName, 
                                         Int64& redefTime)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "Enter HiveClient_JNI::getRedefTime(%s, %s, %lld).", schName, tabName, redefTime);
  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HVC_ERROR_INIT_PARAM;
  jstring js_schName = jenv_->NewStringUTF(schName);
  if (js_schName == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HVC_ERROR_GET_REDEFTIME_PARAM));
    return HVC_ERROR_GET_REDEFTIME_PARAM;
  }
  jstring js_tabName = jenv_->NewStringUTF(tabName);
  if (js_tabName == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HVC_ERROR_GET_REDEFTIME_PARAM));
    return HVC_ERROR_GET_REDEFTIME_PARAM;
  }

  //  jlong getRedefTime(java.lang.String, java.lang.String);
  jlong jresult = jenv_->CallLongMethod(javaObj_, 
                                        JavaMethods_[JM_GET_RDT].methodID, 
                                        js_schName, js_tabName);

  jenv_->DeleteLocalRef(js_schName);
  jenv_->DeleteLocalRef(js_tabName);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HiveClient_JNI::getRedefTime()", getLastError());
    return HVC_ERROR_GET_REDEFTIME_EXCEPTION;
  }

  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "Exit HiveClient_JNI::getRedefTime(%s, %s, %lld).", schName, tabName, redefTime);

  if (jresult < 0)
    return HVC_DONE; // Table does not exist

  redefTime = jresult ;
  return HVC_OK;  // Table exists.
  
}

//////////////////////////////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////////////////////////////// 
HVC_RetCode HiveClient_JNI::getAllSchemas(LIST(Text *)& schNames)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "Enter HiveClient_JNI::getAllSchemas(%p) called.", (void *) &schNames);
  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HVC_ERROR_INIT_PARAM;

  jarray j_schNames= 
     (jarray)jenv_->CallObjectMethod(javaObj_, JavaMethods_[JM_GET_ASH].methodID);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HiveClient_JNI::getAllSchemas()", getLastError());
    return HVC_ERROR_GET_ALLSCH_EXCEPTION;
  }

  int numSchemas = convertStringObjectArrayToList(heap_, j_schNames,
                   schNames);           
  jenv_->DeleteLocalRef(j_schNames);
  if (numSchemas == 0)
     return HVC_DONE;
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, 
       "Exit HiveClient_JNI::getAllSchemas(%p) called.", (void *) &schNames);
  return HVC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
HVC_RetCode HiveClient_JNI::getAllTables(const char* schName, 
                                         LIST(Text *)& tblNames)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, "Enter HiveClient_JNI::getAllTables(%s, %p) called.", schName, (void *) &tblNames);
  if (jenv_ == NULL)
     if (initJVM() != JOI_OK)
         return HVC_ERROR_INIT_PARAM;

  jstring js_schName = jenv_->NewStringUTF(schName);
  if (js_schName == NULL) 
  {
    GetCliGlobals()->setJniErrorStr(getErrorText(HVC_ERROR_GET_ALLTBL_PARAM));
    return HVC_ERROR_GET_ALLTBL_PARAM;
  }

  jarray j_tblNames = 
    (jarray)jenv_->CallObjectMethod(javaObj_, JavaMethods_[JM_GET_ATL].methodID, 
                            js_schName);

  if (jenv_->ExceptionCheck())
  {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HiveClient_JNI::getAllTables()", getLastError());
    return HVC_ERROR_GET_ALLTBL_EXCEPTION;
  }

  int numTables = convertStringObjectArrayToList(heap_, j_tblNames,
                   tblNames);           
  jenv_->DeleteLocalRef(j_tblNames);
  if (numTables == 0)
     return HVC_DONE;

  return HVC_OK;
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////  
void HiveClient_JNI::logIt(const char* str)
{
  HdfsLogger::log(CAT_HBASE, LL_DEBUG, str);
}

#ifdef __cplusplus
extern "C" {
#endif
/*
 *   Class:     org_trafodion_sql_HBaseAccess_HTableClient
 *   Method:    setResultInfo
 *   Signature: (J[I[I[I[I[I[I[J[[B[B)I
 *  
 */

JNIEXPORT jint JNICALL Java_org_trafodion_sql_HBaseAccess_HTableClient_setResultInfo
  (JNIEnv *jenv, jobject jobj, jlong jniObject, 
	jintArray jKvValLen, jintArray jKvValOffset, 
        jintArray jKvQualLen, jintArray jKvQualOffset,
        jintArray jKvFamLen, jintArray jKvFamOffset, 
        jlongArray jTimestamp, 
        jobjectArray jKvBuffer, jobjectArray jRowIDs,
        jintArray jKvsPerRow, jint numCellsReturned)
{
   HTableClient_JNI *htc = (HTableClient_JNI *)jniObject;
   htc->setResultInfo(jKvValLen, jKvValOffset,
                jKvQualLen, jKvQualOffset, jKvFamLen, jKvFamOffset,
                jTimestamp, jKvBuffer, jRowIDs, jKvsPerRow, numCellsReturned);  
   return 0;
}

JNIEXPORT void JNICALL Java_org_trafodion_sql_HBaseAccess_HTableClient_cleanup
  (JNIEnv *jenv, jobject jobj, jlong jniObject)
{
   HTableClient_JNI *htc = (HTableClient_JNI *)jniObject;
   NADELETE(htc, HTableClient_JNI, htc->getHeap()); 
}

#ifdef __cplusplus
}
#endif

void HTableClient_JNI::setResultInfo( jintArray jKvValLen, jintArray jKvValOffset,
        jintArray jKvQualLen, jintArray jKvQualOffset,
        jintArray jKvFamLen, jintArray jKvFamOffset,
        jlongArray jTimestamp, 
        jobjectArray jKvBuffer, jobjectArray jRowIDs,
        jintArray jKvsPerRow, jint numCellsReturned)
{
  if (numRowsReturned_ > 0)
     cleanupResultInfo();
   NABoolean exceptionFound = FALSE;
   jKvValLen_ = (jintArray)jenv_->NewGlobalRef(jKvValLen);
   if (jenv_->ExceptionCheck())
       exceptionFound = TRUE;
   if (! exceptionFound)
   {
      jKvValOffset_ = (jintArray)jenv_->NewGlobalRef(jKvValOffset);
      if (jenv_->ExceptionCheck())
         exceptionFound = TRUE;
   }
   if (! exceptionFound)
   {
      jKvQualLen_ = (jintArray)jenv_->NewGlobalRef(jKvQualLen);
      if (jenv_->ExceptionCheck())
         exceptionFound = TRUE;
   }
   if (! exceptionFound)
   {
      jKvQualOffset_ = (jintArray)jenv_->NewGlobalRef(jKvQualOffset);
      if (jenv_->ExceptionCheck())
         exceptionFound = TRUE;
   }
   if (! exceptionFound)
   {
      jKvFamLen_ = (jintArray)jenv_->NewGlobalRef(jKvFamLen);
      if (jenv_->ExceptionCheck())
         exceptionFound = TRUE;
   }
   if (! exceptionFound)
   {
      jKvFamOffset_ = (jintArray)jenv_->NewGlobalRef(jKvFamOffset);
      if (jenv_->ExceptionCheck())
         exceptionFound = TRUE;
   }
   if (! exceptionFound)
   {
      jTimestamp_ = (jlongArray)jenv_->NewGlobalRef(jTimestamp);
      if (jenv_->ExceptionCheck())
         exceptionFound = TRUE;
   }
   if (! exceptionFound)
   {
      jKvBuffer_ = (jobjectArray)jenv_->NewGlobalRef(jKvBuffer);
      if (jenv_->ExceptionCheck())
         exceptionFound = TRUE;
   }
   if (! exceptionFound)
   {
      jRowIDs_ = (jobjectArray)jenv_->NewGlobalRef(jRowIDs);
      if (jenv_->ExceptionCheck())
         exceptionFound = TRUE;
   }
   if (! exceptionFound)
   {
      jKvsPerRow_ = (jintArray)jenv_->NewGlobalRef(jKvsPerRow);
      if (jenv_->ExceptionCheck())
         exceptionFound = TRUE;
   }
   numCellsReturned_ = numCellsReturned;
   currentRowNum_ = 0;
   prevRowCellNum_ = 0;
   cleanupDone_ = FALSE;
   ex_assert(! exceptionFound, "Exception in HTableClient_JNI::setResultInfo");
   return;
} 

void HTableClient_JNI::cleanupResultInfo()
{
   if (cleanupDone_)
      return;
   jenv_->DeleteGlobalRef(jKvValLen_);
   jenv_->DeleteGlobalRef(jKvValOffset_);
   jenv_->DeleteGlobalRef(jKvQualLen_);
   jenv_->DeleteGlobalRef(jKvQualOffset_);
   jenv_->DeleteGlobalRef(jKvFamLen_);
   jenv_->DeleteGlobalRef(jKvFamOffset_);
   jenv_->DeleteGlobalRef(jTimestamp_);
   jenv_->DeleteGlobalRef(jKvBuffer_);
   jenv_->DeleteGlobalRef(jRowIDs_);
   if (jba_kvBuffer_ != NULL)
   {
      jenv_->DeleteGlobalRef(jba_kvBuffer_);
      jba_kvBuffer_ = NULL;
   }
   if (p_rowID_ != NULL)
   {
      jenv_->ReleaseByteArrayElements(jba_rowID_, p_rowID_, JNI_ABORT);
      p_rowID_ = NULL;
      jenv_->DeleteGlobalRef(jba_rowID_);
   }
   if (p_kvsPerRow_ != NULL)
   {
      jenv_->ReleaseIntArrayElements(jKvsPerRow_, p_kvsPerRow_, JNI_ABORT);
       p_kvsPerRow_ = NULL;
   }
   jenv_->DeleteGlobalRef(jKvsPerRow_);
   cleanupDone_ = TRUE;
   return;
}

HTC_RetCode HTableClient_JNI::nextRow()
{
    HTC_RetCode retCode;

    ex_assert(fetchMode_ != UNKNOWN, "invalid fetchMode_");
    if (fetchMode_ == GET_ROW && numRowsReturned_ == -1)
       return HTC_DONE;
    if (currentRowNum_ == -1 || ((currentRowNum_+1) >= numRowsReturned_))
    {
        if (currentRowNum_ != -1)
        {
            switch (fetchMode_)
            {
              case BATCH_GET:
                cleanupResultInfo();
                return HTC_DONE_RESULT;
              case GET_ROW:
                cleanupResultInfo();
                return HTC_DONE;
              case SCAN_FETCH:
                if (numRowsReturned_ < numReqRows_)
                {
                   cleanupResultInfo();
                   return HTC_DONE;
                }   
                break;
            }
        }
        retCode = fetchRows();
        if (retCode != HTC_OK)
        {
           cleanupResultInfo();
           return retCode;
        }
        // Allocate Buffer and copy the cell info
        int numCellsNeeded;
        if (numCellsAllocated_ == 0 || 
			numCellsAllocated_ < numCellsReturned_)
        {
           NAHeap *heap = getHeap();
           if (numCellsAllocated_ > 0)
           {
              NADELETEBASIC(p_kvValLen_, heap);
              NADELETEBASIC(p_kvValOffset_, heap);
              NADELETEBASIC(p_kvFamLen_, heap);
              NADELETEBASIC(p_kvFamOffset_, heap);
              NADELETEBASIC(p_kvQualLen_, heap);
              NADELETEBASIC(p_kvQualOffset_, heap);
              NADELETEBASIC(p_timestamp_, heap);
              numCellsNeeded = numCellsReturned_;
           }
           else
           {  
              if (numColsInScan_ == 0)
                  numCellsNeeded = numCellsReturned_;
              else    
                  numCellsNeeded = 2 * numReqRows_ * numColsInScan_;
           }
           p_kvValLen_ = new (heap) jint[numCellsNeeded];
           p_kvValOffset_ = new (heap) jint[numCellsNeeded];
           p_kvFamLen_ = new (heap) jint[numCellsNeeded];
           p_kvFamOffset_ = new (heap) jint[numCellsNeeded];
           p_kvQualLen_ = new (heap) jint[numCellsNeeded];
           p_kvQualOffset_ = new (heap) jint[numCellsNeeded];
           p_timestamp_ = new (heap) jlong[numCellsNeeded];
           numCellsAllocated_ = numCellsNeeded;
        }
        jenv_->GetIntArrayRegion(jKvValLen_, 0, numCellsReturned_, p_kvValLen_);
        jenv_->GetIntArrayRegion(jKvValOffset_, 0, numCellsReturned_, p_kvValOffset_);
        jenv_->GetIntArrayRegion(jKvQualLen_, 0, numCellsReturned_, p_kvQualLen_);
        jenv_->GetIntArrayRegion(jKvQualOffset_, 0, numCellsReturned_, p_kvQualOffset_);
        jenv_->GetIntArrayRegion(jKvFamLen_, 0, numCellsReturned_, p_kvFamLen_);
        jenv_->GetIntArrayRegion(jKvFamOffset_, 0, numCellsReturned_, p_kvFamOffset_);
        jenv_->GetLongArrayRegion(jTimestamp_, 0, numCellsReturned_, p_timestamp_);
        p_kvsPerRow_ = jenv_->GetIntArrayElements(jKvsPerRow_, NULL);
        currentRowNum_ = 0;
        currentRowCellNum_ = 0;
        prevRowCellNum_ = 0;
    }
    else
    {
        // Add the number of previous cells returned
        jint kvsPerRow = p_kvsPerRow_[currentRowNum_];
        prevRowCellNum_ += kvsPerRow;  
        currentRowNum_++;
        currentRowCellNum_ = 0;
    }
    // clean the rowID of the previous row
    if (p_rowID_ != NULL)
    {
       jenv_->ReleaseByteArrayElements(jba_rowID_, p_rowID_, JNI_ABORT);
       p_rowID_ = NULL;
       jenv_->DeleteGlobalRef(jba_rowID_);
    }
    return HTC_OK;
}

HTC_RetCode HTableClient_JNI::getColName(int colNo,
              char **outColName, 
              short &colNameLen,
              Int64 &timestamp)
{
    jint kvsPerRow = p_kvsPerRow_[currentRowNum_];
    if (kvsPerRow == 0 || colNo >= kvsPerRow)
    {
       *outColName == NULL;
       timestamp = 0;
       return HTC_OK;
    }
    int idx = prevRowCellNum_ + colNo;
    ex_assert((idx < numCellsReturned_), "Buffer overflow");
    jint kvQualLen = p_kvQualLen_[idx];
    jint kvQualOffset = p_kvQualOffset_[idx];
    jint kvFamLen = p_kvFamLen_[idx];
    jint kvFamOffset = p_kvFamOffset_[idx];

    // clean the kvBuffer of the previous column
    // And get the kvBuffer for the current column
    if (jba_kvBuffer_ != NULL)
    {
       jenv_->DeleteGlobalRef(jba_kvBuffer_);
       jba_kvBuffer_ = NULL;
    }
    jobject kvBufferObj;
    kvBufferObj = jenv_->GetObjectArrayElement(jKvBuffer_, idx);
    if (jenv_->ExceptionCheck())
    {
      getExceptionDetails();
      logError(CAT_HBASE, __FILE__, __LINE__);
      logError(CAT_HBASE, "HTableClient_JNI::getColName()", getLastError());
      return HTC_GET_COLNAME_EXCEPTION;
    }

    jba_kvBuffer_ = (jbyteArray)jenv_->NewGlobalRef(kvBufferObj);
    if (jenv_->ExceptionCheck())
    {
      getExceptionDetails();
      logError(CAT_HBASE, __FILE__, __LINE__);
      logError(CAT_HBASE, "HTableClient_JNI::getColName()", getLastError());
      return HTC_GET_COLNAME_EXCEPTION;
    }
    jenv_->DeleteLocalRef(kvBufferObj);

    colNameLen = kvQualLen + kvFamLen + 1; // 1 for ':'
    char * colName;
    if (colNameAllocLen_ == 0  && colNameLen <= INLINE_COLNAME_LEN)
    	colName = inlineColName_;
    else
    {
        if (colNameLen > colNameAllocLen_)
        {
	   if (colNameAllocLen_ != 0)
              NADELETEBASIC(colName_, heap_);
           colName_ = new (heap_) char[colNameLen+1];
           colNameAllocLen_ = colNameLen;
        }
        colName = colName_; 
    }
    jenv_->GetByteArrayRegion(jba_kvBuffer_, kvFamOffset, kvFamLen, 
            (jbyte *)colName);
    colName[kvFamLen] = ':';
    char *temp = colName+ kvFamLen+1;
    jenv_->GetByteArrayRegion(jba_kvBuffer_, kvQualOffset, kvQualLen, 
            (jbyte *)temp);
    timestamp = p_timestamp_[idx];
    *outColName = colName;
    if (hbs_)
      hbs_->incBytesRead(sizeof(timestamp) + colNameLen);
    return HTC_OK; 
}

HTC_RetCode HTableClient_JNI::getColVal(int colNo, BYTE *colVal, 
          Lng32 &colValLen, NABoolean nullable, BYTE &nullVal)
{
    jint kvsPerRow = p_kvsPerRow_[currentRowNum_];
    if (kvsPerRow == 0 || colNo >= kvsPerRow)
       return HTC_GET_COLVAL_EXCEPTION;
    int idx = prevRowCellNum_ + colNo;
    ex_assert((idx < numCellsReturned_), "Buffer overflow");
    jint kvValLen = p_kvValLen_[idx];
    jint kvValOffset = p_kvValOffset_[idx];
    Lng32 copyLen;
    jbyte nullByte;
    // If the column is nullable, get the first byte
    // The first byte determines if the column is null(0xff) or not (0)
    if (nullable)
    {
       copyLen = MINOF(kvValLen-1, colValLen);
       jenv_->GetByteArrayRegion(jba_kvBuffer_, kvValOffset, 1, &nullByte); 
       jenv_->GetByteArrayRegion(jba_kvBuffer_, kvValOffset+1, copyLen, 
               (jbyte *)colVal); 
    }
    else 
    {
        copyLen = MINOF(kvValLen, colValLen);
        nullByte = 0;
    	jenv_->GetByteArrayRegion(jba_kvBuffer_, kvValOffset, copyLen,
             (jbyte *)colVal); 
    }
    nullVal = nullByte;
    colValLen = copyLen;
    if (hbs_)
      hbs_->incBytesRead(colValLen);
    return HTC_OK;
}

HTC_RetCode HTableClient_JNI::getColVal(NAHeap *heap, int colNo, BYTE **colVal, 
          Lng32 &colValLen)
{
    jint kvsPerRow = p_kvsPerRow_[currentRowNum_];
    if (kvsPerRow == 0 || colNo >= kvsPerRow)
       return HTC_GET_COLVAL_EXCEPTION;
    int idx = prevRowCellNum_ + colNo;
    ex_assert((idx < numCellsReturned_), "Buffer overflow");
    jint kvValLen = p_kvValLen_[idx];
    jint kvValOffset = p_kvValOffset_[idx];
   
    BYTE *colValTmp;
    int colValLenTmp;
    if (heap == NULL)
    {
       colValTmp = *colVal; 
       colValLenTmp = colValLen;
       if (colValLenTmp > kvValLen)
          colValLenTmp = kvValLen;
    }
    else
    {
       colValTmp = new (heap) BYTE[kvValLen];
       colValLenTmp = kvValLen;
    }
    jenv_->GetByteArrayRegion(jba_kvBuffer_, kvValOffset, colValLenTmp,
             (jbyte *)colValTmp); 
    *colVal = colValTmp;
    colValLen = colValLenTmp;
    if (hbs_)
      hbs_->incBytesRead(colValLen);
    return HTC_OK;
}

HTC_RetCode HTableClient_JNI::getNumCols(int &numCols)
{
    jint kvsPerRow = p_kvsPerRow_[currentRowNum_];
    numCols = kvsPerRow;
    if (numCols == 0)
       return HTC_DONE_DATA;
    else
       return HTC_OK;
}  


HTC_RetCode HTableClient_JNI::getRowID(HbaseStr &rowID)
{
    jint kvsPerRow = p_kvsPerRow_[currentRowNum_];
    if (p_rowID_ != NULL)
    {
      jenv_->ReleaseByteArrayElements(jba_rowID_, p_rowID_, JNI_ABORT);
      p_rowID_ = NULL;
      jenv_->DeleteGlobalRef(jba_rowID_);
    }

    if (kvsPerRow == 0) 
    {
       rowID.len = 0;
       rowID.val = NULL;
    }
    else
    {
       jobject rowIDObj;
       rowIDObj = jenv_->GetObjectArrayElement(jRowIDs_, currentRowNum_);
       jba_rowID_ = (jbyteArray)jenv_->NewGlobalRef(rowIDObj);
       if (jenv_->ExceptionCheck())
       {
          getExceptionDetails();
          logError(CAT_HBASE, __FILE__, __LINE__);
          logError(CAT_HBASE, "HTableClient_JNI::getRowID()", getLastError());
          return HTC_GET_ROWID_EXCEPTION;
       }
       jenv_->DeleteLocalRef(rowIDObj);
       p_rowID_ = jenv_->GetByteArrayElements(jba_rowID_, NULL);
       rowIDLen_ = jenv_->GetArrayLength(jba_rowID_); 
       rowID.len = rowIDLen_;
       rowID.val = (char *)p_rowID_;
    }
    return HTC_OK;
}

HTC_RetCode HTableClient_JNI::fetchRows()
{
   HdfsLogger::log(CAT_HBASE, LL_DEBUG, "HTableClient_JNI::fetchRows() called.");
   jlong jniObject = (jlong)this;
   if (hbs_)
     hbs_->getTimer().start();
   jlong jRowsReturned = jenv_->CallLongMethod(javaObj_, 
             JavaMethods_[JM_FETCH_ROWS].methodID,
             jniObject);
   if (hbs_)
    {
      hbs_->getTimer().stop();
      hbs_->incHbaseCalls();
    }

   if (jenv_->ExceptionCheck())
   {
      getExceptionDetails();
      logError(CAT_HBASE, __FILE__, __LINE__);
      logError(CAT_HBASE, "HTableClient_JNI::fetchRows()", getLastError());
      return HTC_ERROR_FETCHROWS_EXCEPTION;
   }

   numRowsReturned_ = jRowsReturned;
   if (numRowsReturned_ == -1)
   {
      logError(CAT_HBASE, "HTableClient_JNI::fetchRows()", getLastJavaError());
      return HTC_ERROR_FETCHROWS_EXCEPTION;
   }
   else
   if (numRowsReturned_ == 0)
      return HTC_DONE;
   if (hbs_)
      hbs_->incAccessedRows(numRowsReturned_);
   return HTC_OK; 
}

HTC_RetCode HTableClient_JNI::setJniObject()
{
   jlong jniObject = (jlong)this;
   jenv_->CallVoidMethod(javaObj_, JavaMethods_[JM_SET_JNIOBJECT].methodID, 
         jniObject);
   if (jenv_->ExceptionCheck())
   {
    getExceptionDetails();
    logError(CAT_HBASE, __FILE__, __LINE__);
    logError(CAT_HBASE, "HBaseClient_JNI::setJniObject()", getLastError());
    return HTC_SET_JNIOBJECT_EXCEPTION;
  }
  return HTC_OK;
}

HTC_RetCode HTableClient_JNI::nextCell(
        	 HbaseStr &rowId,
                 HbaseStr &colFamName,
                 HbaseStr &colQualName,
                 HbaseStr &colVal,
                 Int64 &timestamp)
{
   HTC_RetCode retcode;
   jint kvsPerRow = p_kvsPerRow_[currentRowNum_];
   if (currentRowCellNum_ >= kvsPerRow)
   {
      currentRowCellNum_ = -1;
      return HTC_DONE;
   }
   if (p_rowID_ != NULL)
   {
      rowId.val = (char *)p_rowID_;
      rowId.len = rowIDLen_;
   }
   else
   {
      retcode = getRowID(rowId);
      if (retcode != HTC_OK)
         return retcode;
   }
   int idx = prevRowCellNum_ + currentRowCellNum_;
   ex_assert((idx < numCellsReturned_), "Buffer overflow");
   jint kvQualLen = p_kvQualLen_[idx];
   jint kvQualOffset = p_kvQualOffset_[idx];
   jint kvFamLen = p_kvFamLen_[idx];
   jint kvFamOffset = p_kvFamOffset_[idx];

   // clean the kvBuffer of the previous column
   // And get the kvBuffer for the current column

  if (jba_kvBuffer_ != NULL)
   {
       jenv_->DeleteGlobalRef(jba_kvBuffer_);
       jba_kvBuffer_ = NULL;
   }
   jobject kvBufferObj;
   kvBufferObj = jenv_->GetObjectArrayElement(jKvBuffer_, idx);
   if (jenv_->ExceptionCheck())
   {
      getExceptionDetails();
      logError(CAT_HBASE, __FILE__, __LINE__);
      logError(CAT_HBASE, "HTableClient_JNI::nextCell()", getLastError());
      return HTC_NEXTCELL_EXCEPTION;
   }

   jba_kvBuffer_ = (jbyteArray)jenv_->NewGlobalRef(kvBufferObj);
   if (jenv_->ExceptionCheck())
   {
      getExceptionDetails();
      logError(CAT_HBASE, __FILE__, __LINE__);
      logError(CAT_HBASE, "HTableClient_JNI::nextCell()", getLastError());
      return HTC_NEXTCELL_EXCEPTION;
   }
   jenv_->DeleteLocalRef(kvBufferObj);

   int colNameLen = kvQualLen + kvFamLen + 1; // 1 for ':'
   char * colName;
   if (colNameAllocLen_ == 0  && colNameLen <= INLINE_COLNAME_LEN)
      colName = inlineColName_;
   else
   {
      if (colNameLen > colNameAllocLen_)
      {
         if (colNameAllocLen_ != 0)
             NADELETEBASIC(colName_, heap_);
         colName_ = new (heap_) char[colNameLen+1];
         colNameAllocLen_ = colNameLen;
      }
      colName = colName_;
   }
   jenv_->GetByteArrayRegion(jba_kvBuffer_, kvFamOffset, kvFamLen,
            (jbyte *)colName);
   colName[kvFamLen] = ':';
   colFamName.val = colName;
   colFamName.len = kvFamLen; 
   char *temp = colName+ kvFamLen+1;
   jenv_->GetByteArrayRegion(jba_kvBuffer_, kvQualOffset, kvQualLen,
            (jbyte *)temp);
   colQualName.val = temp;
   colQualName.len = kvQualLen;
   timestamp = p_timestamp_[idx];
   retcode = getColVal(NULL, currentRowCellNum_, (BYTE **)&colVal.val,
                         colVal.len);
   if (retcode != HTC_OK)
      return retcode;
   currentRowCellNum_++;
   return HTC_OK;
}

jobjectArray convertToByteArrayObjectArray(const TextVec &vec)
{
   int vecLen = vec.size();
   int i = 0;
   jobjectArray j_objArray = NULL;
   for (std::vector<Text>::const_iterator it = vec.begin(); 
           it != vec.end(); ++it, i++)
   {
       jbyteArray j_obj = jenv_->NewByteArray((*it).size());
       if (jenv_->ExceptionCheck())
       {
          if (j_objArray != NULL)
             jenv_->DeleteLocalRef(j_objArray);
          return NULL; 
       }
       jenv_->SetByteArrayRegion(j_obj, 0, (*it).size(), (const jbyte *)(*it).data());
       if (j_objArray == NULL)
       {
          j_objArray = jenv_->NewObjectArray(vecLen,
                 jenv_->GetObjectClass(j_obj), NULL);
          if (jenv_->ExceptionCheck())
          {
             jenv_->DeleteLocalRef(j_obj);
             return NULL;
          }
       }
       jenv_->SetObjectArrayElement(j_objArray, i, (jobject)j_obj);
       jenv_->DeleteLocalRef(j_obj);
   }
   return j_objArray;
}

jobjectArray convertToByteArrayObjectArray(const char **array,
                   int numElements, int elementLen)
{
   int i = 0;
   jobjectArray j_objArray = NULL;
   for (i = 0; i < numElements; i++)
   {
       jbyteArray j_obj = jenv_->NewByteArray(elementLen);
       if (jenv_->ExceptionCheck())
       {
          if (j_objArray != NULL)
             jenv_->DeleteLocalRef(j_objArray);
          return NULL; 
       }
       jenv_->SetByteArrayRegion(j_obj, 0, elementLen,
             (const jbyte *)(array[i]));
       if (j_objArray == NULL)
       {
          j_objArray = jenv_->NewObjectArray(numElements,
                 jenv_->GetObjectClass(j_obj), NULL);
          if (jenv_->ExceptionCheck())
          {
             jenv_->DeleteLocalRef(j_obj);
             return NULL;
          }
       }
       jenv_->SetObjectArrayElement(j_objArray, i, (jobject)j_obj);
       jenv_->DeleteLocalRef(j_obj);
   }
   return j_objArray;
}

jobjectArray convertToStringObjectArray(const TextVec &vec)
{
   int vecLen = vec.size();
   int i = 0;
   jobjectArray j_objArray = NULL;
   for (std::vector<Text>::const_iterator it = vec.begin(); 
           it != vec.end(); ++it, i++)
   {
       jstring j_obj = jenv_->NewStringUTF((*it).data());
       if (jenv_->ExceptionCheck())
       {
          if (j_objArray != NULL)
             jenv_->DeleteLocalRef(j_objArray);
          return NULL; 
       }
       if (j_objArray == NULL)
       {
          j_objArray = jenv_->NewObjectArray(vecLen,
                 jenv_->GetObjectClass(j_obj), NULL);
          if (jenv_->ExceptionCheck())
          {
             jenv_->DeleteLocalRef(j_obj);
             return NULL;
          }
       }
       jenv_->SetObjectArrayElement(j_objArray, i, (jobject)j_obj);
       jenv_->DeleteLocalRef(j_obj);
   }
   return j_objArray;
}

jobjectArray convertToStringObjectArray(const HBASE_NAMELIST& nameList)
{
   int listLen = nameList.entries();
   int i = 0;
   jobjectArray j_objArray = NULL;
   for ( i = 0; i < listLen ; i++)
   {
       jstring j_obj = jenv_->NewStringUTF(nameList.at(i).val);
       if (jenv_->ExceptionCheck())
       {
          if (j_objArray != NULL)
             jenv_->DeleteLocalRef(j_objArray);
          return NULL; 
       }
       if (j_objArray == NULL)
       {
          j_objArray = jenv_->NewObjectArray(listLen,
                 jenv_->GetObjectClass(j_obj), NULL);
          if (jenv_->ExceptionCheck())
          {
             jenv_->DeleteLocalRef(j_obj);
             return NULL;
          }
       }
       jenv_->SetObjectArrayElement(j_objArray, i, (jobject)j_obj);
       jenv_->DeleteLocalRef(j_obj);
   }
   return j_objArray;
}

jobjectArray convertToStringObjectArray(const NAText *textArray, int arrayLen)
{
   int i = 0;
   jobjectArray j_objArray = NULL;
   for ( i = 0; i < arrayLen ; i++)
   {
       jstring j_obj = jenv_->NewStringUTF(textArray[i].c_str());
       if (jenv_->ExceptionCheck())
       {
          if (j_objArray != NULL)
             jenv_->DeleteLocalRef(j_objArray);
          return NULL; 
       }
       if (j_objArray == NULL)
       {
          j_objArray = jenv_->NewObjectArray(arrayLen,
                 jenv_->GetObjectClass(j_obj), NULL);
          if (jenv_->ExceptionCheck())
          {
             jenv_->DeleteLocalRef(j_obj);
             return NULL;
          }
       }
       jenv_->SetObjectArrayElement(j_objArray, i, (jobject)j_obj);
       jenv_->DeleteLocalRef(j_obj);
   }
   return j_objArray;
}

int convertStringObjectArrayToList(NAHeap *heap, jarray j_objArray, 
                                         LIST(Text *)&list)
{

    if (j_objArray == NULL)
        return 0;
    int arrayLen = jenv_->GetArrayLength(j_objArray);
    jstring j_str;
    const char *str;
    jboolean isCopy;

    for (int i = 0; i < arrayLen; i++)
    {
        j_str = (jstring)jenv_->GetObjectArrayElement((jobjectArray)j_objArray, i);
        str = jenv_->GetStringUTFChars(j_str, &isCopy);
        list.insert(new (heap) Text(str));
        jenv_->ReleaseStringUTFChars(j_str, str);        
    }
    return arrayLen;
}

