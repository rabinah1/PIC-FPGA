library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

package states_package is
    type pic_state is (do_nop, do_wait, do_ram_read, do_ram_dump, do_alu_input_sel, do_alu, do_wreg, do_ram_write, do_result);
    type i2c_state is (i2c_nop, create_start_cond, create_stop_cond, create_repeated_start_cond, write_slave_addr_write_mode, receive_ack, write_word_addr, write_slave_addr_read_mode, read_data, write_data, send_ack, enable_scl, write_to_ram);
end states_package;
