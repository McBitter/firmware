LOCAL_PATH:= $(call my-dir)

common_src_files := \
	VolumeManager.cpp \
	CommandListener.cpp \
	VoldCommand.cpp \
	NetlinkManager.cpp \
	NetlinkHandler.cpp \
	Volume.cpp \
	DirectVolume.cpp \
	Process.cpp \
	Ext4.cpp \
	Fat.cpp \
	Loop.cpp \
	Devmapper.cpp \
	ResponseCode.cpp \
	CheckBattery.cpp \
	VoldUtil.c \
	fstrim.c \
	cryptfs.c \
	Bicr.cpp

common_c_includes := \
	system/extras/ext4_utils \
	system/extras/f2fs_utils \
	external/openssl/include \
	external/stlport/stlport \
	bionic \
	external/scrypt/lib/crypto \
	frameworks/native/include \
	system/security/keystore \
	hardware/libhardware/include/hardware \
	system/security/softkeymaster/include/keymaster

#ifneq ($(MTK_EMULATOR_SUPPORT),yes)
	common_c_includes += $(MTK_ROOT)/external/nvram/libnvram \

#endif

common_shared_libraries := \
	libsysutils \
	libstlport \
	libbinder \
	libcutils \
	liblog \
	libdiskconfig \
	libhardware_legacy \
	liblogwrap \
	libext4_utils \
	libf2fs_sparseblock \
	libcrypto \
	libselinux \
	libutils \
	libhardware \
	libsoftkeymaster

common_static_libraries := \
	libfs_mgr \
	libscrypt_static \
	libmincrypt \
	libbatteryservice

ifneq ($(MTK_EMULATOR_SUPPORT),yes)
	common_shared_libraries += 	libcustom_nvram \
	                            libnvram \
                                libfile_op
endif

include $(CLEAR_VARS)

LOCAL_MODULE := libvold

LOCAL_SRC_FILES := $(common_src_files)

LOCAL_C_INCLUDES := $(common_c_includes)

LOCAL_SHARED_LIBRARIES := $(common_shared_libraries)

LOCAL_STATIC_LIBRARIES := $(common_static_libraries)

LOCAL_MODULE_TAGS := eng tests

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE:= vold

LOCAL_SRC_FILES := \
	main.cpp \
	$(common_src_files)

LOCAL_C_INCLUDES := $(common_c_includes)

LOCAL_CFLAGS := -Werror=format

ifeq ($(TARGET_HW_DISK_ENCRYPTION),true)
LOCAL_C_INCLUDES += $(TARGET_CRYPTFS_HW_PATH)
common_shared_libraries += libcryptfs_hw
LOCAL_CFLAGS += -DCONFIG_HW_DISK_ENCRYPTION
endif

LOCAL_SHARED_LIBRARIES := $(common_shared_libraries)

LOCAL_STATIC_LIBRARIES := $(common_static_libraries)

ifeq ($(TARGET_BUILD_VARIANT),eng)
 LOCAL_CFLAGS += -DENG_BUILD_ENG
endif

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= vdc.c

LOCAL_MODULE:= vdc

LOCAL_C_INCLUDES :=

LOCAL_CFLAGS := 

ifeq ($(MTK_BICR_SUPPORT),yes)
	LOCAL_CFLAGS += -DMTK_BICR_SUPPORT
endif

ifeq ($(TARGET_BUILD_VARIANT),eng)
 LOCAL_CFLAGS += -DENG_BUILD_ENG
endif


LOCAL_SHARED_LIBRARIES := libcutils

include $(BUILD_EXECUTABLE)
