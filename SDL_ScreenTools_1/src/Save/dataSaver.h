#pragma once

#include "../Log/LogSystem.h"
#include "FileManager.h"

#include <filesystem>
#include <json/json.h>

namespace fs = std::filesystem;

// ========== JSON 文件操作（只保留 fs::path 版本） ==========
inline bool saveJsonFile(const fs::path& filename, const Json::Value& root) {
    if (!FILE_MGR.EnsureDirectoryExists(filename.parent_path())) {
        LOG_ERROR_STREAM << u8"无法创建目录: " << filename.parent_path().u8string();
        return false;
    }

    auto file = FILE_MGR.OpenForWrite(filename);
    if (!file) {
        LOG_ERROR_STREAM << u8"无法打开文件进行写入: " << filename.u8string();
        return false;
    }

    Json::StreamWriterBuilder writerBuilder;
    writerBuilder["indentation"] = "  ";
    writerBuilder["commentStyle"] = "None";
    writerBuilder["emitUTF8"] = true;

    try {
        std::unique_ptr<Json::StreamWriter> writer(writerBuilder.newStreamWriter());
        writer->write(root, file.get());
        file->close();
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR_STREAM << u8"保存JSON文件时发生异常: " << e.what();
        file->close();
        return false;
    }
}

inline bool readJsonFile(const fs::path& filename, Json::Value& root) {
    auto file = FILE_MGR.OpenForRead(filename);

    if (!file) {
        LOG_WARNING_STREAM << u8"配置文件不存在，将创建默认配置: " << filename.u8string();
        FILE_MGR.EnsureDirectoryExists(filename.parent_path());

        Json::Value defaultRoot;
        if (saveJsonFile(filename, defaultRoot)) {
            root = defaultRoot;
            return true;
        }
        return false;
    }

    Json::CharReaderBuilder readerBuilder;
    std::string parseErrors;

    file->seekg(0, std::ios::end);
    std::streampos fileSize = file->tellg();
    file->seekg(0, std::ios::beg);

    std::string fileContent;
    fileContent.resize(static_cast<size_t>(fileSize));
    file->read(fileContent.data(), fileContent.size());
    file->close();

    std::istringstream contentStream(fileContent);
    bool success = Json::parseFromStream(readerBuilder, contentStream, &root, &parseErrors);

    if (!success) {
        LOG_ERROR_STREAM << u8"JSON解析失败: " << parseErrors << u8" 文件: " << filename.u8string();
        return false;
    }

    return true;
}

class App;

void SaveConfig(App* app);
void LoadConfig(App* app);