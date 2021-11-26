APPLICATION = app-compression-test

RIOTBASE ?= $(CURDIR)/../..

BOARD ?= native

INCLUDES += -I$(RIOTBASE) -I$(RIOTBASE)/sys/include -I$(RIOTBASE)/core/include -I$(RIOTBASE)/drivers/include 

USEMODULE += compression

CFLAGS += -Wno-unused-function

DEVELHELP ?= 1
QUIET ?= 1

ifneq (,$(filter compression,$(USEMODULE)))
    DIRS += $(RIOTBASE)/iot-data-compression/compression
endif

include $(RIOTBASE)/Makefile.include
include $(RIOTBASE)/iot-data-compression/compression/Makefile.include