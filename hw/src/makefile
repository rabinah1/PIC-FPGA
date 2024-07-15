SHELL = /bin/bash

.PHONY: sta build check load netlist clean

STYLE_CHECK_FILES = \
	alu.vhd \
	alu_input_mux.vhd \
	alu_output_demux.vhd \
	clk_div.vhd \
	constants_package.vhd \
	i2c.vhd \
	input_receive.vhd \
	parallel_to_serial_output.vhd \
	pcf8582_simulator.vhd \
	pic16f84a.vhd	\
	../testbench/E2E/pic16f84a_tb.vhd \
	../testbench/component/tb_w_register.vhd \
	../testbench/component/tb_alu_input_mux.vhd \
	../testbench/component/tb_alu_output_demux.vhd \
	../testbench/component/tb_alu.vhd \
	ram.vhd \
	serial_to_parallel_instruction.vhd \
	states_package.vhd \
	state_machine.vhd \
	timer.vhd \
	w_register.vhd \
	hps_adder.vhd

SIMULATION_FILES = \
	constants_package.vhd \
	states_package.vhd \
	alu.vhd \
	alu_input_mux.vhd \
	alu_output_demux.vhd \
	input_receive.vhd \
	parallel_to_serial_output.vhd \
	pic16f84a.vhd \
	../testbench/E2E/pic16f84a_tb.vhd \
	ram.vhd \
	serial_to_parallel_instruction.vhd \
	state_machine.vhd \
	w_register.vhd \
	timer.vhd \
	clk_div.vhd \
	i2c.vhd \
	pcf8582_simulator.vhd

FILES_TO_REMOVE = \
	db \
	incremental_db \
	output_files \
	simulation \
	c5_pin_model_dump.txt \
	vsim.wlf \
	transcript \
	work \
	hps_adder_qsys.sopcinfo \
	hps_sdram_p0_summary.csv \
	hps_isw_handoff

BITFILE = ./output_files/pic16f84a.sof

STORE_RESULT = no

sta:
	@for file in $(STYLE_CHECK_FILES); do \
		vsg -f $${file} -c ../style_check/vsg_config.json; \
	done

check:
	@python3 ../run.py
	@python3 ../../test_data/scripts/tb_input_parser.py ../../test_data/data
	@vlib ./work
	@for file in $(SIMULATION_FILES); do \
		vcom -2008 -reportprogress 300 -work ./work ./$${file}; \
	done
	@vsim -c -lib ./work -l ./transcript -wlf ./vsim.wlf -ginput_file="../../test_data/data/tb_result.txt" \
	-goutput_file="../../test_data/data/tb_input_parsed.txt" -do "../testbench/E2E/vsim_commands.txt" pic16f84a_tb
	@python3 ../../test_data/scripts/verify_simulation_result.py ../../test_data/data
	@if [[ $(STORE_RESULT) != "yes" ]]; then \
		rm ./transcript; \
		rm -rf ./work; \
		rm -rf ./vunit_out; \
		rm ../../test_data/data/tb_input_parsed.txt; \
		rm ../../test_data/data/tb_result.txt; \
		rm ../../test_data/data/tb_result_formatted.txt; \
	fi

build:
	@quartus_map --read_settings_files=on --write_settings_files=off ./pic16f84a -c pic16f84a
	@quartus_fit --read_settings_files=off --write_settings_files=off ./pic16f84a -c pic16f84a
	@quartus_asm --read_settings_files=off --write_settings_files=off ./pic16f84a -c pic16f84a
	@quartus_sta ./pic16f84a -c pic16f84a
	@quartus_eda --read_settings_files=off --write_settings_files=off ./pic16f84a -c pic16f84a

load:
	@if [[ ! -f $(BITFILE) ]]; then \
		echo "Error: the bitfile $(BITFILE) was not found."; \
	else \
		quartus_pgm -m jtag -o "p;$(BITFILE)@2"; \
	fi

netlist:
	@quartus_npp ./pic16f84a -c pic16f84a --netlist_type=sgate
	@qnui ./pic16f84a

clean:
	@ for file in $(FILES_TO_REMOVE); do \
		rm -rf $${file}; \
	done
