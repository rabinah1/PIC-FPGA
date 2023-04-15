SHELL = C:\Windows\system32\WindowsPowerShell\v1.0\powershell.exe

.PHONY: sta build check load netlist clean

sta:
	@./fpga_tool.ps1 -check_style

build:
	@./fpga_tool.ps1 -build

check:
	@./fpga_tool.ps1 -test $(TEST_ARGS)

load:
	@./fpga_tool.ps1 -load $(LOAD_ARGS)

netlist:
	@./fpga_tool.ps1 -show_netlist

clean:
	@./fpga_tool.ps1 -clean
