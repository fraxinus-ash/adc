/*
 * ADCSession.h
 *
 *  Created on: 26.06.1999
 *      Author: Fraxinus Ash
 *
 *  Version 0.9.21
 */

#ifndef ADCSESSION_H_
#define ADCSESSION_H_

#include "ADC.h"

class ADCSession {

private:
	static int SessionCounter;

	ADAID_T m_AdaId;
	char m_UserId[9];
	ADC m_ADC;
	ADC* m_RecentADC;
	int m_DatabaseId;
	char m_ResposeString[512];

	int ADCSession::process(ADC* adc);

public:
	ADCSession(const char* username);
	virtual ~ADCSession();

	const char* getUserId();
	ADC* getControlBlock();

	bool isOpen();

	void setTraceLevel(int trace);
	int getTraceLevel();

	int open(int databaseId, const char* expressions, int nonActivityTimeLimit,
			int transactionTimeLimit);
	int close();

	int rollback();
	int commit();

	int insert(const char *formatBuf, const void *recordBuf,
			int recordBufLength);

	int find(int fileNo, const char* commandId, bool hold,
			const char *formatBuf, const char *searchBuf, const void *valueBuf,
			int valueBufLength, int recordBufLength);
	int getNext();
	int update(const char *formatBuf, const void *recordBuf,
			int recordBufLength);
	int remove();
	int release();

	int process();

	void dumpResponse();
};

#endif /* ADCSESSION_H_ */
