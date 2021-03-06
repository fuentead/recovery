/**********************************************************************
// @@@ START COPYRIGHT @@@
//
// (C) Copyright 1996-2014 Hewlett-Packard Development Company, L.P.
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
******************************************************************************
*
* File:         ExExplain.C
* Description:  Methods for the tdb and tcb of a Explain operation
*
*
* Created:      4/17/96
* Language:     C++
*
*
*
*
******************************************************************************
*/

//
// This file contains all the executor methods associated
// with a explain operator
//
#include "ex_stdh.h"
#include "ComTdb.h"
#include "ex_tcb.h"
#include "ex_root.h"
#include "ex_queue.h"
#include "ExExplain.h"
#include "ExplainTuple.h"
#include "ex_expr.h"
#include "exp_like.h"
#include "ComQueue.h"
#include "sql_id.h"
#include "SqlStats.h"
#include "ssmpipc.h"
#include "ComCextdecs.h"
// Default Constructor.  This is used by the ex_tdb::fixupVTblPtr()
// routine which fixes up the virtual function table pointer after
// a node has been unpacked.

//
// Build a explain tcb from an explain TDB.
// Allocates a new ExExplainTcb.  
// Initializes the paramsTuple.
// Adds this tcb to the schedulers task list.
ex_tcb *
ExExplainTdb::build(ex_globals * glob)
{

  // Allocate and initialize a new explain TCB.
  ExExplainTcb *explainTcb = new(glob->getSpace()) ExExplainTcb(*this, glob);

  // initParamsTuple: Allocate the paramsTuple_, the paramsAtp, the
  // modName_ buffer and the stmtPattern_ buffer.  The paramsTuple_ will
  // hold the result of evaluating the paramsExpr.  The modName_ and
  // stmtPattern_ buffers are used to hold a NULL terminated copy of the
  // parameters.  Also, initialize the paramsTuppDesc_ to point to the
  // paramsTuple_ and the paramsTupp_ to point to the paramsTuppDesc_.
  // This is done to create the necessary tuple structure so that the
  // paramsExpr can be evaluateed.
  explainTcb->initParamsTuple((unsigned short)tupleLength_,
			      criDescParams_,
			      getLengthModName(),
			      getLengthStmtPattern());

  // add the explain tcb to the scheduler's task list.
  explainTcb->registerSubtasks();
  
  return (explainTcb);
}



//
//  TCB procedures
//

//
// Constructor for ExExplainTcb.  Called by the build function of the
// ExExplainTdb.  This will initialize the internal state of the explain
// Tcb and allocate the queues used to communicate with the parent.
// Fixup all expression used by the TCB.

ExExplainTcb::ExExplainTcb(const ExExplainTdb &explainTdb, ex_globals *glob)
: ex_tcb(explainTdb, 1, glob),
  stmtList_(0),
  currentStmt_(0),
  explainTree_(0),
  currentExplain_(0),
  cameFrom_(0),
  seqNum_(0),
  matchNo_(0),
  debugCounter_(0),
  workState_(EXPL_IDLE),
  paramsTuple_(0),
  modName_(NULL),
  modDir_(NULL),
  stmtPattern_(0),
  qid_(NULL),
  diagsArea_(NULL),
  retryAttempts_(0),
  stmtName_(NULL)
{

  Space * space = (glob ? glob->getSpace() : 0);
  CollHeap * heap = (glob ? glob->getDefaultHeap() : 0);

  // Allocate the buffer pool
  // Allocate the specified number of buffers each can hold 5 tuples.
#pragma nowarn(1506)   // warning elimination 
  pool_ = new(space) sql_buffer_pool(explainTdb.numBuffers_,
				     explainTdb.bufferSize_,
				     space);
#pragma warn(1506)  // warning elimination 

  // Allocate the queues used to communicate with parent
  qParent_.down = new(space) ex_queue(ex_queue::DOWN_QUEUE,
				      explainTdb.queueSizeDown_,
				      explainTdb.criDescDown_,
				      space);


  qParent_.up = new(space) ex_queue(ex_queue::UP_QUEUE,
				    explainTdb.queueSizeUp_,
				    explainTdb.criDescUp_,
				    space);

  // fixup expressions

  if (getScanPred())
    (void) getScanPred()->fixup(0, getExpressionMode(), this,
				space, heap, FALSE, glob);

  if (getParamsExpr())
    (void) getParamsExpr()->fixup(0, getExpressionMode(), this,
				  space, heap, FALSE, glob);

}

// Destructor for explain tcb
ExExplainTcb::~ExExplainTcb()
{
  if (qid_)
  {
    NADELETEBASIC(qid_, getHeap());
    qid_ = NULL;
  }
  if (stmtName_)
  {
    NADELETEBASIC(stmtName_, getHeap());
    stmtName_ = NULL;
  }
  delete qParent_.up;
  delete qParent_.down;
}
  
// Free Resources (what should this do?)
void ExExplainTcb::freeResources()
{
  if (qid_)
  {
    NADELETEBASIC(qid_, getHeap());
    qid_ = NULL;
  }
  if (stmtName_)
  {
    NADELETEBASIC(stmtName_, getHeap());
    stmtName_ = NULL;
  }
  delete qParent_.up;
  delete qParent_.down;
  retryAttempts_ = 0;
}

// Explain has no children
Int32
ExExplainTcb::numChildren() const
{
  return(0);
}

const ex_tcb *ExExplainTcb::getChild(Int32 /*pos*/) const
{
  ex_assert(0,"Explain TCB has 0 children");
  return NULL;
}

ex_queue_pair  
ExExplainTcb::getParentQueue() const
{
  return (qParent_);
};


