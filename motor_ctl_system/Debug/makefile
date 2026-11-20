#
# This file was created by Xilinx Vitis tool.
#


#
# tools
#

RM = rm -f
RMDIR = rm -rf



#
# variables
#

SDCARD_GEN = sdcard_gen
GENERATE_BIF = ::scw::generate_bif

XPFM_PATH = /home/ubuntu-z5/proj/motor_uart_00/bare_zynq_proc/export/bare_zynq_proc/bare_zynq_proc.xpfm
SYS_CONFIG = bare_zynq_proc
DOMAINS = standalone_domain
BIF_PATH = /home/ubuntu-z5/proj/motor_uart_00/motor_ctl_system/Debug/system.bif

BITSTREAM_PATH = /home/ubuntu-z5/proj/motor_uart_00/motor_ctl/_ide/bitstream/zynq_proc_wrapper.bit
SD_FILES = --elf /home/ubuntu-z5/proj/motor_uart_00/motor_ctl/Debug/motor_ctl.elf,ps7_cortexa9_0

GENERATE_BIF_XSCT_CMD = ${GENERATE_BIF} -xpfm ${XPFM_PATH} -domains ${DOMAINS} -bifpath ${BIF_PATH}

#
# primary build targets
#

package:
	-@echo Generating bif file for the system project
	-@echo Executing command \'${GENERATE_BIF_XSCT_CMD}\' on XSCT
	-@echo "connect -u TCP:localhost:36047; tcf send_command $$::xsdb::curchan xsdb eval s es [list \"${GENERATE_BIF_XSCT_CMD}\"]" | xsct
	${SDCARD_GEN} --xpfm ${XPFM_PATH} --sys_config ${SYS_CONFIG} --bif ${BIF_PATH} --bitstream ${BITSTREAM_PATH} ${SD_FILES}

clean:
	$(RMDIR) ./_sds
	${RMDIR} package
	${RM} system.bif

.PHONY: all package clean

all: package


