#pragma once
#include <string>


namespace guitard {
  namespace File {
    const std::string PATH_DELIMITER =
#ifdef _WIN32
      "\\";
#else
      "/";
#endif

    std::string getFileExtension(std::string string) {
      const char* s = string.c_str();
      const char* endp = s + string.size();
      const char* p = endp - 1;
      const char delimiter = PATH_DELIMITER[0];
      while (p >= s && delimiter != *p) {
        if (*p == '.') {
          return p;
        }
        p--;
      }
      return endp;
    }

    std::string getFilePart(std::string string) {
      const char* s = string.c_str();
      const char* p = s + string.size() - 1;
      const char delimiter = PATH_DELIMITER[0];
      while (p >= s && *p != delimiter) {
        p--;
      }
      return (p + 1);
    }

  }
}
