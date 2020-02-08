#pragma once
#include "thirdparty/dirent.h"
#include "./string.h"
#include "./pointerList.h"

namespace guitard {
  struct FileInfo {
    String name;
    String relative;
    String absolute;
    bool isFolder;
  };

  class ScanDir : public PointerList<FileInfo> {
  public:
    ScanDir(const char* dir) {
      struct dirent** files;
      const int count = scandir(dir, &files, nullptr, alphasort);
      if (count >= 0) {
        for (int i = 0; i < count; i++) {
          const struct dirent* ent = files[i];
          if (ent->d_name[0] != '.') {
            FileInfo* info = new FileInfo;

           // add(new String(ent->d_name));
          }
          free(files[i]);
        }
        free(files);
      }
    }

    ~ScanDir() {
      clear(true);
    }
  };
}