// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dataencrypt.h"
#include "commondef.h"
#include <QtDebug>

//暂时加解密不适用，待后续补充
//QByteArray DataEncrypt::aesCbcEncrypt(const QByteArray &data, const QByteArray &key,
//                                      const QByteArray &initialVector)
//{
//    qCDebug(ServiceLogger) << "Starting AES CBC encryption with data size:" << data.size() << "key size:" << key.size();
//    QByteArray result;
//    AES_KEY aesKey;
//    if (AES_set_encrypt_key(reinterpret_cast<const unsigned char *>(key.data()), key.size() * 8,
//                            &aesKey)
//            != 0) {
//        qCWarning(ServiceLogger) << "AES encryption key init failed";
//        return result;
//    }

//    result = aesCbcEncrypt(data, aesKey, initialVector, AES_ENCRYPT);
//    if (result.isEmpty())
//        qCWarning(ServiceLogger) << "AES CBC encrypt failed";
//    else
//        qCDebug(ServiceLogger) << "AES CBC encrypt completed, result size:" << result.size();

//    return result;
//}

//QByteArray DataEncrypt::aesCbcDecrypt(const QByteArray &data, const QByteArray &key,
//                                      const QByteArray &initialVector)
//{
//    qCDebug(ServiceLogger) << "Starting AES CBC decryption with data size:" << data.size() << "key size:" << key.size();
//    QByteArray result;
//    AES_KEY aesKey;
//    if (AES_set_decrypt_key(reinterpret_cast<const unsigned char *>(key.data()), key.size() * 8,
//                            &aesKey)
//            != 0) {
//        qCWarning(ServiceLogger) << "AES decryption key init failed";
//        return result;
//    }

//    result = aesCbcEncrypt(data, aesKey, initialVector, AES_DECRYPT);
//    if (result.isEmpty())
//        qCWarning(ServiceLogger) << "AES CBC decrypt failed";
//    else
//        qCDebug(ServiceLogger) << "AES CBC decrypt completed, result size:" << result.size();

//    return result;
//}

//QByteArray DataEncrypt::aesCbcEncrypt(const QByteArray &data, const AES_KEY &key,
//                                      const QByteArray &initialVector, bool isEncrypt)
//{
//    qCDebug(ServiceLogger) << "Performing AES CBC operation, encrypt mode:" << isEncrypt << "data size:" << data.size();
//    QByteArray result;
//    if (initialVector.size() != AES_BLOCK_SIZE) {
//        qCWarning(ServiceLogger) << "Invalid init vector length:" << initialVector.size() << "expected:" << AES_BLOCK_SIZE;
//        return result;
//    }

//    int remainder = data.size() % AES_BLOCK_SIZE;
//    int paddingSize = (remainder == 0) ? 0 : (AES_BLOCK_SIZE - remainder);
//    qCDebug(ServiceLogger) << "AES padding calculation - remainder:" << remainder << "padding size:" << paddingSize;
//    result.resize(data.size() + paddingSize);
//    result.fill(0);
//    QByteArray tmpIinitialVector = initialVector; // 初始向量会被修改，故需要临时变量来暂存
//    AES_cbc_encrypt(reinterpret_cast<const unsigned char *>(data.data()),
//                    reinterpret_cast<unsigned char *>(result.data()),
//                    static_cast<size_t>(data.size()), &key,
//                    reinterpret_cast<unsigned char *>(tmpIinitialVector.data()),
//                    (isEncrypt ? AES_ENCRYPT : AES_DECRYPT));
//    qCDebug(ServiceLogger) << "AES CBC operation completed";
//    return result;
//}

//QByteArray DataEncrypt::aesCfb128Encrypt(const QByteArray &data, const QByteArray &key,
//                                         const QByteArray &initialVector)
//{
//    qCDebug(ServiceLogger) << "Starting AES CFB128 encryption with data size:" << data.size() << "key size:" << key.size();
//    QByteArray result;

//    AES_KEY aesKey;
//    if (AES_set_encrypt_key(reinterpret_cast<const unsigned char *>(key.data()), key.size() * 8,
//                            &aesKey)
//            != 0) {
//        qCWarning(ServiceLogger) << "AES CFB128 encryption key init failed";
//        return result;
//    }

//    result = aesCfb128Encrypt(data, aesKey, initialVector, AES_ENCRYPT);
//    if (result.isEmpty())
//        qCWarning(ServiceLogger) << "AES CFB128 encrypt failed";
//    else
//        qCDebug(ServiceLogger) << "AES CFB128 encrypt completed, result size:" << result.size();

//    return result;
//}

//QByteArray DataEncrypt::aesCfb128Decrypt(const QByteArray &data, const QByteArray &key,
//                                         const QByteArray &initialVector)
//{
//    qCDebug(ServiceLogger) << "Starting AES CFB128 decryption with data size:" << data.size() << "key size:" << key.size();
//    QByteArray result;

//    if (data.isEmpty()) {
//        qCDebug(ServiceLogger) << "Empty data provided for CFB128 decryption, returning empty result";
//        return result;
//    }

//    AES_KEY aesKey;
//    if (AES_set_encrypt_key(reinterpret_cast<const unsigned char *>(key.data()), key.size() * 8,
//                            &aesKey)
//            != 0) {
//        qCWarning(ServiceLogger) << "AES CFB128 decryption key init failed";
//        return result;
//    }

//    result = aesCfb128Encrypt(data, aesKey, initialVector, AES_DECRYPT);
//    if (result.isEmpty())
//        qCWarning(ServiceLogger) << "AES CFB128 decrypt failed";
//    else
//        qCDebug(ServiceLogger) << "AES CFB128 decrypt completed, result size:" << result.size();

//    return result;
//}


//QByteArray DataEncrypt::aesCfb128Encrypt(const QByteArray &data, const AES_KEY &key,
//                                         const QByteArray &initialVector, bool isEncrypt)
//{
//    qCDebug(ServiceLogger) << "Performing AES CFB128 operation, encrypt mode:" << isEncrypt << "data size:" << data.size();
//    QByteArray result;
//    if (initialVector.size() != AES_BLOCK_SIZE) {
//        qCWarning(ServiceLogger) << "Invalid init vector length for CFB128:" << initialVector.size() << "expected:" << AES_BLOCK_SIZE;
//        return result;
//    }

////    int remainder = data.size() % AES_BLOCK_SIZE;
////    int paddingSize = (remainder == 0) ? 0 : (AES_BLOCK_SIZE - remainder);
//    result.resize(data.size() + 0);
//    result.fill(0);
//    int num = 0;
//    QByteArray tmpIinitialVector = initialVector; // 初始向量会被修改，故需要临时变量来暂存
//    qCDebug(ServiceLogger) << "Executing AES CFB128 encryption/decryption";
//    AES_cfb128_encrypt(reinterpret_cast<const unsigned char *>(data.data()),
//                       reinterpret_cast<unsigned char *>(result.data()),
//                       static_cast<size_t>(data.size()), &key,
//                       reinterpret_cast<unsigned char *>(tmpIinitialVector.data()), &num,
//                       (isEncrypt ? AES_ENCRYPT : AES_DECRYPT));
//    qCDebug(ServiceLogger) << "AES CFB128 operation completed";
//    return result;
//}
