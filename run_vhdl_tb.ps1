python3 tb_input_parser.py
vlib work
vcom -2008 -reportprogress 300 -work work C:/Users/henry/PIC-FPGA/states_package.vhd
vcom -2008 -reportprogress 300 -work work C:/Users/henry/PIC-FPGA/ALU.vhd
vcom -2008 -reportprogress 300 -work work C:/Users/henry/PIC-FPGA/alu_input_mux.vhd
vcom -2008 -reportprogress 300 -work work C:/Users/henry/PIC-FPGA/alu_output_demux.vhd
vcom -2008 -reportprogress 300 -work work C:/Users/henry/PIC-FPGA/input_receive.vhd
vcom -2008 -reportprogress 300 -work work C:/Users/henry/PIC-FPGA/parallel_to_serial_output.vhd
vcom -2008 -reportprogress 300 -work work C:/Users/henry/PIC-FPGA/PIC16F84A.vhd
vcom -2008 -reportprogress 300 -work work C:/Users/henry/PIC-FPGA/PIC16F84A_tb.vhd
vcom -2008 -reportprogress 300 -work work C:/Users/henry/PIC-FPGA/ram.vhd
vcom -2008 -reportprogress 300 -work work C:/Users/henry/PIC-FPGA/serial_to_parallel_instruction.vhd
vcom -2008 -reportprogress 300 -work work C:/Users/henry/PIC-FPGA/state_machine.vhd
vcom -2008 -reportprogress 300 -work work C:/Users/henry/PIC-FPGA/status_register.vhd
vcom -2008 -reportprogress 300 -work work C:/Users/henry/PIC-FPGA/W_register.vhd
vsim -c -do "vsim_commands.txt" work.pic16f84a_tb
python3 verify_simulation_result.py
rm transcript
rm vsim.wlf
Remove-Item .\work\ -Force -Recurse
rm .\tb_input_parsed.txt
rm .\tb_result.txt
