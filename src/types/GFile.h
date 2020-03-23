#pragma once

#include "./GTypes.h"

namespace guitard {
  namespace File {
    const String PATH_DELIMITER =
#ifdef _WIN32
      "\\";
#else
      "/";
#endif

    String getFileExtension(String string) {
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

    String getFilePart(String string) {
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
