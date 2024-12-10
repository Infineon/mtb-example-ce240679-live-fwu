/*****************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the Live firmware update Example
*              for ModusToolbox.
*
* Related Document: See README.md
*
*
******************************************************************************
* Copyright 2023-2024, Cypress Semiconductor Corporation (an Infineon company) or
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
******************************************************************************/

/*****************************************************************************
 * Header Files
 *****************************************************************************/

#include "mtb_hal.h"
#include "cybsp.h"
#include "partition_ARMCM33.h"
#include "image_auth.h"
#if defined(MCUBOOT_IMAGE)
#include "psa/crypto.h"
#endif /* MCUBOOT_IMAGE) */
#include "cy_retarget_io.h"
#include "cy_dfu.h"
#include "dfu_user.h"
#include "transport_i2c.h"
#include "cy_dfu_logging.h"
#include "mtb_hal_i2c.h"
#include "cy_scb_i2c.h"
#include "cy_sysint.h"


/*******************************************************************************
 * Macros
 ******************************************************************************/

#define BOOT_ADDR                     (FLASH_SBUS_S_OFFSET + SLOT_OFFSET)

/* Timeout for Cy_DFU_Continue(), in milliseconds */
#define DFU_SESSION_TIMEOUT_MS                     (20u)

/* DFU idle wait timeout: 300 seconds*/
#define DFU_IDLE_TIMEOUT_MS                        (300000u)

/* DFU session timeout: 5 seconds */
#define DFU_COMMAND_TIMEOUT_MS                     (5000u)

#define IMG_CTR_MASK                               (0xFFFF)

#if(UPDATE_IMG)
#define LED_TOGGLE_INTERVAL_MS                     (200u)
#else
#define LED_TOGGLE_INTERVAL_MS                     (1000u)
#endif

/* Function pointer for next application's ResetHandler() */
typedef __NO_RETURN void (*reset_handler_t)(void);

/******************************************************************************
 * Global variables
 *****************************************************************************/
const uint32_t __attribute__((section (".fwctrSection"))) __USED ctr = (0x5A3C0000 | DUAL_BANK_CTR);

/* DFU params, used to configure DFU. */
cy_stc_dfu_params_t dfu_params;

/* For the Retarget -IO (Debug UART) usage */
static cy_stc_scb_uart_context_t    DEBUG_UART_context;           /** UART context */
static mtb_hal_uart_t               DEBUG_UART_hal_obj;           /** Debug UART HAL object  */

/* For DFU I2C interface */
static mtb_hal_i2c_t                dfuI2cHalObj;                 /* I2C transport HAL object  */
static cy_stc_scb_i2c_context_t     dfuI2cContext;                /* I2C transport PDL context structure*/


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

 /*******************************************************************************
 * Function Name: launch_app
 ********************************************************************************
 * Summary:
 * Validates the stack pointer and reset handler of image and launch the image.
 *
 * Parameters:
 *  boot_addr  The start address of image.
 *
 * Return:
 *  Does not return
 *
 *******************************************************************************/
void launch_app(uint32_t boot_addr);

 /*******************************************************************************
 * Function Name: start_app
 ********************************************************************************
 * Summary:
 * Performs the bank switching and transfers the control to Reset vector.
 *
 * Parameters:
 *  sp  The stack pointer of image
 * rst_handler  The reset vector of image.
 *
 * Return:
 *  Does not return
 *
 *******************************************************************************/
void start_app(uint32_t sp, uint32_t rst_handler);

/*******************************************************************************
 * Function Name: dfu_status_in_str
 ********************************************************************************
 * Summary:
 * This is the function to convert DFU status in elaborative text
 *
 * Parameters:
 *  dfu_status
 *
 * Return:
 *  string pointer
 *
 *******************************************************************************/
char* dfu_status_in_str(cy_en_dfu_status_t dfu_status);

/*******************************************************************************
 * Function Name: counter_timeout_seconds
 ********************************************************************************
 * Summary:
 *  Function to return DFU loop count for various target timeout .
 *
 * Parameters:
 *  seconds - time in seconds for particular timeout
 *  timeout - timeout for particular DFU loop
 *
 * Return:
 *  count - Total DFU Loop count value for particular timeout.
 *
 *******************************************************************************/
static uint32_t counter_timeout_seconds(uint32_t seconds, uint32_t timeout);

void dfuI2cIsr(void);

void dfuI2cTransportCallback(cy_en_dfu_transport_i2c_action_t action);


/*******************************************************************************
* Function Definitions
*******************************************************************************/

void launch_app(uint32_t boot_addr)
{
    uint32_t stack_pointer;
    uint32_t reset_handler;

#if defined (MCUBOOT_IMAGE)
    stack_pointer = *(uint32_t*)(boot_addr + ((const struct image_header *)BOOT_ADDR)->ih_hdr_size);
    reset_handler = *(uint32_t*)(boot_addr + ((const struct image_header *)BOOT_ADDR)->ih_hdr_size + 4);
#else
    stack_pointer = *(uint32_t*)boot_addr;
    reset_handler = *(uint32_t*)(boot_addr + 4);
#endif
    start_app(stack_pointer, reset_handler);
}

