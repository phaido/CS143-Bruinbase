#include "BTreeNode.h"
#include <cstring>
#include <cstdlib>
//testing
#include <iostream>

using namespace std;

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{ return pf.read(pid,buffer); }
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{ return pf.write(pid,buffer); }

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
	int cnt; //we are storing the number of keys in first 4 bytes
	         //of the buffer
	memcpy(&cnt,buffer,sizeof(int));
	return cnt;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
	//Considering that each leafnode contains 1024 bytes,
	//and it should be in...
	//rid,key | rid,key | rid,key | ... | pid
	//so we should be able to fit whatever 
	//(1024-sizeof(PageId))/(sizeof(RecordId)+sizeof(int))
	//within the node, it's sort of like a sorted array, but
	//we have to keep track of size.  Also, ordering will follow
	//slides where least/left -> greatest/right.
	char* tmpBuffer = (char*)malloc(1024); //creating another buffer to hold
										   //a possible new buffer
	memset(tmpBuffer, 0, 1024); //zeroing it out
	//the size of each entry should be sizeof(RecordId)+sizeof(int)
	int entSize = sizeof(RecordId)+sizeof(int);
	//total set of keys this can contain is 1024 - sizeof(pageId)
	int totSize = 1024-sizeof(PageId);
	if((getKeyCount()*entSize)>(totSize-entSize))
	{
		free(tmpBuffer); //making sure the tmpBuffer is gone
		return -1010; //Bruinbase.h, node full
	}
	else
	{
		//fnd variable to keep track if we can insert or not
		int fnd = 0;//We are storing number of keys in the beginning of buffer
		int bufKey; //necessary to retrieve key from buffer
		int i;
		for(i=sizeof(int); (i<totSize)&&(!fnd); i+=entSize)
		{
			memcpy(&bufKey, (buffer+i+sizeof(RecordId)), sizeof(int)); //retrieving key from buffer
			//cout << "bufKey: " << bufKey << endl;
			//if((buffer+i)>key)  CAN'T DO THIS cause 1 byte > 4 byte
			//	fnd=1;
			
			// fix. changed > to >=.
			
			if(bufKey>key)
			{
				fnd=1;
				break;
			}
		}
		
		if (fnd == 1)
		{
		//i is the displacement where we want to insert
		// Fix by Junkyum. i is NOT the displacement where we want to insert, for loop increments i by entSize.	
		memcpy(tmpBuffer, buffer, i );
		memcpy(tmpBuffer+i , &rid, sizeof(RecordId));//inserting rid
		memcpy(tmpBuffer+i + sizeof(RecordId),&key,sizeof(int));//then key
		//now copying in the rest...
		
		// fix by Junkyum Kim ?
		if(i+entSize<1024)
		{
			memcpy(tmpBuffer+i+entSize, buffer+i, 1024-i-entSize-sizeof(PageId));
			memcpy(tmpBuffer+(1024-sizeof(PageId)), buffer+(1024-sizeof(PageId)), sizeof(PageId));
		}
		memcpy(buffer, tmpBuffer, 1024); //this should fill it
														  //in entirely
		//memcpy(buffer, tmpBuffer, 1024); //making new buffer
		//free(tmpBuffer);//making sure the tmpBuffer is gone
		}
		
		// stopped here. some error with count......?????????
		else
		{
			int displacement = getKeyCount() * entSize + sizeof(int);
			memcpy(buffer+displacement, &rid, sizeof(RecordId));
			memcpy(buffer+displacement + sizeof(RecordId), &key, sizeof(int));
		}
	}
	free(tmpBuffer);//making sure the tmpBuffer is gone
	//update key count
	
	int tmpNK;
	memcpy(&tmpNK,buffer,sizeof(int));
	tmpNK++;
	memcpy(buffer,&tmpNK,sizeof(int));
	return 0;//shouldn't get here
	//free(tmpBuffer); //making sure the tmpBuffer is gone
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{
	int entSize = sizeof(RecordId) + sizeof(int);
	int totSize = 1024 - sizeof(PageId);
	int atSecHalf = 0;
	//from slides, should only split when full
	//sibling node must be empty
	if(sibling.getKeyCount()!=0)
		return -1015; //sibling is not empty...exit?
		//memset(sibling.buffer, 0, 1024);//or do we memset to zero it out?
	//so were putting half within current, other half in sibling (that empty)
	int divisor = 2; //make sure we get int/int
	int insNKeys = getKeyCount()/divisor;
	int noninsNKeys = insNKeys + (getKeyCount()%divisor);
	//first check the new entry is sorted within first half or later
	//cause depending on that we have consider how to balance the split
	int bufKey;
	int i;
	for(i=sizeof(int); (i<totSize); i+=entSize)
	{
		memcpy(&bufKey, (buffer+i+sizeof(RecordId)), sizeof(int));
		if(bufKey==key)//key should no be the same
			return RC_INVALID_ATTRIBUTE;
		else if(bufKey>key)
		{
			atSecHalf = (i>=(entSize*noninsNKeys+sizeof(int)) ? 1 : 0);
			break; //to insure i doesn't get incremented...
		}
	}
	if(i>=totSize)
		atSecHalf = 1;
	char* tmpMainBuffer = (char*)malloc(1024);
	char* tmpSiblingBuffer = (char*)malloc(1024);
	memset(tmpMainBuffer, 0, 1024); //zeroing it out
	memset(tmpSiblingBuffer, 0, 1024); //zeroing it out
	if(atSecHalf==1) //insertion should take place in second half
	{
		//constructing halved buffers without insertion
		memcpy(tmpMainBuffer+sizeof(int), buffer+sizeof(int), noninsNKeys*entSize);
		memcpy(tmpMainBuffer+totSize,buffer+totSize, sizeof(PageId));
		memcpy(tmpSiblingBuffer+sizeof(int), buffer+((noninsNKeys*entSize)+sizeof(int)), insNKeys*entSize);
		memcpy(tmpSiblingBuffer+totSize, buffer+totSize, sizeof(PageId));
		//now insert to correct half
		sibling.nodecopy(tmpSiblingBuffer);
		//sibling.insert(key,rid);
		nodecopy(tmpMainBuffer);
		upNumKeys(noninsNKeys);
		sibling.upNumKeys(insNKeys);
		sibling.insert(key,rid);
	}
	else //insertion to take place in first half
	{
		//constructing halved buffers without insertion
		memcpy(tmpMainBuffer+sizeof(int), buffer+sizeof(int), insNKeys*entSize);
		memcpy(tmpMainBuffer+totSize,buffer+totSize, sizeof(PageId));
		memcpy(tmpSiblingBuffer+sizeof(int), buffer+((insNKeys*entSize)+sizeof(int)), noninsNKeys*entSize);
		memcpy(tmpSiblingBuffer+totSize, buffer+totSize, sizeof(PageId));
		nodecopy(tmpMainBuffer);
		//insert(key,rid);
		sibling.nodecopy(tmpSiblingBuffer);
		upNumKeys(insNKeys);
		insert(key,rid);
		sibling.upNumKeys(noninsNKeys);
	}
	free(tmpMainBuffer);//making sure the tmpBuffer is gone
	free(tmpSiblingBuffer);
	//update key count
	return 0;
	//Since all we know is that the pid for sibling should contain
	//what the original node's pid was, and at this level
	//we have no idea where sibling would be placed in pagefile
	//we defaulted to retain original node's pid
}

