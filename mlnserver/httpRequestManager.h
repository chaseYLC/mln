#pragma once 

#include <map>
#include <string>

class HttpRequestManager
{
public:
	using RequestParamsType = std::map<std::string, std::string>;
	enum class HttpVerb {
		get,
		post,
	};

public:
	static std::string RequestSyncPostQuery(std::string host, std::string port, std::string uri, std::string json);
	static std::string RequestSyncPostQueryJson(std::string host, std::string port, std::string uri, std::string json);

	static std::string RequestForm(const std::string& method, const std::string &url, const RequestParamsType& paramsBody);
	static std::string RequestJson(const std::string& method, const std::string &url, const std::string &jsonBody);

#ifdef MLN_USE_HTTPS_REQUEST
	static std::string RequestHttps(const std::string& host, const std::string& port, const std::string& target
		, const std::string &bodyString = std::string("")
		, const HttpVerb httpVerb = HttpVerb::post);
#endif
};