/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);


RC SqlEngine::run(FILE* commandline)
{
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
	RecordFile rf;   // RecordFile containing the table
	RecordId   rid;  // record cursor for table scanning

	RC     rc;
	int    key;     
	string value;
	int    count;
	int    diff;
	bool   indexExists = 0;
	
	BTreeIndex BTindex;
	
	// BTreeIndex stuff!
	rc = BTindex.open(table + ".idx", 'w');
	if (rc != 0){
			cout << "error when opening IndexFile " << table << ".idx .\n";
			return RC_FILE_OPEN_FAILED;
	}
	else
		indexExists = 1;
	
	// open the table file
	if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
		fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
		return rc;
	}

	
	
	// scan the table file from the beginning
	rid.pid = rid.sid = 0;
	count = 0;
	while (rid < rf.endRid()) {
		// read the tuple
		if ((rc = rf.read(rid, key, value)) < 0) {
			fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
			goto exit_select;
		}

		// check the conditions on the tuple
		for (unsigned i = 0; i < cond.size(); i++) {
			// compute the difference between the tuple value and the condition value
			switch (cond[i].attr) {
				case 1:
				diff = key - atoi(cond[i].value);
				break;
				case 2:
				diff = strcmp(value.c_str(), cond[i].value);
				break;
			}

			// skip the tuple if any condition is not met
			switch (cond[i].comp) {
				case SelCond::EQ:
					if (diff != 0) goto next_tuple;
						break;
				case SelCond::NE:
					if (diff == 0) goto next_tuple;
						break;
				case SelCond::GT:
					if (diff <= 0) goto next_tuple;
						break;
				case SelCond::LT:
					if (diff >= 0) goto next_tuple;
						break;
				case SelCond::GE:
					if (diff < 0) goto next_tuple;
						break;
				case SelCond::LE:
					if (diff > 0) goto next_tuple;
						break;
			}
		}

		// the condition is met for the tuple. 
		// increase matching tuple counter
		count++;

		// print the tuple 
		switch (attr) {
			case 1:  // SELECT key
				fprintf(stdout, "%d\n", key);
				break;
			case 2:  // SELECT value
				fprintf(stdout, "%s\n", value.c_str());
				break;
			case 3:  // SELECT *
				fprintf(stdout, "%d '%s'\n", key, value.c_str());
				break;
		}

		// move to the next tuple
		next_tuple:
		++rid;
	}

	// print matching tuple count if "select count(*)"
	if (attr == 4) {
	fprintf(stdout, "%d\n", count);
	}
	rc = 0;

	// close the table file and return
	exit_select:
	rf.close();
	return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
	/* your code here */

	// Junkyum Kim
	// part 2D now. adding the index

	fstream fs;
	RecordFile rf;
	BTreeIndex BTindex;

	RecordId rid;
	int tempKey;
	string tempLine, tempValue;
	int errorCode = -1015; // error value to return in case of an error. For now set to -1015
	
	if (index){
		if (BTindex.open(table + ".idx", 'w') != 0){
			cout << "error when opening IndexFile " << table << ".idx .\n";
			return RC_FILE_OPEN_FAILED;
		}
	}
	
	if (rf.open(table + ".tbl",'w') != 0){
		cout << "error when opening RecordFile " << table << ".tbl .\n";
		return RC_FILE_OPEN_FAILED;
	}
		
	fs.open(loadfile.c_str(), fstream::in);

	if (!fs.is_open()){
		cout << "load file " << loadfile << " does not exist.\n";
		return errorCode;
	}

	else {
		while (getline(fs,tempLine)){
			parseLoadLine(tempLine, tempKey, tempValue);
			rf.append(tempKey, tempValue, rid);
			if (index)
				BTindex.insert(tempKey, rid);
		}
	}
	
	if (index){
		BTindex.close();
	}
	
	return 0;
}

RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
    const char *s;
    char        c;
    string::size_type loc;
    
    // ignore beginning white spaces
    c = *(s = line.c_str());
    while (c == ' ' || c == '\t') { c = *++s; }

    // get the integer key value
    key = atoi(s);

    // look for comma
    s = strchr(s, ',');
    if (s == NULL) { return RC_INVALID_FILE_FORMAT; }

    // ignore white spaces
    do { c = *++s; } while (c == ' ' || c == '\t');
    
    // if there is nothing left, set the value to empty string
    if (c == 0) { 
        value.erase();
        return 0;
    }

    // is the value field delimited by ' or "?
    if (c == '\'' || c == '"') {
        s++;
    } else {
        c = '\n';
    }

    // get the value string
    value.assign(s);
    loc = value.find(c, 0);
    if (loc != string::npos) { value.erase(loc); }

    return 0;
}