// loadModule: load the named module if it has not already been loaded.
// assumes that the buffer modName_ has been allocated and contains the
// name of the module calculated by evaluating the paramsExpr.  Also,
// initialize the statement list (stmtList_) to point to the statement
// list of the current context.  The context is only available from the
// master executor.
Int32
ExExplainTcb::loadModule()
{
  Int32 rc = 0;

  // Get the globals stucture of the master executor.
  ExExeStmtGlobals *exeGlob = getGlobals()->castToExExeStmtGlobals();
  ExMasterStmtGlobals *masterGlob = exeGlob->castToExMasterStmtGlobals();

  // If there are no master globals, this is not the master executor.
  // The optimizer should ensure that this node executes in the master.
  ex_assert(masterGlob,"ExExplainTcb : No master globals Available\n");

  // Get the current context from the master globals.
  ContextCli *context = masterGlob->getContext();

  // There should always be a valid pointer to the current context
  ex_assert(context,"ExExplainTcb : No context Available\n");

  // If the module name is NULL, the current module should be used,
  // so no module needs to be loaded.  getNextExplainTree() should
  // interpret a NULL module name as the current module.
  if(!isNullModName())
    {
      // Construct a module ID to be used to search the module list
      // and possible load a new module.
      SQLMODULE_ID moduleId;

      // Added for multi charset module names
#pragma nowarn(1506)   // warning elimination 
      init_SQLMODULE_ID(&moduleId, 
			SQLCLI_CURRENT_VERSION,
			modName_,
			0, SQLCHARSETSTRING_ISO88591, strlen(modName_)
		       );
#pragma warn(1506)  // warning elimination 
      //moduleId.module_name = modName_;

      // If the module isn't already loaded (added) load it
      if(!context->moduleAdded(&moduleId))
	{
	  rc = context->addModule(&moduleId, FALSE /*no timestamp check*/,
				  TRUE /* unpack tdbs*/, 
				  (modDir_[0] ? modDir_ : NULL),
				  FALSE /*do not look in global mod dir*/);
	}
    }
  
  // Get a pointer to the statement list for this context.
  stmtList_ = context->getStatementList();
  stmtList_->position();
  
  currentStmt_ = 0;

  return rc;
}

//  visitNode: visit a node of the explainTree.  This routine is called
//  by traverseTree() on each node of the explainTree.  visitNode will
//  check to make sure that this node has been initialized and has been
//  populated with explain Info.  If it has, it will then apply the 
//  scan predicate (if it exists) to the node and if true will insert
//  the explain Tuple into the parent up queue.
Int32
ExExplainTcb::visitNode()
{
  
  ex_queue_entry *pEntryDown = qParent_.down->getHeadEntry();

  ex_assert(!qParent_.up->isFull(),
	    "Explain: Visiting a node when parant queue is full\n");
      
  
  // If there is actually a buffer allocated and it contains some
  // Explain Info. then apply the predicate and if true, insert
  // onto the parents up queue
  if(currentExplain_->getState() == ExplainTuple::SOME_EXPLAIN_INFO) 
    {
      
      // Get a pointer to the available entry in the parent up queue.
      // It is guaranteed to be available since we checked in
      // traverseTree() before calling this routine.
      ex_queue_entry *pEntryUp = qParent_.up->getTailEntry();
      
      // Copy given values from parent down queue to parent up queue.
      pEntryUp->copyAtp(pEntryDown);

      // Get a new tuple from the POOL.
      // If we can't get one now, return and will check again later.
      // Should eventually be able to get one.

      if (pool_->get_free_tuple(pEntryUp->getTupp(explainTdb().getTuppIndex()),
				currentExplain_->getRecLength()))
	return 0;

      // Keep track of the sequence number for each node.
      // The sequence number is a enumeration of the nodes in
      // traversal order.  (traverseTree() traverses post-order)
      seqNum_++;
      
      // Set the module name and statement name for this node.
      // These fields are set at run time since they are not
      // know at generate time.
      
      if(isNullModName())
	currentExplain_->setModuleName(0);
      else
	currentExplain_->setModuleName(modName_);
      
      if (qid_ == NULL)
        currentExplain_->setStatementName(currentStmt_->getIdentifier());
      else
        currentExplain_->setStatementName(stmtName_);

      // Copy row from Explain Tree to buffer.
      // This should probably be a move expression of some sort,
      // but for now this seems to work fine.

      str_cpy_all(pEntryUp->getTupp(explainTdb().getTuppIndex()).getDataPointer(),
		  currentExplain_->getExplainTuple(),
		  currentExplain_->getRecLength());

      // Apply scan Predicate to the explain Info. (now in the
      // tail of the parent up queue.  If the predicat evaluates
      // to TRUE or is non-existant, the commit (insert) this queue
      // entry. Otherwise, ignore it.
      
      ex_expr::exp_return_type predVal = ex_expr::EXPR_TRUE;
      
      if (getScanPred()) 
	predVal = getScanPred()->eval(pEntryUp->getAtp(), 0);
      
      if(predVal == ex_expr::EXPR_TRUE) {
	// Finialize the queue entry, then insert it
	matchNo_++;
	pEntryUp->upState.status = ex_queue::Q_OK_MMORE;
	pEntryUp->upState.parentIndex = pEntryDown->downState.parentIndex;
	pEntryUp->upState.downIndex = qParent_.down->getHeadIndex();  
	pEntryUp->upState.setMatchNo(matchNo_);
	
	qParent_.up->insert();
      }
      else if (predVal == ex_expr::EXPR_ERROR) {
        pEntryUp->upState.status = ex_queue::Q_SQLERROR;
        pEntryUp->upState.parentIndex = pEntryDown->downState.parentIndex;
        pEntryUp->upState.downIndex = qParent_.down->getHeadIndex();
	pEntryUp->upState.setMatchNo(matchNo_);

        qParent_.up->insert();

        workState_ = EXPL_DONE;
        return 0;
      }
    }
  return 1;
}

