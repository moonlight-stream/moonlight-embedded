/*
 * Created by Diego Waxemberg on 10/16/14.
 * Copyright (c) 2014 Limelight Stream. All rights reserved.
 *
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

#ifndef Limelight_mkcert_h
#define Limelight_mkcert_h

#include <openssl/x509v3.h>
#include <openssl/pkcs12.h>

typedef struct CertKeyPair {
    X509 *x509;
    EVP_PKEY *pkey;
    PKCS12 *p12;
} CertKeyPair;

struct CertKeyPair generateCertKeyPair();
void freeCertKeyPair(CertKeyPair);
void saveCertKeyPair(const char* certFile, const char* p12File, const char* keyPairFile, CertKeyPair certKeyPair);
#endif