CY_SECTION_RAMFUNC_BEGIN CY_NOINLINE
void start_app(uint32_t sp, uint32_t rst_handler)
{
    reset_handler_t reset_handler;

    reset_handler = (reset_handler_t)rst_handler;

    __set_MSP(sp);

    /* Toggle the bank mapping bit */
    FLASHC_FLASH_CTL ^= (1 << FLASHC_FLASH_CTL_BANK_MAPPING_Pos);

    /* Invalidate cache and flush pipeline */
    ICACHE0->CMD = ICACHE0->CMD | ICACHE_CMD_INV_Msk;
    /*wait for invalidation complete */
    while(ICACHE0->CMD & ICACHE_CMD_INV_Msk){};
    __ISB();

    reset_handler();
}
CY_SECTION_RAMFUNC_END

void dfuI2cIsr(void)
{
    mtb_hal_i2c_process_interrupt(&dfuI2cHalObj);
}

void dfuI2cTransportCallback(cy_en_dfu_transport_i2c_action_t action)
{
    if (action == CY_DFU_TRANSPORT_I2C_INIT)
    {
        Cy_SCB_I2C_Enable(DFU_I2C_HW);
        CY_DFU_LOG_INF("I2C transport is enabled");
    }
    else if (action == CY_DFU_TRANSPORT_I2C_DEINIT)
    {
        Cy_SCB_I2C_Disable(DFU_I2C_HW, &dfuI2cContext);
        CY_DFU_LOG_INF("I2C transport is disabled");
    }
}

static uint32_t counter_timeout_seconds(uint32_t seconds, uint32_t timeout) {
    uint32_t count = 1;

    if (timeout != 0) {
        count = ((seconds) / timeout);
    }

    return count;
}

char* dfu_status_in_str(cy_en_dfu_status_t dfu_status) {
    switch (dfu_status) {
    case CY_DFU_SUCCESS:
        return "DFU: success";

    case CY_DFU_ERROR_VERIFY:
        return "DFU:Verification failed";

    case CY_DFU_ERROR_LENGTH:
        return "DFU: The length the packet is outside of the expected range";

    case CY_DFU_ERROR_DATA:
        return "DFU: The data in the received packet is invalid";

    case CY_DFU_ERROR_CMD:
        return "DFU: The command is not recognized";

    case CY_DFU_ERROR_CHECKSUM:
        return "DFU: The checksum does not match the expected value ";

    case CY_DFU_ERROR_ADDRESS:
        return "DFU: The wrong address";

    case CY_DFU_ERROR_TIMEOUT:
        return "DFU: The command timed out";

    case CY_DFU_ERROR_BAD_PARAM:
        return "DFU: One or more of input parameters are invalid";

    case CY_DFU_ERROR_UNKNOWN:
        return "DFU: did not recognize error";

    default:
        return "Not recognized DFU status code";
    }
}


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for CPU. It  and launches
*    1. Validates the application
*    2. Launches the validated image
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    int status;

    uint32_t count = 0;
    uint32_t timeout_seconds = 0;

    /* Status codes for DFU API. */
    cy_en_dfu_status_t dfu_status;

    uint32_t dfu_state = CY_DFU_STATE_NONE;

    /* Buffer to store DFU commands. */
    CY_ALIGN(4) static uint8_t dfu_buffer[CY_DFU_SIZEOF_DATA_BUFFER];

    /* Buffer for DFU data packets for transport API. */
    CY_ALIGN(4) static uint8_t dfu_packet[CY_DFU_SIZEOF_CMD_BUFFER];

    cy_en_scb_i2c_status_t pdlI2cStatus;
    cy_en_sysint_status_t  pdlSysIntStatus;

    cy_en_dfu_transport_t dfu_transport = CY_DFU_I2C;

    /* DFU params, used to configure DFU. */
    cy_stc_dfu_params_t dfu_params =
    {
        .timeout = DFU_SESSION_TIMEOUT_MS,
        .dataBuffer = &dfu_buffer[0],
        .packetBuffer = &dfu_packet[0]
    };


    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Debug UART init */
    result = (cy_rslt_t)Cy_SCB_UART_Init(DEBUG_UART_HW, &DEBUG_UART_config, &DEBUG_UART_context);

    /* UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    Cy_SCB_UART_Enable(DEBUG_UART_HW);

    /* Setup the HAL UART */
    result = mtb_hal_uart_setup(&DEBUG_UART_hal_obj, &DEBUG_UART_hal_config, &DEBUG_UART_context, NULL);

    /* HAL UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    result = cy_retarget_io_init(&DEBUG_UART_hal_obj);

    /* HAL retarget_io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

#if defined (MCUBOOT_IMAGE)
    /* Crypto library initialization */
    status = psa_crypto_init();
    if(PSA_SUCCESS != status)
    {
        CY_ASSERT(0);
    }
