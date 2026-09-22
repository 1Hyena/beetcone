// SPDX-License-Identifier: MIT
////////////////////////////////////////////////////////////////////////////////
// MIT License                                                                //
//                                                                            //
// Copyright (c) 2026 Erich Erstu                                             //
//                                                                            //
// Permission is hereby granted, free of charge, to any person obtaining a    //
// copy of this software and associated documentation files (the "Software"), //
// to deal in the Software without restriction, including without limitation  //
// the rights to use, copy, modify, merge, publish, distribute, sublicense,   //
// and/or sell copies of the Software, and to permit persons to whom the      //
// Software is furnished to do so, subject to the following conditions:       //
//                                                                            //
// The above copyright notice and this permission notice shall be included in //
// all copies or substantial portions of the Software.                        //
//                                                                            //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    //
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    //
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        //
// DEALINGS IN THE SOFTWARE.                                                  //
////////////////////////////////////////////////////////////////////////////////

#ifndef BEETCONE_H_18_09_2026
#define BEETCONE_H_18_09_2026
////////////////////////////////////////////////////////////////////////////////
#include <openssl/evp.h>
#include <openssl/ec.h>
////////////////////////////////////////////////////////////////////////////////

#define BEETCONE_MAJOR_VERSION  1
#define BEETCONE_MINOR_VERSION  0
#define BEETCONE_REVISION       0
#define BEETCONE_VERSION        "1.0.0"

struct beetcone_key_type;
struct beetcone_addr_type;

static constexpr size_t BEETCONE_KEY_SZ     = 32;
static constexpr size_t BEETCONE_PUBKEY_SZ  = 65;
static constexpr size_t BEETCONE_ADDR_SZ    = 50;

