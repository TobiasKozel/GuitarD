#pragma once

#include "./soundwooferHttp.h"

#ifndef SOUNDWOOFER_CUSTOM_HTTP
  // #define CPPHTTPLIB_OPENSSL_SUPPORT
  #include "./dependencies/httplib.h"
#endif

namespace soundwoofer {
  namespace http {

#ifndef SOUNDWOOFER_CUSTOM_HTTP
    namespace _ {
      void setup () {
        if (isSetup) { return; }
        _::cli.set_timeout_sec(3);
        _::cli.set_read_timeout(3, 0);
        //_::cli.set_logger([](const auto& req, const auto& res) {
        //  int i = 0;
        //});
        cli.set_keep_alive_max_count(3);
        isSetup = true;
      }
    }
#endif

#ifndef SOUNDWOOFER_CUSTOM_HTTP
    std::string get(const std::string endpoint) {
      _::setup();
      auto res = _::cli.Get(endpoint.c_str(), _::headers);
      if (res && res->status == 200) {
        offline = false;
        return res->body;
      }
      offline = true;
      return "";
    }

    Status post(const std::string endpoint, const char* data, const size_t length, const std::string mime) {
      _::setup();
      std::string body;
      body.append(data, length);
      // body += "\0"; // make sure it's null terminated
      auto res = _::cli.Post(endpoint.c_str(), _::headers, body, mime.c_str());
      if (res && res->status == 200) {
        offline = false;
        return SUCCESS;
      }
      offline = true;
      return SERVER_ERROR;
    }
#endif
  }
}