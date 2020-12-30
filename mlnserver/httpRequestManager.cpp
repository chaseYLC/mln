#include "stdafx.h"
#include "httpRequestManager.h"

#include <iostream>
#include <boost/asio.hpp>
#include <boost/property_tree/json_parser.hpp>
#if defined(__GNUC__)
#include <curl/curl.h>
#endif

#include <net/http/httpRequest.h>
#include <net/logManager.h>
#include "configuration.h"

#ifdef MLN_USE_HTTPS_REQUEST
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#endif//#ifdef MLN_USE_HTTPS_REQUEST

std::string  HttpRequestManager::RequestSyncPostQuery(std::string host, std::string port, std::string uri, std::string json)
{
	std::string resp = "";

	try
	{
		boost::asio::io_context  io_context ;

		boost::asio::ip::tcp::resolver resolver(io_context );
		//tcp::resolver::query query(host, "http");		
		boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), host, port);
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		boost::asio::ip::tcp::resolver::iterator end;

		// Try each endpoint until we successfully establish a connection.
		boost::asio::ip::tcp::socket socket(io_context );
		boost::system::error_code error = boost::asio::error::host_not_found;
		while (error && endpoint_iterator != end) {
			socket.close();
			LOGT("<rest|request> connect to web. host:{}, port:{}", host, port);
			socket.connect(*endpoint_iterator++, error);
		}
		if (error) {
			LOGE("<rest|request> HttpRequestManager::RequestSyncPostQuery() host:{}, port:{} Error:{}", host, port, error.message());
			throw boost::system::system_error(error);
		}

		// Form the request. We specify the "Connection: close" header so that the
		// server will close the socket after transmitting the response. This will
		// allow us to treat all data up until the EOF as the content.
		boost::asio::streambuf request;
		std::ostream request_stream(&request);

		host = host + ":" + port;
		request_stream << "service: 0\r\n";
		request_stream << "x-service-info: mlnserver/1.0\r\n";
		request_stream << "x-os-info: ubuntu\r\n";
		request_stream << "x-device-info: tidc\r\n";
		request_stream << "x-lang-info: ko\r\n";
		request_stream << "POST " << uri.c_str() << " HTTP/1.1\r\n";
		request_stream << "Host: " << host.c_str() << "\r\n";
		request_stream << "User-Agent: C/1.0";
		request_stream << "Content-Type: application/json; charset=utf-8 \r\n";
		request_stream << "Accept: */*\r\n";
		request_stream << "Content-Length: " << json.length() << "\r\n";
		request_stream << "Connection: close\r\n\r\n";  //NOTE THE Double line feed
		request_stream << json;

		LOGT("<rest|request> HttpRequestManager::RequestSyncPostQuery(). request. host:{}, port:{}, uri:{}, json:{}", host, port, uri, json);

		// Send the request.
		boost::asio::write(socket, request);

		// Read the response status line.
		boost::asio::streambuf response;
		boost::asio::read_until(socket, response, "\r\n");

		// Check that response is OK.
		std::istream response_stream(&response);
		std::string http_version;
		response_stream >> http_version;
		unsigned int status_code;
		response_stream >> status_code;
		std::string status_message;
		std::getline(response_stream, status_message);
		if (!response_stream || http_version.substr(0, 5) != "HTTP/"){
			LOGT("<rest|response> HttpRequestManager::RequestSyncPostQuery(). invalid response. host:{}, port:{}, uri:{}, json:{}", host, port, uri, json);
			return "";
		}
		if (status_code != 200)
		{
			LOGT("<rest|response> HttpRequestManager::RequestSyncPostQuery(). status_code:{}. host:{}, port:{}, uri:{}, json:{}", status_code, host, port, uri, json);
			return "";
		}

		// Read the response headers, which are terminated by a blank line.
		boost::asio::read_until(socket, response, "\r\n\r\n");

		// Process the response headers.
		std::string header;
		while (std::getline(response_stream, header) && header != "\r")
			;
			//_LDEBUG("%s", header.c_str()); 
		//std::cout << "\n";

		// Write whatever content we already have to output.		
		if (response.size() > 0) {
			std::ostringstream ss;
			ss << &response;
			std::string s = ss.str();
			resp = std::string(s);

			LOGT("<rest|response> HttpRequestManager::RequestSyncPostQuery(). uri:{}, response:{}", uri, resp);
		}

		// Read until EOF, writing data to output as we go.
		while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error)){
			std::ostringstream ss;
			ss << &response;
			std::string s = ss.str();
			resp = std::string(s);

			LOGT("<rest|response> HttpRequestManager::RequestSyncPostQuery(). uri:{}, response:{}", uri, resp);
			//std::cout << "loop cnt --->" << cnt++ << std::endl;
		}

		if (error != boost::asio::error::eof) {
			return std::string("");
			//throw boost::system::system_error(error);
		}
	}
	catch (std::exception& e) {
		LOGE("<rest|response> HttpRequestManager::RequestSyncPostQuery(). exception:{}, uri:{}, response:{}", e.what(), uri, resp);
	}

	return resp;
}


