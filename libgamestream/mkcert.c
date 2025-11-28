/*
 * Moonlight is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moonlight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonlight; if not, see <http://www.gnu.org/licenses/>.
 */

#include "mkcert.h"

#include <stdio.h>
#include <stdlib.h>

#include <openssl/crypto.h>
#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/pkcs12.h>
#include <openssl/rsa.h>

static const int NUM_BITS = 2048;
static const int SERIAL = 0;
static const int NUM_YEARS = 10;

int mkcert(X509 **x509p, EVP_PKEY **pkeyp, int bits, int serial, int years);

CERT_KEY_PAIR mkcert_generate() {
    BIO *bio_err;
    X509 *x509 = NULL;
    EVP_PKEY *pkey = NULL;
    PKCS12 *p12 = NULL;

    bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);

    mkcert(&x509, &pkey, NUM_BITS, SERIAL, NUM_YEARS);

    p12 = PKCS12_create("limelight", "GameStream", pkey, x509, NULL, 0, 0, 0, 0, 0);

    BIO_free(bio_err);

    return (CERT_KEY_PAIR) {x509, pkey, p12};
}

void mkcert_free(CERT_KEY_PAIR certKeyPair) {
    X509_free(certKeyPair.x509);
    EVP_PKEY_free(certKeyPair.pkey);
    PKCS12_free(certKeyPair.p12);
}

void mkcert_save(const char* certFile, const char* p12File, const char* keyPairFile, CERT_KEY_PAIR certKeyPair) {
    FILE* certFilePtr = fopen(certFile, "w");
    FILE* keyPairFilePtr = fopen(keyPairFile, "w");
    FILE* p12FilePtr = fopen(p12File, "wb");

    //TODO: error check
    PEM_write_PrivateKey(keyPairFilePtr, certKeyPair.pkey, NULL, NULL, 0, NULL, NULL);
    PEM_write_X509(certFilePtr, certKeyPair.x509);
    i2d_PKCS12_fp(p12FilePtr, certKeyPair.p12);

    fclose(p12FilePtr);
    fclose(certFilePtr);
    fclose(keyPairFilePtr);
}

int mkcert(X509 **x509p, EVP_PKEY **pkeyp, int bits, int serial, int years) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    EVP_PKEY_keygen_init(ctx);
    EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits);

    // pk must be initialized on input
    EVP_PKEY *pk = NULL;;
    EVP_PKEY_keygen(ctx, &pk);

    EVP_PKEY_CTX_free(ctx);

    X509* cert = X509_new();
    X509_set_version(cert, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(cert), serial);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    X509_gmtime_adj(X509_get_notBefore(cert), 0);
    X509_gmtime_adj(X509_get_notAfter(cert), 60 * 60 * 24 * 365 * years);
#else
    ASN1_TIME* before = ASN1_STRING_dup(X509_get0_notBefore(cert));
    ASN1_TIME* after = ASN1_STRING_dup(X509_get0_notAfter(cert));

    X509_gmtime_adj(before, 0);
    X509_gmtime_adj(after, 60 * 60 * 24 * 365 * years);

    X509_set1_notBefore(cert, before);
    X509_set1_notAfter(cert, after);

    ASN1_STRING_free(before);
    ASN1_STRING_free(after);
#endif

    X509_set_pubkey(cert, pk);

    X509_NAME* name = X509_get_subject_name(cert);
    X509_NAME_add_entry_by_txt(name,"CN", MBSTRING_ASC, (unsigned char*)"NVIDIA GameStream Client", -1, -1, 0);
    X509_set_issuer_name(cert, name);

    if (!X509_sign(cert, pk, EVP_sha256())) {
        goto err;
    }

    *x509p = cert;
    *pkeyp = pk;

    return(1);
err:
    return(0);
}