static const char beetcone_base58_table[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
    'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};

struct beetcone_key_type {
    uint8_t data[BEETCONE_KEY_SZ];

    struct {
        bool valid:1;
    } bitset;
};

struct beetcone_pubkey_type {
    uint8_t data[BEETCONE_PUBKEY_SZ];

    struct {
        bool valid:1;
    } bitset;
};

struct beetcone_addr_type {
    char base58[BEETCONE_ADDR_SZ];

    struct {
        bool valid:1;
    } bitset;
};

// Public API: /////////////////////////////////////////////////////////////////
static struct beetcone_addr_type    beetcone_pubkey2addr(
    struct beetcone_pubkey_type         pubkey
);

static struct beetcone_pubkey_type  beetcone_key2pubkey(
    struct beetcone_key_type            key
);

static struct beetcone_addr_type    beetcone_key2addr(
    struct beetcone_key_type            key
);

static size_t                       beetcone_base58_estimate_strlen(
    const void *                        data,
    size_t                              size
);

static struct beetcone_key_type     beetcone_make_key(
    const void *                        data,
    size_t                              size
);
////////////////////////////////////////////////////////////////////////////////


// Private API: ////////////////////////////////////////////////////////////////
static void beetcone_array_reverse(
    void *                                  array,
    size_t                                  size
);

static size_t beetcone_leading_zeros(
    const void *                            data,
    size_t                                  size
);
////////////////////////////////////////////////////////////////////////////////

static inline struct beetcone_key_type beetcone_make_key(
    const void *data, size_t size
) {
    struct beetcone_key_type key = {};

    if (data && size == 32) {
        bool all_zeros = true;

        for (size_t i=0; i<sizeof(key.data); ++i) {
            key.data[i] = ((const uint8_t*) data)[i];

            if (key.data[i]) {
                all_zeros = false;
            }
        }

        key.bitset.valid = !all_zeros;
    }

    return key;
}

static inline size_t beetcone_base58_estimate_strlen(
    const void *data, size_t size
) {
    size_t leading_zeros = beetcone_leading_zeros(data, size);
    return leading_zeros + ((size - leading_zeros) * 138 + 100) / 101;
}

static inline void beetcone_array_reverse(void *array, size_t size) {
    uint8_t *a = (uint8_t *) array;

    for (size_t i = 0, end = size / 2; i < end; ++i) {
        auto val = a[i];
        size_t j = size - (i + 1);

        a[i] = a[j];
        a[j] = val;
    }
}

static inline size_t beetcone_leading_zeros(const void *data, size_t size) {
    const uint8_t *bytes = data;
    size_t leading_zeros = 0;

    for (; leading_zeros<size && !bytes[leading_zeros]; ++leading_zeros);

    return leading_zeros;
}

static inline size_t beetcone_base58_encode(
    const void *src, size_t src_sz, char *dst, size_t dst_sz
) {
    if (!src) {
        abort();
    }

    const size_t max_base58_strlen = beetcone_base58_estimate_strlen(
        src, src_sz
    );

    if (!dst) {
        return max_base58_strlen;
    }

    const size_t leading_zeros = beetcone_leading_zeros(src, src_sz);

    size_t dst_len = 0;

    memset(dst, 0, dst_sz > max_base58_strlen ? max_base58_strlen : dst_sz);

    const uint8_t *src_arr = src;

    for (size_t src_i=leading_zeros; src_i<src_sz; ++src_i) {
        size_t carry = src_arr[src_i];

        for (size_t dst_i=0; dst_i<dst_len; ++dst_i) {
            size_t current = (size_t) dst[dst_i] * 256 + carry;

            if (dst_i + 1 < dst_sz) {
                dst[dst_i] = (char) (current % sizeof(beetcone_base58_table));
            }
            else {
                *dst = '\0';
                return max_base58_strlen;
            }

            carry = current / sizeof(beetcone_base58_table);
        }

        while (carry) {
            if (dst_len + 1 < dst_sz) {
                dst[dst_len] = (char) (carry % sizeof(beetcone_base58_table));
                carry /= sizeof(beetcone_base58_table);
            }
            else {
                *dst = '\0';
                return max_base58_strlen;
            }

            ++dst_len;
        }
    }

    for (size_t i=0; i<leading_zeros; ++i) {
        if (dst_len + 1 < dst_sz) {
            dst[dst_len] = 0;
        }
        else {
            *dst = '\0';
            return max_base58_strlen;
        }

        ++dst_len;
    }

    beetcone_array_reverse(dst, dst_len);

    for (size_t i=0; i<dst_len; ++i) {
        dst[i] = beetcone_base58_table[(uint8_t) dst[i]];
    }

    dst[dst_len] = '\0';

    return (
        // The maximum number of characters that would have been written if
        // dst_sz had been sufficiently large, not counting the terminating null
        // character.
        dst_len
    );
}

static inline struct beetcone_addr_type beetcone_key2addr(
    struct beetcone_key_type key
) {
    return beetcone_pubkey2addr(beetcone_key2pubkey(key));
}

static inline struct beetcone_pubkey_type beetcone_key2pubkey(
    struct beetcone_key_type key
) {
    const struct beetcone_pubkey_type error = {
        .bitset = {
            .valid = false
        }
    };

    if (!key.bitset.valid) {
        return error;
    }

    BIGNUM* priv_bn = BN_bin2bn(key.data, sizeof(key.data), nullptr);

    if (priv_bn == nullptr) {
        return error;
    }

    EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);

    if (group == nullptr) {
        BN_free(priv_bn);
        return error;
    }

    EC_POINT* pub_pnt = EC_POINT_new(group);

    if (pub_pnt == nullptr) {
        EC_GROUP_free(group);
        BN_free(priv_bn);
        return error;
    }

    if (EC_POINT_mul(group, pub_pnt, priv_bn, nullptr, nullptr, nullptr) != 1) {
        EC_POINT_free(pub_pnt);
        EC_GROUP_free(group);
        BN_free(priv_bn);
        return error;
    }

    struct beetcone_pubkey_type pubkey = {
        .bitset = {
            .valid = true
        }
    };

    size_t pub_len = EC_POINT_point2oct(
        group, pub_pnt, POINT_CONVERSION_UNCOMPRESSED, pubkey.data,
        sizeof(pubkey.data), nullptr
    );

    EC_POINT_free(pub_pnt);
    EC_GROUP_free(group);
    BN_free(priv_bn);

    if (pub_len == 0 || pub_len != sizeof(pubkey.data)) {
        return error;
    }

    return pubkey;
}

static inline struct beetcone_addr_type beetcone_pubkey2addr(
    struct beetcone_pubkey_type pubkey
) {
    const struct beetcone_addr_type error = {
        .bitset = {
            .valid = false
        }
    };

    if (!pubkey.bitset.valid) {
        return error;
    }

    unsigned char sha256_res[32];
    unsigned int sha256_len = 0;

    EVP_Digest(
        pubkey.data, sizeof(pubkey.data), sha256_res, &sha256_len, EVP_sha256(),
        nullptr
    );

    unsigned char ripemd160_res[20];
    unsigned int ripemd_len = 0;

    EVP_Digest(
        sha256_res, sha256_len, ripemd160_res, &ripemd_len, EVP_ripemd160(),
        nullptr
    );

    unsigned char address_payload[25];
    address_payload[0] = 0x00; // Mainnet network byte
    memcpy(&address_payload[1], ripemd160_res, 20);

    // Double SHA-256 for Checksum computation
    unsigned char check_sha1[32];
    unsigned char check_sha2[32];

    EVP_Digest(
        address_payload, 21, check_sha1, &sha256_len, EVP_sha256(), nullptr
    );

    EVP_Digest(
        check_sha1, 32, check_sha2, &sha256_len, EVP_sha256(), nullptr
    );

    memcpy(&address_payload[21], check_sha2, 4);

    struct beetcone_addr_type addr = {
        .bitset = {
            .valid = true
        }
    };

    auto ret = beetcone_base58_encode(
        address_payload, 25, addr.base58, sizeof(addr.base58)
    );

    if (ret >= sizeof(addr.base58)) {
        return error;
    }

    return addr;
}

#endif
