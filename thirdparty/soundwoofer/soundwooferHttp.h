#pragma once

#include "./soundwooferTypes.h"
#ifndef SOUNDWOOFER_CUSTOM_HTTP
  // #define CPPHTTPLIB_OPENSSL_SUPPORT
  #include "./dependencies/httplib.h"
#endif

namespace soundwoofer {
  namespace http {
    bool offline = false;
    const std::string BACKEND_URL = "api.tobias-kozel.de";
    const std::string API_VERSION = "0.1";
    const int BACKEND_PORT = 80;

#ifndef SOUNDWOOFER_CUSTOM_HTTP
    namespace _ {
      httplib::Client cli(BACKEND_URL, BACKEND_PORT);
      httplib::Headers headers = {
        { "User-Agent", "soundwoofer c++ lib" },
        { "upgrade-insecure-requests", "1" },
        { "accept", "text/plain" }
      };
      bool isSetup = false;
      void setup();
    }
#endif

#ifndef SOUNDWOOFER_CUSTOM_HTTP
    /**
     * Will do a synchronous http request to the url + port + the endpoint parameter
     * Returns a string of the response body or an empty string for any error
     */
    std::string get(const std::string endpoint);

    Status post(const std::string endpoint, const char* data, const size_t length, const std::string mime = "application/json");
#endif
  }
}

#ifdef SOUNDWOOFER_IMPL
  #include "./soundwooferHttpImpl.h"
#endif