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

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
	treeHeight = -1;
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

  return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	return pf.close();
	
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
		cout << "insert error? " << error << endl;
		rootPid = 0;
		treeHeight = 1;
		
		/* 	Note to June. As of right now, I have no idea how the PageFile handles everything. 
			I'll try my best to write code w/o all the PageFile stuff.
		error = x.write(rootPid, pf);
		cout << "write error? " << error << endl;
		*/
	}
	
	// root exists, but there's still only 1 node, which must be a BTLeafNode.
	else if (treeHeight == 1)
	{
		BTLeafNode tmpNode = BTLeafNode();
		error = tmpNode.read(rootPid, pf);
		cout << "read error? " << error << endl;
		
		error = tmpNode.insert(key, rid);
		
		// node is full
		if (error == -1010)
		{
			int siblingKey;
			BTLeafNode tmpSiblingNode = BTLeafNode();
			tmpNode.insertAndSplit(key, rid, tmpSiblingNode, siblingKey);
			
			BTNonLeafNode newRoot;
			
			//newRoot blah blah blah
		}
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
