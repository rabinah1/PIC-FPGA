param(
    [switch] $build = $false,
    [switch] $test = $false,
    [switch] $save_test_output = $false,
    [switch] $load = $false,
    [switch] $check_style = $false,
    [switch] $help = $false
)

if ((!$build -and !$test -and !$load -and !$check_style) -or $help) {
   Write-Host "Script for building the FPGA design, running tests on Modelsim, and loading the bitfile to the FPGA. `n"
              "Valid arguments are:"
              "- build: Build the FPGA design."
              "- test: Run the FPGA tests using Modelsim."
              "- save_test_output: If set to true, save the output data generated by Modelsim testrun. Set to false by default."
              "- load: Load the bitfile to the FPGA."
              "- check_style: Run VHDL style check with VSG."
              "- help: Print this help message."
   Exit
}

if ($check_style) {
   $src_files = @("alu.vhd", "alu_input_mux.vhd", "alu_output_demux.vhd", "clk_div.vhd",
                  "constants_package.vhd", "i2c.vhd", "input_receive.vhd",
                  "parallel_to_serial_output.vhd", "pcf8582_simulator.vhd", "pic16f84a.vhd",
                  "pic16f84a_tb.vhd", "ram.vhd", "serial_to_parallel_instruction.vhd",
                  "states_package.vhd", "state_machine.vhd", "timer.vhd", "w_register.vhd")
   foreach ($file in $src_files) {
       vsg -f $PSScriptRoot/vhdl_src/$file -c $PSScriptRoot/vsg_config.json
   }
}

if ($build) {
   quartus_map --read_settings_files=on --write_settings_files=off $PSScriptRoot/vhdl_src/PIC16F84A -c PIC16F84A
   quartus_fit --read_settings_files=off --write_settings_files=off $PSScriptRoot/vhdl_src/PIC16F84A -c PIC16F84A
   quartus_asm --read_settings_files=off --write_settings_files=off $PSScriptRoot/vhdl_src/PIC16F84A -c PIC16F84A
   quartus_sta $PSScriptRoot/vhdl_src/PIC16F84A -c PIC16F84A
   quartus_eda --read_settings_files=off --write_settings_files=off $PSScriptRoot/vhdl_src/PIC16F84A -c PIC16F84A
}

if ($test) {
   python3.8 $PSScriptRoot/test_data/tb_input_parser.py $PSScriptRoot/test_data
   vlib $PSScriptRoot/vhdl_src/work
   $src_files = @("constants_package.vhd", "states_package.vhd", "alu.vhd", "alu_input_mux.vhd",
                  "alu_output_demux.vhd", "input_receive.vhd", "parallel_to_serial_output.vhd",
                  "pic16f84a.vhd", "pic16f84a_tb.vhd", "ram.vhd", "serial_to_parallel_instruction.vhd",
                  "state_machine.vhd", "w_register.vhd", "timer.vhd", "clk_div.vhd", "i2c.vhd",
                  "pcf8582_simulator.vhd")
   foreach ($file in $src_files) {
       vcom -2008 -reportprogress 300 -work $PSScriptRoot/vhdl_src/work $PSScriptRoot/vhdl_src/$file
   }
   vsim -c -lib $PSScriptRoot/vhdl_src/work -l $PSScriptRoot/vhdl_src/transcript -wlf $PSScriptRoot/vhdl_src/vsim.wlf `
   -ginput_file="$PSScriptRoot/test_data/tb_result.txt" -goutput_file="$PSScriptRoot/test_data/tb_input_parsed.txt" `
   -do "$PSScriptRoot/vsim_commands.txt" pic16f84a_tb
   python3.8 $PSScriptRoot/test_data/verify_simulation_result.py $PSScriptRoot/test_data
   if (-not ($save_test_output)) {
      rm $PSScriptRoot/vhdl_src/transcript
      Remove-Item $PSScriptRoot\vhdl_src\work\ -Force -Recurse
      rm $PSScriptRoot/test_data/tb_input_parsed.txt
      rm $PSScriptRoot/test_data/tb_result.txt
      rm $PSScriptRoot/test_data/tb_result_formatted.txt
   }
}

if ($load) {
   if (-not (Test-Path -Path $PSScriptRoot\vhdl_src\output_files\PIC16F84A.sof)) {
      Write-Host "Error: the bitstream sof-file was not found, please build the design first."
      Exit
   }
   quartus_pgm -m jtag -o "p;$PSScriptRoot\vhdl_src\output_files\PIC16F84A.sof@2"
}