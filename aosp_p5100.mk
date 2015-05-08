#
# Copyright 2013 The Android Open-Source Project
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

# Use 44.1 kHz UI sounds
$(call inherit-product-if-exists, frameworks/base/data/sounds/AudioPackage13.mk)

# Inherit device configuration
$(call inherit-product, device/samsung/p5100/full_p5100.mk)

# Device identifier. This must come after all inclusions
PRODUCT_NAME := aosp_p5100

# Set Product Name etc.
PRODUCT_BUILD_PROP_OVERRIDES += \
    PRODUCT_NAME=espresso10rfxx \
    TARGET_DEVICE=espresso10rf

PRODUCT_PACKAGES += \
    Launcher3