std::string HttpRequestManager::RequestSyncPostQueryJson(std::string host, std::string port, std::string uri, std::string json)
{
	std::string resp = "";

	try
	{
		boost::asio::io_context  io_context ;

		// Get a list of endpoints corresponding to the server name.
		boost::asio::ip::tcp::resolver resolver(io_context );
		//tcp::resolver::query query(host, "http");		
		boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), host, port);
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		boost::asio::ip::tcp::resolver::iterator end;

		// Try each endpoint until we successfully establish a connection.
		boost::asio::ip::tcp::socket socket(io_context );
		boost::system::error_code error = boost::asio::error::host_not_found;
		while (error && endpoint_iterator != end)
		{
			LOGT("<rest|request> connect to web. host:{}, port:{}", host, port);
			socket.close();
			socket.connect(*endpoint_iterator++, error);
		}
		if (error)
		{
			LOGE("<rest|request> HttpRequestManager::RequestSyncPostQuery() host:{}, port:{} Error:{}", host, port, error.message());
			throw boost::system::system_error(error);
		}

		// Form the request. We specify the "Connection: close" header so that the
		// server will close the socket after transmitting the response. This will
		// allow us to treat all data up until the EOF as the content.
		boost::asio::streambuf request;
		std::ostream request_stream(&request);

		host = host + ":" + port;
		request_stream << "service: 0\r\n";
		request_stream << "x-service-info: mlnserver/1.0\r\n";
		request_stream << "x-os-info: ubuntu\r\n";
		request_stream << "x-device-info: tidc\r\n";
		request_stream << "x-lang-info: ko\r\n";
		request_stream << "POST " << uri.c_str() << " HTTP/1.1\r\n";
		request_stream << "Host: " << host.c_str() << "\r\n";
		request_stream << "User-Agent: C/1.0";
		request_stream << "Content-Type: application/json; charset=utf-8 \r\n";
		request_stream << "Accept: */*\r\n";
		request_stream << "Content-Length: " << json.length() << "\r\n";
		request_stream << "Connection: close\r\n\r\n";  //NOTE THE Double line feed
		request_stream << json;

		LOGT("<rest|request> HttpRequestManager::RequestSyncPostQuery(). request. host:{}, port:{}, uri:{}, json:{}", host, port, uri, json);

		// Send the request.
		boost::asio::write(socket, request);

		// Read the response status line.
		boost::asio::streambuf response;
		boost::asio::read_until(socket, response, "\r\n\r\n");

		// Check that response is OK.
		std::istream response_stream(&response);
		std::string http_version;
		response_stream >> http_version;
		unsigned int status_code;
		response_stream >> status_code;
		std::string status_message;
		std::getline(response_stream, status_message);
		if (!response_stream || http_version.substr(0, 5) != "HTTP/")
		{
			LOGT("<rest|response> HttpRequestManager::RequestSyncPostQuery(). invalid response. host:{}, port:{}, uri:{}, json:{}", host, port, uri, json);
			return "";
		}
		if (status_code != 200)
		{
			LOGT("<rest|response> HttpRequestManager::RequestSyncPostQuery(). status_code:{}. host:{}, port:{}, uri:{}, json:{}", status_code, host, port, uri, json);
			return "";
		}

		//read the headers.
		std::string header;
		while (std::getline(response_stream, header) && header != "\r") {
			std::cout << "H: " << header << std::endl;
		}

		std::ostringstream ostringstream_content;
		if (response.size() > 0) {
			ostringstream_content << &response;
		}

		while (true) {
			size_t n = boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error);
			if (!error) {
				if (n) {
					ostringstream_content << &response;
				}
			}

			if (error == boost::asio::error::eof) {
				break;
			}
			if (error) {
				//return -1; //throw boost::system::system_error(error);
				return std::string("");
			}
		}

		auto str_response = ostringstream_content.str();
		//std::cout << str_response << std::endl;

		return str_response;
	}
	catch (std::exception & e)
	{
		LOGE("<rest|response> HttpRequestManager::RequestSyncPostQuery(). exception:{}, uri:{}, response:{}", e.what(), uri, resp);
		return "";
	}
}

