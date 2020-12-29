#pragma once

#include "singleton.h"

#include <memory>
#include <mutex>
#include <string>
#include <sstream>

#ifndef SPDLOG_FMT_EXTERNAL
#define SPDLOG_FMT_EXTERNAL
#endif

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif

#include <spdlog/spdlog.h>

namespace spdlog {
	class logger;
}

namespace mln
{
	class LogManager
		: public mln::SingletonLight<LogManager>
	{
	public:
		class Initializer;
		class GlobalConfig;
		class Config;
		class ConfigFile;
		
		class Switcher {
		public:
			Switcher(Initializer& init)
				: init_(init)
			{}

			GlobalConfig& global() { return init_.global(); }
			Config& console() { return init_.console(); }
			ConfigFile& file() { return init_.file(); }
			void done() { return init_.done(); }

			Initializer& init_;
		};

		class GlobalConfig
			: public Switcher
		{
		public:
			friend class Initializer;

			GlobalConfig(Initializer& init);

		public:
			GlobalConfig& loggerName(const std::string& loggerName) { loggerName_ = loggerName; return *this; }
			GlobalConfig& flushEverySec(const size_t flushEverySec) { flushEverySec_ = flushEverySec; return *this; }

		private:
			Initializer& init_;

			std::string loggerName_;
			
			size_t flushEverySec_;
		};

		class Config
			: public Switcher
		{
		public:
			friend class Initializer;

			Config(Initializer &init)
				: init_(init)
				, Switcher(init)
			{}

			Config& lv(const spdlog::level::level_enum lv);
			Config& pattern(const char* pattern);

		private:
			spdlog::level::level_enum lv_ = spdlog::level::trace;
			std::string pattern_;
			Initializer& init_;

			bool using_ = false;
		};

		class ConfigFile
			: public Switcher
		{
		public:
			friend class Initializer;

			ConfigFile(Initializer& init)
				: init_(init)
				, Switcher(init)
			{}

			ConfigFile& fileNameBase(const std::string& fileNameBase) { fileNameBase_ = fileNameBase; return *this; }
			ConfigFile& maxFileSize(const size_t maxFileSize) { maxFileSize_ = maxFileSize; return *this; }
			ConfigFile& maxFiles(const size_t maxFiles) { maxFiles_ = maxFiles; return *this; }
			ConfigFile& lv(const spdlog::level::level_enum lv);
			ConfigFile& pattern(const char* pattern);

		private:
			std::string fileNameBase_;
			size_t maxFileSize_;
			size_t maxFiles_;

			spdlog::level::level_enum lv_ = spdlog::level::trace;
			std::string pattern_;
			Initializer& init_;

			bool using_ = false;
		};

		class Initializer {
		public:
			Initializer(LogManager *logManager)
				: pLogManager_(logManager)
				, global_(*this)
				, console_(*this)
				, file_(*this)
			{}

		public:
			GlobalConfig& global() { return global_; }
			Config& console() {return console_;}
			ConfigFile& file() { return file_; }
			void done();

			GlobalConfig global_;
			Config console_;
			ConfigFile file_;
			
			LogManager* pLogManager_ = nullptr;
		};

		void InitRotate(const std::string& loggerName, const std::string& fileNameBase, const size_t maxFileSize, const size_t maxFiles, const int flushEverySec = 0);
		void Init(const std::string& loggerName, const std::string& fileNameBase, const int flushEverySec = 0);
		void Flush();
		Initializer Create();

	public:
		std::shared_ptr<spdlog::logger> m_logger;
	};
}//namespace mln


//#define _LTRACE(...)	((void)(0))

#define LOGT		mln::LogManager::instance()->m_logger->trace
#define LOGD		mln::LogManager::instance()->m_logger->debug
#define LOGI		mln::LogManager::instance()->m_logger->info
#define LOGW		mln::LogManager::instance()->m_logger->warn
#define LOGE		mln::LogManager::instance()->m_logger->error
#define LOGC		mln::LogManager::instance()->m_logger->critical

