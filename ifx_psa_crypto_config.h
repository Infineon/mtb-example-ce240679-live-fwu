/*****************************************************************************
 * \file psa/crypto_config.h
 * \brief PSA crypto configuration options (set of defines)
 *
 *
 *****************************************************************************
 * Copyright 2024, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 *****************************************************************************/


#ifndef PSA_CRYPTO_CONFIG_H
#define PSA_CRYPTO_CONFIG_H

/*
 * CBC-MAC is not yet supported via the PSA API in Mbed TLS.
 */
//#define PSA_WANT_ALG_CBC_MAC                    1
#define PSA_WANT_ALG_CBC_NO_PADDING             1
#define PSA_WANT_ALG_CBC_PKCS7                  1
#define PSA_WANT_ALG_CCM                        1
//#define PSA_WANT_ALG_CCM_STAR_NO_TAG            1
//#define PSA_WANT_ALG_CMAC                       1
//#define PSA_WANT_ALG_CFB                        1
//#define PSA_WANT_ALG_CHACHA20_POLY1305          1
#define PSA_WANT_ALG_CTR                        1
#define PSA_WANT_ALG_DETERMINISTIC_ECDSA        1
//#define PSA_WANT_ALG_ECB_NO_PADDING             1
#define PSA_WANT_ALG_ECDH                       1
//#define PSA_WANT_ALG_FFDH                       1
#define PSA_WANT_ALG_ECDSA                      1
//#define PSA_WANT_ALG_JPAKE                      1
//#define PSA_WANT_ALG_GCM                        1
#define PSA_WANT_ALG_HKDF                       1
#define PSA_WANT_ALG_HKDF_EXTRACT               1
#define PSA_WANT_ALG_HKDF_EXPAND                1
#define PSA_WANT_ALG_HMAC                       1
//#define PSA_WANT_ALG_MD5                        1
//#define PSA_WANT_ALG_OFB                        1
//#define PSA_WANT_ALG_PBKDF2_HMAC                1
//#define PSA_WANT_ALG_PBKDF2_AES_CMAC_PRF_128    1
//#define PSA_WANT_ALG_RIPEMD160                  1
#define PSA_WANT_ALG_RSA_OAEP                   1
//#define PSA_WANT_ALG_RSA_PKCS1V15_CRYPT         1
//#define PSA_WANT_ALG_RSA_PKCS1V15_SIGN          1
//#define PSA_WANT_ALG_RSA_PSS                    1
//#define PSA_WANT_ALG_SHA_1                      1
#define PSA_WANT_ALG_SHA_224                    1
#define PSA_WANT_ALG_SHA_256                    1
//#define PSA_WANT_ALG_SHA_384                    1
//#define PSA_WANT_ALG_SHA_512                    1
//#define PSA_WANT_ALG_SHA3_224                   1
//#define PSA_WANT_ALG_SHA3_256                   1
//#define PSA_WANT_ALG_SHA3_384                   1
//#define PSA_WANT_ALG_SHA3_512                   1
#define PSA_WANT_ALG_STREAM_CIPHER              1
//#define PSA_WANT_ALG_TLS12_PRF                  1
//#define PSA_WANT_ALG_TLS12_PSK_TO_MS            1
//#define PSA_WANT_ALG_TLS12_ECJPAKE_TO_PMS       1

/* XTS is not yet supported via the PSA API in Mbed TLS.
 * Note: when adding support, also adjust include/mbedtls/config_psa.h */
//#define PSA_WANT_ALG_XTS                        1

//#define PSA_WANT_ECC_BRAINPOOL_P_R1_256         1
//#define PSA_WANT_ECC_BRAINPOOL_P_R1_384         1
//#define PSA_WANT_ECC_BRAINPOOL_P_R1_512         1
//#define PSA_WANT_ECC_MONTGOMERY_255             1
//#define PSA_WANT_ECC_MONTGOMERY_448             1
//#define PSA_WANT_ECC_SECP_K1_192                1
/*
 * SECP224K1 is buggy via the PSA API in Mbed TLS
 * (https://github.com/Mbed-TLS/mbedtls/issues/3541). Thus, do not enable it by
 * default.
 */
//#define PSA_WANT_ECC_SECP_K1_224                1
//#define PSA_WANT_ECC_SECP_K1_256                1
//#define PSA_WANT_ECC_SECP_R1_192                1
//#define PSA_WANT_ECC_SECP_R1_224                1
/* For secp256r1, consider enabling #MBEDTLS_PSA_P256M_DRIVER_ENABLED
 * (see the description in mbedtls/mbedtls_config.h for details). */
#define PSA_WANT_ECC_SECP_R1_256                1
//#define PSA_WANT_ECC_SECP_R1_384                1
//#define PSA_WANT_ECC_SECP_R1_521                1

#define PSA_WANT_KEY_TYPE_DERIVE                1
//#define PSA_WANT_KEY_TYPE_PASSWORD              1
//#define PSA_WANT_KEY_TYPE_PASSWORD_HASH         1
#define PSA_WANT_KEY_TYPE_HMAC                  1
#define PSA_WANT_KEY_TYPE_AES                   1
//#define PSA_WANT_KEY_TYPE_ARIA                  1
//#define PSA_WANT_KEY_TYPE_CAMELLIA              1
//#define PSA_WANT_KEY_TYPE_CHACHA20              1
//#define PSA_WANT_KEY_TYPE_DES                   1
//#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR          1 /* Deprecated */
#define PSA_WANT_KEY_TYPE_ECC_PUBLIC_KEY        1
//#define PSA_WANT_KEY_TYPE_DH_PUBLIC_KEY         1
#define PSA_WANT_KEY_TYPE_RAW_DATA              1
//#define PSA_WANT_KEY_TYPE_RSA_KEY_PAIR          1 /* Deprecated */
#define PSA_WANT_KEY_TYPE_RSA_PUBLIC_KEY        1

/*
 * The following symbols extend and deprecate the legacy
 * PSA_WANT_KEY_TYPE_xxx_KEY_PAIR ones. They include the usage of that key in
 * the name's suffix. "_USE" is the most generic and it can be used to describe
 * a generic suport, whereas other ones add more features on top of that and
 * they are more specific.
 */
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC    1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_IMPORT   1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_EXPORT   1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_GENERATE 1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_DERIVE   1

//#define PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_BASIC      1
//#define PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_IMPORT   1
//#define PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_EXPORT   1
//#define PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_GENERATE 1
//#define PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_DERIVE   1 /* Not supported */

//#define PSA_WANT_KEY_TYPE_DH_KEY_PAIR_BASIC       1
//#define PSA_WANT_KEY_TYPE_DH_KEY_PAIR_IMPORT    1
//#define PSA_WANT_KEY_TYPE_DH_KEY_PAIR_EXPORT    1
//#define PSA_WANT_KEY_TYPE_DH_KEY_PAIR_GENERATE  1
//#define PSA_WANT_KEY_TYPE_DH_KEY_PAIR_DERIVE    1 /* Not supported */

#endif /* PSA_CRYPTO_CONFIG_H */
