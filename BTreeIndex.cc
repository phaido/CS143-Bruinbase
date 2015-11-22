/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"
#include "BTreeNode.h"
#include <iostream>
#include <cstring>
#include <cstdlib>

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
	treeHeight = 0;
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
  RC   rc;

  // open the page file
  if ((rc = pf.open(indexname, mode)) < 0) return rc;
  if(pf.endPid()!=0)//if the pagefile is not empty
  {
  	//basically we are using the first page (pid 0) as the page
  	//where we hold the most basic/vital information.  If this pagefile
  	//is not empty then we proceed to read the page and load the vital
  	//information into memory (basically copying it to BTreeIndex private
  	//member).
  	char* tmpBuffer = (char*)malloc(1024); //tmpBuffer to store page 0
  	if(pr.read(0, tmpBuffer))
  	{
  		free(tmpBuffer);
  		return RC_FILE_READ_FAILED;
  	}
  	memcpy(&rootPid, tmpBuffer, sizeof(PageId)); //first part contains rootPid
  	memcpy(&treeHeight, tmpBuffer+sizeof(PageId), sizeof(int)); //second contains Height
  	free(tmpBuffer);
  }
  return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	//So before we close any file, we have to update page 0.
	char* tmpBuffer = (char*)malloc(1024);
	memcpy(tmpBuffer, &rootPid, sizeof(PageId)); //first part contains rootPid
  	memcpy(tmpBuffer+sizeof(PageId), &treeHeight, sizeof(int)); //second contains Height
  	if(pf.write(0, tmpBuffer))
  	{
  		free(tmpBuffer);
  		return RC_FILE_WRITE_FAILED;
  	}
	free(tmpBuffer);
	if(pf.close())
		return RC_FILE_CLOSE_FAILED;
	return 0;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	int error;
	
	// if root doesn't exist, create a new root, of BTLeafNode
	if (rootPid == -1) 
	{
		BTLeafNode x = BTLeafNode();
		error = x.insert(key,rid);
		//cout << "insert error? " << error << endl;
		rootPid = 1; //has to be 1, since pid 0 is reserved space
		treeHeight = 1;
		if(x.write(1, pf))
			return RC_FILE_WRITE_FAILED;
		
		/* 	Note to June. As of right now, I have no idea how the PageFile handles everything. 
			I'll try my best to write code w/o all the PageFile stuff.
		error = x.write(rootPid, pf);
		cout << "write error? " << error << endl;
		*/
	}
	
	// So there exists at least one node in tree...
	else if (rootPid >= 1)
	{
		/*Justin,
		So I thought about doing this nonrecursively, but it would mean I would have
		to create a stack structure to keep track of how I traversed the nodes (for possible
		updates).  Thought it might be easier to go a naive recursive route for the purposes
		of using internal stack to keep track of route.
		*/
		int parent_update = 0;
		int tmpkey = key;
		PageId apid=0;
		PageId bpid=0;
		if(rInsert(tmpkey, rid, 1, parent_update, rootPid, apid, bpid))
			return -1015;
		if(parent_update==1)//case where root gets changed
		{
			//when root gets extended it is always nonleaf
			BTNonLeafNode nlNode;
			int newRootPid = pf.endPid();
			if(nlNode.initializeRoot(apid, tmpkey, bpid))
				return -1015;
			if(nlNode.write(newRootPid, pf))
				return RC_FILE_WRITE_FAILED;
			treeHeight++; //extend height
			rootPid=newRootPid; //update rootPid
		}
	}
	
    return 0;
}

