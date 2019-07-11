SUMMARY = "Base image for Fox7"
LICENSE = "none"

inherit core-image

DEPENDS += "bcm2835-bootfiles"

IMAGE_INSTALL += " \
	kernel-modules \
"

SPLASH = ""

IMAGE_FEATURES += " ssh-server-openssh"

IMAGE_INSTALL += " 	roscpp-tutorials \
					roslaunch \
					rostopic \
"

# ENABLE RPI UART
ENABLE_UART = "1"

# ENABLE RPI I2C
ENABLE_I2C = "1"

DISABLE_RPI_BOOT_LOGO = "1"

export IMAGE_BASENAME = "foxette"
