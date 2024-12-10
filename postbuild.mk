#This file cofngures the MCU Boot Image header/signature

COMPILED_HEX=$(MTB_TOOLS__OUTPUT_CONFIG_DIR)/$(APPNAME)

#The user application file
INPUT_IMAGE?=$(COMPILED_HEX)

# Output application image
OUTPUT_IMAGE?=$(COMPILED_HEX)

ifeq ($(IMG_TYPE),BOOT)
APP_START=0x32000000
else ifeq ($(IMG_TYPE),UPDATE)
APP_START=0x32800000
endif #($(IMG_TYPE),BOOT)

ifeq ($(SECURED_BOOT),TRUE)

#The value, which is read back from erased flash. Default: 0
ERASED_VAL?=0
ifeq ($(IMG_TYPE),BOOT)
IMAGE_VERSION=1.0.0
else ifeq ($(IMG_TYPE),UPDATE)
IMAGE_VERSION?=1.1.0
endif #($(IMG_TYPE),BOOT)

SLOT_SIZE=0x20000

KEY_PATH=./keys/oem_rot_priv_key_0.pem

#Setting up the Additional Arguments
ADDITIONAL_ARGS?=--align 1 -s 0 --public-key-format full --pubkey-encoding raw --signature-encoding raw --min-erase-size 0x200 --overwrite-only

#Signing the image for secured boot
POSTBUILD+=edgeprotecttools sign-image --image $(INPUT_IMAGE).bin \
                                           --output $(OUTPUT_IMAGE).hex \
                                           --erased-val $(ERASED_VAL) \
                                           --header-size $(MCUBOOT_HDR_OFFSET) \
                                           --hex-addr $(APP_START) \
                                           --slot-size $(SLOT_SIZE) \
                                           --key $(KEY_PATH) \
                                           --image-version $(IMAGE_VERSION) \
                                           $(ADDITIONAL_ARGS);

POSTBUILD+=cp $(OUTPUT_IMAGE).hex ./build/last_config/$(APPNAME).hex;
else # ($(SECURED_BOOT),TRUE)
POSTBUILD+=edgeprotecttools bin2hex --image $(INPUT_IMAGE).bin \
                                    --output $(OUTPUT_IMAGE).hex \
                                    --offset $(APP_START);

POSTBUILD+=cp $(OUTPUT_IMAGE).hex ./build/last_config/$(APPNAME).hex;
endif # ($(SECURED_BOOT),TRUE)