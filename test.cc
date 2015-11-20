#include "Bruinbase.h"
#include "SqlEngine.h"
#include <cstdio>
#include <iostream>
#include "BTreeNode.h"

using namespace std;

int main()
{
  BTLeafNode x = BTLeafNode();
  int count = x.getKeyCount();

  RecordId rid;
  rid.pid = 0;
  rid.sid = 0;

  x.insert(12,rid);

  cout << count;

  return 0;
}
