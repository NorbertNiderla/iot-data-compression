# name of your application
APPLICATION = compression_iot

# If no BOARD is found in the environment, use this default:
BOARD ?= native

INCLUDES += -I$(RIOTBASE)/compression_iot/include

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/..

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

USEMODULE += xtimer
CFLAGS += -fsingle-precision-constant
CFLAGS += -Wno-unused-function 

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include