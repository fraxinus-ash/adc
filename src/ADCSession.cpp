/*
 * ADCSession.h
 *
 *  Created on: 26.06.1999
 *      Author: Fraxinus Ash
 *
 *  Version 0.9.21
 */

#include "ADCSession.h"

int ADCSession::SessionCounter = 0;

ADCSession::ADCSession(const char* username) {
	SessionCounter++;

	int cc = lnk_get_adabas_id(sizeof(ADAID_T), (unsigned char *) &m_AdaId);

	m_AdaId.s_level = ADA_SLEVEL_3;
	m_AdaId.s_size = sizeof(ADAID_T);
	m_AdaId.s_pid = (unsigned int) SessionCounter;
	memcpy(m_AdaId.s_user, username, L_USER);
	//memcpy(sAdaId.s_node, username, L_ANID);

	char temp[9];
	sprintf(temp, "%s", m_AdaId.s_user);
	char *s = temp;
	while (*s) {
		*s = toupper((unsigned char) *s);
		s++;
	}
	sprintf(m_UserId, "%.2s%06d%", temp, m_AdaId.s_pid);

	m_ADC.setControlBufferType('X');
	m_ADC.clear();

	m_RecentADC = &m_ADC;

	m_DatabaseId = 0;
}

ADCSession::~ADCSession() {
}

const char* ADCSession::getUserId() {
	return m_UserId;
}

ADC* ADCSession::getControlBlock() {
	return &m_ADC;
}

void ADCSession::setTraceLevel(int trace) {
	m_ADC.setTraceLevel(trace);
}

int ADCSession::getTraceLevel() {
	return m_ADC.getTraceLevel();
}

bool ADCSession::isOpen() {
	if (m_DatabaseId > 0) {
		return true;
	}
	return false;
}

int ADCSession::open(int databaseId, const char* expressions,
		int nonActivityTimeLimit, int transactionTimeLimit) {
	int rc = ADA_NORMAL;

	if (isOpen()) {
		rc = close();
		if (rc != ADA_NORMAL) {
			return rc;
		}
	}

	m_DatabaseId = databaseId;

	m_ADC.clear();
	m_ADC.setDatabase(m_DatabaseId);
	m_ADC.setCommand("OP");

	if ((expressions != 0L) && (strlen(expressions) > 0)) {
		m_ADC.setRecordBuffer(expressions, strlen(expressions));
	}
	if ((getUserId() != 0L) && (strlen(getUserId()) > 0)) {
		m_ADC.setAddition(getUserId(), strlen(getUserId()), 1);
	}

	rc = process();

	if (rc != ADA_NORMAL) {
		m_DatabaseId = 0;
	}

	return rc;
}

int ADCSession::close() {
	int rc = ADA_NORMAL;
	if (isOpen()) {
		m_ADC.clear();
		m_ADC.setDatabase(m_DatabaseId);
		m_ADC.setCommand("CL");

		rc = process();

		if (rc == ADA_NORMAL) {
			m_DatabaseId = 0;
		}
	}
	return rc;
}

int ADCSession::rollback() {
	int rc = ADA_NORMAL;
	if (isOpen()) {
		m_ADC.clear();
		m_ADC.setDatabase(m_DatabaseId);
		m_ADC.setCommand("BT");

		rc = process();
	}
	return rc;
}

int ADCSession::commit() {
	int rc = ADA_NORMAL;
	if (isOpen()) {
		m_ADC.clear();
		m_ADC.setDatabase(m_DatabaseId);
		m_ADC.setCommand("ET");

		rc = process();
	}
	return rc;
}

int ADCSession::find(int fileNo, const char* commandId, bool hold,
		const char *formatBuf, const char *searchBuf, const void *valueBuf,
		int valueBufLength, int recordBufLength) {
	int rc = -100;

	if (isOpen()) {
		m_ADC.clear();
		m_ADC.setDatabase(m_DatabaseId);
		m_ADC.setFileNo(fileNo);
		if (hold) {
			m_ADC.setCommand("S4");
		} else {
			m_ADC.setCommand("S1");
		}
		m_ADC.setCommandId("FIND", 4);
		m_ADC.setFormatBuffer(formatBuf, strlen(formatBuf));
		m_ADC.setSearchBuffer(searchBuf, strlen(searchBuf));
		m_ADC.setValueBuffer((const char*) valueBuf, valueBufLength);
		m_ADC.setRecordBuffer("", recordBufLength);

		rc = process();
	}

	return rc;
}