// traverseTree: Do a post-order traversal of the explainTree.  As each
// node is visited (visitNode()) it could potentially insert one tuple
// into the parents up queue.  This routine will return when either the
// traversal is complete (EXPL_TRAV_DONE) or if the parent up queue is
// full (EXPL_TRAV_QUEUEFULL).  If the queue is full, the work procedure
// will return to the scheduler. When the work procedure is called again,
// it will call traverseTree() which will resume traversing the
// explainTree where it left off.  traverseTree must therefore keep
// enough state in the TCB to be able to resume the traversal (a simple
// recursive traversal will not work since it must be able to return and
// then be restarted). This routine is initially called by the work
// procedure when it has found a matching statement name within the named
// module.  Before being called, currentExplain_ is initialized to point
// to the root of the explainTree and cameFrom_ is initialized to NULL.
// currentExplain_ will be maintained to always point to the node of the
// explainTree that is currently being visited.  cameFrom_ is used to
// determine which child was last visited. After a node has been visited,
// the cameFrom_ pointer is set to point to this node and the traversal
// follows the parent pointer to the parent node.  The parent node then
// iterates over all the children pointers.  If the cameFrom_ pointer is
// NULL, cameFrom_ is set to NULL and the traversal follows that child
// pointer.  If the cameFrom_ pointer is equal to the child pointer (this
// was the last child visited), cameFrom_ is set to NULL so that the 
// traversal will visit the next child.


ExExplainTcb::traverseReturnCode ExExplainTcb::traverseTree() 
{
  
  // Get a pointer to the current request that we are working on
  ex_queue_entry *pEntryDown = qParent_.down->getHeadEntry();
  
  // Visit every node in the explainTree
  while(currentExplain_) 
    {
      // If the parent queue is full, return.
      // traverseTree() will be called again to resume the
      // traversal
      if (qParent_.up->isFull()) 
	return EXPL_TRAV_QUEUEFULL;
      
      // Get the type of request we are dealing with
      const ex_queue::down_request request = pEntryDown->downState.request;
      
      // If the request has been canceled or if the request has
      // been satisfied, the return to the work procedure.
      // The work procedure will see if there are any more trees
      // to traverse.  If so, traverseTree() will be called again.
      // If not the work procedure will put the NO_DATA reply onto the
      // parent up queue and then see if there are any more requests
      // to process
      if ((request == ex_queue::GET_NOMORE) ||
	  ((request == ex_queue::GET_N) && 
	   ((UInt32)pEntryDown->downState.requestValue <= matchNo_)))  // NT_PORT  ( bd 10/22/96 ) cast to unsigned
	return EXPL_TRAV_DONE;
      
      // Visit each child node.  If we have already visited this
      // child, cameFrom_ will be non-NULL.  If this is the last
      // child we visited, cameFrom_ will be equal to child(i).
      // Note that the loop index is altered from within the body
      // of the loop.
      for(Int32 i = 0; i < currentExplain_->numChildren(); )
	{
	  if(cameFrom_ == 0) 
	    // We have not visited this child.  If there is a valid
	    // child pointer, follow it.
	    {
	      // Follow child pointer down the explainTree
	      if(currentExplain_->child(i))
		{
		  currentExplain_ = currentExplain_->child(i);
		  
		  // Reset the child index
		  i = 0;
		}
	      else
		// A NULL child pointer (ignore this child)
		i++;
	    }
	  else if (cameFrom_ == currentExplain_->child(i))
	    {
	      // Reset the cameFrom_ pointer so the traversal will
	      // follow the next child pointer (if there is one)
	      cameFrom_ = 0;
	      i++;
	    }
	  else
	    {
	      // This node has already been visited. Still looking
	      // for child equal to cameFrom_.
	      i++;
	    }
	}

      // We have Visited all the children of this node.
      // Make sure that we don't visit them again.
      cameFrom_ = currentExplain_;
      
      // Visit This node.
      if(!visitNode()) {
        if (workState_ == EXPL_DONE)  
          return EXPL_TRAV_ERROR;
        else
	  // Not really full, but same result.
	  return EXPL_TRAV_NOBUFFERS;
      }
      
      // This node is complete.  Follow the parent pointer up the tree
      // and set cameFrom_ to indicate that this node has just been
      // visited.
      cameFrom_ = currentExplain_;
      currentExplain_ = cameFrom_->getParent();
    }
  
  // Traversal of this tree is complete.
  return EXPL_TRAV_DONE;
}

// Explain work procedure:  The explain work procedure is responsible for
// implementing the explain operator.  Given a request it will:
//
//  - calculate the parameters to the explain function (module name
//    and statement pattern)
//
//  - loaded the named module if necessary
//
//  - travserse the list of statements and for each statement that
//    matches the named module and the statement pattern it will:
//
//    - extracted, copy and unpack the explain fragment for the statement
//
//    - traverse the explainTree, inserting qualifying tuples into the
//      parent up queue.
//
// The work procedure is implemented as a State Machine.  The current
// state is stored in the TCB as workState_.  The state machine has more
// states that are necessary since the work procedure can only return
// while in certain states.  The states are:
//
//     EXPL_IDLE - Waiting for a request.  This is the initial state.  It
//     stays in this state until a request is placed in the parent down
//     queue, in which case it will go to EXPL_GET_PARAMS
//                              
//     EXPL_GET_PARAMS - A request has been received.  The paramExpr() will
//     be evaluated to calculate the values of the explain functions
//     parameters.  This state will always proceed to 
//     EXPL_GETNEXT_EXPLAINTREE.  The work procedure will never return
//     while in this state.
//
//     EXPL_GETNEXT_EXPLAINTREE - The statement list will be traversed to 
//     find the next matching statements.  If none exist, then the state
//     will go to EXPL_DONE.  If one exists, the state will go to
//     EXPL_TRAVERSE_EXPLAINTREE.  The work procedure will never return 
//     while in this state
//
//     EXPL_TRAVERSE_EXPLAINTREE - Traverse the explainTree.  If the parent
//     up queue becomes full, return to the sceduler and then restart in
//     this state when called again.  This state will always go to state
//     EXPL_GETNEXT_EXPLAINTREE.
//
//     EXPL_DONE - Done with all statements that match the module name
//     and statement pattern.  Place the NO_DATA entry on the parent up
//     queue, then go to state EXPL_IDLE to get the next request.

