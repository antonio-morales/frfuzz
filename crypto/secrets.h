/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <cstring>
#include <stdexcept>
#include <vector>

#include <openssl/rand.h>

static const int SALT_LEN = 16;
static const int IV_LEN = 12;
static const int TAG_LEN = 16;

static const int KEY_LEN = 32; // AES-256

static const int PBKDF2_ITER = 100000;

struct EncBlob {
    unsigned char salt[SALT_LEN];
    unsigned char iv[IV_LEN];

    std::vector<unsigned char> ct;

    unsigned char tag[TAG_LEN];
};

EncBlob encrypt_secret(const std::string &password, const std::string &plaintext);
std::string decrypt_secret(const std::string &password, const EncBlob &in);

std::string pack_blob(const EncBlob &e);
EncBlob unpack_blob(const std::string &str);