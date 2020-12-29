#include "stdafx.h"
#include "logManager.h"

#include <array>
#include <vector>

#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>



namespace mln
{

static const char* logPatternDefault = "[%D %T] [%n] [%t] [%l] : %v";


LogManager::GlobalConfig::GlobalConfig(Initializer& init)
	: init_(init)
	, Switcher(init)
{
}

LogManager::Config& LogManager::Config::lv(const spdlog::level::level_enum lv) 
{
	using_ = true;

	lv_ = lv;
	return *this; 
}

LogManager::Config& LogManager::Config::pattern(const char* pattern)
{
	if (nullptr == pattern) {
		pattern_ = logPatternDefault;
	}
	else {
		pattern_ = pattern;
	}
	return *this;
}

LogManager::ConfigFile& LogManager::ConfigFile::lv(const spdlog::level::level_enum lv)
{
	using_ = true;

	lv_ = lv;
	return *this;
}

LogManager::ConfigFile& LogManager::ConfigFile::pattern(const char* pattern)
{
	if (nullptr == pattern) {
		pattern_ = logPatternDefault;
	}
	else {
		pattern_ = pattern;
	}
	return *this;
}

LogManager::Initializer LogManager::Create()
{
	return LogManager::Initializer(this);
}

void LogManager::Initializer::done()
{
	std::vector<spdlog::sink_ptr> sinks;

	if (console_.using_) {
		auto console_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
		console_sink->set_level(console_.lv_);
		console_sink->set_pattern(console_.pattern_);

		sinks.emplace_back(console_sink);
	}

	if (file_.using_) {
#ifdef _WIN32
		auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
			file_.fileNameBase_
			, file_.maxFileSize_
			, file_.maxFiles_
		);
#else
		auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
			file_.fileNameBase_
			, file_.maxFileSize_
			, file_.maxFiles_
			, false
		);
#endif
		file_sink->set_level(file_.lv_);
		file_sink->set_pattern(file_.pattern_);

		sinks.emplace_back(file_sink);
	}

	pLogManager_->m_logger = std::make_shared<spdlog::logger>(global_.loggerName_, sinks.begin(), sinks.end());
	pLogManager_->m_logger->set_level(spdlog::level::trace);

	if (0 < global_.flushEverySec_) {
		pLogManager_->m_logger->flush_on(spdlog::level::trace);
		spdlog::flush_every(std::chrono::seconds(global_.flushEverySec_));
	}
}



void LogManager::InitRotate(const std::string& loggerName, const std::string& fileNameBase
	, const size_t maxFileSize, const size_t maxFiles, const int flushEverySec /*= 0*/)
{
	auto console_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
	console_sink->set_level(spdlog::level::trace);
	console_sink->set_pattern(logPatternDefault);

	//auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(filename_base, daily_h, daily_m, true, max_days);
#ifdef _WIN32
	auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(fileNameBase, maxFileSize, maxFiles);
#else
	auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(fileNameBase, maxFileSize, maxFiles, false);
#endif
	file_sink->set_level(spdlog::level::trace);
	file_sink->set_pattern(logPatternDefault);

	spdlog::sinks_init_list sink_list = { file_sink, console_sink };

	m_logger = std::make_shared<spdlog::logger>(loggerName, sink_list.begin(), sink_list.end());

	m_logger->set_level(spdlog::level::trace);

	if (0 < flushEverySec) {
		m_logger->flush_on(spdlog::level::trace);
		spdlog::flush_every(std::chrono::seconds(flushEverySec));
	}
}

void LogManager::Init(const std::string& loggerName, const std::string& fileNameBase, const int flushEverySec/* = 0*/)
{
	auto console_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
	console_sink->set_level(spdlog::level::trace);
	console_sink->set_pattern(logPatternDefault);

	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(fileNameBase, true);
	file_sink->set_level(spdlog::level::trace);
	file_sink->set_pattern(logPatternDefault);

	spdlog::sinks_init_list sink_list = { file_sink, console_sink };

	m_logger = std::make_shared<spdlog::logger>(loggerName, sink_list.begin(), sink_list.end());

	m_logger->set_level(spdlog::level::trace);

	if (0 < flushEverySec) {
		m_logger->flush_on(spdlog::level::trace);
		spdlog::flush_every(std::chrono::seconds(flushEverySec));
	}
}

void LogManager::Flush()
{
	m_logger->flush();
}

}//namespace mln
