#include "stdafx.h"
#include "logger.h"

#include <algorithm>
#include <boost/asio/ip/host_name.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/debug_output_backend.hpp>
#include <boost/log/support/date_time.hpp>

#include <boost/log/exceptions.hpp>
#include <boost/log/utility/exception_handler.hpp>


#include "xmlParsingSupport.h"

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
namespace trivial = logging::trivial;

#define MAX_LOG_BUFFER	4096
#define TMP_SIZE 512
#define BYTES_PER_LINE 16

const uint64_t MLNLOG_MAX_FILESIZE_ROTATE = (1024 * 1024 * 20);

namespace MLN
{

	namespace Log
	{
		struct Collector_AutoBackup : public boost::log::sinks::file::collector
		{
			Collector_AutoBackup(const std::string filename, const std::string path_root)
				: _filename(filename)
				, _path_root(path_root)
			{
			}

			virtual void store_file(boost::filesystem::path const& src_path)
			{
				moveFIle(src_path);
			}

			virtual uintmax_t scan_for_files(
				boost::log::sinks::file::scan_method method
				, boost::filesystem::path const& pattern = boost::filesystem::path()
				, unsigned int * counter = 0)
			{
				uintmax_t found_cnt = 0;

				const std::string target_path(_path_root);

				boost::filesystem::directory_iterator end_itr;
				for (boost::filesystem::directory_iterator i(target_path); i != end_itr; ++i)
				{
					if (!boost::filesystem::is_regular_file(i->status()))
						continue;

					if (!i->path().has_filename())
						continue;

					if (std::string::npos == i->path().generic_string().find(_filename))
						continue;

					moveFIle(i->path());
					++found_cnt;
				}

				return found_cnt;
			}

		private:
			void moveFIle(boost::filesystem::path const& src_path)
			{
				namespace pt = boost::posix_time;
				pt::ptime now = pt::second_clock::local_time();

				std::string path_backup_root = (boost::format("%s/%04d%02d%02d")
					% _path_root.c_str()
					% (now.date().year()) % (now.date().month()) % now.date().day()).str();

				try {
					std::string path_backup_full = path_backup_root + "/" + src_path.filename().generic_string();
					if (boost::filesystem::exists(path_backup_full))
					{
						int old_idx = 0;
						while (old_idx < 10000)
						{
							std::ostringstream ss_old;
							ss_old << path_backup_full << "_" << old_idx;

							if (boost::filesystem::exists(ss_old.str()))
							{
								++old_idx;
								continue;
							}

							boost::filesystem::rename(path_backup_full, ss_old.str());
							break;
						}

						if (boost::filesystem::exists(path_backup_full))
							boost::filesystem::remove(path_backup_full);
					}

					boost::filesystem::create_directories(boost::filesystem::path(path_backup_root));
					boost::filesystem::rename(src_path, boost::filesystem::path(path_backup_full));
				}
				catch (std::exception& e)
				{
					std::cerr << e.what() << std::endl;
				}
				catch (...)
				{
					std::cerr << "failed to store a log file : " << src_path << std::endl;
				}
			}

		private:
			std::string _path_root;
			std::string _filename;
		};

		std::ostream& operator << (std::ostream& strm, Logger::severity_level level)
		{
			static const char* strings[] =
			{
				"trace",
				"debug",
				" info",
				" warn",
				"error",
				"fatal"
			};

			if (static_cast<std::size_t>(level) < sizeof(strings) / sizeof(*strings))
				strm << strings[level];
			else
				strm << static_cast<int>(level);

			return strm;
		}
		BOOST_LOG_ATTRIBUTE_KEYWORD(MLN_LOG_SEVERITY, "Severity", MLN::Log::Logger::severity_level);




		struct my_log_exception_handler
		{
			void operator()(const logging::runtime_error &ex) const
			{
				std::cerr << "boost::log::runtime_error: " << ex.what() << '\n';
			}

			void operator()(const std::exception &ex) const
			{
				std::cerr << "std::exception: " << ex.what() << '\n';
			}
		};