short ExExplainTcb::work()
{
  Int32 rc = 0;
  char *explainFrag;
  Lng32 topNodeOffset;
  char * fragStart;
  Lng32 fragLen;

  ex_queue_entry *pEntryUp;	// A pointer the the next available entry
  // of the parent up queue.
  
  ex_queue_entry *pEntryDown;	// A pointer to the current request entry
  // in the parent down queue.
  CliGlobals *cliGlobals = getGlobals()->castToExExeStmtGlobals()->
	                castToExMasterStmtGlobals()->getCliGlobals();
  // State Machine Loop
  while(1)
    {
      switch(workState_)
	{
	case EXPL_IDLE:
	  
	  // Looking for inputs from parent
	  
	  if (qParent_.down->isEmpty())
	    return WORK_OK;
	  
	  // We have some inputs from the parent.
	  // Determine the values of the parameters
	  workState_ = EXPL_GET_PARAMS;
	  break;
	  
	case EXPL_GET_PARAMS:
	  ex_assert(getParamsExpr(), "No parameters provided!\n");
	  
	  // Pointer to request entry in parent down queue
	  pEntryDown = qParent_.down->getHeadEntry();
	  
	  // Copy all tuple pointers in the request into the
	  // work ATP (paramsAtp_).
	  paramsAtp_->copyAtp(pEntryDown->getAtp());
	  
	  // Add a pointer to the empty paramsTuple into the last position
	  // of the paramsAtp_.  The paramsTuple will be populated
	  // will the values of the parameters after evaluating the 
	  // paramsExpr.
	  paramsAtp_->getTupp(explainTdb().criDescParams()->noTuples() - 1)
	    = paramsTupp_;
	  
	  // Calculate the input parameters and put results in paramsTupp_
          ex_expr::exp_return_type retCode;
	  retCode = getParamsExpr()->eval(paramsAtp_, 0);
          if (retCode == ex_expr::EXPR_ERROR) {
            if (qParent_.up->isFull())
              return WORK_OK;   

            ex_queue_entry *pEntryUp = qParent_.up->getTailEntry();

            pEntryUp->copyAtp(paramsAtp_);
            pEntryUp->upState.status = ex_queue::Q_SQLERROR;
            pEntryUp->upState.parentIndex = pEntryDown->downState.parentIndex;
            pEntryUp->upState.downIndex = qParent_.down->getHeadIndex();
	    pEntryUp->upState.setMatchNo(matchNo_);

            qParent_.up->insert();

            workState_ = EXPL_DONE;
            break;
          }
	  
	  // Copy the parameters out of the paramsTuple and into buffers
	  // (modName_ and stmtPattern_) allocated by initParamsTuple().
	  // This is so that the paramters can be Null Terminated and
	  // treated as C-strings
	  copyParameters();
	  
	  // starts with an '/'. It is an oss pathname.
	  if ((modDir_) &&
	      (strlen(modDir_) >= 3) &&
	      (strncmp(modDir_, "/E/", 3) == 0) ||
	      (strncmp(modDir_, "/G/", 3) == 0))
	    {
	      // Pathname cannot be an Expand or a Guardian name.
	      // Return error.
	      workState_ = EXPL_ERROR; // add a 'real' error out here. TBD.
	      break;
	    }

	  // Load the named module if necessary.  Also initialize the
	  // pointer to the statement list (stmtList_) of the current
	  // context.
	  rc = loadModule();
	  if (rc < 0)
	    workState_ = EXPL_ERROR;
	  else
	    // Find all statements that match the module name and
	    // statement pattern.
	    workState_ = EXPL_GETNEXT_EXPLAINTREE;
	  break;
	  
	case EXPL_GETNEXT_EXPLAINTREE:
          // When explainTree_ is not null and if the explain is for a given qid
          // explain is already done, swit
          if (qid_ != NULL && explainTree_ != NULL)
          {
            explainTree_ = NULL;
            workState_ = EXPL_DONE;
            break;
          }

          if (qid_ != NULL)
          {
            workState_ = EXPL_SEND_TO_SSMP;
            break;
          }
	  // Find the next statement that matches the module name
	  // and statement pattern and return a pointer to the explainTree.
	  // Otherwise return NULL.  If a statement is found, the EXPLAIN
	  // fragment of this statement will be copied and unpacked.
	  // When the traversal of the tree is complete, the memory 
	  // allocated for the tree should be freed.  Currently, it
	  // is set up so that the root of the tree is at the first
	  // location of the buffer, so explainTree_ can be used to
	  // delete this buffer.
	  explainTree_ = getNextExplainTree();
	  if(!explainTree_)
	    {
	      // If an error occurred, return error indication.
	      // If no more matching statements were found, the
	      // request is complete. Return EOD indication. 
	      // getNextExplainTree has changed the workState_
	      // to EXPL_ERROR or EXPL_DONE.
	      break;
	    }
	  
	  // There is an explainTree to be traversed.  Initialize
	  // the state used by the traversal and then travers the tree.
	  currentExplain_ = explainTree_->getExplainTreeRoot();
	  cameFrom_ = 0;
	  seqNum_ = 0;
	  
	  workState_ = EXPL_TRAVERSE_EXPLAINTREE;
	  break;
	  
	case EXPL_TRAVERSE_EXPLAINTREE:
	  
	  switch(traverseTree())
	    {
	    case EXPL_TRAV_DONE:
	      break;
	    case EXPL_TRAV_NOBUFFERS:
	      return WORK_POOL_BLOCKED;
	    case EXPL_TRAV_QUEUEFULL:
	      return WORK_OK;
            case EXPL_TRAV_ERROR:
              break;
	    default:
	      return WORK_BAD_ERROR;
	    }
	  
          if (workState_ != EXPL_DONE)
            {
              if (qid_ != NULL)
                NADELETEBASIC((char *)explainTree_, cliGlobals->getIpcHeap());
              else
	        NADELETEBASIC((char *)explainTree_, getHeap());
	      workState_ = EXPL_GETNEXT_EXPLAINTREE;
            }
	  break;
        case EXPL_SEND_TO_SSMP:
          {
            RtsExplainFrag *explainInfo = sendToSsmp();
            if (explainInfo == NULL)
            {
              // Pointer to request entry in parent down queue
              ex_queue_entry *pEntryDown = qParent_.down->getHeadEntry();
              ComDiagsArea *diagsArea =
                pEntryDown->getAtp()->getDiagsArea();
              ExRaiseSqlError(getGlobals()->getDefaultHeap(),
                              &diagsArea, EXE_NO_EXPLAIN_INFO);
              if (diagsArea != pEntryDown->getAtp()->getDiagsArea())
                pEntryDown->getAtp()->setDiagsArea(diagsArea);
              workState_ = EXPL_ERROR;
              break;
            }
            explainFrag =(char *)explainInfo->getExplainFrag();
            fragLen = explainInfo->getExplainFragLen();
            topNodeOffset = explainInfo->getTopNodeOffset();
            // Get a 'pointer' to the root node of the explain Tree
            // in the newly copied fragment.
            fragStart = (char *)explainFrag + topNodeOffset;

            // For now the root of the tree must be at the start of
            // the EXPLAIN fragment because the root of the tree is
            // used as a handle on the fragment to delete it.
            if (fragStart != (char *)explainFrag)
            {
              NADELETEBASIC(explainFrag, getHeap());
              explainFrag = NULL;
              ex_assert(0,
                        "Explain: explainTree root is not at the"
                        "beginning of the fragment\n");
            }

            // Unpack the EXPLAIN Fragment
            ExplainDesc *expDesc = (ExplainDesc *)fragStart;

            // Set up space for reallocating objects during unpacking when
            // there is a difference in image sizes at version migration.
            //
            ExplainDesc dummyExpDesc;
            if ((expDesc = (ExplainDesc *)
                  expDesc->driveUnpack(explainFrag,&dummyExpDesc,cliGlobals->getIpcHeap())) == NULL )
            {
              // ERROR during unpacking.
              // Most likely case is verison-unsupported.
              //
              // Add code for erroring handling !!!
              workState_ = EXPL_ERROR;
              break;
            }
            explainTree_ = expDesc;
            // There is an explainTree to be traversed.  Initialize
            // the state used by the traversal and then travers the tree.
            currentExplain_ = explainTree_->getExplainTreeRoot();
            cameFrom_ = 0;
            seqNum_ = 0;
            workState_ = EXPL_TRAVERSE_EXPLAINTREE;
          }
          break;
	case EXPL_ERROR:
	  
	  // Must insert Q_SQLERROR in parent up queue.
	  // If Parent can't take anymore, try again later.
	  
	  if (qParent_.up->isFull())
	    return WORK_OK;
	  
	  pEntryUp = qParent_.up->getTailEntry();
	  pEntryDown = qParent_.down->getHeadEntry();

	  pEntryUp->copyAtp(pEntryDown);
	  pEntryUp->upState.status = ex_queue::Q_SQLERROR;
	  pEntryUp->upState.parentIndex = pEntryDown->downState.parentIndex;
	  pEntryUp->upState.downIndex = qParent_.down->getHeadIndex();  
	  pEntryUp->upState.setMatchNo(matchNo_);
	  
	  qParent_.up->insert();
	  
	  workState_ = EXPL_DONE;
	  break;

	case EXPL_DONE:
	  
	  // Must insert NO_DATA in parent up queue.
	  // If Parent can't take anymore, try again later.
	  
	  if (qParent_.up->isFull())
	    return WORK_OK;
	  
	  pEntryUp = qParent_.up->getTailEntry();
	  pEntryDown = qParent_.down->getHeadEntry();
	  
	  pEntryUp->upState.status = ex_queue::Q_NO_DATA;
	  pEntryUp->upState.parentIndex = pEntryDown->downState.parentIndex;
	  pEntryUp->upState.downIndex = qParent_.down->getHeadIndex();  
	  pEntryUp->upState.setMatchNo(matchNo_);
	  
	  qParent_.up->insert();
	  
	  // this parent request has been processed. 
	  qParent_.down->removeHead();
	  
	  cameFrom_ = 0;
	  matchNo_ = 0;
	  seqNum_ = 0;
	  explainTree_ = 0;
	  currentExplain_ = 0;
	  
	  // Wait for another request
	  workState_ = EXPL_IDLE;
	  break;
	}
    }
}


