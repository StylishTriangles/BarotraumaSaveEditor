#include "saveutil.h"

#include <cstring>

#include <QByteArray>
#include <QDir>

#include <gzip-cpp/decompress.hpp>
#include <gzip-cpp/compress.hpp>

// convert bytes to int32 assuming little endian byte ordering
int32_t SaveUtil::toInt32(const char* bytes, size_t offset) {
    return  (static_cast<unsigned char>(bytes[offset+3]) << 24) |
            (static_cast<unsigned char>(bytes[offset+2]) << 16) |
            (static_cast<unsigned char>(bytes[offset+1]) << 8) |
            (static_cast<unsigned char>(bytes[offset]));
}

// convert bytes to char16 assuming little endian byte ordering
char16_t SaveUtil::toChar16(const char* bytes, size_t offset) {
    return static_cast<char16_t>(
                (static_cast<unsigned char>(bytes[offset+1]) << 8) |
                (static_cast<unsigned char>(bytes[offset]))
    );
}

// Write int32 to bytes buffer starting at offset
void SaveUtil::int32ToBuffer(int32_t in, char* buffer, size_t offset) {
    memcpy(buffer + offset, &in, sizeof(int32_t));
}

// Write char16 to bytes buffer starting at offset
void SaveUtil::char16ToBuffer(char16_t in, char* buffer, size_t offset) {
    memcpy(buffer + offset, &in, sizeof(char16_t));
}

// Append int32 to bytes buffer
void SaveUtil::appendInt32(int32_t in, QByteArray& buffer) {
    char conversionBuffer[sizeof(int32_t)];
    int32ToBuffer(in, conversionBuffer, 0);
    buffer.append(conversionBuffer, sizeof(int32_t));
}

// Append char16 to bytes buffer
void SaveUtil::appendChar16(char16_t in, QByteArray& buffer) {
    char conversionBuffer[sizeof(char16_t)];
    char16ToBuffer(in, conversionBuffer, 0);
    buffer.append(conversionBuffer, sizeof(char16_t));
}

// check if there is enough data in the buffer
// @returns bool: true if there would be an overflow
bool SaveUtil::checkBufferOverflow(size_t offset, size_t bufSize, size_t readSize) {
    return (bufSize - offset > bufSize || bufSize - offset < readSize);
}

// Decompress a gzipped file into a directory, same method as in:
// https://github.com/Regalis11/Barotrauma/blob/0002ad2c501a1a8df323b52edfc82a78d0afc6bc/Barotrauma/BarotraumaShared/SharedSource/Utils/SaveUtil.cs
void SaveUtil::decompressToDirectory(QString const& filePath, QString const& destDirPath) {
    QFile compressedFile(filePath);
    compressedFile.open(QFile::ReadOnly);
    std::string data;
    try {
        data = gzip::decompress(compressedFile.readAll().data(), static_cast<size_t>(compressedFile.size()));
    } catch(std::runtime_error const& e) {
        // rethrow exception
        throw std::runtime_error(std::string("gzip error: ") + e.what());
    }

    // create destination dir
    QDir outDir(destDirPath);
    if (outDir.exists())
        outDir.removeRecursively();
    outDir.mkpath(".");

    size_t progress = 0; // offset from the beginning of file
    bool moreData = true; // true if more files could be extracted
    int index = 1; // index of file being processed
    while (moreData) {
        try {
            moreData = extractFile(destDirPath, data.c_str(), progress, data.size());
        } catch(std::runtime_error const& e) {
            QString errorMsg = QString("File ID: %1 processing error: ").arg(index);
            throw std::runtime_error((errorMsg+e.what()).toStdString());
        }
        index++;
    }
}

/* @param dir: directory where the file should be extracted
 * @param data: pointer to the uncompressed data buffer
 * @param offset: reference to the current offset value in the data (will be modified)
 * @param size: the size of the data buffer
 * @returns bool: is there more data left to read?
 *
 * extract file from the decompressed data
 * method for extraction is specified in decompressFile method:
 * https://github.com/Regalis11/Barotrauma/blob/0002ad2c501a1a8df323b52edfc82a78d0afc6bc/Barotrauma/BarotraumaShared/SharedSource/Utils/SaveUtil.cs
 */
