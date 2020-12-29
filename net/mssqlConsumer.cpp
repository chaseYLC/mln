#include "stdafx.h"
#include "mssqlConsumer.h"
#include "logManager.h"

namespace mln
{
	UINT DBConsumer::s_nSqlTick = 5000;

	static SQLHENV g_henv;
	static char s_szServer[64];
	static char g_szId[80];
	static char g_szPassword[80];

	static DBConsumer::TyShutdownFunc shutdownFunc;

	bool DBConsumer::Open(const char* szServer, const char* uid, const char* upw, DBConsumer::TyShutdownFunc cb)
	{
		HKEY hKey;
		SQLHDBC hDBC = 0;
		DWORD dwType;
		DWORD dwSize;

		strcpy(s_szServer, szServer);
		strcpy(g_szId, uid);
		strcpy(g_szPassword, upw);
		shutdownFunc = cb;

		SQLSetEnvAttr(NULL, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER)SQL_CP_ONE_PER_DRIVER, SQL_IS_INTEGER);

		SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &g_henv);
		if (retcode != SQL_SUCCESS && retcode == SQL_SUCCESS_WITH_INFO)
		{
			LOGE << "Can't Get SQL HENV";
			goto fail;
		}
		// Set the ODBC version environment attribute
		SQLSetEnvAttr(g_henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
		if (SQLAllocHandle(SQL_HANDLE_DBC, g_henv, &hDBC) == SQL_ERROR)
		{
			LOGE << "Can't Allocate SQL Connection Handle";
			goto fail;
		}

		while (true)
		{
			LOGI << "trying DB Connecting...";
			if (SQLConnect(hDBC, (SQLCHAR*)s_szServer, SQL_NTS, (SQLCHAR*)g_szId, SQL_NTS,
				(SQLCHAR*)g_szPassword, SQL_NTS) != SQL_ERROR)
			{
				LOGI << "Connected to DB.";
				SQLFreeHandle(SQL_HANDLE_DBC, hDBC);
				break;
			}
		}
		return true;

	fail:
		if (hDBC) {
			SQLFreeHandle(SQL_HANDLE_DBC, hDBC);
		}
		if (g_henv) {
			SQLFreeHandle(SQL_HANDLE_ENV, g_henv);
			g_henv = 0;
		}
		return false;
	}

	void DBConsumer::Close()
	{
		if (g_henv)
		{
			SQLFreeHandle(SQL_HANDLE_ENV, g_henv);
			g_henv = 0;
		}
	}

	DBConsumer::DBConsumer()
	{
		m_pDBConnect = new DBConnection();
		m_hStmt = m_pDBConnect->m_hStmt;
		m_nBind = 0;
		strcpy(m_szQuery, "");

		m_cbInt = 0;
		m_cbByte = SQL_NTS;
		m_cbTime = SQL_NTS;
		m_cbChar = SQL_NTS;
		m_cbBigint = SQL_NTS;
	}


	DBConsumer::~DBConsumer()
	{
		SQLFreeStmt(m_hStmt, SQL_UNBIND);
		SQLFreeStmt(m_hStmt, SQL_CLOSE);
		SQLFreeStmt(m_hStmt, SQL_RESET_PARAMS);
	}


	bool DBConsumer::Execute(const char* format, ...)
	{
		va_list va;
		va_start(va, format);
		char query[1024];
		int len = sizeof(query);
		query[len - 1] = 0;
		len = _vsnprintf(query, len - 1, format, va);
		if (len == -1)
		{
			va_end(va);

			LOGE << "Critical Error!!!";
#ifdef	_DEBUG
			DebugBreak();
#endif
			return false;
		}
		va_end(va);
		strcpy(m_szQuery, query);
		DWORD dwBefore = GetTickCount();
		SQLRETURN r = SQLExecDirect(m_hStmt, (SQLCHAR*)query, len);
		DWORD dwDiff = GetTickCount() - dwBefore;
		if (dwDiff > s_nSqlTick) {
			LOGW << "SQLExecDirect() delayed : " << std::to_string(dwDiff) << ", Query:" << query;
		}

		if (r != SQL_SUCCESS
			//&& r != SQL_SUCCESS_WITH_INFO
			)
		{
			SQLCHAR szState[SQL_SQLSTATE_SIZE + 1];
			SQLINTEGER nError;
			SQLCHAR szMessage[SQL_MAX_MESSAGE_LENGTH + 1];
			SQLSMALLINT nMessage;
			if (SQL_SUCCEEDED(SQLGetDiagRec(SQL_HANDLE_STMT, m_hStmt, 1, szState, &nError,
				szMessage, sizeof(szMessage), &nMessage)))
			{
				LOGE << "state: " << szState << "/ msg:" << szMessage << std::endl << "Query: " << query;

				if (strcmp((char*)szState, "08S01") == 0)
				{
					LOGE << "SQL Disconnected";
					shutdownFunc("SQL Disconnected");
					return false;
				}
			}
			else
			{
				LOGE << "retValue : " << std::to_string(r) << ", query: " << query;
			}
			return false;
		}

		r = SQLNumResultCols(m_hStmt, &m_resultColumns);
		if (false == SQL_SUCCEEDED(r)) {
			LOGE << "failed SQLNumResultCols()";
			return 0;
		}
		return true;



		SQLLEN  indicator, RowCount;
		char* buf = new char[255];
		do {
			// SQLNumResultCols() returns number of columns in result set.
			// If non zero use SQLFetch until SQL_NO_DATA returned
			r = SQLNumResultCols(m_hStmt, &m_resultColumns);
			//CHECK_ERROR(retcode, "SQLNumResultCols()",
			//	hstmt, SQL_HANDLE_STMT);
			printf("\nColumns : %i", m_resultColumns);
			if (m_resultColumns > 0) {
				printf("\nStart Fetch ...");
				while (SQLFetch(m_hStmt) != SQL_NO_DATA) {
					// Loop through the columns
					memset(buf, ' ', 255);
					printf("\n");
					for (int i = 1; i <= m_resultColumns; i++) {
						// retrieve column data as a string
						r = SQLGetData(m_hStmt, i, SQL_C_CHAR,
							buf, 255, &indicator);
						if (SQL_SUCCEEDED(r)) {
							// Handle null columns
							if (indicator == SQL_NULL_DATA) {
								strcpy(buf, "NULL");
							}
							//std::string resultStr = rtrim(buf, ' ');
							if (i == 1)
								printf("%-8s ", buf);
							else
								printf("%-12s ", buf);
						}
					}
				}
				printf("\nEnd Fetch ...");
			}
			else {
				// SQLRowCount returns number of rows affected by
				// INSERT, UPDATE, DELETE or (if the driver provides) the
				// number of rows returned by a SELECT
				r = SQLRowCount(m_hStmt, &RowCount);
				/*CHECK_ERROR(retcode, "SQLRowCount()", hstmt, SQL_HANDLE_STMT);*/
				printf("\nRow count is : %i\n", (int)RowCount);
			}
		} while (SQLMoreResults(m_hStmt) == SQL_SUCCESS);

		return true;
	}

	SQLRETURN DBConsumer::ExecuteInsertUnique(const char* format, ...)
	{
		va_list va;
		va_start(va, format);
		char query[1024];
		int len = sizeof(query);
		query[len - 1] = 0;
		len = _vsnprintf(query, len - 1, format, va);
		if (len == -1)
		{
			va_end(va);

#ifdef	_DEBUG
			DebugBreak();
#endif
			return SQL_ERROR;
		}
		va_end(va);
		strcpy(m_szQuery, query);
		DWORD dwBefore = GetTickCount();
		SQLRETURN r = SQLExecDirect(m_hStmt, (SQLCHAR*)query, len);
		DWORD dwDiff = GetTickCount() - dwBefore;
		if (dwDiff > s_nSqlTick) {
			LOGW << "SQLExecDirect() delayed : " << std::to_string(dwDiff) << " / query: " << query;
		}
		if (r == SQL_SUCCESS) {
			return SQL_SUCCESS;
		}
		SQLCHAR szState[SQL_SQLSTATE_SIZE + 1];
		SQLINTEGER nError;
		SQLCHAR szMessage[SQL_MAX_MESSAGE_LENGTH + 1];
		SQLSMALLINT nMessage;
		if (SQL_SUCCEEDED(SQLGetDiagRec(SQL_HANDLE_STMT, m_hStmt, 1, szState, &nError,
			szMessage, sizeof(szMessage), &nMessage)))
		{
			if (strcmp((char*)szState, "23000") == 0)
				return SQL_SUCCESS_WITH_INFO;
			else
			{
				LOGE << "state: " << szState << "/ msg:" << szMessage << std::endl << "Query: " << query;
				if (strcmp((char*)szState, "08S01") == 0) {
					LOGE << "SQL Disconnected";
					shutdownFunc("SQL Disconnected");
					return SQL_ERROR;
				}
			}
		}
		else
		{
			LOGE << "retValue : " << std::to_string(r) << ", query: " << query;
		}
		return SQL_ERROR;
	}

	bool	DBConsumer::ExecuteNoData(const char* format, ...)
	{
		va_list va;
		va_start(va, format);
		char query[1024];
		int len = sizeof(query);
		query[len - 1] = 0;
		len = _vsnprintf(query, len - 1, format, va);
		if (len == -1)
		{
			va_end(va);
#ifdef	_DEBUG
			DebugBreak();
#endif
			return false;
		}
		va_end(va);
		strcpy(m_szQuery, query);
		DWORD dwBefore = GetTickCount();
		SQLRETURN r = SQLExecDirect(m_hStmt, (SQLCHAR*)query, len);
		DWORD dwDiff = GetTickCount() - dwBefore;
		if (dwDiff > s_nSqlTick) {
			LOGW << "SQLExecDirect() delayed : " << std::to_string(dwDiff) << ", Query:" << query;
		}
		if (r == SQL_SUCCESS) {
			return true;
		}

		if (r == SQL_NO_DATA) {
			return false;
		}

		SQLCHAR szState[SQL_SQLSTATE_SIZE + 1];
		SQLINTEGER nError;
		SQLCHAR szMessage[SQL_MAX_MESSAGE_LENGTH + 1];
		SQLSMALLINT nMessage;
		if (SQL_SUCCEEDED(SQLGetDiagRec(SQL_HANDLE_STMT, m_hStmt, 1, szState, &nError,
			szMessage, sizeof(szMessage), &nMessage)))
		{
			LOGE << "state: " << szState << "/ msg:" << szMessage << std::endl << "Query: " << query;
			if (strcmp((char*)szState, "08S01") == 0)
			{
				LOGE << "SQL Disconnected";
				shutdownFunc("SQL Disconnected");
				return false;
			}
		}
		else
		{
			LOGE << "retValue : " << std::to_string(r) << ", query: " << query;
		}
		return false;
	}

	bool DBConsumer::Fetch()
	{
		SQLRETURN r = SQLFetch(m_hStmt);
		if (SQL_SUCCEEDED(r)) {
			return true;
		}

		if (r == SQL_NO_DATA) {
			return false;
		}

		SQLCHAR szState[SQL_SQLSTATE_SIZE + 1];
		SQLINTEGER nError;
		SQLCHAR szMessage[SQL_MAX_MESSAGE_LENGTH + 1];
		SQLSMALLINT nMessage;
		if (SQL_SUCCEEDED(SQLGetDiagRec(SQL_HANDLE_STMT, m_hStmt, 1, szState, &nError, szMessage,
			sizeof(szMessage), &nMessage)))
		{
			LOGE << "SQL Fetch Error. state: " << szState << "/ msg:" << szMessage << std::endl << "Query: " << m_szQuery;
		}
		return false;
	}

	void DBConsumer::Reset()
	{
		SQLFreeStmt(m_hStmt, SQL_UNBIND);
		SQLFreeStmt(m_hStmt, SQL_CLOSE);
		m_nBind = 0;
	}

	void DBConsumer::UnBind()
	{
		SQLFreeStmt(m_hStmt, SQL_UNBIND);
		m_nBind = 0;
	}

	bool DBConsumer::FetchResultCode()
	{
		Bind(&m_dbRetCode);
		Bind(m_dbRetMsg, sizeof(m_dbRetMsg));
		if (false == Fetch()) {
			return false;
		}
		return true;
	}

	bool DBConsumer::MoreResults()
	{
		UnBind();

		if (SQL_SUCCESS != SQLMoreResults(m_hStmt)) {
			return false;
		}
		SQLRETURN r = SQLNumResultCols(m_hStmt, &m_resultColumns);
		if (false == SQL_SUCCEEDED(r)) {
			LOGE << "failed SQLNumResultCols()";
			return false;
		}

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	DBConnection::DBConnection()
	{
		m_hDBC = 0;
		m_hStmt = 0;
		if (SQLAllocHandle(SQL_HANDLE_DBC, g_henv, &m_hDBC) == SQL_ERROR)
		{
			LOGE << "Can't Allocate SQL Connection Handle";
			goto err;
		}
		if (SQLConnect(m_hDBC, (SQLCHAR*)s_szServer, SQL_NTS, (SQLCHAR*)g_szId, SQL_NTS,
			(SQLCHAR*)g_szPassword, SQL_NTS) == SQL_ERROR)
		{
			LOGE << "Can't connect to SQL";
			shutdownFunc("Can't connect to SQL");
			goto err;
		}
		if (SQLAllocHandle(SQL_HANDLE_STMT, m_hDBC, &m_hStmt) == SQL_ERROR)
		{
			LOGE << "Can't Allocate SQL Stmt Handle";
			goto err;
		}
		return;

	err:
		if (m_hDBC)
		{
			SQLFreeHandle(SQL_HANDLE_DBC, m_hDBC);
			m_hDBC = 0;
		}
	}

	DBConnection::~DBConnection()
	{
		if (m_hStmt)
			SQLFreeHandle(SQL_HANDLE_STMT, m_hStmt);
		if (m_hDBC)
			SQLFreeHandle(SQL_HANDLE_DBC, m_hDBC);
	}
}//namespace mln