// getNextExplainTree: Get the next explainTree from the next statement
// that matches the parameters of the xplain function (module name and
// statement pattern.  getNextExplainTree uses the Like pattern matching
// objects and methods to perform any wild card matching.  It assumes that
// the modName_ and stmtPattern_ have been initialized by a call to
// copyParameters() and that the named module has been loaded and stmtList_
// has been initialized to point to the statement list of the current
// context.  When a statement is found that matches the module name and
// statement pattern, the Explain Fragment of the statement is copied
// and unpacked.

ExplainDesc *
ExExplainTcb::getNextExplainTree()
{
  // If the statement pattern is NULL then no statements match
  if(!isNullStmtPattern())
    {
      // Define the pattern string
      // The Wild Card character is '\'
#pragma nowarn(1506)   // warning elimination
      LikePatternString 
	patternString(stmtPattern_, 
		      (stmtPattern_ ? str_len(stmtPattern_) : 0), 
		      CharInfo::ISO88591,
		      "\\", 1 );
#pragma warn(1506)  // warning elimination 

      // Define a pattern for the pattern string
      LikePattern pattern(patternString, getHeap());
      if (pattern.error())
	{
	  workState_ = EXPL_ERROR;
	  return 0;
	}
      
      // A pointer to the statment being considered for a match
      Statement *stmt;

      // Reset the cursor to the beginning of the list of statements.
      // This is necessary since the stmtList_ cursor is altered
      // between calls to this routine.  Because of this, the position
      // of the last statement last considered has to be reestablished.
      stmtList_->position();

      // currentStmt_ contains a pointer to the last statement considered.
      // Find this statement in the statement list.
      if(currentStmt_)
	while(stmt = (Statement *)stmtList_->getNext())
	  {
	    if(stmt == currentStmt_) break;
	  }
      
      // Starting at the next statement, search the statement list for
      // a statement that matches the module name and statement pattern
      while(stmt = (Statement *)stmtList_->getNext())
	{
	  // module name of statement being considered.
	  const char *moduleName = stmt->getModuleId()->module_name;

	  // Added for multi charset module names
          Lng32 moduleNameLen = getModNameLen(stmt->getModuleId());

	  // statement name of statement being considered
	  const char *ident = stmt->getIdentifier();

	  Int32 length = str_len(modName_);

	  // Calculate max length of modName_ (explain function parameter)
	  // and moduleName (module name of statement being considered)
	  if(moduleName)
	    length =
	      (length > moduleNameLen) ? length : moduleNameLen;

	  // Module Name matches if both are NULL or if both compare equal.
	  // It should be that if the module name parameter is NULL then
	  // the module name matches if this is the current module.
	  Int32 modNameMatches = 
	    ((isNullModName() && !moduleName) ||
	     (!isNullModName() &&
	      moduleName &&
	      (str_cmp(modName_, moduleName, length) == 0)));

	  if(modNameMatches &&
#pragma nowarn(1506)   // warning elimination 
	     (pattern.matches(ident, (ident ? str_len(ident) : 0), CharInfo::UTF8) == TRUE) &&
#pragma warn(1506)  // warning elimination 
	     stmt->getRootTdb())
          {
	      // Remember the current statement, so that the position
	      // in the statement list can be reestablished.
	      currentStmt_ = stmt;

	      ex_assert(stmt->getRootTdb()->getNodeType() == ComTdb::ex_ROOT,
			"Invald TDB Tree");

	      // Get the fragment directory of the statement.
	      ExFragDir *fragDir = 
		((ex_root_tdb *)(stmt->getRootTdb()))->getFragDir();

              Lng32 fragOffset;
              Lng32 fragLen;
              Lng32 topNodeOffset;
              char * fragStart;
              if (fragDir->getExplainFragDirEntry(fragOffset, fragLen, topNodeOffset) == 0)
              {
              
	        // Allocate a buffer big enough to hold the EXPLAIN
	        // fragment.
	        char *explainFrag = new(getHeap()) char[fragLen + 10];

	        // Get a 'pointer' to the start of the fragment.
	        char * fragBase = ((char *)stmt->getRootTdb() + fragOffset);
	      
	        // Copy all bytes of fragment to  newly allocated buffer
	        str_cpy_all(explainFrag, fragBase, fragLen);
	      
	        // Get a 'pointer' to the root node of the explain Tree
	        // in the newly copied fragment.
	        fragStart = explainFrag + topNodeOffset;

	        // For now the root of the tree must be at the start of
	        // the EXPLAIN fragment because the root of the tree is
	        // used as a handle on the fragment to delete it.
	        if (fragStart != explainFrag)
	        {
		  NADELETEBASIC(explainFrag, getHeap());
		  explainFrag = NULL;
		  ex_assert(0,
			    "Explain: explainTree root is not at the"
			    "beginning of the fragment\n");
                }
              // Unpack the EXPLAIN Fragment
	      ExplainDesc *expDesc = (ExplainDesc *)fragStart;

	      // Set up space for reallocating objects during unpacking when
	      // there is a difference in image sizes at version migration.
	      //
	      ExplainDesc dummyExpDesc;
	      if ( (expDesc = (ExplainDesc *)
	            expDesc->driveUnpack(explainFrag,&dummyExpDesc,stmt->getUnpackSpace())) == NULL )
	      {
		// ERROR during unpacking.
		// Most likely case is verison-unsupported.
		//
                // Add code for erroring handling !!!
		workState_ = EXPL_ERROR;
		NADELETEBASIC(explainFrag, getHeap());
		return 0;
	      }
	      else
	      {
		// The expDescPtr might have been relocated after unpacking
		// due to a version upgrade. Return its new location.
		//
		return expDesc;
	      }
            }
            else
            {
              // error case. Add an error entry.
	      // Pointer to request entry in parent down queue
	      ex_queue_entry *pEntryDown = qParent_.down->getHeadEntry();
	      ComDiagsArea *diagsArea = 
		pEntryDown->getAtp()->getDiagsArea();
	      ExRaiseSqlError(getGlobals()->getDefaultHeap(), 
			      &diagsArea, EXE_NO_EXPLAIN_INFO);
	      if (diagsArea != pEntryDown->getAtp()->getDiagsArea())
		pEntryDown->getAtp()->setDiagsArea(diagsArea);
  	    
	      workState_ = EXPL_ERROR;
	      return 0;
            }
         }
      }
  }

  // No more statements match.  Reset currentStmt and return NULL.
  currentStmt_ = 0;

  workState_ = EXPL_DONE;
  return 0;
}


