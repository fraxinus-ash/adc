/*
 * ADC.cpp
 *
 *  Created on: 17.01.1998
 *      Author: Fraxinus Ash
 *
 *  Version 0.9.21
 */
#include <iostream>

#include "ADC.h"

#if defined(_WIN32) || defined(_WIN64)
#include <algorithm>
#include <windows.h>
#include <time.h>
#else
#include <unistd.h>
#include <macros.h>
#endif

using namespace std;

#define FORMAT_BUFFER	0
#define RECORD_BUFFER	1
#define SEARCH_BUFFER	2
#define VALUE_BUFFER	3

int ADC::InstanceCounter = 0;

ADC::ADC() {
	InstanceCounter++;

	m_Attempts = 3;
	m_CB_Type = ' ';

	m_TraceLevel = 0;
	m_TracePointer = 0;
	m_TraceGlobalCounter = 0;
}

ADC::~ADC() {
	InstanceCounter--;
}

int ADC::getInstanceCounter() {
	return InstanceCounter;
}

/*
 * Control buffer type
 * ' ' ACB
 * 'X' ACBX
 */
void ADC::setControlBufferType(char type) {
	m_CB_Type = type;
}

char ADC::getControlBufferType() {
	return m_CB_Type;
}

bool ADC::isACBX() {
	if (m_CB_Type == 'X') {
		return true;
	}
	return false;
}

/*
 * Trace level
 */
void ADC::setTraceLevel(int trace) {
	m_TraceLevel = trace;
}

int ADC::getTraceLevel() {
	return m_TraceLevel;
}

/*
 * Clear control block and buffers
 */
void ADC::clear() {

	if (!isACBX()) {
		cb.cb_call_type = 0;
		cb.cb_reserved = 0;
		memset(cb.cb_cmd_code, '\0', 2);
		memset(cb.cb_cmd_id, '\0', L_CID);
		cb.cb_db_id = 0;
		cb.cb_file_nr = 0;
		cb.cb_return_code = 0;
		cb.cb_isn = 0;
		cb.cb_isn_ll = 0;
		cb.cb_isn_quantity = 0;
		cb.cb_fmt_buf_lng= 0;
		cb.cb_rec_buf_lng= 0;
		cb.cb_sea_buf_lng= 0;
		cb.cb_val_buf_lng= 0;
		cb.cb_isn_buf_lng= 0;
		cb.cb_cop1 = ' ';
		cb.cb_cop2 = ' ';
		memset(cb.cb_add1, '\0', CB_L_AD1);
		memset(cb.cb_add2, '\0', CB_L_AD2);
		memset(cb.cb_add3, '\0', CB_L_AD3);
		memset(cb.cb_add4, '\0', CB_L_AD4);
		memset(cb.cb_add5, '\0', CB_L_AD5);
		cb.cb_cmd_time = 0;
		memset(cb.cb_user_area, '\0', 4);
	} else {
		SETACBX(&cbx);
	}

	m_BufferLength[FORMAT_BUFFER] = 0;
	m_BufferLength[RECORD_BUFFER] = 0;
	m_BufferLength[SEARCH_BUFFER] = 0;
	m_BufferLength[VALUE_BUFFER] = 0;

	return;
}

int ADC::setCommand(const char* cmd) {

	if (!isACBX()) {
		cb.cb_cmd_code[0] = cmd[0];
		cb.cb_cmd_code[1] = cmd[1];
	} else {
		cbx.acbxcmd[0] = cmd[0];
		cbx.acbxcmd[1] = cmd[1];
	}
	return 0;
}

const char* ADC::getCommand() {

	if (!isACBX()) {
		m_Output[0] = cb.cb_cmd_code[0];
		m_Output[1] = cb.cb_cmd_code[1];
		m_Output[2] = '\0';
	} else {
		m_Output[0] = cbx.acbxcmd[0];
		m_Output[1] = cbx.acbxcmd[1];
		m_Output[2] = '\0';
	}
	return m_Output;
}

