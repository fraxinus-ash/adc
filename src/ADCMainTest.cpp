/*
 * ADCMainTest.cpp
 *
 * Adaption of the SAG C example for ADABAS calls
 *  - provides wrapper classes for Sessions, ACBX and/or ACB
 *
 * Created on: 08.04.2015
 *      Author: Fraxinus Ash
 *
 *  Version 0.8.01
 */

#include <iostream>

#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
#include <algorithm>
#include <windows.h>
#include <time.h>
#else
#include <unistd.h>
#include <macros.h>
#endif

#include "ADCSession.h"

static ADC acb;

int dbid;
int emp_file;

struct TRecord {
	char firstname[20];
	char name[20];
	int salary;
};

char fullname[50];

void dump_usage();
int open_db();
int close_db();
void dump_response();
int update(int salary);
int find();
int rollback();
char* getFullname(TRecord*);

int main(int argc, char **argv) {

	printf("ADC Unit Test\n");

	if (argc == 3) {
		if (sscanf(argv[1], "%d", &dbid) == 0)
			dump_usage();
		if (sscanf(argv[2], "%d", &emp_file) == 0)
			dump_usage();
	} else {
		dump_usage();
	}

	/*
	 * Usage of sessions
	 */
	printf("Demonstrate sessions\n");

	char openrb[100];
	sprintf(openrb, "UPD=%d.", emp_file);

	TRecord findrb;
	char findfb[] = "BA,20,BC,20,LB1,4,F.";
	char updfb[] = "LB1,4,F.";

	ADCSession as1 = ADCSession("user_01");
	//as1.setTraceLevel(9);
	int rc = as1.open(dbid, openrb, -1, -1);
	if (rc != ADA_NORMAL) {
		as1.dumpResponse();
	}
	//as1.setTraceLevel(0);

	ADCSession as2 = ADCSession("user_02");
	rc = as2.open(dbid, openrb, -1, -1);
	if (rc != ADA_NORMAL) {
		as2.dumpResponse();
	}

	as1.setTraceLevel(9);
	rc = as1.find(emp_file, "FIND", true, findfb, "BC,5.", "SMITH",
			strlen("SMITH"), sizeof(TRecord));
	as1.setTraceLevel(0);
	if (rc == ADA_NORMAL) {
		printf("Found %ld records with name 'SMITH'\n",
				as1.getControlBlock()->getISNQuantity());
		rc = as1.getNext();
		if (rc == ADA_NORMAL) {
			as1.getControlBlock()->getRecordBuffer((char*) &findrb,
					sizeof(TRecord));
			printf(" ISN=%6ld, name='%-25s', salary=%7d\n",
					as1.getControlBlock()->getISN(), getFullname(&findrb),
					findrb.salary);
		}
		/*as1.setTraceLevel(9);
		 if ((rc = as1.remove()) != ADA_NORMAL) {
		 as1.dumpResponse();
		 }
		 as1.setTraceLevel(0);*/
	}

	rc = as2.find(emp_file, "FIND", true, findfb, "BC,9.", "Johansson",
			strlen("Johansson"), sizeof(TRecord));
	if (rc == ADA_NORMAL) {
		printf("Found %ld records with name 'Johansson'\n",
				as2.getControlBlock()->getISNQuantity());
		rc = as2.getNext();
		if (rc == ADA_NORMAL) {
			as2.getControlBlock()->getRecordBuffer((char*) &findrb,
					sizeof(TRecord));
			findrb.name[19] = 0;
			printf(" ISN=%6ld, name='%-25s', salary=%7d\n",
					as2.getControlBlock()->getISN(), getFullname(&findrb),
					findrb.salary);
		}
	}

	printf("Read and update the remaining records\n");

	while ((rc = as1.getNext()) == ADA_NORMAL) {
		as1.getControlBlock()->getRecordBuffer((char*) &findrb,
				sizeof(TRecord));
		int new_salary = findrb.salary + findrb.salary / 10;
		//as1.setTraceLevel(9);
		if ((rc = as1.update(updfb, &new_salary, 4)) != ADA_NORMAL) {
			as1.dumpResponse();
			new_salary = findrb.salary;
		}
		//as1.setTraceLevel(0);
		printf(" ISN=%6ld, name='%-25s', salary=%7d, new_salary=%7d\n",
				as1.getControlBlock()->getISN(), getFullname(&findrb),
				findrb.salary, new_salary);
	}

	while ((rc = as2.getNext()) == ADA_NORMAL) {
		as2.getControlBlock()->getRecordBuffer((char*) &findrb,
				sizeof(TRecord));
		int new_salary = findrb.salary + 100;
		//as2.setTraceLevel(9);
		if ((rc = as2.update(updfb, &new_salary, 4)) != ADA_NORMAL) {
			as2.dumpResponse();
			new_salary = findrb.salary;
		}
		//as2.setTraceLevel(0);
		printf(" ISN=%6ld, name='%-25s', salary=%7d, new_salary=%7d\n",
				as2.getControlBlock()->getISN(), getFullname(&findrb),
				findrb.salary, new_salary);
	}

	if (as1.rollback() != ADA_NORMAL) {
		as1.dumpResponse();
	}

	if (as2.rollback() != ADA_NORMAL) {
		as2.dumpResponse();
	}

	if (as1.close() != ADA_NORMAL) {
		as1.dumpResponse();
	}

	if (as2.close() != ADA_NORMAL) {
		as2.dumpResponse();
	}

	printf("Press Enter to Continue");
	fflush(stdout);
	getchar();

	/*
	 * Normal usage (ACBX)
	 */
	printf("Demonstrate single user access\n");

	acb.setControlBufferType('X');

	if (open_db() != ADA_NORMAL) {
		dump_response();
	} else {
		int upd = 0;
		acb.clear();

		if (find() == ADA_NORMAL) {
			printf(
					"Found  %ld records with name 'SMITH', increase salary by 10 %%\n",
					acb.getISNQuantity());

			while (acb.getReturnCode() == ADA_NORMAL
					&& acb.getISNQuantity() != 0) {
				int old_salary;
				acb.getRecordBuffer((char*) &old_salary, 4);
				int new_salary = old_salary + old_salary / 10;

				if (update(new_salary) != ADA_NORMAL) {
					//cb.cb_isn_quantity = 0;
				} else {
					upd++;
					printf(
							"%3d. ISN = %8d  old salary = %10ld   new salary = %10ld\n",
							upd, acb.getISN(), old_salary, new_salary);
					find(); // next record
				}
			}
		}

		if (acb.getReturnCode() != ADA_NORMAL) {
			dump_response();
			//cb.cb_return_code = ADA_NORMAL;
			if (upd != 0) {
				if (rollback() == ADA_NORMAL)
					upd = 0;
				else
					dump_response();
			}
		}
		rollback();

		if (close_db() != ADA_NORMAL)
			dump_response();
	}

	return 0;
}

