library ieee;

package states_package is

    type pic_state is (
        do_nop, do_wait, do_ram_read, do_ram_dump, do_eeprom_dump, do_alu_input_sel, do_alu, do_wreg, do_ram_write,
        do_result
    );

    type i2c_state_master is (
        i2c_nop, create_start_cond, create_stop_cond, create_repeated_start_cond, write_slave_addr_write_mode,
        receive_ack, write_word_addr, write_slave_addr_read_mode, read_data, write_data, send_ack, enable_scl,
        write_to_ram
    );

    type i2c_state_slave is (
        receive_slave_address, send_ack, receive_word_address, receive_data, send_data, receive_ack, expect_stop_cond
    );

end package states_package;
