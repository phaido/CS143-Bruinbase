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
  BTLeafNode x = BTLeafNode();
  BTLeafNode y = BTLeafNode();
  
  RecordId rid;
  rid.pid = 1;
  rid.sid = 5;

  /*
  x.insert(1,rid);
  x.insert(25,rid);
  x.insert(3,rid);
*/

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
  
  x.PrintNode();
  
  cout << error << endl;
  
  /*
  int siblingKey;
  
  x.insertAndSplit(2, rid, y, siblingKey);
  
  x.PrintNode();
  y.PrintNode();
  
  int xCount = x.getKeyCount();
  int yCount = y.getKeyCount();
  
  cout << "sibling key is: " << siblingKey << endl;
	cout << "Count of first node is: " << xCount << endl;
	cout << "Count of second node is: " << yCount << endl;
	*/
	PageId pid = 1;
	
	BTNonLeafNode z = BTNonLeafNode();
	z.insert(5,pid);
	
	//z.PrintNode();
  
  return 0;
}

