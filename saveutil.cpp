#include "saveutil.h"

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
    while (extractFile(destDirPath, data.c_str(), progress, data.size())) {}
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
void compressDirectory(QString const& inDirPath, QString const& outFilePath) {

}
