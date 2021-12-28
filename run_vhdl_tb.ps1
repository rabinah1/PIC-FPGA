python3 $PSScriptRoot/tb_input_parser.py $PSScriptRoot
vlib $PSScriptRoot/work
vcom -2008 -reportprogress 300 -work $PSScriptRoot/work $PSScriptRoot/states_package.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/work $PSScriptRoot/ALU.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/work $PSScriptRoot/alu_input_mux.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/work $PSScriptRoot/alu_output_demux.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/work $PSScriptRoot/input_receive.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/work $PSScriptRoot/parallel_to_serial_output.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/work $PSScriptRoot/PIC16F84A.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/work $PSScriptRoot/PIC16F84A_tb.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/work $PSScriptRoot/ram.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/work $PSScriptRoot/serial_to_parallel_instruction.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/work $PSScriptRoot/state_machine.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/work $PSScriptRoot/status_register.vhd
vcom -2008 -reportprogress 300 -work $PSScriptRoot/work $PSScriptRoot/W_register.vhd
vsim -c -lib $PSScriptRoot/work -l $PSScriptRoot/transcript -wlf $PSScriptRoot/vsim.wlf -ginput_file="$PSScriptRoot/tb_result.txt" -goutput_file="$PSScriptRoot/tb_input_parsed.txt" -do "$PSScriptRoot/vsim_commands.txt" pic16f84a_tb
python3 $PSScriptRoot/verify_simulation_result.py $PSScriptRoot
rm $PSScriptRoot/transcript
rm $PSScriptRoot/vsim.wlf
Remove-Item $PSScriptRoot\work\ -Force -Recurse
rm $PSScriptRoot/tb_input_parsed.txt
rm $PSScriptRoot/tb_result.txt
rm $PSScriptRoot/tb_result_formatted.txt