RC BTreeIndex::rInsert(int &key, const RecordId& rid, int depth, int &parent_update,
					   PageId pid, PageId &apid, PageId &bpid)
{
	if(depth<treeHeight)//so on some nonleaf node, first traverse recursively and check
		                //if parent update is necessary
	{
		BTNonLeafNode nlNode;
		BTNonLeafNode sibnlNode;
		PageId nextPid;
		nlNode.read(pid, pf);
		nlNode.locateChildPtr(key, nextPid);
		rInsert(key, rid ++depth, parent_update,  nextPid, apid, bpid);
		if(parent_update==1)
		{
			parent_update=0;
			if(nlNode.insert(key, pid))
			{
				//if we can't insert, then split.
				//if we split then we must update parent node.
				int tmpKey;
				if(nlNode.insertAndSplit(key, pid, sibnlNode, tmpKey))
					return -1015;
				key=tmpKey;
				parent_update=1;
				int sibpid = pf.endPid();
				apid=pid;
				bpid=sibpid;
				if(sibnlNode.write(sibpid, pf))
					return RC_FILE_WRITE_FAILED;

			}

		}
		if(nlNode.write(pid, pf))
			return RC_FILE_WRITE_FAILED; 
	}
	else//were at leaf
	{
		BTLeafNode lNode;
		BTLeafNode siblNode;
		lNode.read(pid, pf);
		//RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
        //                      BTLeafNode& sibling, int& siblingKey)
		if(lNode.insert(key, rid))
		{
			//if we can't insert, then split.
			//if we split then we must update parent node.
			int tmpKey;
			if(lNode.insertAndSplit(key, rid, siblNode, tmpKey))
				return -1015;
			key=tmpKey;
			parent_update=1;
			int sibpid = pf.endPid();
			apid=pid;
			bpid=sibpid;
			lnode.setNextNodePtr(sibpid);//original node now points to sibling
			if(siblNode.write(sibpid, pf))
				return RC_FILE_WRITE_FAILED;
		}
		if(lNode.write(pid, pf))
			return RC_FILE_WRITE_FAILED;
	}
	return 0;
}

/**
 * Run the standard B+Tree key search algorithm and identify the
 * leaf node where searchKey may exist. If an index entry with
 * searchKey exists in the leaf node, set IndexCursor to its location
 * (i.e., IndexCursor.pid = PageId of the leaf node, and
 * IndexCursor.eid = the searchKey index entry number.) and return 0.
 * If not, set IndexCursor.pid = PageId of the leaf node and
 * IndexCursor.eid = the index entry immediately after the largest
 * index key that is smaller than searchKey, and return the error
 * code RC_NO_SUCH_RECORD.
 * Using the returned "IndexCursor", you will have to call readForward()
 * to retrieve the actual (key, rid) pair from the index.
 * @param key[IN] the key to find
 * @param cursor[OUT] the cursor pointing to the index entry with
 *                    searchKey or immediately behind the largest key
 *                    smaller than searchKey.
 * @return 0 if searchKey is found. Othewise an error code
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
    //So basically find the searchKey at leafnode level.
    BTNonLeafNode nonLeaf; //temp container to traverse down the tree.
    PageId atPid = rootPid; //initialize at root, cause we start there
    int atHeight = 1; //keeps track of the height were going down
    while(atHeight<treeHeight) //basically traverse all non leafe
    {
    	atHeight++; //updating it now for next level
    	if(nonLeaf.read(atPid, pf))//reading the page from pagefile
    		return RC_FILE_READ_FAILED;
    	if(nonLeaf.locateChildPtr(searchKey, atPid))
    		return RC_FILE_SEEK_FAILED;
    	//basically using nonleaf's locatechildptr function to find
    	//the correct pid to next node and update atPid with new Pid
    	//we have to head towards.
    }
    //After traversing all possible nonleaf nodes, ideally we are
    //at the leaf node that should contain the searchKey
    BTLeafNode leaf;
    if(leaf.read(atPid, pf))//read in the leaf node page
    	return RC_FILE_READ_FAILED;
    cursor.pid = atPid;
    if(leaf.locate(searchKey,cursor.eid))
    	return RC_NO_SUCH_RECORD;
    return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
    return 0;
}
