quartus_map --read_settings_files=on --write_settings_files=off PIC16F84A -c PIC16F84A
quartus_fit --read_settings_files=off --write_settings_files=off PIC16F84A -c PIC16F84A
quartus_asm --read_settings_files=off --write_settings_files=off PIC16F84A -c PIC16F84A
quartus_sta PIC16F84A -c PIC16F84A
quartus_eda --read_settings_files=off --write_settings_files=off PIC16F84A -c PIC16F84A
