#pragma once

#include <string>
#include <vector>

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cassert>

#include "./soundwooferTypes.h"

namespace soundwoofer {
  /**
   * Bundles a few convenient file operations
   */
  namespace file {
    const std::string PATH_DELIMITER =
#ifdef _WIN32
      "\\";
#else
      "/";
#endif

    /**
     * Struct used to index directories
     */
    struct FileInfo {
      std::string name;
      std::string relative;
      std::string absolute;
      bool isFolder = false;
      std::vector<FileInfo> children;
    };

#ifndef SOUNDWOOFER_CUSTOM_DIR
    std::vector<FileInfo> scanDir(FileInfo& root, bool recursive = false);
#endif

    std::vector<FileInfo> scanDir(const std::string path, const bool recursive = false);

    /**
     * Creates a directory
     * Returns success if the folder was created or already existing
     */
    Status createFolder(const char* path);

    /**
     * Writes a binary file, won't append to an existing one
     */
    Status writeFile(const char* path, const char* data, const size_t length);

    Status deleteFile(const char* path);

    /**
     * Simple file hashing from
     * https://gist.github.com/nitrix/34196ff0c93fdfb01d51
     */
    std::string hashFile(const std::string path);

    bool isWaveName(std::string& name);

    bool isJSONName(std::string& name);

    std::string platformPath(std::string path);

    /**
     * Not really up to spec, but this should happen on the backend anyways
     */
    std::string generateUUID();

    bool isUUID(const std::string id);

    bool isRelative(const std::string path);
  }
}

#ifdef SOUNDWOOFER_IMPL
  #include "./soundwooferFileImpl.h"
#endif