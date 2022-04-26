APPLICATION = Individuale

BOARD ?= nucleo-f401re

RIOTBASE ?= $(CURDIR)/RIOT

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for IPv6
USEMODULE += gnrc_ipv6_default
# Include MQTT-SN
USEMODULE += emcute
# Add also the shell, some shell commands
USEMODULE += shell_commands

USEMODULE += periph_gpio
USEMODULE += xtimer
USEMODULE += periph_gpio_irq


USEMODULE += servo

USEMODULE += stdio_ethos gnrc_uhcpc

IPV6_PREFIX ?= fe80:2::/64
STATIC_ROUTES ?= 1

UPLINK ?= ethos

ETHOS_BAUDRATE ?= 115200
CFLAGS += -DETHOS_BAUDRATE=$(ETHOS_BAUDRATE)

TAP ?= tap0

host-tools:
	$(Q)env -u CC -u CFLAGS $(MAKE) -C $(RIOTTOOLS)

# Configure terminal parameters
TERMDEPS += host-tools
TERMPROG ?= sudo sh $(RIOTTOOLS)/ethos/start_network.sh
TERMFLAGS ?= $(FLAGS_EXTRAS) $(PORT) $(TAP) $(IPV6_PREFIX) $(ETHOS_BAUDRATE)

#CFLAGS += -DCONFIG_GNRC_NETIF_IPV6_ADDRS_NUMOF=3
CFLAGS += -DCONFIG_GNRC_NETIF_IPV6_ADDRS_NUMOF=5


FEATURES_REQUIRED = periph_gpio
FEATURES_REQUIRED = periph_adc

include $(RIOTBASE)/Makefile.include
