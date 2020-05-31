#pragma once

#include "./soundwooferFile.h"

#ifndef SOUNDWOOFER_CUSTOM_DIR
  #ifdef _WIN32
    #include "./dependencies/dirent.h"
  #else
    #include "dirent.h"
    #include <sys/stat.h>
  #endif
#endif

namespace soundwoofer {
  /**
   * Bundles a few convenient file operations
   */
  namespace file {

#ifndef SOUNDWOOFER_CUSTOM_DIR
    std::vector<FileInfo> scanDir(FileInfo& root, bool recursive) {
      std::vector<FileInfo> ret;
      struct dirent** files = nullptr;
      const int count = scandir(root.absolute.c_str(), &files, nullptr, alphasort);
      if (count >= 0) {
        for (int i = 0; i < count; i++) {
          const struct dirent* ent = files[i];
          if (ent->d_name[0] != '.') {
            FileInfo info;
            info.isFolder = ent->d_type == DT_DIR;
            info.name = ent->d_name;
            info.relative = root.relative + info.name + (info.isFolder ? PATH_DELIMITER : "");
            info.absolute = root.absolute + info.name + (info.isFolder ? PATH_DELIMITER : "");
            root.children.push_back(info);
            if (recursive && info.isFolder) {
              scanDir(info, true);
            }
            ret.push_back(info);
          }
          free(files[i]);
        }
        free(files);
      }
      else {
        root.isFolder = false;
      }
      return ret;
    }
#endif

    std::vector<FileInfo> scanDir(const std::string path, const bool recursive) {
      FileInfo root;
      root.absolute = path;
      root.relative = "." + PATH_DELIMITER;
      root.name = "";
      return scanDir(root, recursive);
    }

    Status createFolder(const char* path) {
#ifdef _WIN32
      bool ok = CreateDirectory(path, nullptr);
      if (!ok) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
          return GENERIC_ERROR; // Probably permission, locked file etc
        }
      }
      return SUCCESS;
#else 
      mode_t process_mask = umask(0);
      int result_code = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
      umask(process_mask);
      return !result_code ? SUCCESS : GENERIC_ERROR;
#endif
    }

    Status writeFile(const char* path, const char* data, const size_t length) {
      try {
        auto file = std::fstream(path, std::ios::out | std::ios::binary);
        file.write(data, length);
        file.close();
        return SUCCESS;
      }
      catch (...) {
        return GENERIC_ERROR;
      }
    }

    Status deleteFile(const char* path) {
      return std::remove(path) == 0 ? SUCCESS : GENERIC_ERROR;
    }

    std::string hashFile(const std::string path) {
      std::ifstream fp(path);
      std::stringstream ss;

      // Unable to hash file, return an empty hash.
      if (!fp.is_open()) {
        return "";
      }

      // Hashing
      uint32_t magic = 5381;
      char c;
      while (fp.get(c)) {
        magic = ((magic << 5) + magic) + c; // magic * 33 + c
      }

      ss << std::hex << std::setw(8) << std::setfill('0') << magic;
      return ss.str();
    }

    bool isWaveName(std::string& name) {
      return name.length() - name.find_last_of(".WAV") == 4
        || name.length() - name.find_last_of(".wav") == 4;
    }

    bool isJSONName(std::string& name) {
      return name.length() - name.find_last_of(".JSON") == 5
        || name.length() - name.find_last_of(".json") == 5;
    }

    std::string platformPath(std::string path) {
      for (size_t i = 1; i < path.size(); i++) {
        if ((path[i - 1] == '/' || path[i - 1] == '\\') && (path[i] == '/' || path[i] == '\\')) {
          // need to get rid of double slashes
          assert(false);
        }
      }
      for (size_t i = 0; i < path.size(); i++) {
        if (path[i] == '/' || path[i] == '\\') {
          path[i] = PATH_DELIMITER[0];
        }
      }
      return path;
    }

    std::string generateUUID() {
      srand(time(nullptr));
      const int charLength = 10 + 26;
      const char chars[] = "abcdefghijklmnopqrstuvwxyz0123456789";
      const int UUIDLength = 36;
      char out[UUIDLength];
      for (int i = 0; i < UUIDLength; i++) {
        out[i] = chars[rand() % charLength];
      }
      out[9] = out[14] = out[19] = out[24] = '-';
      std::string ret;
      ret.append(out, UUIDLength);
      return ret;
    }

    bool isUUID(const std::string id) {
      if (id.size() != 36) { return false; }
      if ((id[9] & id[14] & id[19] & id[24]) != '-') { return false; }
      for (size_t i = 0; i < id.size(); i++) {
        const char c = id[i];
        if ((('a' <= c) && (c <= 'z')) || (('0' <= c) && (c <= '9')) || (c == '-')) {}
        else { return false; }
      }
      return true;
    }

    bool isRelative(const std::string path) {
      return path[0] == '.';
    }
  }
}