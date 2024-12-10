/*****************************************************************************
 * File Name:   image_auth.c
 *
 * Description: This file provides function for Image Authentication
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

/*******************************************************************************
* Header Files
*******************************************************************************/

#include <string.h>
#include <stdint.h>
#include "image_auth.h"
#if defined (MCUBOOT_IMAGE)
#include "mbedtls/ecdsa.h"
#include "psa/crypto.h"
#endif /* MCUBOOT_IMAGE */

/*******************************************************************************
* Function Definitions
*******************************************************************************/
#if defined (MCUBOOT_IMAGE)
int tlv_iter_begin(struct image_tlv_iter *it, const struct image_header *hdr)
{
    uint32_t off;
    struct image_tlv_info *info;

    if (it == NULL || hdr == NULL)
    {
            return -1;
    }

    /* TLV start Offset */
    off = hdr->ih_hdr_size + hdr->ih_img_size;

    info = (struct image_tlv_info *)FLASH_ADDR(off);

    if (info->it_magic != IMAGE_TLV_INFO_MAGIC)
    {
        return -1;
    }

    /* Offset of 1st TLV */
    it->tlv_off = off + sizeof(struct image_tlv_info);

    /* End offset of TLV */
    it->tlv_end = off + info->it_tlv_tot;

    return 0;
}

int tlv_iter_next(struct image_tlv_iter *it, uint32_t *off, uint16_t *len, uint16_t *type)
{
    struct image_tlv *tlv;

    if (it == NULL || off == NULL || len == NULL)
    {
        return -1;
    }

    /* All TLV traversed */
    if(it->tlv_off >= it->tlv_end)
    {
        return 1;
    }

    tlv = (struct image_tlv *)FLASH_ADDR(it->tlv_off);

    /* Assign TLV values */
    *type = tlv->it_type;
    *off = it->tlv_off + sizeof(struct image_tlv);
    *len = tlv->it_len;

    /* Update it struct to point to next TLV */
    it->tlv_off += sizeof(struct image_tlv) + tlv->it_len;

    return 0;
}

int is_img_magic_valid(const struct image_header *hdr)
{
    if(hdr->ih_magic != IMAGE_MAGIC)
    {
        return -1;
    }

    return 0;
}

int is_pub_key_valid(uint8_t *key_addr)
{
    psa_status_t status;
    uint8_t key_hash[32];
    size_t hash_len;

    status = psa_hash_compute(PSA_ALG_SHA_256, (key_addr + 1), (size_t)64, key_hash, sizeof(key_hash), &hash_len);
    if(status != PSA_SUCCESS)
    {
        return -1;
    }

    if(0 == memcmp(key_hash, (uint8_t*)SFLASH_OEM_KEY0_HASH_ADDR, 16))
    {
        return 0;
    }
    else if(0 == memcmp(key_hash, (uint8_t*)SFLASH_OEM_KEY1_HASH_ADDR, 16))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

#endif /* MCUBOOT_IMAGE */

int validate_image(uint32_t boot_addr)
{
#if defined (MCUBOOT_IMAGE)
    const struct image_header *hdr;
    struct image_tlv_iter tlv_it;
    uint8_t *img_hash = NULL;
    uint32_t off;
    uint16_t len;
    uint16_t type;
    int status;
    psa_key_attributes_t ec_key_attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_id_t key_id = 0;
    psa_status_t psa_status;

    hdr = (const struct image_header *)boot_addr;

    status = is_img_magic_valid(hdr);
    if (status != 0)
    {
        return -1;
    }

    status = tlv_iter_begin(&tlv_it, hdr);
    if (status != 0)
    {
        return -1;
    }

    while(1)
    {
        status = tlv_iter_next(&tlv_it, &off, &len, &type);
        if (status > 0)
        {
            /* All TLV traversed */
            break;
        }
        else if(status > 0)
        {
            return -1;
        }

        if (type == IMAGE_TLV_SHA256)
        {
            /* Compare hash of image with reference hash */
            psa_status = psa_hash_compare(PSA_ALG_SHA_256, (uint8_t *)hdr, (size_t)((hdr->ih_img_size) + (hdr->ih_hdr_size)), (uint8_t*) FLASH_ADDR(off), len);
            if(psa_status != PSA_SUCCESS)
            {
                return -1;
            }
            img_hash = (uint8_t*) FLASH_ADDR(off);
        }

        else if (type == IMAGE_TLV_PUBKEY)
        {
            if(0 != is_pub_key_valid((uint8_t *)FLASH_ADDR(off)))
            {
                return -1;
            }

            psa_set_key_usage_flags(&ec_key_attributes, PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_VERIFY_MESSAGE | PSA_KEY_USAGE_EXPORT);
            psa_set_key_algorithm(&ec_key_attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
            psa_set_key_lifetime(&ec_key_attributes, PSA_KEY_LIFETIME_VOLATILE);
            psa_set_key_type(&ec_key_attributes, PSA_KEY_TYPE_ECC_PUBLIC_KEY(PSA_ECC_FAMILY_SECP_R1));
            psa_set_key_bits(&ec_key_attributes, ECC_KEY_BITS);


            psa_status = psa_import_key(&ec_key_attributes, (uint8_t *)FLASH_ADDR(off), len, &key_id);
            if(psa_status != PSA_SUCCESS)
            {
                return -1;
            }
        }

        else if (type == IMAGE_TLV_ECDSA256)
        {
            psa_status = psa_verify_hash(key_id, PSA_ALG_ECDSA(PSA_ALG_SHA_256), img_hash, PSA_HASH_LENGTH(PSA_ALG_SHA_256), (unsigned char *)FLASH_ADDR(off), len);
            if(psa_status != PSA_SUCCESS)
            {
                return -1;
            }
        }
        else
        {
            /* Invalid TLV type */
            return -1;
        }
    }
    return 0;
#else
    uint32_t stack_pointer;
    uint32_t reset_handler;

    /* For images without MCUBoot format check if stack pointer and reset handler are non-zero */
    stack_pointer = *(uint32_t*)boot_addr;
    reset_handler = *(uint32_t*)(boot_addr + 4);
    if ((stack_pointer == 0) || (reset_handler == 0))
    {
        return -1;
    }
    return 0;
#endif
}

/* [] END OF FILE */
