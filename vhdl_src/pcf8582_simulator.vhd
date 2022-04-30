library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;
use work.states_package.all;

entity pcf8582_simulator is
    port (reset : in std_logic;
          clk_100khz : in std_logic;
          clk_200khz : in std_logic;
          scl : in std_logic;
          sda : inout std_logic);
end pcf8582_simulator;

architecture rtl of pcf8582_simulator is
    type memory is array(255 downto 0) of std_logic_vector(7 downto 0);
    signal eeprom : memory;
    signal state : i2c_state_slave;
    signal next_state : i2c_state_slave;
    signal prev_state : i2c_state_slave;
    signal sda_output_enable : std_logic;
    signal sda_output : std_logic;
    signal sda_input : std_logic;
    signal enable_send_ack : std_logic;
    signal enable_receive_ack : std_logic;
    signal enable_receive_slave_address : std_logic;
    signal enable_receive_word_address : std_logic;
    signal enable_receive_data : std_logic;
    signal enable_send_data : std_logic;
    signal slave_address : std_logic_vector(7 downto 0);
    signal start_cond_found : std_logic;
    signal num_wait_cycles : unsigned(7 downto 0);
    signal cycles_waited : unsigned(7 downto 0);
    signal trig_state_machine : std_logic;
    signal i2c_wait : std_logic;
    signal received_data : std_logic_vector(7 downto 0);
    signal mem_address : std_logic_vector(7 downto 0);
    signal start_idx : unsigned(3 downto 0);
    signal bit_count : unsigned(3 downto 0);
    signal read_bits_used : unsigned(3 downto 0);
    signal jump_to_slave_address_read : std_logic;
    signal is_read : std_logic;
    signal write_bits_used : unsigned(3 downto 0);
