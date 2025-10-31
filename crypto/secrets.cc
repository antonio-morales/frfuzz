/* SPDX-License-Identifier: AGPL-3.0-only */
#include "secrets.h"

// PBKDF2-HMAC-SHA256
std::vector<unsigned char> derive_key(const std::string &password, const unsigned char *salt, int salt_len) {

    std::vector<unsigned char> key(KEY_LEN);
    if (!PKCS5_PBKDF2_HMAC(password.c_str(), password.size(), salt, salt_len, PBKDF2_ITER, EVP_sha256(), KEY_LEN, key.data())) {
        throw std::runtime_error("PBKDF2 failed");
    }

    return key;
}

EncBlob encrypt_secret(const std::string &password, const std::string &plaintext) {

    EncBlob out;
    // out.salt.resize(SALT_LEN);
    // out.iv.resize(IV_LEN);

    if (RAND_bytes(out.salt, SALT_LEN) != 1) {
        throw std::runtime_error("RAND_bytes(salt) failed");
    }

    if (RAND_bytes(out.iv, IV_LEN) != 1) {
        throw std::runtime_error("RAND_bytes(iv) failed");
    }

    // derive key
    auto key = derive_key(password, out.salt, SALT_LEN);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("EVP_CIPHER_CTX_new failed");
    }

    int len;
    int ciphertext_len;

    // init AES-256-GCM
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) {
        throw std::runtime_error("EncryptInit failed");
    }

    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, NULL)) {
        throw std::runtime_error("set iv len failed");
    }

    if (1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key.data(), out.iv)) {
        throw std::runtime_error("EncryptInit key/iv failed");
    }

    out.ct.resize(plaintext.size());
    if (1 != EVP_EncryptUpdate(ctx, out.ct.data(), &len, reinterpret_cast<const unsigned char *>(plaintext.data()), plaintext.size())) {
        throw std::runtime_error("EncryptUpdate failed");
    }
    ciphertext_len = len;

    // finalize (GCM doesn’t output more ct here usually)
    if (1 != EVP_EncryptFinal_ex(ctx, out.ct.data() + len, &len)) {
        throw std::runtime_error("EncryptFinal failed");
    }

    ciphertext_len += len;
    out.ct.resize(ciphertext_len);

    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LEN, out.tag)) {
        throw std::runtime_error("get tag failed");
    }

    EVP_CIPHER_CTX_free(ctx);

    // (optional) zero key in memory
    OPENSSL_cleanse(key.data(), key.size());

    return out;
}

std::string decrypt_secret(const std::string &password, const EncBlob &in) {

    // derive key
    auto key = derive_key(password, in.salt, SALT_LEN);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("EVP_CIPHER_CTX_new failed");
    }

    int len;
    int pt_len;
    std::vector<unsigned char> plaintext(in.ct.size());

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) {
        throw std::runtime_error("DecryptInit failed");
    }

    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, NULL)) {
        throw std::runtime_error("set iv len failed");
    }

    if (1 != EVP_DecryptInit_ex(ctx, NULL, NULL, key.data(), in.iv)) {
        throw std::runtime_error("DecryptInit key/iv failed");
    }

    if (1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, in.ct.data(), in.ct.size())) {
        throw std::runtime_error("DecryptUpdate failed");
    }
    pt_len = len;

    // set expected tag
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_LEN, (void *)in.tag)) {
        throw std::runtime_error("set tag failed");
    }

    int ret = EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);
    EVP_CIPHER_CTX_free(ctx);

    // wipe key
    OPENSSL_cleanse(key.data(), key.size());

    if (ret <= 0) {
        throw std::runtime_error("auth failed: wrong password or data corrupted");
    }
    pt_len += len;
    plaintext.resize(pt_len);

    return std::string(reinterpret_cast<char *>(plaintext.data()), plaintext.size());
}

std::string pack_blob(const EncBlob &e) {

    std::string out;

    out.resize(SALT_LEN + IV_LEN + e.ct.size() + TAG_LEN);

    unsigned char *ptr = reinterpret_cast<unsigned char *>(&out[0]);

    memcpy(ptr, e.salt, SALT_LEN);
    ptr += SALT_LEN;

    memcpy(ptr, e.iv, IV_LEN);
    ptr += IV_LEN;

    memcpy(ptr, e.ct.data(), e.ct.size());
    ptr += e.ct.size();

    memcpy(ptr, e.tag, TAG_LEN);

    return out;
}

EncBlob unpack_blob(const std::string &str) {

    if (str.size() < SALT_LEN + IV_LEN + TAG_LEN) {
        throw std::runtime_error("blob too small");
    }

    EncBlob e;

    unsigned char *ptr = reinterpret_cast<unsigned char *>(const_cast<char *>(str.data()));

    memcpy(e.salt, ptr, SALT_LEN);
    ptr += SALT_LEN;

    memcpy(e.iv, ptr, IV_LEN);
    ptr += IV_LEN;

    size_t ct_len = str.size() - SALT_LEN - IV_LEN - TAG_LEN;
    e.ct.resize(ct_len);
    memcpy(e.ct.data(), ptr, ct_len);
    ptr += ct_len;

    memcpy(e.tag, ptr, TAG_LEN);

    return e;
}