int ADC::setCommandId(const char* cmdId, int len) {

	len = min((int )len, L_CID);
	if (!isACBX()) {
		memcpy(cb.cb_cmd_id, cmdId, len);
	} else {
		memcpy(cbx.acbxcid, cmdId, len);
	}
	return 0;
}

const char* ADC::getCommandId() {

	if (!isACBX()) {
		memcpy(m_CommandId, cb.cb_cmd_code, L_CID);
	} else {
		memcpy(m_CommandId, cbx.acbxcid, L_CID);
	}
	m_CommandId[L_CID] = '\0';
	return m_CommandId;
}

// deprecated
int ADC::setDatabase(int database) {

	if (database > 0) {
		if (!isACBX()) {
			cb.cb_db_id = database;
		} else {
			cbx.acbxdbid = database;
		}
	}
	return 0;
}

int ADC::getDatabase() {

	if (!isACBX()) {
		return (int) cb.cb_db_id;
	}
	return cbx.acbxdbid;
}

// deprecated
int ADC::setFileNo(int table) {

	if (table > 0) {
		if (!isACBX()) {
			cb.cb_file_nr = table;
		} else {
			cbx.acbxfnr = table;
		}
	}
	return 0;
}

int ADC::getFileNo() {

	if (!isACBX()) {
		return (int) cb.cb_file_nr;
	}
	return cbx.acbxfnr;
}

int ADC::setDatabaseFileNo(int database, int table) {

	if (!isACBX()) {
		CB_SET_FD(&cb, database, table);
	} else {
		cbx.acbxdbid = database;
		cbx.acbxfnr = table;
	}
	return 0;
}

int ADC::setISN(unsigned long isn) {

	if (!isACBX()) {
		cb.cb_isn = isn;
	} else {
		cbx.acbxisn = isn;
	}
	return 0;
}

int ADC::getISN() {

	if (!isACBX()) {
		return (int) cb.cb_isn;
	}
	return cbx.acbxisn;
}

int ADC::setOption(char option, int number) {

	if (option == ' ') {
		return 0;
	}

	if (!isACBX()) {
		switch (number) {
		case 1:
			cb.cb_cop1 = option;
			break;
		case 2:
			cb.cb_cop2 = option;
			break;
		}
	} else {
		switch (number) {
		case 1:
			cbx.acbxcop1 = option;
			break;
		case 2:
			cbx.acbxcop2 = option;
			break;
		case 3:
			cbx.acbxcop3 = option;
			break;
		case 4:
			cbx.acbxcop4 = option;
			break;
		case 5:
			cbx.acbxcop5 = option;
			break;
		case 6:
			cbx.acbxcop6 = option;
			break;
		case 7:
			cbx.acbxcop7 = option;
			break;
		case 8:
			cbx.acbxcop8 = option;
			break;
		}
	}
	return 0;
}

int ADC::setFormatBuffer(const char* buffer, int len) {
	len = min((int )len, DC_L_FB);
	m_BufferLength[FORMAT_BUFFER] = len;
	memcpy(m_FormatBuffer, buffer, m_BufferLength[FORMAT_BUFFER]);
	return 0;
}

int ADC::setRecordBuffer(const char* buffer, int len) {
	len = min((int )len, DC_L_RB);
	m_BufferLength[RECORD_BUFFER] = len;
	memcpy(m_RecordBuffer, buffer, m_BufferLength[RECORD_BUFFER]);
	return 0;
}

int ADC::getRecordBuffer(char* buffer, int len) {
	len = min(len, m_BufferLength[RECORD_BUFFER]);
	memcpy(buffer, m_RecordBuffer, len);
	//m_BufferLength[RECORD_BUFFER] = len;
	//memcpy(buffer, m_RecordBuffer, m_BufferLength[RECORD_BUFFER]);
	return 0;
}

int ADC::setSearchBuffer(const char* buffer, int len) {
	len = min((int )len, DC_L_SB);
	m_BufferLength[SEARCH_BUFFER] = len;
	memcpy(m_SearchBuffer, buffer, m_BufferLength[SEARCH_BUFFER]);
	return 0;
}

