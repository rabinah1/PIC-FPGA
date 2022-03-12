python3 $PSScriptRoot/test_data/tb_input_parser.py $PSScriptRoot/test_data
vlib $PSScriptRoot/vhdl_src/work
vcom -2008 -reportprogress 300 -work $PSScriptRoot/vhdl_src/work $PSScriptRoot/vhdl_src/states_package.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/vhdl_src/work $PSScriptRoot/vhdl_src/ALU.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/vhdl_src/work $PSScriptRoot/vhdl_src/alu_input_mux.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/vhdl_src/work $PSScriptRoot/vhdl_src/alu_output_demux.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/vhdl_src/work $PSScriptRoot/vhdl_src/input_receive.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/vhdl_src/work $PSScriptRoot/vhdl_src/parallel_to_serial_output.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/vhdl_src/work $PSScriptRoot/vhdl_src/PIC16F84A.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/vhdl_src/work $PSScriptRoot/vhdl_src/PIC16F84A_tb.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/vhdl_src/work $PSScriptRoot/vhdl_src/ram.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/vhdl_src/work $PSScriptRoot/vhdl_src/serial_to_parallel_instruction.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/vhdl_src/work $PSScriptRoot/vhdl_src/state_machine.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/vhdl_src/work $PSScriptRoot/vhdl_src/W_register.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/vhdl_src/work $PSScriptRoot/vhdl_src/timer.vhd
vsim -c -lib $PSScriptRoot/vhdl_src/work -l $PSScriptRoot/vhdl_src/transcript -wlf $PSScriptRoot/vhdl_src/vsim.wlf -ginput_file="$PSScriptRoot/test_data/tb_result.txt" -goutput_file="$PSScriptRoot/test_data/tb_input_parsed.txt" -do "$PSScriptRoot/vsim_commands.txt" pic16f84a_tb
python3 $PSScriptRoot/test_data/verify_simulation_result.py $PSScriptRoot/test_data
rm $PSScriptRoot/vhdl_src/transcript
Remove-Item $PSScriptRoot\vhdl_src\work\ -Force -Recurse
rm $PSScriptRoot/test_data/tb_input_parsed.txt
rm $PSScriptRoot/test_data/tb_result.txt
rm $PSScriptRoot/test_data/tb_result_formatted.txt
