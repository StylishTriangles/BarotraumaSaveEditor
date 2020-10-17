#ifndef SAVEUTIL_H
#define SAVEUTIL_H

#include <QFile>
#include <QString>

#include <cinttypes>
#include <stdexcept>

class SaveUtil
{
private:
    static int32_t toInt32(const char* bytes, size_t offset = 0);
    static char16_t toChar16(const char* bytes, size_t offset = 0);
    static bool checkBufferOverflow(size_t offset, size_t bufSize, size_t readSize);
    // In C# char's size is 2 bytes, this is due to usage of UTF-16
    static constexpr size_t CHAR_SIZE = 2;

public:
    SaveUtil() = delete;
    static void decompressToDirectory(QString const& filePath, QString const& destDirPath);
    static bool extractFile(const QString& dir, const char* data, size_t& offset, size_t size);
    static void compressDirectory(QString const& inDirPath, QString const& outFilePath);
};

#endif // SAVEUTIL_H
