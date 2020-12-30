#pragma once 

#include <map>
#include <variant>
#include <net/enumStrings.h>
#include <net/simdjson.h>
#include <net/singleton.h>


//ENUM_WITH_STRING_CONVERSION(MyEnums,
//	(version)
//	...
//)

namespace mln {

	//class ConfigurationFieldPrint
	//	: public std::static_visitor<>
	//{
	//public:
	//	template <typename T>
	//	T operator()(const T& t) const {
	//		std::cout << t << std::endl;
	//	}
	//};


	template< typename ENUMS >
	class Config
		: public mln::Singleton< Config<ENUMS> >
	{
	public:
		using EntType = std::variant<std::string, int64_t, double, bool >;
		using ContType = std::map<ENUMS, EntType>;

		bool LoadScript(const std::string& path) {
			using namespace simdjson::dom;

			parser parser;
			try {
				element conf = parser.load(path);

				//conf.dump_raw_tape(std::cout);
				int64_t version;
				conf["version"].get(version);
				fmt::print("version: {}\n", version);

				m_fileds[ENUMS::version] = conf["version"].get<int64_t>();

				for (object section : conf["sections"]) {
					std::string_view sv;
					section["name"].get(sv);
					fmt::print("section name : {}\n", sv);

					object fields = section["fields"];
					std::string stringValue;

					for (auto field : fields) {
						auto& fieldValue = m_fileds[ConfigTags_fromString(field.key.data())];

						auto prt = [&](auto v) {
							fmt::print("key : {:35} / value : {}\n", field.key.data(), v);
							fieldValue = v;
						};

						switch (field.value.type()) {
						case element_type::ARRAY:
							break;
						case element_type::OBJECT:
							break;
						case element_type::INT64:
							prt(int64_t(field.value));
							break;
						case element_type::UINT64:
							prt(int64_t(field.value));
							break;
						case element_type::DOUBLE:
							prt(double(field.value));
							break;
						case element_type::STRING:
							stringValue = std::string_view(field.value).data();
							prt(stringValue);
							break;
						case element_type::BOOL:
							prt(bool(field.value));
							break;
						case element_type::NULL_VALUE:
							fmt::print("  !!! null value. key:{}\n", field.key.data());
							break;
						}
					}
				}
			}
			catch (std::exception e) {
				fmt::print("Failed loading config. file:{} msg:{}\n", path, e.what());
			}

			//PrintFields(true);

			return true;
		}
		std::string PrintFields(const bool consolePrint) const {
			std::string msg = "<Configurations>";

			for (const auto& ent : m_fileds) {
				//boost::apply_visitor(ConfigurationFieldPrint(), ent.second);

				msg += "\r\n";
				msg += ConfigTags_toString(ent.first) + " : ";

				switch (ent.second.index()) {
				case 0://std::string
					msg += std::get<std::string>(ent.second);
					break;
				case 1://int
					msg += std::to_string(std::get<int64_t>(ent.second));
					break;
				case 2://double
					msg += std::to_string(std::get<double>(ent.second));
					break;
				case 3://bool
					msg += std::to_string(std::get<bool>(ent.second));
					break;

				default:
					msg += "??? not defined datatype";
				}
			}
			if (true == consolePrint) {
				fmt::print("{}\n", msg);
			}
			return msg;
		}

		bool GetValueBool(const ENUMS tag, const bool defaultValue = false) const { return GetConfValue(tag, defaultValue); }
		int64_t GetValueInt(const ENUMS tag) const { return GetConfValue(tag, (int64_t)0); }
		int64_t GetValueInt(const ENUMS tag, const int64_t defaultValue) const { return GetConfValue(tag, defaultValue); }
		double GetValueDouble(const ENUMS tag, const double defaultValue = 0.0) const { return GetConfValue(tag, defaultValue); }
		std::string GetValueString(const ENUMS tag) const { return GetConfValue(tag, std::string("")); }
		std::string GetValueString(const ENUMS tag, const std::string& defaultValue) const { return GetConfValue(tag, defaultValue); }

		template< typename T >
		void SetConfValue(const ENUMS tag, const T& defaultValue) {
			m_fileds[tag] = defaultValue;
		}

	private:
		template< typename T >
		T GetConfValue(const ENUMS tag, const T& defaultValue) const {
			try {
				if (m_fileds.end() != m_fileds.find(tag)) {
					return std::get<T>(m_fileds[tag]);
				}
				else {
					return defaultValue;
				}
			}
			catch (std::bad_variant_access& e) {
				/*std::cerr << e.what() << "  => field: " << ConfigTags_toString(tag) << std::endl;*/
				return defaultValue;
			}
		}

	private:
		mutable ContType m_fileds;
	};

	namespace APP_ENV {
		const static std::string LOCAL = "LOCAL";
		const static std::string DEV = "DEV";
		const static std::string STAGE = "STAGE";
		const static std::string PROD = "PROD";
	};


}//namespace mln{