begin

    sda_tristate_buffer : process(all) is
    begin

        if (sda_output_enable = '1') then
            sda <= sda_output;
        else
            sda <= 'Z';
        end if;
        sda_input <= sda;

    end process sda_tristate_buffer;

    state_change : process(clk_200khz, clk_100khz, reset) is
    begin

        if (reset = '1') then
            state <= receive_slave_address;
            cycles_waited <= to_unsigned(0, cycles_waited'length);
            is_read <= '0';
        elsif (rising_edge(clk_200khz)) then
            if (clk_100khz = '0') then
                if (next_state = receive_slave_address and prev_state = receive_slave_address) then
                    is_read <= '0';
                end if;
                if (jump_to_slave_address_read = '1') then
                    state <= receive_slave_address;
                    cycles_waited <= to_unsigned(0, cycles_waited'length);
                    is_read <= '1';
                elsif (i2c_wait = '1' and cycles_waited < num_wait_cycles) then
                    cycles_waited <= cycles_waited + 1;
                else
                    state <= next_state;
                    cycles_waited <= to_unsigned(0, cycles_waited'length);
                end if;
            end if;
        end if;

    end process state_change;

    state_machine : process(all) is
    begin

        if (reset = '1') then
            i2c_wait <= '0';
            next_state <= receive_slave_address;
            prev_state <= receive_slave_address;
            enable_send_ack <= '0';
            enable_receive_ack <= '0';
            enable_receive_slave_address <= '0';
            enable_receive_word_address <= '0';
            enable_receive_data <= '0';
            enable_send_data <= '0';
            num_wait_cycles <= to_unsigned(0, num_wait_cycles'length);
            start_idx <= to_unsigned(0, start_idx'length);
            bit_count <= to_unsigned(0, bit_count'length);
        elsif (falling_edge(clk_200khz)) then
            case state is
                when receive_slave_address =>
                    if (trig_state_machine = '1') then
                        i2c_wait <= '1';
                        num_wait_cycles <= to_unsigned(7, num_wait_cycles'length);
                        start_idx <= to_unsigned(7, start_idx'length);
                        bit_count <= to_unsigned(7, bit_count'length);
                        enable_receive_slave_address <= '1';
                        enable_send_ack <= '0';
                        enable_receive_ack <= '0';
                        enable_receive_word_address <= '0';
                        enable_receive_data <= '0';
                        enable_send_data <= '0';
                        next_state <= send_ack;
                        prev_state <= receive_slave_address;
                    else
                        i2c_wait <= '0';
                        num_wait_cycles <= to_unsigned(0, num_wait_cycles'length);
                        start_idx <= to_unsigned(0, start_idx'length);
                        bit_count <= to_unsigned(0, bit_count'length);
                        enable_receive_slave_address <= '0';
                        enable_send_ack <= '0';
                        enable_receive_ack <= '0';
                        enable_receive_word_address <= '0';
                        enable_receive_data <= '0';
                        enable_send_data <= '0';
                        next_state <= receive_slave_address;
                        prev_state <= receive_slave_address;
                    end if;
                when send_ack =>
                    i2c_wait <= '0';
                    num_wait_cycles <= to_unsigned(0, num_wait_cycles'length);
                    start_idx <= to_unsigned(0, start_idx'length);
                    bit_count <= to_unsigned(0, bit_count'length);
                    enable_receive_slave_address <= '0';
                    enable_send_ack <= '1';
                    enable_receive_ack <= '0';
                    enable_receive_word_address <= '0';
                    enable_receive_data <= '0';
                    enable_send_data <= '0';
                    if (prev_state = receive_slave_address) then
                        if (is_read = '1') then
                            next_state <= send_data;
                            prev_state <= send_ack;
                        else
                            next_state <= receive_word_address;
                            prev_state <= send_ack;
                        end if;
                    elsif (prev_state = receive_word_address) then
                        next_state <= receive_data;
                        prev_state <= send_ack;
                    elsif (prev_state = receive_data) then
                        next_state <= expect_stop_cond;
                        prev_state <= send_ack;
                    end if;
                when receive_word_address =>
                    i2c_wait <= '1';
                    num_wait_cycles <= to_unsigned(7, num_wait_cycles'length);
                    start_idx <= to_unsigned(7, start_idx'length);
                    bit_count <= to_unsigned(7, bit_count'length);
                    enable_receive_slave_address <= '0';
                    enable_send_ack <= '0';
                    enable_receive_ack <= '0';
                    enable_receive_word_address <= '1';
                    enable_receive_data <= '0';
                    enable_send_data <= '0';
                    next_state <= send_ack;
                    prev_state <= receive_word_address;
                when receive_data =>
                    i2c_wait <= '1';
                    num_wait_cycles <= to_unsigned(7, num_wait_cycles'length);
                    start_idx <= to_unsigned(7, start_idx'length);
                    bit_count <= to_unsigned(7, bit_count'length);
                    enable_receive_slave_address <= '0';
                    enable_send_ack <= '0';
                    enable_receive_ack <= '0';
                    enable_receive_word_address <= '0';
                    enable_receive_data <= '1';
                    enable_send_data <= '0';
                    next_state <= send_ack;
                    prev_state <= receive_data;
                when receive_ack =>
                    i2c_wait <= '0';
                    num_wait_cycles <= to_unsigned(0, num_wait_cycles'length);
                    start_idx <= to_unsigned(0, start_idx'length);
                    bit_count <= to_unsigned(0, bit_count'length);
                    enable_receive_slave_address <= '0';
                    enable_send_ack <= '0';
                    enable_receive_ack <= '1';
                    enable_receive_word_address <= '0';
                    enable_receive_data <= '0';
                    enable_send_data <= '0';
                    next_state <= expect_stop_cond;
                    prev_state <= receive_ack;
                when send_data =>
                    i2c_wait <= '1';
                    num_wait_cycles <= to_unsigned(7, num_wait_cycles'length);
                    start_idx <= to_unsigned(7, start_idx'length);
                    bit_count <= to_unsigned(7, bit_count'length);
                    enable_receive_slave_address <= '0';
                    enable_send_ack <= '0';
                    enable_receive_ack <= '0';
                    enable_receive_word_address <= '0';
                    enable_receive_data <= '0';
                    enable_send_data <= '1';
                    next_state <= receive_ack;
                    prev_state <= send_data;
                when expect_stop_cond =>
                    i2c_wait <= '0';
                    num_wait_cycles <= to_unsigned(0, num_wait_cycles'length);
                    start_idx <= to_unsigned(0, start_idx'length);
                    bit_count <= to_unsigned(0, bit_count'length);
                    enable_receive_slave_address <= '0';
                    enable_send_ack <= '0';
                    enable_receive_ack <= '0';
                    enable_receive_word_address <= '0';
                    enable_receive_data <= '0';
                    enable_send_data <= '0';
                    next_state <= receive_slave_address;
                    prev_state <= expect_stop_cond;
            end case;
        end if;

    end process state_machine;

    poll_start_cond : process(all) is
    begin

        if (reset = '1') then
            start_cond_found <= '0';
        elsif (falling_edge(sda_input)) then
            if (scl = '1') then
                start_cond_found <= '1';
            else
                start_cond_found <= '0';
            end if;
        end if;

    end process poll_start_cond;

    state_machine_controller : process(all) is
        variable count : natural;
    begin

        if (reset = '1') then
            count := 0;
            trig_state_machine <= '0';
        elsif (rising_edge(scl)) then
            if (start_cond_found = '1' and sda_output_enable = '0') then
                count := 12;
                trig_state_machine <= '1';
            end if;
            if (count > 0) then
                count := count - 1;
                trig_state_machine <= '1';
            else
                trig_state_machine <= '0';
            end if;
        end if;

    end process state_machine_controller;

    write_data_proc : process(all) is
        variable index : natural;
    begin

        if (reset = '1') then
            sda_output <= '1';
            sda_output_enable <= '0';
            write_bits_used <= to_unsigned(0, write_bits_used'length);
            index := 0;
            mem_address <= (others => '0');
            eeprom <= (others => (others => '0'));
        elsif (rising_edge(scl)) then
            index := to_integer(start_idx - write_bits_used);
            if (enable_send_ack = '1') then
                sda_output_enable <= '1';
                sda_output <= '0';
                if (next_state = receive_data) then
                    mem_address <= received_data;
                elsif (next_state = expect_stop_cond) then
                    eeprom(to_integer(unsigned(mem_address))) <= received_data;
                end if;
            elsif (enable_send_data = '1') then
                sda_output_enable <= '1';
                sda_output <= eeprom(to_integer(unsigned(mem_address)))(index);
                if (write_bits_used < bit_count) then
                    write_bits_used <= write_bits_used + 1;
                else
                    write_bits_used <= to_unsigned(0, write_bits_used'length);
                end if;
            else
                sda_output_enable <= '0';
                sda_output <= '1';
            end if;
        end if;

    end process write_data_proc;

    read_data_proc : process(all) is
        variable index : natural;
    begin

        if (reset = '1') then
            received_data <= (others => '0');
            read_bits_used <= to_unsigned(0, read_bits_used'length);
            jump_to_slave_address_read <= '0';
            index := 0;
        elsif (falling_edge(scl) and (enable_receive_slave_address = '1' or
                                      enable_receive_word_address = '1' or
                                      enable_receive_data = '1' or
                                      enable_receive_ack = '1')) then
            index := to_integer(start_idx - read_bits_used);
            received_data(index) <= sda_input;
            if (read_bits_used < bit_count) then
                read_bits_used <= read_bits_used + 1;
                if (enable_receive_data = '1' and start_cond_found = '1') then
                    jump_to_slave_address_read <= '1';
                    read_bits_used <= to_unsigned(0, read_bits_used'length);
                else
                    jump_to_slave_address_read <= '0';
                end if;
            else
                read_bits_used <= to_unsigned(0, read_bits_used'length);
                jump_to_slave_address_read <= '0';
            end if;
        end if;

    end process read_data_proc;

end architecture rtl;
