# CNE
ifneq ($(QCPATH),)
BOARD_USES_QCNE := true
TARGET_LDPRELOAD := libNimsWrap.so
endif