/*
 * Print usage
 */
void dump_usage() {
	printf("usage: <program> <dbid> <employees file number>\n");
	exit(1);
}

/*
 * Print response
 */
void dump_response() {
	printf("** Response code %d(%d) from ADABAS for Command %-2.2s\n",
			acb.getReturnCode(), acb.getErrorSubCode(), acb.getCommand());
	char add2[4];
	acb.getAddition(add2, CB_L_AD2, 2);
	printf("** Additions2 %d %d\n", add2[2], add2[3]);
	return;
}

/*
 * Open DB
 */
int open_db() {
	printf("Open database.\n");

	acb.clear();
	acb.setDatabase(dbid);
	acb.setCommand("OP");

	char openrb[100];
	sprintf(openrb, "UPD=%d.", emp_file);
	acb.setRecordBuffer(openrb, strlen(openrb));
	acb.setAddition("FRAX_ASH", 8, 1);

	int result = 0;

	do {
		result = acb.process();
	} while (result == ADA_TABT);

	return result;
}

/*
 * Close DB
 */
int close_db() {
	printf("Close database.\n");

	acb.clear();
	acb.setDatabase(dbid);
	acb.setCommand("CL");
	return acb.process();
}

/*
 * Find record(s)
 */
int find() {
	if (acb.getISNQuantity() == 0) {
		printf("Find record(s).\n");
		//acb.clear();
		acb.setDatabase(dbid);
		acb.setFileNo(emp_file);
		acb.setCommand("S4");
		acb.setCommandId("FIND", 4);
		acb.setFormatBuffer("LB1,4,F.", 8);
		acb.setSearchBuffer("BC,5.", 5);
		acb.setValueBuffer("SMITH", 5);
		acb.setRecordBuffer("", 4);

		if (acb.process() == ADA_EOF) {
			acb.clear();
			return acb.getReturnCode();
		}
	}
	acb.setCommand("L4");
	acb.setOption(ADA_GET_NEXT, (short) 2);
	if (acb.process() == ADA_EOF) {
		acb.clear();
	}
	return acb.getReturnCode();
}

/*
 * Update record
 */
int update(int salary) {
	acb.setCommand("A1");
	acb.setRecordBuffer((const char*) &salary, 4);
	return acb.process();
}

/*
 * Backout transaction
 */
int rollback() {
	acb.clear();
	acb.setDatabase(dbid);
	acb.setCommand("BT");
	return acb.process();
}

char* getFullname(TRecord* findrb) {
	findrb->name[19] = 0;
	findrb->firstname[19] = 0;
	sscanf(findrb->name, "%s", findrb->name);
	sscanf(findrb->firstname, "%s", findrb->firstname);
	sprintf(fullname, "%s, %s", findrb->name, findrb->firstname);
	return fullname;
}