		Logger::Logger()
		{
			boost::log::core::get()->set_exception_handler(logging::make_exception_handler<logging::runtime_error, std::exception>(my_log_exception_handler()));

			// global attributes
			logging::core::get()->add_global_attribute("TimeStamp", attrs::local_clock());
			logging::core::get()->add_global_attribute(boost::log::aux::default_attribute_names::process_id(), attrs::current_process_id());
			logging::core::get()->add_global_attribute(boost::log::aux::default_attribute_names::thread_id(), attrs::current_thread_id());

			boost::system::error_code	ec;
			std::string hostName = boost::asio::ip::host_name(ec);

			std::string log_filename;
			log_filename = "default_log";
			log_filename += "_";
			log_filename += hostName;

			//CLog::log_config cnf;
			//cnf.type_ = CLog::LOG_TYPE_CONSOL | CLog::LOG_TYPE_FILE;
			//cnf.fileName_ = log_filename;

			//global_default_log_ = new CLog(cnf);
		}

		Logger::~Logger()
		{
			logging::core::get()->flush();
			logging::core::get()->remove_all_sinks();

			RemoveAllSinks();
		}

		bool Logger::init(const std::string &path)
		{
			XML_PARSING_BEGIN(path);

			auto loggerPtList = XML_TREEOBJ_REF.get_child("config.logger");

			for (const auto& info_node : loggerPtList)
			{
				std::string name;
				boost::property_tree::ptree sub_pt;
				std::tie(name, sub_pt) = info_node;
				if (name != "set") {
					continue;
				}
				resetConfig(sub_pt);
			}

			XML_PARSING_END;

			return true;
		}

		bool Logger::resetConfig(boost::property_tree::ptree &pt)
		{
			_config._channel = pt.get("channel", "default");
			_config._useLogConsole = pt.get("useLogConsole", true);
			_config._useLogFile = pt.get("useLogFile", true);
			_config._useLogTrace = pt.get("useLogTrace", false);
			_config._useLogUDP = pt.get("useLogUDP", false);

			_config._dirName = pt.get("dir", "");
			_config._fileName = pt.get("file", "");

			unsigned int level = pt.get("level", 1);
			if (fatal >= level) {
				_config._level = Logger::severity_level(level);
			}

			_config._rotation_time = pt.get("rotate_minute", std::numeric_limits<long>::digits);
			_config._rotation_size = pt.get("rotate_size", 0);
			_config._rotation_size *= 1024;

			_config._rotation_size = std::min(MLNLOG_MAX_FILESIZE_ROTATE
				, _config._rotation_size);

			InitBoostLog();

			return true;
		}

		bool Logger::resetConfig(const Logger::LogConfig& conf)
		{
			_config._channel = conf._channel;

			_config._useLogConsole = conf._useLogConsole;
			_config._useLogFile = conf._useLogFile;
			_config._useLogTrace = conf._useLogTrace;
			_config._useLogUDP = conf._useLogUDP;

			_config._dirName = conf._dirName;
			_config._fileName = conf._fileName;
			_config._rotation_time = conf._rotation_time;
			_config._rotation_size = conf._rotation_size;

			_config._rotation_size = std::min(
				MLNLOG_MAX_FILESIZE_ROTATE
				, _config._rotation_size);

			InitBoostLog();

			return true;
		}

		bool Logger::InitBoostLog()
		{
			RemoveAllSinks();

			if (true == _config._useLogConsole)
			{
				boost::shared_ptr<sinks::text_ostream_backend> backend = boost::make_shared<sinks::text_ostream_backend>();
				backend->add_stream(boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));
				backend->auto_flush(true);

				typedef sinks::synchronous_sink<sinks::text_ostream_backend> sink_t;
				boost::shared_ptr<sink_t> sink(new sink_t(backend));

				sink->set_formatter(
					expr::format("%1% (%2%) [%3%] %4%")
					% expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%d %H:%M:%S.%f")
					% expr::attr<attrs::current_thread_id::value_type>("ThreadID")
					% MLN_LOG_SEVERITY
					% expr::message
					);

				sink->set_filter
					(
						expr::attr< Logger::severity_level >("Severity") >= _config._level
						&& expr::attr< std::string >("Channel") == _config._channel
						);

				logging::core::get()->add_sink(sink);
				_sinks.push_back(sink);
			}

