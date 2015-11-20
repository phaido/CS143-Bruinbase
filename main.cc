/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 

 /*
=======
>>>>>>> bf9d518bd4d5ec6fc0039d8c357b9ef039473824
#include "Bruinbase.h"
#include "SqlEngine.h"
#include <cstdio>

int main()
{
  // run the SQL engine taking user commands from standard input (console).
  SqlEngine::run(stdin);

  return 0;
}


*/

//testing
#include "Bruinbase.h"
#include "SqlEngine.h"
#include <cstdio>
#include <iostream>
#include "BTreeNode.h"

using namespace std;

int main()
{
	// all the test code for BTLeafNode
	/*
  BTLeafNode x = BTLeafNode();
  BTLeafNode y = BTLeafNode();
  
  RecordId rid;
  rid.pid = 1;
  rid.sid = 5;
  
	x.insert(111,rid);
	x.insert(112,rid);
	x.insert(113,rid);
	x.insert(114,rid);
	x.insert(115,rid);
	x.insert(1,rid);
	x.insert(200, rid);
	x.insert(400, rid);
	x.insert(600, rid);
	x.insert(800, rid);
	x.insert(150, rid);
	x.insert(350, rid);
	x.insert(750, rid);
	
	int i;
	int error;
	
	for (i =1000; i <1090; i++)
		error = x.insert(i,rid);

  int count = x.getKeyCount();
  
  cout << count << endl;
  cout << endl;
  
  char* tmpBuffer;
  
  x.setNextNodePtr(999);
  
  x.PrintNode();
  
  cout << error << endl;
  
  
  int siblingKey;
  
  x.insertAndSplit(2, rid, y, siblingKey);
  
  cout << "printing the x after split" << endl;
  x.PrintNode();
  cout << "now for the sibling" << endl;
  y.PrintNode();
  
  int xCount = x.getKeyCount();
  int yCount = y.getKeyCount();
  
  cout << "sibling key is: " << siblingKey << endl;
	cout << "Count of first node is: " << xCount << endl;
	cout << "Count of second node is: " << yCount << endl;
	
	int eid;
	
	x.locate(5000, eid);
	cout << "eid is: " << eid << endl;
	
	int index = 14;
	int keyToRead;
	RecordId ridToRead;
	x.readEntry(index,keyToRead,ridToRead);
	
	cout << "at index " << index << ", the key is: " << keyToRead << ", and the rid's pid is: " << ridToRead.pid 
	<< ", and the rid's sid is: " << ridToRead.sid << endl;
	*/
	
	// test code for BTNonLeafNode
	
	PageId pid = 9999;
	
	BTNonLeafNode z; 
	z.initializeRoot(777, 9, 888);
		
		/*
	z.insert(111,pid);
	z.insert(112,pid);
	z.insert(113,pid);
	z.insert(114,pid);
	z.insert(115,pid);
	z.insert(1,pid);
	z.insert(200, pid);
	z.insert(400, pid);
	z.insert(600, pid);
	z.insert(800, pid);
	z.insert(150, pid);
	z.insert(350, pid);
	z.insert(750, pid);
	*/
	
	
	int i;
	int error;
	
	for (i =1000; i <1130; i++)
		error = z.insert(i,pid);

	
	z.insert(255, pid);
	z.insert(455, pid);
	z.insert(755, pid);
	z.insert(3, pid);
	z.insert(4, pid);
	
	
	z.PrintNode();
	
  int count = z.getKeyCount();

  cout << endl;
  
  cout << count << endl;
  
  
	BTNonLeafNode w = BTNonLeafNode();
    int middleKey;
  
  z.insertAndSplit(2, pid, w, middleKey);

	int findPid;
	
	PageId pid2 = 8;
	z.insert(5, pid2);
	z.insert(3,88);
	
	z.locateChildPtr(2, findPid);
  
  cout << "printing the z after split" << endl;
  z.PrintNode();
  cout << "now for the sibling" << endl;
  w.PrintNode();
  
  int zCount = z.getKeyCount();
  int wCount = w.getKeyCount();
   
	cout << "middle key is: " << middleKey << endl;
	cout << "Count of first node is: " << zCount << endl;
	cout << "Count of second node is: " << wCount << endl;
	
	cout << "The findPid is: " << findPid << endl;
	
	BTNonLeafNode v; 
	v.initializeRoot(33, 878, 56);
	v.insert(999, 44);
	v.insert(76, 66);
	v.insert(88,1);
	
	for (i=1; i < 250; i+=2)
		v.insert(i, i*2);
	
	v.PrintNode();
	
	cout << "Total count: " << v.getKeyCount() << endl;
  
	BTNonLeafNode u = BTNonLeafNode();
	
	v.insertAndSplit(100, 200, u, middleKey);
	
	v.PrintNode();
	u.PrintNode();
	
	int vCount = v.getKeyCount();
	int uCount = u.getKeyCount();
   
	cout << "middle key is: " << middleKey << endl;
	cout << "Count of first node is: " << vCount << endl;
	cout << "Count of second node is: " << uCount << endl;
	
	int childPid;
	
	v.locateChildPtr(-1,childPid);
	cout << "Child pid: " << childPid << endl;
  
  return 0;
}