#endif /* (MCUBOOT_IMAGE == TRUE) */

    printf("\x1b[2J\x1b[;H");

    printf("****************** "
           "PSOC Control MCU: DFU Live Firmware Update "
           "****************** \r\n\n");

    printf("Image counter is - %ld\r\n", (unsigned long)(ctr & IMG_CTR_MASK));

    pdlI2cStatus = Cy_SCB_I2C_Init(DFU_I2C_HW, &DFU_I2C_config, &dfuI2cContext);
    if (CY_SCB_I2C_SUCCESS != pdlI2cStatus)
    {
        CY_DFU_LOG_ERR("Error during I2C PDL initialization. Status: %X", pdlI2cStatus);
        CY_ASSERT(0);
    }

    result = mtb_hal_i2c_setup(&dfuI2cHalObj, &DFU_I2C_hal_config, &dfuI2cContext, NULL);
    if (CY_RSLT_SUCCESS != result)
    {
        CY_DFU_LOG_ERR("Error during I2C HAL initialization. Status: %lX", (unsigned long)result);
    }
    else
    {
        cy_stc_sysint_t i2cIsrCfg =
        {
            .intrSrc = DFU_I2C_IRQ,
            .intrPriority = 3U
        };
        pdlSysIntStatus = Cy_SysInt_Init(&i2cIsrCfg, dfuI2cIsr);
        if (CY_SYSINT_SUCCESS != pdlSysIntStatus)
        {
            CY_DFU_LOG_ERR("Error during I2C Interrupt initialization. Status: %X", pdlSysIntStatus);
        }
        else
        {
            NVIC_EnableIRQ((IRQn_Type) i2cIsrCfg.intrSrc);
            CY_DFU_LOG_INF("I2C transport is initialized");
        }
    }

    cy_stc_dfu_transport_i2c_cfg_t i2cTransportCfg =
    {
        .i2c = &dfuI2cHalObj,
        .callback = dfuI2cTransportCallback,
    };
    Cy_DFU_TransportI2cConfig(&i2cTransportCfg);

    /* Initialize DFU Structure. */
    dfu_status = Cy_DFU_Init(&dfu_state, &dfu_params);
    if (CY_DFU_SUCCESS != dfu_status)
    {
        printf("DFU initialization failed \r\n");
        CY_ASSERT(0);
    }

    /* Initialize DFU communication. */
    Cy_DFU_TransportStart(dfu_transport);

    printf("\r\nSTARTING DFU \r\n ");


    for (;;)
    {
        dfu_status = Cy_DFU_Continue(&dfu_state, &dfu_params);
        count++;
        if (CY_DFU_STATE_FINISHED == dfu_state)
        {
            count = 0u;
            if (CY_DFU_SUCCESS == dfu_status)
            {
                printf("\r\nAuthenticating  Application\r\n");

                /* Validate image */
                status = validate_image(BOOT_ADDR);

                if (status != 0)
                {
                    CY_ASSERT(0);
                }

                Cy_DFU_TransportStop();
                printf("Image Authentication successful\r\n");
                printf("Launching new firmware\r\n");
                cy_retarget_io_deinit();

                /* Launch validated image */
                launch_app(BOOT_ADDR);
            }
            else
            {
                  Cy_DFU_Init(&dfu_state, &dfu_params);
                  printf("DFU_STATE_FINISHED: %s \r\n",
                                      dfu_status_in_str(dfu_status));
            }
        }
        else if (CY_DFU_STATE_FAILED == dfu_state)
        {
            count = 0u;
            Cy_DFU_Init(&dfu_state, &dfu_params);
            printf("DFU_STATE_FAILED: %s \r\n", dfu_status_in_str(dfu_status));
        }
        else if (dfu_state == CY_DFU_STATE_UPDATING)
        {
            timeout_seconds = (count >= counter_timeout_seconds(DFU_COMMAND_TIMEOUT_MS, DFU_SESSION_TIMEOUT_MS)) ? 1U : 0u;

            /* if no command has been received during 5 seconds when the loading
             * has started then restart loading.
             */
            if (dfu_status == CY_DFU_SUCCESS)
            {
                count = 0u;
            }
            else if (dfu_status == CY_DFU_ERROR_TIMEOUT)
            {
                if (timeout_seconds != 0u)
                {
                    count = 0u;

                  /* Restart DFU. */
                }
            }
            else
            {
                count = 0u;

                /* Delay because Transport still may be sending error response to a host. */
                Cy_SysLib_Delay(DFU_SESSION_TIMEOUT_MS);

                /* Restart DFU. */
             }
        }

        /* Blink LED */
        if ((count % counter_timeout_seconds(LED_TOGGLE_INTERVAL_MS, DFU_SESSION_TIMEOUT_MS)) == 0u)
        {
            /* Invert the USER LED state */
            Cy_GPIO_Inv(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_PIN);

        }

        if ((count >= counter_timeout_seconds(DFU_IDLE_TIMEOUT_MS, DFU_SESSION_TIMEOUT_MS)) && (dfu_state == CY_DFU_STATE_NONE))
        {
            /* In case, no valid user application, lets start fresh all over.
             * This is just for demonstration.
             * Final application can change it to either assert, reboot, enter low power mode etc,
             * based on usecase requirements.
             */
            count = 0;
        }
          Cy_SysLib_Delay(1);
    }
}
/* [] END OF FILE */