// initParamsTuple: Allocate the paramsTuple_, the paramsAtp, the
// modName_ buffer and the stmtPattern_ buffer.  The paramsTuple_ will
// hold the result of evaluating the paramsExpr.  The modName_ and
// stmtPattern_ buffers are used to hold a NULL terminated copy of the
// parameters.  Also, initialize the paramsTuppDesc_ to point to the
// paramsTuple_ and the paramsTupp_ to point to the paramsTuppDesc_.
// This is done to create the necessary tuple structure so that the
// paramsExpr can be evaluateed.
//
// Arguments:
//
// tupleLength - length of the paramsTuple_ to be allocated
// criDescParams - criDesc descripting the paramsAtp_
// lengthModName - length of the modName_ buffer to be allocated
// lengthStmtPattern - length of the stmtPattern_ buffer to be allocated
void
ExExplainTcb::initParamsTuple(unsigned short tupleLength,
			      ex_cri_desc *criDescParams,
			      Lng32 lengthModName,
			      Lng32 lengthStmtPattern)
{

  // Allocate the required buffers
  paramsTuple_ = new(getSpace()) char[tupleLength];
  modName_ = new(getSpace()) char[lengthModName + 1];
  modDir_  = new(getSpace()) char[1024 + 1]; // use a literal here. TBD.
  stmtPattern_ = new(getSpace()) char[lengthStmtPattern + 1];
  
  // Initialize the tuple structure so it can be used when
  // evaluating the paramsExpr
  paramsTuppDesc_.init(tupleLength, 0, paramsTuple_);
  paramsTupp_.init();
  paramsTupp_ = &paramsTuppDesc_;
  
  // Allocate the ATP used to hold the inputs given from the parent
  // request and the paramsTuple to be populated by evaluation of
  // the paramsExpr.  
  paramsAtp_ = allocateAtp(criDescParams, getSpace());
  
}


