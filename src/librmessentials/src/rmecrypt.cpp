#include "rmecrypt.h"

#include <QByteArray>
#include <QScopedPointer>

#include <xxtea/xxtea.h>
#include <zlib.h>

QByteArray RmeCrypt::decryptXxteaOnly(const QByteArray &encrypted, const QByteArray &_decryptKey)
{
    QByteArray decryptKey = _decryptKey;
    decryptKey.resize(16); // TO PREVENT UNDEFINED BEHAVIOR IN xxtea_decrypt

    QByteArray encryptedBase64 = QByteArray::fromBase64(encrypted);
    size_t outLen = 0;

    QScopedPointer<char, QScopedPointerPodDeleter> decrypted(
        reinterpret_cast<char *>(xxtea_decrypt(encryptedBase64.constData(), encryptedBase64.length(), decryptKey.constData(), &outLen)));

    // Let's assume that outLen != 0......
    // There seems to be no way other than deep copy, since decrypted should be freed to prevent memory leak
    // how to make the above variable owned by QByteArray without deep copy?
    if (decrypted != nullptr)
        return QByteArray(decrypted.data(), outLen);

    return {};
}

QByteArray RmeCrypt::decryptFull(const QByteArray &encrypted, const QByteArray &decryptKey)
{
    QByteArray decryptedXxtea = RmeCrypt::decryptXxteaOnly(encrypted, decryptKey);

    if (!decryptedXxtea.isEmpty()) {
        QByteArray decryptedBase64 = QByteArray::fromBase64(decryptedXxtea);

        QByteArray finalBa;
        for (size_t s = 4096; s < 4096 * 4096; s *= 2) {
            finalBa.resize(s);
            uLongf destLen = uLongf(s - 1);
            int err = uncompress(reinterpret_cast<Bytef *>(finalBa.data()), &destLen, reinterpret_cast<const Bytef *>(decryptedBase64.constData()), decryptedBase64.length());
            if (err == Z_OK) {
                finalBa.resize(destLen);
                break;
            }
            if (err != Z_BUF_ERROR) {
                finalBa.clear();
                break;
            }
        }

        if (!finalBa.isEmpty())
            return finalBa;
    }

    return {};
}

QByteArray RmeCrypt::encryptXxteaOnly(const QByteArray &original, const QByteArray &_encryptKey)
{
    QByteArray encryptKey = _encryptKey;
    encryptKey.resize(16); // TO PREVENT UNDEFINED BEHAVIOR IN xxtea_encrypt
    size_t outLen = 0;
    QScopedPointer<char, QScopedPointerPodDeleter> encrypted(reinterpret_cast<char *>(xxtea_encrypt(original.constData(), original.length(), encryptKey.constData(), &outLen)));

    // Let's assume that outLen != 0......
    if (encrypted != nullptr) {
        QByteArray encryptedBa = QByteArray::fromRawData(encrypted.data(), outLen);
        return encryptedBa.toBase64();
    }

    return {};
}

QByteArray RmeCrypt::encryptFull(const QByteArray &original, const QByteArray &encryptKey)
{
    QByteArray initialBa;
    uLong s = compressBound(original.length()) + 1;
    initialBa.resize(s);
    uLongf destLen = uLongf(s - 1);
    int err = compress(reinterpret_cast<Bytef *>(initialBa.data()), &destLen, reinterpret_cast<const Bytef *>(original.constData()), original.length());
    if (err == Z_OK) {
        initialBa.resize(destLen);
        return RmeCrypt::encryptXxteaOnly(initialBa.toBase64(), encryptKey);
    }

    return {};
}
