#pragma once

#ifndef SOUNDWOOFER_CUSTOM_HTTP
  // #define CPPHTTPLIB_OPENSSL_SUPPORT
  #include "./dependencies/httplib.h"
#endif

#include "./soundwooferTypes.h"

namespace soundwoofer {
  namespace http {
    // const std::string BACKEND_URL = "svenssj.tech";
    const std::string API_VERSION = "0.1";
    const std::string BACKEND_URL = "localhost";
    const int BACKEND_PORT = 5000;

#ifndef SOUNDWOOFER_CUSTOM_HTTP
    /**
     * Will do a synchronous http request to the url + port + the endpoint parameter
     * Returns a string of the response body or an empty string for any error
     */
    std::string get(const std::string endpoint) {
      httplib::Client cli(BACKEND_URL, BACKEND_PORT);
      auto res = cli.Get(endpoint.c_str());
      if (res && res->status == 200) {
        return res->body;
      }
      assert(false); // You need to override this function if you want to use a different http lib!
    }

    Status post(const std::string endpoint, const char* data, const size_t length, const std::string mime = "application/json") {
      httplib::Client cli(BACKEND_URL, BACKEND_PORT);
      std::string body;
      body.append(data, length);
      // body += "\0"; // make sure it's null terminated
      auto res = cli.Post(endpoint.c_str(), body, mime.c_str());
      if (res && res->status == 200) {
        return SUCCESS;
      }
      return SERVER_ERROR;
    }
#endif
  }
}