// copyParameters : copy the parameters from the paramsTuple_ to
// the buffers (modName_ and stmtPattern_).  Remove any trailing
// spaces and NULL Terminate.
void
ExExplainTcb::copyParameters()
{
  Lng32 i;
  
  if(!isNullModName())
    {
      // Copy each byte of the module name to modName_
      for(i = 0; i < explainTdb().getLengthModName(); i++)
	modName_[i] = paramsTuple_[explainTdb().getOffsetModName() + i];

      // NULL Terminate.
      modName_[i] = '\0';
      
      // Remove any trailing spaces (replace with NULL character)
      for(i = explainTdb().getLengthModName() - 1; i >= 0; i--)
	if(modName_[i] == ' ')
	  modName_[i] = '\0';
	else
	  break;

      // find out if this modName_ contains an oss pathname.
      // If it does, then use the oss pathname to search for the module.
      modDir_[0] = 0;
      if (modName_[0] == '/')
	{
	  Lng32 len = (Int32)strlen(modName_);

	  // find the directory name.
	  Lng32 i = len-1;
	  NABoolean done = FALSE;
	  NABoolean dQuoteSeen = FALSE;
	  while ((i > 0) && (NOT done))
	    {
	      if (modName_[i] == '"')
		dQuoteSeen = NOT dQuoteSeen;

	      if (NOT dQuoteSeen)
		{
		  if (modName_[i] == '/')
		    {
		      done = TRUE;
		      continue;
		    }
		}
	      i--;
	    }

	  i++;
	  strncpy(modDir_, modName_, i);
	  modDir_[i] = 0;

	  Int32 j = 0;
	  while (i < len)
	    modName_[j++] = modName_[i++];
	  modName_[j] = 0;

	} // modDir prepended to modName
    }
  else
    // If it is a NULL Value, make string zero length
    modName_[0] = '\0';
  
  if(!isNullStmtPattern())
    {
      // Copy each byte of the module name to modName_
      for(i = 0; i < explainTdb().getLengthStmtPattern(); i++)
	stmtPattern_[i] =
	  paramsTuple_[explainTdb().getOffsetStmtPattern()+i];

      // NULL Terminate.
      stmtPattern_[i] = '\0';
      
      // Remove any trailing spaces (replace with NULL character)
      for(i = explainTdb().getLengthStmtPattern() - 1; i >= 0; i--)
	{
	  if(stmtPattern_[i] == ' ')
	    stmtPattern_[i] = '\0';
	  else
	    break;
	}
    }
  else
    // If it is a NULL Value, make string zero length
    stmtPattern_[0] = '\0';

  if (modName_[0] == '\0')
  {
    Lng32 len = str_len(stmtPattern_);
    if (len > 4 && str_ncmp(stmtPattern_, "QID=", 4) == 0)
      setQid(&stmtPattern_[4], len-4);
  }
}

// isNullModName: Returns true (1) if the module name parameter stored
// in the paramsTuple_ is NULL, false (0) otherwise.
short
ExExplainTcb::isNullModName()
{
  // If the module name attribute is Nullable
  if(explainTdb().getAttrModName()->getNullFlag())
    {
      // Get the offset to the Null indicator in the paramsTuple_
      Lng32 nullOffset = explainTdb().getAttrModName()->getNullIndOffset();

      // We are assuming that the sizeof the Null Indicator is the
      // size of a short.
      ex_assert((explainTdb().getAttrModName()->getNullIndicatorLength() ==
		 sizeof(short)),
		"Null indicator not equal to sizeof(short)\n");

      // Return the value of the null indicator
      return *(short *)&(paramsTuple_[nullOffset]);
    }
  else
    // If not NULLable, it can't be NULL
    return 0;
  
}

// isNullStmtPattern: Returns true (1) if the statement pattern parameter
// stored in the paramsTuple_ is NULL, false (0) otherwise.
short
ExExplainTcb::isNullStmtPattern()
{
  // If the statement parameter attribute is Nullable
  if(explainTdb().getAttrStmtPattern()->getNullFlag())
    {
      // Get the offset to the Null indicator in the paramsTuple_
      Lng32 nullOffset = 
	explainTdb().getAttrStmtPattern()->getNullIndOffset();
      
      // We are assuming that the sizeof the Null Indicator is the
      // size of a short.
      ex_assert((explainTdb().getAttrStmtPattern()->
		 getNullIndicatorLength() == sizeof(short)),
		"Null indicator not equal to sizeof(short)\n");
      
      // Return the value of the null indicator
      return *(short *)&(paramsTuple_[nullOffset]);
    }
  else
    // If not NULLable, it can't be NULL
    return 0;
}

