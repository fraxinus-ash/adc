/*
 * ADC.h
 *
 *  Created on: 17.01.1998
 *      Author: Fraxinus Ash
 *
 *  Version 0.9.21
 */

#ifndef ADC_H_
#define ADC_H_

#include <iostream>

extern "C" {
#include <adabas.h>
}

#define DC_L_RB	60000
#define DC_L_FB	12000
#define DC_L_SB	12000
#define DC_L_VB	12000

#define DC_L_OPTIONS 8

#define TRC_STACK_SIZE 10
#define TRC_BUFFER_LEN 1024

class ADC {

	static int InstanceCounter;

	char m_RecordBuffer[DC_L_RB];
	char m_FormatBuffer[DC_L_FB];
	char m_SearchBuffer[DC_L_SB];
	char m_ValueBuffer[DC_L_VB];
	int m_BufferLength[5];

	int m_Attempts;

	int m_TraceLevel;
	CB_PAR m_cbTrace[TRC_STACK_SIZE];
	ACBX m_cbxTrace[TRC_STACK_SIZE];
	char m_rbTrace[TRC_STACK_SIZE][TRC_BUFFER_LEN];
	char m_fbTrace[TRC_STACK_SIZE][TRC_BUFFER_LEN];
	char m_sbTrace[TRC_STACK_SIZE][TRC_BUFFER_LEN];
	char m_vbTrace[TRC_STACK_SIZE][TRC_BUFFER_LEN];
	int m_TracePointer;
	int m_TraceGlobalCounter;

	char m_TraceOutput[L_ACBX + 1024 + (4 * 128)];

	char m_Output[64];
	char m_CommandId[L_CID + 1];

	CB_PAR cb;

	/*
	 * ACBX stuff
	 */
	char m_CB_Type;
	ACBX cbx;

	void dumpTrace();
	char* format4Trace(char *out, void *val, int len);

public:

	ADC();
	~ADC();

	int getInstanceCounter();

	void setControlBufferType(char type);
	char getControlBufferType();

	bool isACBX();

	void clear();

	int setCommand(const char* command);
	const char* getCommand();

	int setCommandId(const char* commandId, int len);
	const char* getCommandId();

	int setDatabase(int database);
	int getDatabase();
	int setFileNo(int fileNo);
	int getFileNo();

	int setDatabaseFileNo(int database, int fileNo);

	int setISN(unsigned long isn);
	int getISN();

	int setOption(char option, int number);

	int setAddition(const char* addition, int length, int number);
	int getAddition(char* addition, int length, int number);

	int setFormatBuffer(const char* buffer, int len);

	int setRecordBuffer(const char* buffer, int len);
	int getRecordBuffer(char* buffer, int len);

	int setSearchBuffer(const char* buffer, int len);

	int setValueBuffer(const char* buffer, int len);

	int getISNQuantity();
	int getReturnCode();
	int getErrorSubCode();

	void setTraceLevel(int trace);
	int getTraceLevel();
	char* getTrace();
	char* getTrace(int deep);

	int process();
};

#endif /* ADC_H_ */