/**
 * If searchKey exists in the node, set eid to the index entry
 * with searchKey and return 0. If not, set eid to the index entry
 * immediately after the largest index key that is smaller than searchKey,
 * and return the error code RC_NO_SUCH_RECORD.
 * Remember that keys inside a B+tree node are always kept sorted.
 * @param searchKey[IN] the key to search for.
 * @param eid[OUT] the index entry number with searchKey or immediately
                   behind the largest key smaller than searchKey.
 * @return 0 if searchKey is found. Otherwise return an error code.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{
	//So basic idea is to iterate through the buffer looking for
	//searchKey.  When found, the eid is the index of where it's
	//located.  If it's the first element, eid is 0, if fifth, eid
	//is 4.
	//		int fnd = 0;
	//	for(i=0; (i<totSize)&&(!fnd); i+=entSize)
	//	{
	//		if((buffer+i)==0 || ((buffer+i)<key))
	//			fnd=1;
	//	}
	int entSize = sizeof(RecordId)+sizeof(int);
	int totSize = 1024-sizeof(PageId);
	int fnd = 0;
	int nfnd = 0;
	int j = 0;
	int bufKey; 
	int i;
	for(i=sizeof(RecordId)+sizeof(int); (i<totSize)&&(!fnd)&&(nfnd); i+=entSize)
	//i offset due to key location
	{
		//So if searchKey
		memcpy(&bufKey, (buffer+i), sizeof(int));
		if(bufKey==searchKey)
			fnd=1;
		else if(bufKey<searchKey)
			nfnd=1;
		else if((!fnd)&&(!nfnd))
			j++;
	}
	int ret_code;
	eid=j;
	(fnd)?(ret_code = 0):(ret_code = RC_NO_SUCH_RECORD);
	return ret_code;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{
	int entSize = sizeof(RecordId) + sizeof(int);
	memcpy(&rid,buffer+(eid*entSize)+(sizeof(int)), sizeof(RecordId));
	memcpy(&key,buffer+(eid*entSize)+sizeof(RecordId)+(sizeof(int)), sizeof(int));
	return 0;
}

//Personal helper function...
RC BTLeafNode::nodecopy(char* replicant)
{
	memcpy(buffer,replicant,1024);
	return 0;
}
RC BTLeafNode::upNumKeys(int uKey)
{
	memcpy(buffer,&uKey,sizeof(int));
	return 0;
}

//testing
BTLeafNode::BTLeafNode()
{
	memset(buffer, 0, 1024);
}

void BTLeafNode::PrintNode()
{
	//the size of each entry should be sizeof(RecordId)+sizeof(int)
	int entSize = sizeof(RecordId)+sizeof(int);
	//total set of keys this can contain is 1024 - sizeof(pageId)
	int totSize = 1024-sizeof(PageId);

	int i;
	int j=0;
	int tmp;
	RecordId tmpRid;
	for (i = sizeof(int); i < totSize; i+= entSize)
	{	
		memcpy(&tmpRid,buffer+i,sizeof(RecordId));
		memcpy(&tmp, buffer+i+sizeof(RecordId),sizeof(int));
		cout <<"Tuple:"<<j<<", "<< tmpRid.pid <<" " << tmpRid.sid << " " << tmp << endl;
		j+=1;
	}	}

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{
	//So within a PageFile, each page occupies 1024 bytes.
	//So whatever the address "buffer" (from BTreeNode.h,
	//a private member which describes what's currently
	//on memory). So to grab it's sibling it must be
	//offset by 1024-sizeof(pageid) (accounting for how much
	//space current one takes).
	PageId pid = 0;
	memcpy(&pid,buffer+(1024-sizeof(PageId)),sizeof(PageId));
	return pid;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	int totSize = 1024 - sizeof(PageId);
	memcpy(buffer+totSize, &pid, sizeof(PageId));
	return 0;
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ return pf.read(pid,buffer); }
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ return pf.write(pid,buffer); }

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
	int cnt; //we are storing the number of keys in first 4 bytes
	         //of the buffer
	memcpy(&cnt,buffer,sizeof(int));
	return cnt;
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
	//Considering that each nonleafnode contains 1024 bytes,
	//and it should be in...
	//pid|key|pid|key|....|pid|
	//so we should be able to fit whatever 
	//(1024-sizeof(PageId))/(sizeof(PageId)+sizeof(int))
	//within the node, it's sort of like a sorted array, but
	//we have to keep track of size.  Also, ordering will follow
	//slides where least/left -> greatest/right.
	char* tmpBuffer = (char*)malloc(1024); //creating another buffer to hold
										   //a possible new buffer
	memset(tmpBuffer, 0, 1024); //zeroing it out
	//the size of each entry should be sizeof(RecordId)+sizeof(int)
	int entSize = sizeof(PageId)+sizeof(int);
	//total set of keys this can contain is 1024 - sizeof(pageId)
	int totSize = 1024-sizeof(PageId);
	if((getKeyCount()*entSize)>(totSize-entSize))
	{
		free(tmpBuffer); //making sure the tmpBuffer is gone
		return -1010; //Bruinbase.h, node full
	}
	else
	{
		//fnd variable to keep track if we can insert or not
		int fnd = 0;
		int bufKey;
		int i;
		for(i=sizeof(int); (i<totSize)&&(!fnd); i+=entSize)
		{
			memcpy(&bufKey, (buffer+i+sizeof(PageId)), sizeof(int)); //retrieving key from buffer
			if(bufKey>key)
			{
				fnd=1;
				break;
			}
		}
		if(fnd==1)
		{
			//i is the displacement where we want to insert
			memcpy(tmpBuffer, buffer, i);
			memcpy(tmpBuffer+i,&pid,sizeof(PageId));//inserting pid
			memcpy(tmpBuffer+i+sizeof(PageId),&key,sizeof(int));//then key
			//now copying in the rest...
			if(i+entSize<1024)
			{
				memcpy(tmpBuffer+i+entSize, buffer+i, 1024-i-entSize-sizeof(PageId));
				memcpy(tmpBuffer+(1024-sizeof(PageId)), buffer+(1024-sizeof(PageId)), sizeof(PageId));
			}
			memcpy(buffer, tmpBuffer, 1024); //making new buffer
			//free(tmpBuffer);//making sure the tmpBuffer is gone
		}
		else
		{
			int displacement = getKeyCount() * entSize + sizeof(int);
			memcpy(buffer+displacement, &pid, sizeof(PageId));
			memcpy(buffer+displacement+sizeof(PageId),&key,sizeof(int));
		}
	}
	free(tmpBuffer);//making sure the tmpBuffer is gone
	int tmpNK;
	memcpy(&tmpNK,buffer,sizeof(int));
	tmpNK++;
	memcpy(buffer,&tmpNK,sizeof(int));
	return 0;//shouldn't get here
	//free(tmpBuffer); //making sure the tmpBuffer is gone
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{
	int entSize = sizeof(PageId) + sizeof(int);
	int totSize = 1024 - sizeof(PageId);
	int atSecHalf = 0;
	//from slides, should only split when full
	//sibling node must be empty
	if(sibling.getKeyCount()!=0)
		return -1015; //sibling is not empty...exit?
		//memset(sibling.buffer, 0, 1024);//or do we memset to zero it out?
	//so were putting half within current, other half in sibling (that empty)
	int divisor = 2; //make sure we get int/int
	int insNKeys = getKeyCount()/divisor;
	int isOdd = getKeyCount()%divisor;
	int noninsNKeys = insNKeys + isOdd;
	//first check the new entry is sorted within first half or later
	//cause depending on that we have consider how to balance the split
	int bufKey;
	int i;
	for(i=sizeof(PageId)+sizeof(int); (i<totSize); i+=entSize)
	{
		memcpy(&bufKey, (buffer+i), sizeof(int));
		if(bufKey==key)//key should no be the same
			return RC_INVALID_ATTRIBUTE;
		else if(bufKey>key)
		{
			atSecHalf =((i>(getKeyCount()*insNKeys)) ? 1:0);
			break; //to insure i doesn't get incremented...
		}
	}
	char* tmpMainBuffer = (char*)malloc(1024);
	char* tmpSiblingBuffer = (char*)malloc(1024);
	memset(tmpMainBuffer, 0, 1024); //zeroing it out
	memset(tmpSiblingBuffer, 0, 1024); //zeroing it out
	//int tmpNK;
	if(atSecHalf) //insertion should take place in second half
	{
		//constructing halved buffers without insertion
		memcpy(tmpMainBuffer, buffer+sizeof(int), ((noninsNKeys*entSize)-sizeof(int)));
		//memcpy(tmpMainBuffer+totSize,buffer+totSize, sizeof(PageId));
		memcpy(tmpSiblingBuffer, buffer+((noninsNKeys*entSize)+sizeof(int)), insNKeys*entSize);
		//memcpy(tmpSiblingBuffer+totSize, buffer+totSize, sizeof(PageId));
		//now insert to correct half
		memcpy(&midKey,buffer+((noninsNKeys*entSize)-sizeof(int)),sizeof(int));
		sibling.nodecopy(tmpSiblingBuffer);
		sibling.insert(key,pid);
		nodecopy(tmpMainBuffer);
		upNumKeys(noninsNKeys-1);
		sibling.upNumKeys(insNKeys);
	}
	else //insertion to take place in first half
	{
		//constructing halved buffers without insertion
		memcpy(tmpMainBuffer, buffer+sizeof(int), ((insNKeys*entSize)-sizeof(int)));
		//memcpy(tmpMainBuffer+totSize,buffer+totSize, sizeof(PageId));
		memcpy(tmpSiblingBuffer, buffer+((insNKeys*entSize)+sizeof(int)), noninsNKeys*entSize);
		//memcpy(tmpSiblingBuffer+totSize, buffer+totSize, sizeof(PageId));
		memcpy(&midKey,buffer+((insNKeys*entSize)-sizeof(int)),sizeof(int));
		nodecopy(tmpMainBuffer);
		insert(key,pid);
		sibling.nodecopy(tmpSiblingBuffer);
		upNumKeys(insNKeys-1);
		sibling.upNumKeys(noninsNKeys);
	}
	//memcpy(sibling.buffer, tmpSiblingBuffer, 1024);
	//sibling.nodecopy(tmpSiblingBuffer);
	free(tmpMainBuffer);  //making sure the tmpBuffer is gone
	free(tmpSiblingBuffer);
	return 0;
	//Since all we know is that the pid for sibling should contain
	//what the original node's pid was, and at this level
	//we have no idea where sibling would be placed in pagefile
	//we defaulted to retain original node's pid
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{
	//So basic rund down is let's say
	// ...|pid|key|pid|
	//if searchKey < key the first pid is the one we pass back
	//if searchKey > then we move on
	//if searchKey < key not found, then last pid
	int entSize = sizeof(PageId)+sizeof(int);
	int totSize = 1024-sizeof(PageId);
	int fnd = 0;
	int bufKey;
	int i;
	for(i=sizeof(PageId)+sizeof(int); (i<totSize)&&(!fnd); i+=entSize)
	//i offset due to key location
	{
		//So if searchKey
		memcpy(&bufKey, (buffer+i), sizeof(int));
		if(bufKey>searchKey)
			fnd=1;
	}
	memcpy(&pid,buffer+(i-sizeof(PageId)), sizeof(PageId));
	return 0;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{
	memset(buffer, 0, 1024); 
	int tmpNK = 1;  //set number of keys to 1
	memcpy(buffer,&tmpNK,sizeof(int)); 
	memcpy(buffer+sizeof(int),&pid1, sizeof(PageId));
	memcpy(buffer+sizeof(int)+sizeof(PageId),&key, sizeof(int));
	memcpy(buffer+sizeof(int)+sizeof(PageId)+sizeof(int),&pid2, sizeof(PageId));
	return 0;
}

//Personal helper function...
RC BTNonLeafNode::nodecopy(char* replicant)
{
	memcpy(buffer,replicant,1024);
	return 0;
}
RC BTNonLeafNode::upNumKeys(int uKey)
{
	memcpy(buffer,&uKey,sizeof(int));
	return 0;
}
