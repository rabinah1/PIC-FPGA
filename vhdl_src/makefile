SHELL = /bin/bash
POWERSHELL = C:/Windows/system32/WindowsPowerShell/v1.0/powershell.exe

.PHONY: sta build check load netlist clean

sta:
	@POWERSHELL ./fpga_tool.ps1 -check_style

build:
	@POWERSHELL ./fpga_tool.ps1 -build

check:
	@POWERSHELL ./fpga_tool.ps1 -test $(TEST_ARGS)

load:
	@POWERSHELL ./fpga_tool.ps1 -load $(LOAD_ARGS)

netlist:
	@POWERSHELL ./fpga_tool.ps1 -show_netlist

clean:
	@POWERSHELL ./fpga_tool.ps1 -clean
