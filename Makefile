APPLICATION = app-compression-test

CFLAGS += -DDEVELHELP

QUIET ?= 1

ifneq (,$(filter compression,$(USEMODULE)))
    DIRS += $(RIOTBASE)/iot-data-compression/compression
endif

include $(RIOTBASE)/Makefile.include
include $(RIOTBASE)/iot-data-compression/compression/Makefile.include

