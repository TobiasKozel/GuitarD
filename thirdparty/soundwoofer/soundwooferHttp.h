#pragma once

#ifndef SOUNDWOOFER_CUSTOM_HTTP
  // #define CPPHTTPLIB_OPENSSL_SUPPORT
  #include "./dependencies/httplib.h"
#endif

#include "./soundwooferTypes.h"

namespace soundwoofer {
  namespace http {
    const std::string BACKEND_URL = "svenssj.tech";
    // const std::string BACKEND_URL = "localhost";
    const std::string API_VERSION = "0.1";
    const int BACKEND_PORT = 5000;

#ifndef SOUNDWOOFER_CUSTOM_HTTP
    namespace _ {
      httplib::Client cli(BACKEND_URL, BACKEND_PORT);
      httplib::Headers headers = {
        { "User-Agent", "soundwoofer c++ lib" },
        { "upgrade-insecure-requests", "1" },
        { "accept", "text/plain" }
      };
      bool isSetup = false;
      void setup () {
        if (isSetup) { return; }
        _::cli.set_timeout_sec(10);
        _::cli.set_read_timeout(10, 0);
        //_::cli.set_logger([](const auto& req, const auto& res) {
        //  int i = 0;
        //});
        cli.set_keep_alive_max_count(10);
        isSetup = true;
      }
    }
#endif

#ifndef SOUNDWOOFER_CUSTOM_HTTP
    /**
     * Will do a synchronous http request to the url + port + the endpoint parameter
     * Returns a string of the response body or an empty string for any error
     */
    std::string get(const std::string endpoint) {
      _::setup();

      httplib::Client cli(BACKEND_URL, BACKEND_PORT);
      auto res = _::cli.Get(endpoint.c_str(), _::headers);
      if (res && res->status == 200) {
        return res->body;
      }
      //std::vector<httplib::Request> requests;
      //httplib::Get(requests, endpoint.c_str(), _::headers);
      //std::vector<httplib::Response> responses;
      //if (cli.send(requests, responses)) {
      //  for (const auto& res : responses) {
      //    if (res.status == 200) {
      //      return res.body;
      //    }
      //  }
      //}
      return "";
    }

    Status post(const std::string endpoint, const char* data, const size_t length, const std::string mime = "application/json") {
      _::setup();
      std::string body;
      body.append(data, length);
      // body += "\0"; // make sure it's null terminated
      auto res = _::cli.Post(endpoint.c_str(), _::headers, body, mime.c_str());
      if (res && res->status == 200) {
        return SUCCESS;
      }
      return SERVER_ERROR;
    }
#endif
  }
}