int ADC::setValueBuffer(const char* buffer, int len) {
	len = min((int )len, DC_L_VB);
	m_BufferLength[VALUE_BUFFER] = len;
	memcpy(m_ValueBuffer, buffer, m_BufferLength[VALUE_BUFFER]);
	return 0;
}

int ADC::setAddition(const char* addition, int length, int number) {
	int len;

	if (!isACBX()) {
		switch (number) {
		case 1:
			len = CB_L_AD1;
			memcpy(cb.cb_add1, addition, min(len, length));
			break;
		case 2:
			len = CB_L_AD2;
			memcpy(cb.cb_add2, addition, min(len, length));
			break;
		case 3:
			len = CB_L_AD3;
			memcpy(cb.cb_add3, addition, min(len, length));
			break;
		case 4:
			len = CB_L_AD4;
			memcpy(cb.cb_add4, addition, min(len, length));
			break;
		case 5:
			len = CB_L_AD5;
			memcpy(cb.cb_add5, addition, min(len, length));
			break;
		default:
			return -1;
		}
	} else {
		switch (number) {
		case 1:
			memcpy(cbx.acbxadd1, addition, min(8, length));
			break;
		case 2:
			memcpy(cbx.acbxadd2, addition, min(4, length));
			break;
		case 3:
			memcpy(cbx.acbxadd3, addition, min(8, length));
			break;
		case 4:
			memcpy(cbx.acbxadd4, addition, min(8, length));
			break;
		case 5:
			memcpy(cbx.acbxadd5, addition, min(8, length));
			break;
		case 6:
			memcpy(cbx.acbxadd6, addition, min(8, length));
			break;
		default:
			return -1;
		}
	}
	return 0;
}

int ADC::getAddition(char* addition, int length, int number) {

	int len;

	if (!isACBX()) {
		switch (number) {
		case 1:
			len = CB_L_AD1;
			memcpy(addition, cb.cb_add1, min(len, length));
			break;
		case 2:
			len = CB_L_AD2;
			memcpy(addition, cb.cb_add2, min(len, length));
			break;
		case 3:
			len = CB_L_AD3;
			memcpy(addition, cb.cb_add3, min(len, length));
			break;
		case 4:
			len = CB_L_AD4;
			memcpy(addition, cb.cb_add4, min(len, length));
			break;
		case 5:
			len = CB_L_AD5;
			memcpy(addition, cb.cb_add5, min(len, length));
			break;
		default:
			return -1;
		}
	} else {
		switch (number) {
		case 1:
			len = 8;
			memcpy(addition, cbx.acbxadd1, min(len, length));
			break;
		case 2:
			len = 4;
			memcpy(addition, cbx.acbxadd2, min(len, length));
			break;
		case 3:
			len = 8;
			memcpy(addition, cbx.acbxadd3, min(len, length));
			break;
		case 4:
			len = 8;
			memcpy(addition, cbx.acbxadd4, min(len, length));
			break;
		case 5:
			len = 8;
			memcpy(addition, cbx.acbxadd5, min(len, length));
			break;
		case 6:
			len = 8;
			memcpy(addition, cbx.acbxadd6, min(len, length));
			break;
		default:
			return -1;
		}
	}
	return 0;
}

int ADC::getISNQuantity() {

	if (!isACBX()) {
		return (int) cb.cb_isn_quantity;
	}
	return cbx.acbxisq;
}

int ADC::getReturnCode() {

	if (!isACBX()) {
		return cb.cb_return_code;
	}
	return cbx.acbxrsp;
}

int ADC::getErrorSubCode() {

	if (!isACBX()) {
		return 0;
	}
	return cbx.acbxerrc;
}