			if (true == _config._useLogFile)
			{
				boost::system::error_code	ec;
				std::string hostName = boost::asio::ip::host_name(ec);

				std::ostringstream oss;

				oss << _config._dirName << "/" << _config._fileName;	// set_filter 를 쓰면 set_file_collector 로 log folder 지정이 안됨. 그래서 여기서 directory 경로도 넣어줌.
				if (!ec) {
					oss << "_" << hostName;
				}
				oss << "_%Y%m%d_%H%M%S_%N.log";

				std::string _filename(oss.str());

				boost::shared_ptr< sinks::text_file_backend > file_backend = boost::make_shared< sinks::text_file_backend >(
					keywords::file_name = _filename
					);

				if (0 < _config._rotation_size)
					file_backend->set_rotation_size(_config._rotation_size);

				if (_config._rotation_time != std::numeric_limits<long>::digits)
					file_backend->set_time_based_rotation(sinks::file::rotation_at_time_interval(boost::posix_time::minutes(_config._rotation_time)));

				file_backend->auto_flush(true);

				typedef sinks::synchronous_sink<sinks::text_file_backend> sink_t;
				boost::shared_ptr< sink_t > sink(new sink_t(file_backend));

				boost::shared_ptr<Collector_AutoBackup> my_col = boost::make_shared <Collector_AutoBackup>(_config._fileName, _config._dirName);
				sink->locked_backend()->set_file_collector(my_col);

				sink->set_formatter(
					expr::format("%1% (%2%) [%3%] %4%")
					% expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%d %H:%M:%S.%f")
					% expr::attr<attrs::current_thread_id::value_type>("ThreadID")
					% MLN_LOG_SEVERITY
					% expr::message
					);

				sink->set_filter
					(
						expr::attr< Logger::severity_level >("Severity") >= _config._level
						&& expr::attr< std::string >("Channel") == _config._channel
						);

				logging::core::get()->add_sink(sink);
				_sinks.push_back(sink);

				try
				{
					sink->locked_backend()->scan_for_files();
				}
				catch (...)
				{
				}
			}

			if (true == _config._useLogTrace)
			{
				typedef sinks::synchronous_sink< sinks::debug_output_backend > sink_t;
				boost::shared_ptr<sink_t> sink(new sink_t());

				sink->set_formatter(
					expr::format("%1% (%2%) [%3%] %4%\r\n")		// ::OutputDebugString 에서는 \r\n 추가로 필요함. 젠장.
					% expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%d %H:%M:%S.%f")
					% expr::attr<attrs::current_thread_id::value_type>("ThreadID")
					% MLN_LOG_SEVERITY
					% expr::message
					);

				sink->set_filter
					(
						expr::is_debugger_present()
						&& expr::attr< Logger::severity_level >("Severity") >= _config._level
						&& expr::attr< std::string >("Channel") == _config._channel
						);

				logging::core::get()->add_sink(sink);
				_sinks.push_back(sink);
			}

			return true;
		}

		void Logger::RemoveAllSinks() {

			for (auto sink : _sinks)
			{
				sink->flush();
				logging::core::get()->remove_sink(sink);
			}
			_sinks.clear();
		}

		//#ifdef WIN32
		//#define WRITE_LOG(_LEVEL)	\
		//	do\
		//	{\
		//	if (_config.level_ > _LEVEL) return; \
		//	char strLog[MAX_LOG_BUFFER];	\
		//	va_list	vaList;	\
		//	va_start(vaList, pFmt);	\
		//	_vsnprintf_s(strLog, MAX_LOG_BUFFER, _TRUNCATE, (const char *)pFmt, vaList);	\
		//	va_end(vaList);	\
		//	src::severity_channel_logger_mt< CLog::severity_level > logger(keywords::channel = _config._channel);	\
		//	BOOST_LOG_SEV(logger, _LEVEL) << strLog; \
		//	} while (0)
		//#else
		//
		//#define WRITE_LOG(_LEVEL)	\
		//	do\
		//	{\
		//	if (_config.level_ > _LEVEL) return;\
		//	char strLog[MAX_LOG_BUFFER];	\
		//	va_list	vaList;	\
		//	va_start(vaList, pFmt);	\
		//	vsnprintf(strLog, MAX_LOG_BUFFER, (const char *)pFmt, vaList);	\
		//	va_end(vaList);	\
		//	src::severity_channel_logger_mt< CLog::severity_level > logger(keywords::channel = _config._channel);	\
		//	BOOST_LOG_SEV(logger, _LEVEL) << strLog; \
		//	} while (0)
		//#endif

