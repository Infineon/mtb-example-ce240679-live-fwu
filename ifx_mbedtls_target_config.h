/*****************************************************************************
 * \file ifx-mbedtls-target-config.h
 *
 * \brief Configuration options (set of defines)
 *
 *  This set of compile-time options may be used to enable
 *  or disable platform specific features.
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


#ifndef IFX_MBEDTLS_TARGET_CONFIG_H
#define IFX_MBEDTLS_TARGET_CONFIG_H

/* *** DO NOT CHANGE ANY SETTINGS IN THIS SECTION *** */

/* Enable SE RT Services crypto driver */
//#define IFX_PSA_SE_DPA_PRESENT

/* Enable MXCRYPTO transparent driver */
//#define IFX_PSA_MXCRYPTO_PRESENT

/* Enable CRYPTOLITE transparent driver */
#define IFX_PSA_CRYPTOLITE_PRESENT

/* Use SE RT Services to calculate SHA256 digest */
//#define IFX_PSA_SHA256_BY_SE_DPA

/* Use SE RT Services to generate random values */
//#define IFX_PSA_RANDOM_BY_SE_DPA

/* Use SE RT Services builtin keys */
//#define IFX_PSA_CRYPTO_BUILTIN_KEYS

/* Enable support for platform built-in keys.
   Built-in keys are stored in SE RT Services */
//#define MBEDTLS_PSA_CRYPTO_BUILTIN_KEYS

//#define MBEDTLS_PLATFORM_SETUP_TEARDOWN_ALT

#endif /* IFX_MBEDTLS_TARGET_CONFIG_H */