int ADC::process() {

	int time = 0;

	char* rb = NULL;
	char* fb = NULL;
	char* sb = NULL;
	char* vb = NULL;

	if (m_BufferLength[FORMAT_BUFFER] > 0) {
		fb = m_FormatBuffer;
	}
	if (m_BufferLength[RECORD_BUFFER] > 0) {
		rb = m_RecordBuffer;
	}
	if (m_BufferLength[SEARCH_BUFFER] > 0) {
		sb = m_SearchBuffer;
	}
	if (m_BufferLength[VALUE_BUFFER] > 0) {
		vb = m_ValueBuffer;
	}

	if (!isACBX()) {

		cb.cb_fmt_buf_lng= m_BufferLength[FORMAT_BUFFER];
		cb.cb_rec_buf_lng= m_BufferLength[RECORD_BUFFER];
		cb.cb_sea_buf_lng= m_BufferLength[SEARCH_BUFFER];
		cb.cb_val_buf_lng= m_BufferLength[VALUE_BUFFER];

		dumpTrace();
		adabas(&cb, fb, rb, sb, vb, NULL);
		dumpTrace();

		while (cb.cb_return_code == ADA_ALOCK && time < m_Attempts) {
#if defined(_WIN32) || defined(_WIN64)
			Sleep(1000);
#else
			sleep(1);
#endif
			time++;
			dumpTrace();
			adabas(&cb, fb, rb, sb, vb, NULL);
			dumpTrace();
		}

		return cb.cb_return_code;

	} else {

		ABD arb;
		ABD afb;
		ABD asb;
		ABD avb;
		PABD abdL[4];
		int adbIndex = 0;

		SETABD(&afb);
		afb.abdid = ABDQFB;
		afb.abdsize = m_BufferLength[FORMAT_BUFFER];
		afb.abdsend = afb.abdsize;
		if (afb.abdsize > 0) {
			afb.abdaddr = fb;
		}
		afb.abdloc = 'I';
		abdL[adbIndex++] = &afb;

		SETABD(&arb);
		arb.abdid = ABDQRB;
		arb.abdsize = m_BufferLength[RECORD_BUFFER];
		arb.abdsend = arb.abdsize;
		if (arb.abdsize > 0) {
			arb.abdaddr = rb;
		}
		arb.abdloc = 'I';
		abdL[adbIndex++] = &arb;

		SETABD(&asb);
		asb.abdid = ABDQSB;
		asb.abdsize = m_BufferLength[SEARCH_BUFFER];
		asb.abdsend = asb.abdsize;
		if (asb.abdsize > 0) {
			asb.abdaddr = sb;
		}
		asb.abdloc = 'I';
		abdL[adbIndex++] = &asb;

		SETABD(&avb);
		avb.abdid = ABDQVB;
		avb.abdsize = m_BufferLength[VALUE_BUFFER];
		avb.abdsend = avb.abdsize;
		if (avb.abdsize > 0) {
			avb.abdaddr = vb;
		}
		avb.abdloc = 'I';
		abdL[adbIndex++] = &avb;

		dumpTrace();
		adabasx(&cbx, adbIndex, abdL);
		dumpTrace();

		while (cbx.acbxrsp == ADA_ALOCK && time < m_Attempts) {
#if defined(_WIN32) || defined(_WIN64)
			Sleep(1000);
#else
			sleep(1);
#endif
			time++;

			dumpTrace();
			adabasx(&cbx, adbIndex, abdL);
			dumpTrace();
		}

		return cbx.acbxrsp;
	}
}

void ADC::dumpTrace() {

	if (m_TraceLevel <= 0) {
		return;
	}

	if (!isACBX()) {
		memcpy(&m_cbTrace[m_TracePointer], &cb, sizeof(cb));
	} else {
		memcpy(&m_cbxTrace[m_TracePointer], &cbx, sizeof(cbx));
	}

	memcpy(&m_rbTrace[m_TracePointer], &m_RecordBuffer, TRC_BUFFER_LEN);
	memcpy(&m_fbTrace[m_TracePointer], &m_FormatBuffer, TRC_BUFFER_LEN);
	memcpy(&m_sbTrace[m_TracePointer], &m_SearchBuffer, TRC_BUFFER_LEN);
	memcpy(&m_vbTrace[m_TracePointer], &m_ValueBuffer, TRC_BUFFER_LEN);

	m_TraceGlobalCounter++;
	if (m_TraceGlobalCounter >= 10000) {
		m_TraceGlobalCounter = 0;
	}

	m_TracePointer++;
	if (m_TracePointer > (TRC_STACK_SIZE - 1)) {
		m_TracePointer = 0;
	}
}

