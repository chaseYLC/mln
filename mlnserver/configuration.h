#pragma once 

#include <map>
#include <variant>
#include <net/enumStrings.h>
#include <net/singleton.h>

ENUM_WITH_STRING_CONVERSION(ConfigTags,
	(ENV)
	(version)
	(SERVER_NAME)
	(SERVER_PORT)
	(KEEP_ALIVE)
	(UPDATE_TIME_MS)
	(UPDATE_TIME_FOR_ROOM_MS)
	(UPDATE_TIME_FOR_MONITOR_MS)
	(LOG_FLUSH_SEC)
	(LOG_FILE_SIZE_MB)
	(LOG_FILE_KEEP_MAX_CNT)
	(HTTP_REQUEST_TIMEOUT)
	(RESTSVC_BIND_PORT)
	(NET_IO_WORKER_CNT)

	(TELNET_TEST_CONSOLE_PORT)
)


class Configuration
	: public mln::Singleton< Configuration >
{
public:
	using EntType = std::variant<std::string, int64_t, double, bool >;
	using ContType = std::map<ConfigTags, EntType>;

	bool LoadScript(const std::string& path);
	std::string PrintFields(const bool consolePrint) const;

	bool GetValueBool(const ConfigTags tag, const bool defaultValue = false) const { return GetConfValue(tag, defaultValue); }
	int64_t GetValueInt(const ConfigTags tag) const { return GetConfValue(tag, (int64_t)0); }
	int64_t GetValueInt(const ConfigTags tag, const int64_t defaultValue) const { return GetConfValue(tag, defaultValue); }
	double GetValueDouble(const ConfigTags tag, const double defaultValue = 0.0) const { return GetConfValue(tag, defaultValue); }
	std::string GetValueString(const ConfigTags tag) const { return GetConfValue(tag, std::string("")); }
	std::string GetValueString(const ConfigTags tag, const std::string &defaultValue) const { return GetConfValue(tag, defaultValue); }

	template< typename T >
	void SetConfValue(const ConfigTags tag, const T& defaultValue) {
		m_fileds[tag] = defaultValue;
	}

private:
	template< typename T >
	T GetConfValue(const ConfigTags tag, const T& defaultValue) const {
		try {
			if (m_fileds.end() != m_fileds.find(tag)) {
				return std::get<T>(m_fileds[tag]);
			}
			else {
				return defaultValue;
			}
		}
		catch (std::bad_variant_access &e) {
			/*std::cerr << e.what() << "  => field: " << ConfigTags_toString(tag) << std::endl;*/
			return defaultValue;
		}
	}

private:
	mutable ContType m_fileds;
};

#define ConfGetInt(tag)						Configuration::instance()->GetValueInt(tag)
#define ConfGetIntD(tag, defaultValue)		Configuration::instance()->GetValueInt(tag, defaultValue)
#define ConfGetString(tag)					Configuration::instance()->GetValueString(tag)
#define ConfGetStringD(tag, defaultValue)	Configuration::instance()->GetValueString(tag, defaultValue)

namespace APP_ENV {
	const static std::string LOCAL = "LOCAL";
	const static std::string DEV = "DEV";
	const static std::string STAGE = "STAGE";
	const static std::string PROD = "PROD";
};
