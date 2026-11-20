# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: /home/ubuntu-z5/proj/motor_uart_00/motor_ctl_system/_ide/scripts/systemdebugger_motor_ctl_system_standalone.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source /home/ubuntu-z5/proj/motor_uart_00/motor_ctl_system/_ide/scripts/systemdebugger_motor_ctl_system_standalone.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Digilent Zed 210248C07F2F" && level==0 && jtag_device_ctx=="jsn-Zed-210248C07F2F-23727093-0"}
fpga -file /home/ubuntu-z5/proj/motor_uart_00/motor_ctl/_ide/bitstream/zynq_proc_wrapper.bit
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw /home/ubuntu-z5/proj/motor_uart_00/bare_zynq_proc/export/bare_zynq_proc/hw/zynq_proc_wrapper.xsa -mem-ranges [list {0x40000000 0xbfffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
source /home/ubuntu-z5/proj/motor_uart_00/motor_ctl/_ide/psinit/ps7_init.tcl
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "*A9*#0"}
dow /home/ubuntu-z5/proj/motor_uart_00/motor_ctl/Debug/motor_ctl.elf
configparams force-mem-access 0
targets -set -nocase -filter {name =~ "*A9*#0"}
con
