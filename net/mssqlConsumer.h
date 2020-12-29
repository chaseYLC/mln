#pragma once

#include <Sql.h>
#include <Sqlext.h>
#include <Sqltypes.h>
#include <Odbcss.h>
#include <stdint.h>
#include <string>

namespace mln {
	class DBConsumer
	{
		friend class DBConnection;
	public:
		typedef std::function<void(const std::string&)> TyShutdownFunc;

		static uint32_t s_nSqlTick;
		SQLHSTMT m_hStmt;
		SQLLEN m_nInd;

		SQLLEN			m_cbInt;
		SQLLEN			m_cbChar;
		SQLLEN			m_cbTime;
		SQLLEN			m_cbBin;
		SQLLEN			m_cbByte;
		SQLLEN			m_cbBigint;

		char m_szQuery[1024];

		SQLSMALLINT m_resultColumns;
		int m_dbRetCode = 0;
		char m_dbRetMsg[256];

		DBConsumer();
		~DBConsumer();

		static bool	Open(const char* szServer, const char* uid, const char* upw, DBConsumer::TyShutdownFunc cb);
		static void Close();

		bool Execute(const char* format, ...);

		SQLRETURN ExecuteInsertUnique(const char* format, ...);
		bool ExecuteNoData(const char* format, ...);
		void BindCol(SQLSMALLINT nType, SQLPOINTER pValue, SQLINTEGER nLength, SQLLEN* pnInd)
		{
			SQLBindCol(m_hStmt, ++m_nBind, nType, pValue, nLength, pnInd);
		}
		void Skip()
		{
			++m_nBind;
		}
		void Skip(int n)
		{
			m_nBind += n;
		}
		void BindParam(SQLUSMALLINT ipar, SQLSMALLINT fParamType, SQLSMALLINT fCType,
			SQLSMALLINT fSqlType, SQLUINTEGER cbColDef, SQLSMALLINT ibScale, SQLPOINTER rgbValue,
			SQLINTEGER cbValueMax, SQLLEN* pcbValue)
		{
			SQLBindParameter(m_hStmt, ipar, fParamType, fCType, fSqlType, cbColDef, ibScale, rgbValue, cbValueMax, pcbValue);
		}

		void Bind(char* n)
		{
			*n = 0;
			SQLBindCol(m_hStmt, ++m_nBind, SQL_C_TINYINT, n, 0, &m_cbByte);
		}

		void Bind(unsigned char* n)
		{
			*n = 0;
			SQLBindCol(m_hStmt, ++m_nBind, SQL_C_TINYINT, n, 0, &m_cbByte);
		}

		void Bind(int* n)
		{
			*n = 0;
			SQLBindCol(m_hStmt, ++m_nBind, SQL_C_LONG, n, 0, &m_cbInt);
		}

		void Bind(float* n)
		{
			*n = 0;
			SQLBindCol(m_hStmt, ++m_nBind, SQL_C_LONG, n, 0, &m_nInd);
		}
		void Bind(int64_t* n)
		{
			*n = 0;
			SQLBindCol(m_hStmt, ++m_nBind, SQL_C_SBIGINT, n, 0, &m_cbBigint);
		}
		void Bind(uint64_t* n)
		{
			*n = 0;
			SQLBindCol(m_hStmt, ++m_nBind, SQL_C_UBIGINT, n, 0, &m_cbBigint);
		}
		void Bind(char* str, int size)
		{
			str[0] = 0;
			SQLBindCol(m_hStmt, ++m_nBind, SQL_C_CHAR, str, size, &m_cbChar);
		}

		void Bind(std::string& str, const int size)
		{
			str = "";
			str.resize(size);
			SQLBindCol(m_hStmt, ++m_nBind, SQL_C_CHAR, (void*)str.c_str(), size, &m_cbChar);
		}

		void Bind(TIMESTAMP_STRUCT* str)
		{
			SQLBindCol(m_hStmt, ++m_nBind, SQL_C_TIMESTAMP, str, sizeof(TIMESTAMP_STRUCT), &m_cbTime);
		}

		bool Fetch();
		bool MoreResults();

		void Reset();
		void UnBind();

		bool FetchResultCode();

	protected:
		DBConnection* m_pDBConnect;
		int	m_nBind;
	};

	class DBConnection
	{
		friend class DBConsumer;
	public:
		SQLHSTMT m_hStmt;
		SQLHDBC m_hDBC;

		DBConnection();
		~DBConnection();
	};
}//namespace mln