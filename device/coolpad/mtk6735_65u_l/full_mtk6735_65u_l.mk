# Inherit for devices that support 64-bit primary and 32-bit secondary zygote startup script
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)

# Set target and base project for flavor build
MTK_TARGET_PROJECT := $(subst full_,,$(TARGET_PRODUCT))
MTK_BASE_PROJECT := $(MTK_TARGET_PROJECT)
MTK_PROJECT_FOLDER := $(shell find device/* -maxdepth 1 -name $(MTK_BASE_PROJECT))
MTK_TARGET_PROJECT_FOLDER := $(shell find device/* -maxdepth 1 -name $(MTK_TARGET_PROJECT))

# This is where we'd set a backup provider if we had one
#$(call inherit-product, device/sample/products/backup_overlay.mk)
# Inherit from maguro device
$(call inherit-product, device/coolpad/$(MTK_TARGET_PROJECT)/device.mk)

# set locales & aapt config.
PRODUCT_LOCALES := zh_CN en_US zh_TW es_ES pt_BR ru_RU fr_FR de_DE tr_TR vi_VN ms_MY in_ID th_TH it_IT ar_EG hi_IN bn_IN ur_PK fa_IR pt_PT nl_NL el_GR hu_HU tl_PH ro_RO cs_CZ ko_KR km_KH iw_IL my_MM pl_PL es_US bg_BG hr_HR lv_LV lt_LT sk_SK uk_UA de_AT da_DK fi_FI nb_NO sv_SE en_GB hy_AM zh_HK et_EE ja_JP kk_KZ sr_RS sl_SI ca_ES
#PRODUCT_LOCALES := en_US zh_CN
# Set those variables here to overwrite the inherited values.
PRODUCT_MANUFACTURER := Coolpad
PRODUCT_NAME := full_mtk6735_65u_l
PRODUCT_DEVICE := mtk6735_65u_l
PRODUCT_MODEL := mtk6735_65u_l
PRODUCT_POLICY := android.policy_phone
PRODUCT_BRAND := Coolpad

ifeq ($(TARGET_BUILD_VARIANT), eng)
KERNEL_DEFCONFIG ?= mtk6735_65u_l_debug_defconfig
else
KERNEL_DEFCONFIG ?= mtk6735_65u_l_defconfig
endif
PRELOADER_TARGET_PRODUCT ?= mtk6735_65u_l
LK_PROJECT ?= mtk6735_65u_l
