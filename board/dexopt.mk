ifeq ($(HOST_OS),linux)
    ifeq ($(TARGET_BUILD_VARIANT),user)
        ifeq ($(WITH_DEXPREOPT),)
            WITH_DEXPREOPT := true
        endif
    endif
endif
WITH_DEXPREOPT_BOOT_IMG_AND_SYSTEM_SERVER_ONLY ?= true
