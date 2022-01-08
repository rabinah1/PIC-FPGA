quartus_map --read_settings_files=on --write_settings_files=off $PSScriptRoot/vhdl_src/PIC16F84A -c PIC16F84A
quartus_fit --read_settings_files=off --write_settings_files=off $PSScriptRoot/vhdl_src/PIC16F84A -c PIC16F84A
quartus_asm --read_settings_files=off --write_settings_files=off $PSScriptRoot/vhdl_src/PIC16F84A -c PIC16F84A
quartus_sta $PSScriptRoot/vhdl_src/PIC16F84A -c PIC16F84A
quartus_eda --read_settings_files=off --write_settings_files=off $PSScriptRoot/vhdl_src/PIC16F84A -c PIC16F84A