int ADCSession::getNext() {
	int rc = ADA_EOF;

	if (m_ADC.getISNQuantity() > 0) {
		if (m_ADC.getCommand()[1] == '4') {
			m_ADC.setCommand("L4");
		} else {
			m_ADC.setCommand("L1");
		}
		m_ADC.setOption(ADA_GET_NEXT, (short) 2);

		rc = process();

		if (rc == ADA_EOF) {
			m_ADC.clear();
		}
	}

	return rc;
}

int ADCSession::update(const char *formatBuf, const void *recordBuf,
		int recordBufLength) {
	int rc = -101;

	if (m_ADC.getISNQuantity() > 0) {
		ADC adc;
		adc.setControlBufferType(m_ADC.getControlBufferType());
		adc.setTraceLevel(m_ADC.getTraceLevel());
		adc.clear();
		adc.setISN(m_ADC.getISN());
		adc.setDatabase(m_ADC.getDatabase());
		adc.setFileNo(m_ADC.getFileNo());
		adc.setCommand("A1");
		if (m_ADC.getCommand()[1] == '4') { /* hold */
			adc.setOption(ADA_RETURN_OPT, (short) 1);
		}
		adc.setFormatBuffer(formatBuf, strlen(formatBuf));
		adc.setRecordBuffer((const char*) recordBuf, recordBufLength);

		rc = process(&adc);
	}

	return rc;
}

int ADCSession::remove() {
	int rc = -101;

	if (m_ADC.getISNQuantity() > 0) {
		ADC adc;
		adc.setControlBufferType(m_ADC.getControlBufferType());
		adc.setTraceLevel(m_ADC.getTraceLevel());
		adc.clear();
		adc.setISN(m_ADC.getISN());
		adc.setDatabase(m_ADC.getDatabase());
		adc.setFileNo(m_ADC.getFileNo());
		adc.setCommand("E1");
		if (m_ADC.getCommand()[1] == '4') { /* hold */
			adc.setOption(ADA_RETURN_OPT, (short) 1);
		}
		rc = process(&adc);
	}

	return rc;
}

int ADCSession::release() {
	int rc = -100;

	if (isOpen()) {
		ADC adc;
		adc.setControlBufferType(m_ADC.getControlBufferType());
		adc.setTraceLevel(m_ADC.getTraceLevel());
		adc.clear();
		adc.setDatabase(m_ADC.getDatabase());
		adc.setCommandId(m_ADC.getCommandId(), 4);
		adc.setCommand("RC");

		rc = process(&adc);
	}

	return rc;
}

int ADCSession::insert(const char *formatBuf, const void *recordBuf,
		int recordBufLength) {
	int rc = -100;

	if (isOpen()) {
		ADC adc;
		adc.setControlBufferType(m_ADC.getControlBufferType());
		adc.setTraceLevel(m_ADC.getTraceLevel());
		adc.clear();
		adc.setDatabase(m_ADC.getDatabase());
		adc.setFileNo(m_ADC.getFileNo());
		adc.setCommand("N1");
		adc.setFormatBuffer(formatBuf, strlen(formatBuf));
		adc.setRecordBuffer((const char*) recordBuf, recordBufLength);

		rc = process(&adc);
		/*if (rc = ADA_NORMAL) {
		 m_ADC.setISN(adc.getISN());
		 }*/
	}

	return rc;
}

int ADCSession::process() {
	return process(&m_ADC);
}

int ADCSession::process(ADC* adc) {

	if ((getTraceLevel() > 3)) {
		printf("USERID=%s, %s\n", getUserId(), adc->getCommand());
	}

	int rc = lnk_set_adabas_id((unsigned char *) &m_AdaId);

	if (rc != ADA_SUCCESS) {
		if ((getTraceLevel() > 0)) {
			printf("CALL_SETID returned %d\n", rc);
		}
		return rc;
	}

	m_RecentADC = adc;

	rc = adc->process();

	if (getTraceLevel() > 8) {
		printf("%s\n", adc->getTrace(1)); // request
		printf("%s\n", adc->getTrace());  // response
	}

	return rc;
}

void ADCSession::dumpResponse() {
	printf("** Response code %d(subcode %d) from ADABAS for Command %-2.2s\n",
			m_RecentADC->getReturnCode(), m_RecentADC->getErrorSubCode(),
			m_RecentADC->getCommand());

	char add2[4];
	m_RecentADC->getAddition(add2, CB_L_AD2, 2);
	printf("** Additions2 %d %d %d %d\n", add2[0], add2[1], add2[2], add2[3]);
}