bool SaveUtil::extractFile(const QString& dir, const char* data, size_t& offset, size_t size) {
    //// Extract file name
    if (checkBufferOverflow(offset, size, sizeof(int32_t))) // [result] < 0 || [result] < 4
        return false;
    size_t nameLen = static_cast<size_t>(toInt32(data, offset));
    offset += sizeof(int32_t); // reading an int32
    if (nameLen > 255)
    {
        throw std::runtime_error(("Failed to decompress to \"" + dir.toStdString() + "\" (file name length > 255). The file may be corrupted."));
    }

    // c# uses utf-16 strings
    std::u16string filename;
    filename.reserve(nameLen);
    if (checkBufferOverflow(offset, size, nameLen * sizeof(char16_t)))
        return false;
    // read the filename from buffer and advance offset
    for (unsigned i = 0; i < nameLen; i++) {
        filename.push_back(toChar16(data, offset));
        offset += sizeof(char16_t);
    }

    //// Extract file content
    if (checkBufferOverflow(offset, size, sizeof(int32_t))) // [result] < 0 || [result] < 4
        return false;
    size_t contentLen = static_cast<size_t>(toInt32(data, offset));
    offset += sizeof(int32_t); // reading an int32
    if (checkBufferOverflow(offset, size, contentLen * sizeof(char)))
        return false;
    const char * contentPtr = &data[offset];
    offset += contentLen * sizeof(char); // read the entire content

    QString extractedFilePath(dir + QDir::separator() + QString::fromUtf16(filename.data()));
    QFile outFile(extractedFilePath);
    outFile.open(QFile::WriteOnly | QFile::Truncate);
    qint64 written = outFile.write(contentPtr, static_cast<qint64>(contentLen));
    if (written != static_cast<qint64>(contentLen))
        throw std::runtime_error("Write failed (not enough data written) when saving \"" + extractedFilePath.toStdString() + "\"");
    outFile.close();

    return true;
}

/* Compress contents of directory
 * @param inDirPath: directory to compress
 * @param outFilePath: where the file should be written
 */
void SaveUtil::compressDirectory(QString const& inDirPath, QString const& outFilePath) {
    QDir inDir(inDirPath);
    QFileInfoList inFiles = inDir.entryInfoList(QDir::Files);
    if (inFiles.empty())
        throw std::runtime_error(("Could not compress directory \"" + inDirPath + "\" - directory is empty").toStdString());
    QFile outFile(outFilePath);
    outFile.open(QFile::Truncate | QFile::WriteOnly);
    QByteArray buffer;
    buffer.reserve(10000000); // 1MB should cover small save files
    for (QFileInfo const& inFileInfo: inFiles) {
        compressFile(inFileInfo.absoluteFilePath(), buffer);
    }
    std::string compressedData = gzip::compress(buffer.data(), static_cast<size_t>(buffer.size()));
    outFile.write(compressedData.data(), static_cast<qint64>(compressedData.size()));
//    outFile.write(buffer.data(), buffer.size());
}

void SaveUtil::compressFile(QString const& inFilePath, QByteArray& buffer) {
    QFile inFile(inFilePath);
    QFileInfo inFileInfo(inFile);
    if (!inFile.open(QFile::ReadOnly)) {
        throw std::runtime_error(("Could not open file \"" + inFilePath + "\" for reading. Save aborted!").toStdString());
    }

    //// write file name
    std::u16string fileName = inFileInfo.fileName().toStdU16String();
    int32_t nameLen = static_cast<int32_t>(fileName.length());
    // write file name length
    appendInt32(nameLen, buffer);
    // write file name characters
    for (unsigned i = 0; i < fileName.size(); i++) {
        appendChar16(fileName[i], buffer);
    }

    //// write file contents
    int32_t contentLen = static_cast<int32_t>(inFileInfo.size());
    // write file content length
    appendInt32(contentLen, buffer);
    buffer.append(inFile.readAll());
}

// Create backup of a provided file by appending .bakx to its name, "x" is an incrementing integer
bool SaveUtil::backupFile(QString const& filePath, unsigned backups_limit) {
    QFile sourceFile(filePath);
    if (!sourceFile.exists())
        return false;

    // enumerate nearest available backup
    QString backupExt = ".bak";
    for (unsigned i = 1;; i++) {
        if (i > backups_limit) // limit spam
            return false;
        QString fullExt = backupExt + QString::number(i);
        if (!QFile::exists(filePath+fullExt))
            return sourceFile.rename(sourceFile.fileName() + fullExt);
    }
}
