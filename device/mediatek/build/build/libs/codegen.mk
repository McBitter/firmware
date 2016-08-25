
MTK_CGEN_MAKEFILE := $(lastword $(MAKEFILE_LIST))

ifndef MTK_CGEN_MAKE_ALONE
  ifeq ($(words $(MAKEFILE_LIST)),1)
    # make -f codegen.mk
    MTK_CGEN_MAKE_ALONE := true
  else
    # included from makemtk.mk
    MTK_CGEN_MAKE_ALONE := false
  endif
endif

ifeq ($(MTK_CGEN_MAKE_ALONE), true)
  hide ?= @
  OUT_DIR ?= out
  LOGDIR ?= out/target/product
 ifeq (AOSP,AOSP)
  PROJECT ?= $(TARGET_DEVICE)
  MTK_PROJECT_NAME := $(subst full_,,$(TARGET_DEVICE))
  MTK_PROJECT_FOLDER := $(shell find device/* -maxdepth 1 -name $(MTK_PROJECT_NAME))
  PRJ_MF := $(MTK_PROJECT_FOLDER)/ProjectConfig.mk
  ###PRJ_MF := device/mediatek/$(TARGET_DEVICE)/ProjectConfig.mk
  include $(MTK_PROJECT_FOLDER)/full_$(TARGET_DEVICE).mk
  include $(PRJ_MF)
  include device/mediatek/build/build/Makefile
  ifeq ($(strip $(MTK_BASE_PROJECT)),)
    MTK_PROJECT := $(TARGET_DEVICE)
  else
    MTK_PROJECT := $(MTK_BASE_PROJECT)
  endif
  MTK_BASE_PROJECT_FOLDER := $(shell find device/* -maxdepth 1 -name $(MTK_PROJECT))
  MTK_CGEN_COMPILE_OPTION := $(call mtk.custom.generate-macros)
  MTK_ROOT_BUILD := device/mediatek/build/build
 else
  include mediatek/build/Makefile
  $(call codebase-path)
 endif
  LOG ?= $(LOGDIR)/$(PROJECT)_
  SHOWRSLT ?= /usr/bin/perl $(MTK_ROOT_BUILD)/tools/showRslt.pl
  include $(MTK_ROOT_BUILD)/libs/pack_dep_gen.mk
endif

MTK_INCLUDE_CHECK_VARIABLE := PROJECT LOGDIR LOG PRJ_MF SHOWRSLT MTK_PROJECT MTK_PLATFORM MTK_CHIP_VER MTK_BRANCH MTK_WEEK_NO MTK_DEPENDENCY_OUTPUT
ifeq (AOSP,AOSP)
MTK_INCLUDE_CHECK_VARIABLE += TARGET_DEVICE
endif
MTK_PREGEN_TEMP := $(strip $(foreach v,$(MTK_INCLUDE_CHECK_VARIABLE),$(if $($(v)),,$(v))))
ifneq ($(MTK_PREGEN_TEMP),)
  $(warning *** These variables are not defined:)
  $(warning *** $(MTK_PREGEN_TEMP))
  $(error Please check makefile inclusion)
endif

S_CODEGEN_LOG := out/target/product/$(PROJECT)_codegen.log
CODEGEN_LOG := $(LOGDIR)/$(PROJECT)_codegen.log
ifeq ($(ENABLE_TEE), TRUE)
  DEAL_STDOUT_CODEGEN := 2>&1 | tee -a $(CODEGEN_LOG) && $(SHOWRSLT) $${PIPESTATUS[0]} $(CODEGEN_LOG) || $(SHOWRSLT) $${PIPESTATUS[0]} $(CODEGEN_LOG)
  DEAL_STDOUT_BTCODEGEN := 2>&1 | tee -a $(LOG)btcodegen.log && $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)btcodegen.log || $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)btcodegen.log
else
  DEAL_STDOUT_CODEGEN := > $(CODEGEN_LOG) 2>&1 && $(SHOWRSLT) $${PIPESTATUS[0]} $(CODEGEN_LOG) || $(SHOWRSLT) $${PIPESTATUS[0]} $(CODEGEN_LOG)
  DEAL_STDOUT_BTCODEGEN := > $(LOG)btcodegen.log 2>&1 $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)btcodegen.log || $(SHOWRSLT) $${PIPESTATUS[0]} $(LOG)btcodegen.log
endif
ifeq (AOSP,AOSP)
  DEAL_STDOUT_CODEGEN :=
  DEAL_STDOUT_BTCODEGEN :=
endif


ifeq (AOSP,AOSP)
MTK_CGEN_BIN_DIR := vendor/mediatek/proprietary/cgen
MTK_CGEN_OUT_DIR := device/mediatek/build/cgen/$(TARGET_DEVICE)
MTK_CGEN_CUSTOM_DIR := vendor/mediatek/proprietary/custom
MTK_CGEN_BUILD_VERNO_CONFIG := ${PRJ_MF}
else
MTK_CGEN_BIN_DIR := mediatek/cgen
MTK_CGEN_OUT_DIR := mediatek/cgen
MTK_CGEN_CUSTOM_DIR := mediatek/custom
MTK_CGEN_BUILD_VERNO_CONFIG := mediatek/config/common/ProjectConfig.mk
endif
MTK_CGEN_CUSTOM_COMMON_DIR := $(MTK_CGEN_CUSTOM_DIR)/common/cgen
MTK_CGEN_APDB_SourceFile := $(MTK_CGEN_OUT_DIR)/apeditor/app_parse_db.c
MTK_CGEN_AP_Temp_CL := $(MTK_CGEN_OUT_DIR)/apeditor/app_temp_db

MTK_CGEN_Err_file	:= $(MTK_CGEN_OUT_DIR)/Cgen_db.log
MTK_CGEN_Log_file	:= $(MTK_CGEN_OUT_DIR)/Cgen_db.err
MTK_CGEN_TARGET_CFG	:= $(MTK_CGEN_BIN_DIR)/cgencfg/tgt_cnf
MTK_CGEN_HOST_CFG	:= $(MTK_CGEN_BIN_DIR)/cgencfg/pc_cnf
MTK_CGEN_EXECUTABLE	:= $(MTK_CGEN_BIN_DIR)/Cgen

MTK_CGEN_cfg_module_file_dir	:= $(MTK_CGEN_CUSTOM_COMMON_DIR)/cfgfileinc
MTK_CGEN_cfg_module_default_dir	:= $(MTK_CGEN_CUSTOM_COMMON_DIR)/cfgdefault
MTK_CGEN_custom_cfg_module_file_dir	:= $(MTK_CGEN_CUSTOM_DIR)/$(strip $(MTK_PROJECT))/cgen/cfgfileinc
MTK_CGEN_custom_cfg_default_file_dir	:= $(MTK_CGEN_CUSTOM_DIR)/$(strip $(MTK_PROJECT))/cgen/cfgdefault

ifeq (AOSP,AOSP)
MTK_CGEN_cfg_module_file	:= $(MTK_CGEN_OUT_DIR)/inc/cfg_module_file.h
MTK_CGEN_cfg_module_default	:= $(MTK_CGEN_OUT_DIR)/inc/cfg_module_default.h
MTK_CGEN_custom_cfg_module_file		:= $(MTK_CGEN_OUT_DIR)/inc/custom_cfg_module_file.h
MTK_CGEN_custom_cfg_default_file	:= $(MTK_CGEN_OUT_DIR)/inc/custom_cfg_module_default.h
MTK_CGEN_COMPILE_OPTION += -I$(MTK_CGEN_OUT_DIR)/inc -I$(MTK_CGEN_BIN_DIR)/apeditor
else
MTK_CGEN_cfg_module_file	:= $(MTK_CGEN_CUSTOM_COMMON_DIR)/inc/cfg_module_file.h
MTK_CGEN_cfg_module_default	:= $(MTK_CGEN_CUSTOM_COMMON_DIR)/inc/cfg_module_default.h
MTK_CGEN_custom_cfg_module_file		:= $(MTK_CGEN_CUSTOM_DIR)/$(strip $(MTK_PROJECT))/cgen/inc/custom_cfg_module_file.h
MTK_CGEN_custom_cfg_default_file	:= $(MTK_CGEN_CUSTOM_DIR)/$(strip $(MTK_PROJECT))/cgen/inc/custom_cfg_module_default.h
MTK_CGEN_COMPILE_OPTION := $(call mtk.custom.generate-macros)
endif

MTK_CGEN_AP_Editor_DB		:= $(MTK_CGEN_OUT_DIR)/APDB_$(MTK_PLATFORM)_$(MTK_CHIP_VER)_$(MTK_BRANCH)_$(MTK_WEEK_NO)
MTK_CGEN_AP_Editor2_Temp_DB	:= $(MTK_CGEN_OUT_DIR)/.temp_APDB_$(MTK_PLATFORM)_$(MTK_CHIP_VER)_$(MTK_BRANCH)_$(MTK_WEEK_NO)
MTK_CGEN_AP_Editor_DB_Enum_File	:= $(MTK_CGEN_OUT_DIR)/APDB_$(MTK_PLATFORM)_$(MTK_CHIP_VER)_$(MTK_BRANCH)_$(MTK_WEEK_NO)_ENUM

MTK_CGEN_hardWareVersion := $(MTK_PLATFORM)_$(MTK_CHIP_VER)
MTK_CGEN_SWVersion := $(shell cat $(MTK_CGEN_BUILD_VERNO_CONFIG) | grep "^\s*MTK_BUILD_VERNO" | sed 's/.*\s*=\s*//g')
ifeq ($(strip $(MTK_CGEN_SWVersion)),)
  MTK_CGEN_SWVersion := $(MTK_BUILD_VERNO)
endif

ifneq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
.PHONY: $(MTK_CGEN_APDB_SourceFile)
endif
$(MTK_CGEN_APDB_SourceFile): $(MTK_CGEN_MAKEFILE)
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) if [ -f $@ ]; then rm -f $@; fi
	$(hide) mkdir -p $(dir $@)
	$(hide) echo \#include \"tst_assert_header_file.h\"	>$@
	$(hide) echo \#include \"ap_editor_data_item.h\"	>>$@
	$(hide) echo \#include \"Custom_NvRam_data_item.h\"	>>$@

