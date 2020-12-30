#include "../stdafx.h"
#include "restServer.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
// ref : https://www.boost.org/doc/libs/1_72_0/libs/beast/example/http/server/sync/http_server_sync.cpp
//////////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <net/logManager.h>
#include <def/errorCode.h>

#include "configuration.h"
#include "globalObjects.h"


// for contents...
#include "restHandler.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

using namespace mlnserver;

using RestServiceHandlerType = std::function < std::string(/*Document*/) > ;
static std::map < std::string, RestServiceHandlerType > s_handlers;

static void RegistURL()
{
    auto regist = [](const std::string & url, RestServiceHandlerType handler) {
        if (url.size() > 0) {
            if (true == s_handlers.emplace(url, handler).second) {
                LOGI("registed rest-api. url:{}", url);
            }
            else {
                LOGE("registration failed. rest-api. url:{}", url);
            }
        }
    };

    regist("/myrequest", RestHandler::handler_myrequest);
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string
path_cat(
    beast::string_view base,
    beast::string_view path)
{
    if (base.empty())
        return std::string(path);
    std::string result(base);
#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
    if (result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for (auto& c : result)
        if (c == '/')
            c = path_separator;
#else
    char constexpr path_separator = '/';
    if (result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
    return result;
}

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template<
    class Body, class Allocator,
    class Send>
    void
    handle_request(
        beast::string_view doc_root,
        http::request<Body, http::basic_fields<Allocator>>&& req,
        Send&& send)
{
    std::string path = path_cat(doc_root, req.target());

    // Returns a bad request response
    auto const bad_request =
        [&req, &path](beast::string_view why)
    {
        http::response<http::string_body> res{ http::status::bad_request, req.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();

        LOGE("<restsvc:op> bad request url:{} requestString:{}", path, req.body());

        return res;
    };

    // Returns a not found response
    auto const not_found =
        [&req, &path](beast::string_view target)
    {
        http::response<http::string_body> res{ http::status::not_found, req.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "The resource '" + std::string(target) + "' was not found.";
        res.prepare_payload();

        LOGE("<restsvc:op> not_found url:{} requestString:{}", path, req.body());

        return res;
    };

    // Returns a server error response
    auto const server_error =
        [&req, &path](beast::string_view what)
    {
        http::response<http::string_body> res{ http::status::internal_server_error, req.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "An error occurred: '" + std::string(what) + "'";
        res.prepare_payload();

        LOGE("<restsvc:op> server_error url:{} requestString:{}", path, req.body());

        return res;
    };

    // Make sure we can handle the method
    //if (req.method() != http::verb::get) {
    //    return send(bad_request("Unknown HTTP-method"));
    //}

    // Request path must be absolute and not contain "..".
    if (req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos)
        return send(bad_request("Illegal request-target"));

    


    auto it = s_handlers.find(path);
    if (s_handlers.end() == it) {
        return send(not_found(req.target()));
    }

    std::string reqBodyString = req.body();
    if (true == reqBodyString.empty()) {
        reqBodyString = "{}";
    }

    /*Document doc;
    doc.Parse(reqBodyString.c_str());

    if (true == doc.HasParseError()) {
        LOGE("bad body. failed jsonParsing. Error:{}, Offset:{}, reqBodyString:{}, "
            , doc.GetParseError(), doc.GetErrorOffset(), reqBodyString);
        return send(bad_request("bad body. failed jsonParsing."));
    }*/

    try {
        /*std::string responseBodyString = it->second(std::move(doc));*/
        std::string responseBodyString = "parsed string";

        auto const size = responseBodyString.size();

        LOGT("<restsvc:op> request url:{} requestBody:{}, responseBody:{}", path, req.body(), responseBodyString);

        // Respond to GET request
        http::response<http::string_body> res{
            std::piecewise_construct,
            std::make_tuple(std::move(responseBodyString)),
            std::make_tuple(http::status::ok, req.version()) };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.content_length(size);
        res.keep_alive(req.keep_alive());

        return send(std::move(res));

    }
    catch (std::exception e) {
        return send(server_error(e.what()));
    }
}

//------------------------------------------------------------------------------

// Report a failure
void
fail(beast::error_code ec, char const* what)
{
    LOGE("failed RestServer(). error:{}, msg:{}", ec.message(), what);
}

// This is the C++11 equivalent of a generic lambda.
// The function object is used to send an HTTP message.
template<class Stream>
struct send_lambda
{
    Stream& stream_;
    bool& close_;
    beast::error_code& ec_;

    explicit
        send_lambda(
            Stream& stream,
            bool& close,
            beast::error_code& ec)
        : stream_(stream)
        , close_(close)
        , ec_(ec)
    {
    }

    template<bool isRequest, class Body, class Fields>
    void
        operator()(http::message<isRequest, Body, Fields>&& msg) const
    {
        // Determine if we should close the connection after
        close_ = msg.need_eof();

        // We need the serializer here because the serializer requires
        // a non-const file_body, and the message oriented version of
        // http::write only works with const messages.
        http::serializer<isRequest, Body, Fields> sr{ msg };
        http::write(stream_, sr, ec_);
    }
};

// Handles an HTTP server connection
void
do_session(
    tcp::socket &socket,
    std::shared_ptr<std::string const> const& doc_root)
{
    bool close = false;
    beast::error_code ec;

    // This buffer is required to persist across reads
    beast::flat_buffer buffer;

    // This lambda is used to send messages
    send_lambda<tcp::socket> lambda{ socket, close, ec };

    for (;;)
    {
        // Read a request
        http::request<http::string_body> req;
        http::read(socket, buffer, req, ec);
        if (ec == http::error::end_of_stream)
            break;
        if (ec)
            return fail(ec, "read");

        // Send the response
        handle_request(*doc_root, std::move(req), lambda);
        if (ec)
            return fail(ec, "write");
        if (close)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            break;
        }
    }

    // Send a TCP shutdown
    socket.shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
}

void RestServer::Start(const int port)
{
    if (0 >= port) {return;}

    std::thread{ std::bind(&RestServer::Accept, this, port) }.detach();
}

void RestServer::Accept(const int port)
{
    try {
        auto const address = net::ip::make_address("0.0.0.0");
        auto const doc_root = std::make_shared<std::string>("");

        auto& ioc = *GlobalObjects::instance()->shared_ioc().get();

        tcp::acceptor acceptor{ ioc, {address, (uint16_t)port} };

        RegistURL();

        while (true) {
            tcp::socket socket{ ioc };
            acceptor.accept(socket);

            std::thread{ std::bind(
                do_session,
                std::move(socket),
                doc_root) }.detach();
        }
    }
    catch (const std::exception & e)
    {
        LOGE("CreateRestServer(). Exception msg:{}", e.what());
    }
}