#
# Copyright (C) 2012 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Include common espresso BoardConfig
include device/samsung/espresso/BoardConfigCommon.mk

# assert
TARGET_OTA_ASSERT_DEVICE := espresso3g,espresso-common,p3100,GT-P3100,espressorf,espressorfxx,p5100,GT-P5100,espresso10rf,espresso10rfxx

# SELinux
BOARD_SEPOLICY_DIRS += \
    device/samsung/espresso3g/sepolicy

# libril
BOARD_PROVIDES_LIBRIL := true
