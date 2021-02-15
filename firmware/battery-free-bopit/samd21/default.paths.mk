# This makefile includes paths that might be different between different users.
# It should not be included in VCS commits with values other than the defaults.

# default.paths.mk should be copied to paths.mk and customized

# ASF_PATH should point to the root directory of the Atmel ASF
ASF_PATH = $(HOME)/xdk-asf-3.49.1

# This command searches the home directory for folders starting with xdk-asf- and then sets ASF_PATH
# to the highest version number ASF it finds.
# It might be preferable to explicitly set the ASF_PATH to help developers make sure that they're all
# using the same version.
# ASF_PATH := $(shell find ~ -maxdepth 1 -type d -name "xdk-asf-*" | tail -n1)
# $(info Using $(ASF_PATH) as ASF)