void ExExplainTcb::setQid(char *qid, Lng32 len)
{
  if (qid_)
  {
    NADELETEBASIC(qid_, getHeap());
  }
  qid_ = new (getHeap()) char[len+1];
  str_cpy_all(qid_, qid, len);
  qid_[len] = '\0';
  stmtName_ = new (getHeap()) char[60+1]; // showddl for explain returns 60
  if (getStmtNameInQid(qid_, len, stmtName_, (short)61) != 0)
  {
    str_cpy_all(stmtName_, "Unknown Stmt", 12);
    stmtName_[12] = '\0';
  }
}

RtsExplainFrag *ExExplainTcb::sendToSsmp()
{
  RtsExplainFrag *explainFrag = NULL;
  RtsExplainReq *explainReq;
  CliGlobals *cliGlobals = getGlobals()->castToExExeStmtGlobals()->
                        castToExMasterStmtGlobals()->getCliGlobals();
  if (cliGlobals->getStatsGlobals() == NULL)
  {
    // Runtime Stats not running.
    // Fill in diagsArea with details and return NULL
    IpcAllocateDiagsArea(diagsArea_, getHeap());
    (*diagsArea_) << DgSqlCode(-EXE_RTS_NOT_STARTED);
    return NULL;
  }

  // Verify that we have valid parameters for each kind of request. If not, fill in the diagsArea
  // and return NULL.
  ExSsmpManager *ssmpManager = cliGlobals->getSsmpManager();
  short cpu;
  char nodeName[MAX_SEGMENT_NAME_LEN+1];
  if (getMasterCpu(qid_, (Lng32)str_len(qid_), nodeName, MAX_SEGMENT_NAME_LEN+1, cpu) == -1)
  {
    nodeName[0] = '\0';
    cpu = -1;
    IpcAllocateDiagsArea(diagsArea_, getHeap());
    (*diagsArea_) << DgSqlCode(-EXE_RTS_INVALID_QID) << DgString0(stmtPattern_);
    return NULL;
  }

  IpcServer *ssmpServer = ssmpManager->getSsmpServer(nodeName, cpu, diagsArea_);
  if (ssmpServer == NULL)
    return NULL; // diags are in diagsArea_

  //Create the SsmpClientMsgStream on the IpcHeap, since we don't dispose of it immediately.
  //We just add it to the list of completed messages in the IpcEnv, and it is disposed of later.
  //If we create it on the ExStatsTcb's heap, that heap gets deallocated when the statement is
  //finished, and we can corrupt some other statement's heap later on when we deallocate this stream.
 SsmpClientMsgStream *ssmpMsgStream  = new (cliGlobals->getIpcHeap())
        SsmpClientMsgStream((NAHeap *)cliGlobals->getIpcHeap(), ssmpManager);

  ssmpMsgStream->addRecipient(ssmpServer->getControlConnection());
  RtsHandle rtsHandle = (RtsHandle) this;

  // Retrieve the Rts collection interval and active queries. If they are valid, calculate the timeout
  // and send to the SSMP process.
  SessionDefaults *sd = cliGlobals->currContext()->getSessionDefaults();
  Lng32 RtsTimeout;
  if (sd)
    RtsTimeout = sd->getRtsTimeout();
  else
    RtsTimeout = 0;
  explainReq = new (cliGlobals->getIpcHeap()) RtsExplainReq(rtsHandle, cliGlobals->getIpcHeap(),
            qid_, str_len(qid_));
  *ssmpMsgStream << *explainReq;
  if (RtsTimeout != 0)
  {
    // We have a valid value for the timeout, so we use it by converting it to centiseconds.
    RtsTimeout = RtsTimeout * 100;
  }
  else
    //Use the default value of 4 seconds, or 400 centiseconds.
    RtsTimeout = 400;

  // Send the message
  ssmpMsgStream->send(FALSE, -1);
  Int64 startTime = NA_JulianTimestamp();
  Int64 currTime;
  Int64 elapsedTime;
  IpcTimeout timeout = (IpcTimeout) RtsTimeout;
  while (timeout > 0 && ssmpMsgStream->hasIOPending())
  {
    ssmpMsgStream->waitOnMsgStream(timeout);
    currTime = NA_JulianTimestamp();
    elapsedTime = (currTime - startTime) / 10000;
    timeout = (IpcTimeout)(RtsTimeout - elapsedTime);
  }
  if (ssmpMsgStream->getState() == IpcMessageStream::ERROR_STATE && retryAttempts_ < 3)
  {
    explainReq->decrRefCount();
    DELAY(100);
    retryAttempts_++;
    explainFrag = sendToSsmp();
    retryAttempts_ = 0;
    return explainFrag;
  }
  if (ssmpMsgStream->getState() == IpcMessageStream::BREAK_RECEIVED)
  {
    // Break received - set diags area
    IpcAllocateDiagsArea(diagsArea_, getHeap());
    (*diagsArea_) << DgSqlCode(-EXE_CANCELED);

    return NULL;
  }
  if (! ssmpMsgStream->isReplyReceived())
  {
    IpcAllocateDiagsArea(diagsArea_, getHeap());
    (*diagsArea_) << DgSqlCode(-EXE_RTS_TIMED_OUT)
      << DgString0(stmtPattern_) << DgInt0(RtsTimeout/100) ;
    return NULL;
  }
  retryAttempts_ = 0;
  explainFrag = ssmpMsgStream->getExplainFrag();
  explainReq->decrRefCount();
  return explainFrag;
}