			/*void CLog::LogT(const char *pFmt, ...)
			{
				WRITE_LOG(CLog::trace);
			}

			void CLog::LogD(const char *pFmt, ...)
			{
				WRITE_LOG(CLog::debug);
			}

			void CLog::LogI(const char *pFmt, ...)
			{
				WRITE_LOG(CLog::info);
			}

			void CLog::LogW(const char *pFmt, ...)
			{
				WRITE_LOG(CLog::warning);
			}

			void CLog::LogE(const char *pFmt, ...)
			{
				WRITE_LOG(CLog::error);
			}

			void CLog::LogF(const char *pFmt, ...)
			{
				WRITE_LOG(CLog::fatal);
			}*/

			/*void CLog::Log(const CLog::severity_level level, const char *pFmt, ...)
			{
				WRITE_LOG(level);
			}*/

		void Logger::Log_simple(const severity_level level, const char *text)
		{
			if (_config._level > level) return;

			src::severity_channel_logger_mt< Logger::severity_level > logger(keywords::channel = _config._channel);
			BOOST_LOG_SEV(logger, level) << text;
		}

		//	void CLog::Log(const int err){
		//#ifdef WIN32
		//		char strString[MAX_LOG_BUFFER];
		//		FormatMessage(
		//			FORMAT_MESSAGE_FROM_SYSTEM |
		//			FORMAT_MESSAGE_IGNORE_INSERTS,
		//			0,
		//			err,
		//			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		//			(LPTSTR)strString,
		//			(sizeof(char)* MAX_LOG_BUFFER),
		//			0
		//			);
		//
		//		LogE( strString );
		//#else
		//		char* err_msg = strerror( err );
		//		if( err_msg )
		//			return;
		//
		//		LogE(err_msg);
		//#endif	
		//	}

		void Logger::Dump(void* dwOffset, size_t size, const char* dump_title) {
			std::ostringstream oss;

			oss << "\r\n";

			char dump_bar[TMP_SIZE] = { 0, };

			if (dump_title) {
				if (strlen(dump_title) >= (TMP_SIZE - 100))
					strcpy_s(dump_bar, TMP_SIZE - 1, " start-----\r\n");
				else {
					strcpy_s(dump_bar, TMP_SIZE - 1, dump_title);
					strcat_s(dump_bar, TMP_SIZE - 1, " start-----\r\n");
				}

				oss << dump_bar;
			}

			unsigned int dwLoc, dwILoc, dwX;
			unsigned char* pOut = (unsigned char*)dwOffset;
			for (dwLoc = 0; dwLoc < size; dwLoc += 16, pOut += BYTES_PER_LINE) {
				unsigned char* pLine = pOut;
				oss << boost::format("%08lX: ") % (__int64)pOut;
				for (dwX = 0, dwILoc = dwLoc; dwX < BYTES_PER_LINE; dwX++) {
					if (dwX == (BYTES_PER_LINE / 2))
						oss << " ";
					if (++dwILoc > size) {
						oss << "?? ";
					}
					else
						oss << boost::format("%02X ") % ((int)*(pLine++));
				}
				pLine = pOut;
				oss << " ";
				for (dwX = 0, dwILoc = dwLoc; dwX < BYTES_PER_LINE; dwX++) {
					if (++dwILoc > size)
						oss << " ";
					else {
						oss << boost::format("%c") % (unsigned char)((isprint(*pLine) ? *pLine : ('.')));
						pLine++;
					}
				}
				oss << "\r\n";
			}

			Log_simple(Logger::debug, oss.str().c_str());
		}

	};// namespace Logger
};//MLN
