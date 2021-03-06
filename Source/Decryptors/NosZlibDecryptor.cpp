#include "NosZlibDecryptor.h"

NosZlibDecryptor::NosZlibDecryptor() = default;

QByteArray NosZlibDecryptor::encrypt(QByteArray &array, int level) {
    return qCompress(array, level);
}

QByteArray NosZlibDecryptor::decrypt(QByteArray &array) {
    return qUncompress(array);
}
