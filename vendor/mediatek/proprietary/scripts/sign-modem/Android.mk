#
# Copyright (C) 2009-2011 The Android-x86 Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#

ifneq ($(strip $(MTK_PROJECT)),)

sign-modem: 
	$(hide) ./vendor/mediatek/proprietary/scripts/sign-modem/sign_modem.pl
	$(hide) make -j24 snod
	
endif
