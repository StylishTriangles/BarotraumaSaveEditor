#ifndef SAVEUTIL_H
#define SAVEUTIL_H

#include <QByteArray>
#include <QFile>
#include <QString>

#include <cinttypes>
#include <stdexcept>

class SaveUtil
{
private:
    static int32_t toInt32(const char* bytes, size_t offset = 0);
    static char16_t toChar16(const char* bytes, size_t offset = 0);
    static void int32ToBuffer(int32_t in, char* buffer, size_t offset = 0);
    static void char16ToBuffer(char16_t in, char* buffer, size_t offset = 0);
    static void appendInt32(int32_t in, QByteArray& buffer);
    static void appendChar16(char16_t in, QByteArray& buffer);
    static bool checkBufferOverflow(size_t offset, size_t bufSize, size_t readSize);

public:
    SaveUtil() = delete;
    // compression stuff
    static void decompressToDirectory(QString const& filePath, QString const& destDirPath);
    static bool extractFile(const QString& dir, const char* data, size_t& offset, size_t size);
    static void compressDirectory(QString const& inDirPath, QString const& outFilePath);
    static void compressFile(QString const& inFilePath, QByteArray& buffer);
    // addtitional management options
    static bool backupFile(QString const& filePath, unsigned backups_limit = 100);
};

#endif // SAVEUTIL_H