char* ADC::getTrace(int deep) {

	int tx = m_TracePointer - (deep + 1);
	if (tx < 0) {
		tx = TRC_STACK_SIZE + tx;
	} else if (tx > (TRC_STACK_SIZE - 1)) {
		tx = tx - TRC_STACK_SIZE;
	}

	char *p = m_TraceOutput;

	if (m_TraceLevel <= 0) {
		p[0] = '\0';
		return p;
	}

	char tmpbuf[128];
	time_t long_time;
	time(&long_time);
	struct tm *newtime = localtime(&long_time);

	strftime(tmpbuf, 128, "%Y/%m/%d %H:%M:%S", newtime);

	if (!isACBX()) {
		CB_PAR* tcb = &m_cbTrace[tx];

		sprintf(p, "[%d/%d] %s> %s", InstanceCounter,
				(m_TraceGlobalCounter - deep), tmpbuf, "Control Buffer");
		sprintf(p, "%s\n COMMAND CODE    : %c%c", p, tcb->cb_cmd_code[0],
				tcb->cb_cmd_code[1]);
		sprintf(p, "%s\n COMMAND ID      : %X %X %X %X", p, tcb->cb_cmd_id[0],
				tcb->cb_cmd_id[1], tcb->cb_cmd_id[2], tcb->cb_cmd_id[3]);
		sprintf(p, "%s\n DATABASE IDENT  : %d", p, tcb->cb_db_id);
		sprintf(p, "%s\n FILE NUMBER     : %d", p, tcb->cb_file_nr);
		sprintf(p, "%s\n RETURN CODE     : %d", p, (int) tcb->cb_return_code);
		sprintf(p, "%s\n ISN             : %d", p, (int) (tcb->cb_isn));
		sprintf(p, "%s\n ISN LOWER LIMIT : %d", p, (int) (tcb->cb_isn_ll));
		sprintf(p, "%s\n ISN QUANTITY    : %d", p,
				(int) (tcb->cb_isn_quantity));
		sprintf(p, "%s\n I/O BUFFER LEN. : %d,%d,%d,%d,%d", p,
				tcb->cb_fmt_buf_lng,
		tcb->cb_rec_buf_lng,
		tcb->cb_sea_buf_lng,
		tcb->cb_val_buf_lng,
		tcb->cb_isn_buf_lng);
		sprintf(p, "%s\n COMMAND OPTION1 : %c", p, tcb->cb_cop1);
		sprintf(p, "%s\n COMMAND OPTION2 : %c", p, tcb->cb_cop2);
		sprintf(p, "%s\n ADDITION 1      : %.8s", p, tcb->cb_add1);
		sprintf(p, "%s\n ADDITION 2      : %X %X %X %X", p, tcb->cb_add2[0],
				tcb->cb_add2[1], tcb->cb_add2[2], tcb->cb_add2[3]);
		sprintf(p, "%s\n ADDITION 3      : %.8s", p, tcb->cb_add3);
		sprintf(p, "%s\n ADDITION 4      : %.8s", p, tcb->cb_add4);
		sprintf(p, "%s\n ADDITION 5      : %.8s", p, tcb->cb_add5);
		sprintf(p, "%s\n TIME            : %d", p, tcb->cb_cmd_time);
	} else {
		ACBX* tcbx = &m_cbxTrace[tx];

		sprintf(p, "[%d/%d] %s> %s [Version %c%c; length=%d]", InstanceCounter,
				(m_TraceGlobalCounter - deep), tmpbuf, "Control Buffer",
				tcbx->acbxver[0], tcbx->acbxver[1], tcbx->acbxlen);
		sprintf(p, "%s\n COMMAND CODE    : %s", p,
				format4Trace(tmpbuf, tcbx->acbxcmd, 2));
		sprintf(p, "%s\n COMMAND ID      : %s", p,
				format4Trace(tmpbuf, tcbx->acbxcid, 4));
		sprintf(p, "%s\n DATABASE IDENT  : %d", p, tcbx->acbxdbid);
		sprintf(p, "%s\n FILE NUMBER     : %d", p, tcbx->acbxfnr);
		sprintf(p, "%s\n RETURN CODE     : %d", p, tcbx->acbxrsp);
		sprintf(p, "%s\n ISN             : %Lu", p, tcbx->acbxisn);
		sprintf(p, "%s\n ISN LOWER LIMIT : %Lu", p, tcbx->acbxisl);
		sprintf(p, "%s\n ISN QUANTITY    : %Lu", p, tcbx->acbxisq);
		sprintf(p, "%s\n OPTION 1-8      : %s", p,
				format4Trace(tmpbuf, (void*) &tcbx->acbxcop1, 8));
		sprintf(p, "%s\n ADDITION 1      : %s", p,
				format4Trace(tmpbuf, tcbx->acbxadd1, 8));
		sprintf(p, "%s\n ADDITION 2      : %s", p,
				format4Trace(tmpbuf, tcbx->acbxadd2, 4));
		sprintf(p, "%s\n ADDITION 3      : %s", p,
				format4Trace(tmpbuf, tcbx->acbxadd3, 8));
		sprintf(p, "%s\n ADDITION 4      : %s", p,
				format4Trace(tmpbuf, tcbx->acbxadd4, 8));
		sprintf(p, "%s\n ADDITION 5      : %s", p,
				format4Trace(tmpbuf, tcbx->acbxadd5, 8));
		sprintf(p, "%s\n ADDITION 6      : %s", p,
				format4Trace(tmpbuf, tcbx->acbxadd6, 8));
		sprintf(p, "%s\n ERROR OFFSET    : %Lu", p, tcbx->acbxerra);
		sprintf(p, "%s\n ERROR CHAR FIELD: %s", p,
				format4Trace(tmpbuf, tcbx->acbxerrb, 2));
		sprintf(p, "%s\n ERROR SUBCODE   : %d", p, tcbx->acbxerrc);
		sprintf(p, "%s\n ERROR BUFFER ID : %d", p, tcbx->acbxerrd);
		sprintf(p, "%s\n COMPRESSED LEN  : %Lu", p, tcbx->acbxlcmp);
		sprintf(p, "%s\n DECOMPRESSED LEN: %Lu", p, tcbx->acbxldec);
		sprintf(p, "%s\n COMMAND TIME    : %Lu", p, tcbx->acbxcmdt);
		sprintf(p, "%s\n USER FIELD      : %s", p,
				format4Trace(tmpbuf, tcbx->acbxuser, 8));
		sprintf(p, "%s\n SESSION TIME    : %Lu", p, tcbx->acbxsesstime);
	}

	sprintf(p, "%s\n FORMAT BUF(32)  : %s", p,
			format4Trace(tmpbuf, m_fbTrace[tx], 32));
	sprintf(p, "%s\n RECORD BUF(32)  : %s", p,
			format4Trace(tmpbuf, m_rbTrace[tx], 32));
	sprintf(p, "%s\n SERACH BUF(32)  : %s", p,
			format4Trace(tmpbuf, m_sbTrace[tx], 32));
	sprintf(p, "%s\n VALUE  BUF(32)  : %s", p,
			format4Trace(tmpbuf, m_vbTrace[tx], 32));

	return p;
}

char* ADC::getTrace() {
	return getTrace(0);
}

char* ADC::format4Trace(char *out, void *val, int len) {
	int i = 0, j = 0;
	char *p = (char*) val;

	for (i = 0; i < len; i++) {
		if ((p[i] >= 32) && (p[i] <= 127)) {
			out[i] = p[i];
		} else {
			out[i] = '?';
		}
	}

	out[i++] = ' ';
	while (i < 10) {
		out[i++] = ' ';
	}

	for (j = 0; j < len; j++) {

		char c = (char) ((p[j] >> 4) & 0x0f);
		out[i++] = c > 9 ? c - 10 + 'A' : c + '0';
		c = (char) (p[j] & 0x0f);
		out[i++] = c > 9 ? c - 10 + 'A' : c + '0';

		if (((j + 1) % 2) == 0) {
			out[i++] = ' ';
		}
	}

	out[i] = '\0';

	return out;
}