# $(1): $dir_path, the searched directory
# $(2): $fileName, the generated header file name
# $(3): $out_dir
define mtk-cgen-AutoGenHeaderFile
#PRIVATE_CGEN_DEPENDENCY := $$(wildcard $(1)/*.h)
#-include $$(MTK_DEPENDENCY_OUTPUT)/$$(basename $$(notdir $(2))).dep
#$(eval $(call mtk-check-argument,$$(basename $$(notdir $(2))),$(MTK_DEPENDENCY_OUTPUT),$(PRIVATE_CGEN_DEPENDENCY)))
ifneq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
.PHONY: $(2)
endif
$(2): $$(MTK_CGEN_MAKEFILE) $$(wildcard $(1)/*.h) $(4)
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $$@: $$?
endif
	$(hide) if [ -f $$@ ]; then rm -f $$@; fi;
	$(hide) mkdir -p $$(dir $$@)
	$(hide) for x in $$(wildcard $(1)/*.h); do echo "#include \"$(3)$$$$x\"" >>$$@; done;
	$(hide) for x in $(4); do echo "#include \"$$$$x\"" >>$$@; done;
endef

define mtk-cgen-PreprocessFile
-include $$(MTK_DEPENDENCY_OUTPUT)/$$(basename $$(notdir $(1))).dep
ifneq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
.PHONY: $(2)
endif
$(2): $(1) $$(MTK_CGEN_MAKEFILE)
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $$@: $$?
endif
	@mkdir -p $$(dir $(2))
	$(hide) gcc $(3) -E $(1) -o $(2) -MD -MF $$(basename $(2)).d $(4)
	$(hide) perl $$(MTK_DEPENDENCY_SCRIPT) $$(MTK_DEPENDENCY_OUTPUT)/$$(basename $$(notdir $(1))).dep $$@ $$(dir $$@) "\b$$(basename $$(notdir $$@))\.d"
	$(hide) rm $$(basename $(2)).d
endef


$(eval $(call mtk-cgen-AutoGenHeaderFile,$(MTK_CGEN_cfg_module_file_dir),$(MTK_CGEN_cfg_module_file)))
$(eval $(call mtk-cgen-AutoGenHeaderFile,$(MTK_CGEN_cfg_module_default_dir),$(MTK_CGEN_cfg_module_default)))

$(eval $(call mtk-cgen-AutoGenHeaderFile,$(MTK_CGEN_custom_cfg_module_file_dir),$(MTK_CGEN_custom_cfg_module_file)))
$(eval $(call mtk-cgen-AutoGenHeaderFile,$(MTK_CGEN_custom_cfg_default_file_dir),$(MTK_CGEN_custom_cfg_default_file)))

$(eval $(call mtk-cgen-PreprocessFile,$(MTK_CGEN_APDB_SourceFile),$(MTK_CGEN_AP_Temp_CL),$(MTK_CGEN_COMPILE_OPTION) -I. -I$(MTK_CGEN_CUSTOM_DIR)/$(strip $(MTK_PROJECT))/cgen/inc -I$(MTK_CGEN_CUSTOM_COMMON_DIR)/inc,$$(DEAL_STDOUT_CODEGEN)))

$(MTK_CGEN_AP_Temp_CL): $(MTK_CGEN_cfg_module_file)
$(MTK_CGEN_AP_Temp_CL): $(MTK_CGEN_cfg_module_default)
$(MTK_CGEN_AP_Temp_CL): $(MTK_CGEN_custom_cfg_module_file)
$(MTK_CGEN_AP_Temp_CL): $(MTK_CGEN_custom_cfg_default_file)

ifneq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
.PHONY: $(MTK_CGEN_AP_Editor2_Temp_DB)
endif
$(MTK_CGEN_AP_Editor2_Temp_DB): $(MTK_CGEN_EXECUTABLE) $(MTK_CGEN_TARGET_CFG) $(MTK_CGEN_HOST_CFG) $(MTK_CGEN_AP_Temp_CL)
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	@echo $(SHOWTIME) codegening ...
	@echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)codegen.log
	$(hide) rm -f $(CODEGEN_LOG) $(CODEGEN_LOG)_err
	$(hide) $(MTK_CGEN_EXECUTABLE) -c $(MTK_CGEN_AP_Temp_CL) $(MTK_CGEN_TARGET_CFG) $(MTK_CGEN_HOST_CFG) $(MTK_CGEN_AP_Editor2_Temp_DB) $(MTK_CGEN_AP_Editor_DB_Enum_File) $(MTK_CGEN_hardWareVersion) $(MTK_CGEN_SWVersion) $(DEAL_STDOUT_CODEGEN)

ifneq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
.PHONY: $(MTK_CGEN_AP_Editor_DB)
endif
$(MTK_CGEN_AP_Editor_DB): $(MTK_CGEN_EXECUTABLE) $(MTK_CGEN_AP_Editor2_Temp_DB) $(MTK_CGEN_AP_Temp_CL)
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) $(MTK_CGEN_EXECUTABLE) -cm $(MTK_CGEN_AP_Editor_DB) $(MTK_CGEN_AP_Editor2_Temp_DB) $(MTK_CGEN_AP_Temp_CL) $(MTK_CGEN_AP_Editor_DB_Enum_File) $(MTK_CGEN_hardWareVersion) $(MTK_CGEN_SWVersion) $(DEAL_STDOUT_CODEGEN)

.PHONY: cgen
cgen:
ifneq ($(PROJECT),generic)
cgen: $(MTK_CGEN_AP_Editor_DB)
endif


ifeq (AOSP,AOSP)
MTK_BTCODEGEN_ROOT := vendor/mediatek/proprietary/protect/external/bluetooth/blueangel/_bt_scripts
MTK_BTCODEGEN_OUT_DIR := $(MTK_CGEN_OUT_DIR)
MTK_BTCODEGEN_catcher_ps_db_path := $(MTK_CGEN_OUT_DIR)/database/BTCatacherDB
else
MTK_BTCODEGEN_ROOT := mediatek/protect/external/bluetooth/blueangel/_bt_scripts
MTK_BTCODEGEN_OUT_DIR := $(MTK_BTCODEGEN_ROOT)
MTK_BTCODEGEN_catcher_ps_db_path := mediatek/external/bluetooth/database/BTCatacherDB
endif
MTK_BTCODEGEN_dbfolder := $(MTK_CGEN_OUT_DIR)/database
MTK_BTCODEGEN_parsedb := $(MTK_BTCODEGEN_ROOT)/database/parse_db.c
MTK_BTCODEGEN_pridb := $(MTK_BTCODEGEN_dbfolder)/msglog_db/parse.db
MTK_BTCODEGEN_pstrace_db_path := $(MTK_BTCODEGEN_dbfolder)/pstrace_db
MTK_BTCODEGEN_catcher_db_folder := $(MTK_CGEN_OUT_DIR)/database_win32
MTK_BTCODEGEN_ps_trace_h_catcher_path := $(MTK_BTCODEGEN_catcher_db_folder)/ps_trace.h
MTK_BTCODEGEN_ps_trace_file_list := $(MTK_BTCODEGEN_ROOT)/settings/ps_trace_file_list.txt
MTK_BTCODEGEN_pc_cnf := $(MTK_BTCODEGEN_ROOT)/database/Pc_Cnf
MTK_BTCODEGEN_tgt_cnf :=
MTK_BTCODEGEN_version := 1.0
MTK_BTCODEGEN_verno := W0949
ifneq ($(wildcard $(MTK_BTCODEGEN_ROOT)),)
  MTK_BTCODEGEN_TraceListAry := $(shell perl -p -e "s/\#.*$$//g" $(MTK_BTCODEGEN_ps_trace_file_list))
  MTK_BTCODEGEN_Options_ini := $(shell cat $(MTK_BTCODEGEN_ROOT)/settings/GCC_Options.ini)
endif
MTK_BTCODEGEN_GCC_Options := $(foreach item,$(MTK_BTCODEGEN_Options_ini),$(patsubst -I%,-I$(MTK_BTCODEGEN_ROOT)/%,$(item)))


$(eval $(call mtk-cgen-PreprocessFile,$(MTK_BTCODEGEN_parsedb),$(MTK_BTCODEGEN_pridb),$(MTK_CGEN_COMPILE_OPTION) $(MTK_BTCODEGEN_GCC_Options) -DGEN_FOR_CPARSER -DGEN_FOR_PC,$$(DEAL_STDOUT_BTCODEGEN)))

ifneq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
.PHONY: $(MTK_BTCODEGEN_dbfolder)/BPGUInfo
endif
$(MTK_BTCODEGEN_dbfolder)/BPGUInfo: $(MTK_BTCODEGEN_pridb)
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	@echo $(SHOWTIME) btcodegening ...
	@echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)btcodegen.log
	$(hide) rm -f $(LOG)btcodegen.log $(LOG)btcodegen.log_err
	$(hide) $(MTK_CGEN_EXECUTABLE) -c $(MTK_BTCODEGEN_pridb) $(MTK_CGEN_TARGET_CFG) $(MTK_CGEN_HOST_CFG) $(MTK_BTCODEGEN_dbfolder)/BPGUInfo $(MTK_BTCODEGEN_dbfolder)/enumFile MoDIS $(MTK_BTCODEGEN_verno) $(DEAL_STDOUT_BTCODEGEN)

MTK_BTCODEGEN_PTR_LIST :=
$(foreach file,$(MTK_BTCODEGEN_TraceListAry),\
  $(eval $(call mtk-cgen-PreprocessFile,$(MTK_BTCODEGEN_ROOT)/$(file),$(MTK_BTCODEGEN_pstrace_db_path)/$(notdir $(basename $(file))).ptr,$(MTK_CGEN_COMPILE_OPTION) $(MTK_BTCODEGEN_GCC_Options) -DGEN_FOR_PC,$$(DEAL_STDOUT_BTCODEGEN))) \
  $(eval MTK_BTCODEGEN_PTR_LIST += $(MTK_BTCODEGEN_pstrace_db_path)/$(notdir $(basename $(file))).ptr) \
)

ifneq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
.PHONY: $(MTK_BTCODEGEN_catcher_ps_db_path)
endif
$(MTK_BTCODEGEN_catcher_ps_db_path): $(MTK_BTCODEGEN_dbfolder)/BPGUInfo $(MTK_BTCODEGEN_PTR_LIST)
ifeq ($(MTK_DEPENDENCY_AUTO_CHECK), true)
	-@echo [Update] $@: $?
endif
	$(hide) if [ -f $(MTK_BTCODEGEN_catcher_ps_db_path) ]; then rm -f $(MTK_BTCODEGEN_catcher_ps_db_path); fi
	$(hide) mkdir -p $(dir $(MTK_BTCODEGEN_ps_trace_h_catcher_path)) $(dir $(MTK_BTCODEGEN_catcher_ps_db_path))
	$(hide) $(MTK_CGEN_EXECUTABLE) -ps $(MTK_BTCODEGEN_catcher_ps_db_path) $(MTK_BTCODEGEN_dbfolder)/BPGUInfo $(MTK_BTCODEGEN_pstrace_db_path) $(MTK_BTCODEGEN_ps_trace_h_catcher_path) $(DEAL_STDOUT_BTCODEGEN)

.PHONY: btcodegen
btcodegen:
ifneq ($(PROJECT),generic)
  ifneq ($(MTK_BTCODEGEN_SUPPORT),no)
    ifeq ($(MTK_BT_SUPPORT), yes)
      ifneq ($(wildcard $(MTK_BTCODEGEN_ROOT)/BTCodegen.pl),)
btcodegen: $(MTK_BTCODEGEN_catcher_ps_db_path)
      else # partial source building
btcodegen:
	@echo BT database auto-gen process disabled due to BT_DB_AUTO_GEN_SCRIPTS_PATH is not exist.
      endif
    else
btcodegen:
	@echo BT database auto-gen process disabled due to Bluetooth is turned off.
    endif
  endif
endif