std::string HttpRequestManager::RequestForm(const std::string& method, const std::string &url, const RequestParamsType& paramsBody)
{
	try
	{
		// you can pass http::InternetProtocol::V6 to Request to make an IPv6 request
		http::Request request(url);
		
		// send a get request
		//const http::Response getResponse = request.send("GET");
		//std::cout << std::string(getResponse.body.begin(), getResponse.body.end()) << '\n'; // print the result

		// send a post request
		//const http::Response postResponse = request.send("POST", "foo=1&bar=baz", {
		//	"Content-Type: application/x-www-form-urlencoded"
		//	});
		//std::cout << std::string(postResponse.body.begin(), postResponse.body.end()) << '\n'; // print the result
		const http::Response response = request.send(method, paramsBody, {
			"Content-Type: application/x-www-form-urlencoded"
			});
		return std::string(response.body.begin(), response.body.end());
	}
	catch (const std::exception & e)
	{
		LOGE("HttpRequestManager::RequestJson(). exception:{}, url:{}, requestBody:{}", e.what(), url);
		return std::string(e.what());
	}
}

std::string HttpRequestManager::RequestJson(const std::string& method, const std::string& url, const std::string& jsonBody)
{
	LOGT("<rest|request> url:{}, body:{}", url, jsonBody);

	try
	{
		// you can pass http::InternetProtocol::V6 to Request to make an IPv6 request
		http::Request request(url);

		const http::Response response = request.send(method, jsonBody, {
			"Content-Type: application/json; charset=utf-8;",
			"service: 0",
			"x-service-info: mlxserver/1.0",
			"x-os-info: ubuntu",
			"x-lang-info: ko"
			}
			, CONF->GetValueInt(ConfigTags::HTTP_REQUEST_TIMEOUT, 5)
		);

		std::string responseString = std::string(response.body.begin(), response.body.end());

		LOGT("<rest|response> url:{} body:{}", url, responseString);
		return responseString;
	}
	catch (const std::exception & e)
	{
		LOGE("HttpRequestManager::RequestJson(). exception:{}, url:{}, requestBody:{}", e.what(), url, jsonBody);
		return std::string("");
	}
}



#ifdef MLN_USE_HTTPS_REQUEST
std::string HttpRequestManager::RequestHttps(const std::string& host, const std::string& port, const std::string& target
	, const std::string& bodyString /*= std::string("")*/
	, const HttpRequestManager::HttpVerb httpVerb /*= HttpVerb::post*/)
{
	namespace beast = boost::beast; // from <boost/beast.hpp>
	namespace http = beast::http;   // from <boost/beast/http.hpp>
	namespace net = boost::asio;    // from <boost/asio.hpp>
	namespace ssl = net::ssl;       // from <boost/asio/ssl.hpp>
	using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

	try
	{
		/*auto const host = "targethost.co.kr";
		auto const port = "8443";
		auto const target = "/targeturl/";*/
		const int version = 11;	// 1.0 ->10, 1.1 ->11

		// The io_context is required for all I/O
		net::io_context ioc;

		// The SSL context is required, and holds certificates
		ssl::context ctx(ssl::context::tlsv12_client);

		//// This holds the root certificate used for verification
		/*load_root_certificates(ctx);*/

		// Verify the remote server's certificate
		/*ctx.set_verify_mode(ssl::verify_peer);*/
		ctx.set_verify_mode(ssl::verify_none);

		// These objects perform our I/O
		tcp::resolver resolver(ioc);
		beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

		// Set SNI Hostname (many hosts need this to handshake successfully)
		if (!SSL_set_tlsext_host_name(stream.native_handle(), static_cast<void*>(const_cast<char*>( host.c_str()))  ))
		{
			beast::error_code ec{ static_cast<int>(::ERR_get_error()), net::error::get_ssl_category() };
			throw beast::system_error{ ec };
		}

		// Look up the domain name
		auto const results = resolver.resolve(host, port);

		// Make the connection on the IP address we get from a lookup
		beast::get_lowest_layer(stream).connect(results);

		// Perform the SSL handshake
		stream.handshake(ssl::stream_base::client);

		// Set up an HTTP GET request message
		/*http::request<http::string_body> req{ http::verb::get, target, version };*/
		http::request<http::string_body> req{ http::verb::post, target, version };

		req.set(http::field::host, host);
		req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

		// Send the HTTP request to the remote host
		http::write(stream, req);

		// This buffer is used for reading and must be persisted
		beast::flat_buffer buffer;

		// Declare a container to hold the response
		http::response<http::dynamic_body> res;

		// Receive the HTTP response
		http::read(stream, buffer, res);

		// Write the message to standard out
		std::cout << res << std::endl;

		// Gracefully close the stream
		beast::error_code ec;
		stream.shutdown(ec);
		if (ec == net::error::eof)
		{
			// Rationale:
			// http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
			ec = {};
		}
		if (ec)
			throw beast::system_error{ ec };

		// If we get here then the connection is closed gracefully
		return boost::beast::buffers_to_string(res.body().data());
	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return std::string("");
	}
	return EXIT_SUCCESS;

	return std::string("");
}
#endif//#ifdef MLN_USE_HTTPS_REQUEST
