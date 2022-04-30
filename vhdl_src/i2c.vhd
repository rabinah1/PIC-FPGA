library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;
use work.states_package.all;

entity i2c is
    port (reset : in std_logic;
          clk_100khz : in std_logic;
          clk_200khz : in std_logic;
          control : in std_logic_vector(7 downto 0);
          data_in : in std_logic_vector(7 downto 0);
          address_in : in std_logic_vector(7 downto 0);
          sda : inout std_logic;
          scl : out std_logic;
          enable_write_to_ram : out std_logic;
          data_out : out std_logic_vector(7 downto 0));
end i2c;

architecture rtl of i2c is
    signal state : i2c_state_master;
    signal next_state : i2c_state_master;
    signal prev_state : i2c_state_master;
    signal sda_output : std_logic;
    signal sda_output_enable : std_logic;
    signal sda_input : std_logic;
    signal scl_enable : std_logic;
    signal start_read_procedure : std_logic;
    signal start_write_procedure : std_logic;
    signal i2c_wait : std_logic;
    signal num_wait_cycles : unsigned(7 downto 0);
    signal cycles_waited : unsigned(7 downto 0);
    signal start_idx : unsigned(3 downto 0);
    signal bit_count : unsigned(3 downto 0);
    signal read_bits_used : unsigned(3 downto 0);
    signal write_bits_used : unsigned(3 downto 0);
    signal enable_create_start_cond : std_logic;
    signal enable_create_stop_cond : std_logic;
    signal enable_enable_scl : std_logic;
    signal disable_enable_scl : std_logic;
    signal enable_write_slave_addr_write_mode : std_logic;
    signal enable_write_slave_addr_read_mode : std_logic;
    signal enable_check_ack : std_logic;
    signal enable_send_ack : std_logic;
    signal enable_write_word_addr : std_logic;
    signal enable_repeated_start : std_logic;
    signal enable_read_data : std_logic;
    signal enable_write_data : std_logic;
    signal is_ack_received : std_logic;
    signal trig_state_machine : std_logic;
    constant slave_address_read : std_logic_vector(7 downto 0) := "10100001";
    constant slave_address_write : std_logic_vector(7 downto 0) := "10100000";
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

    scl_process : process(all) is
    begin

        if (reset = '1') then
            scl <= '1';
        elsif (scl_enable = '1') then
            scl <= clk_100khz;
        else
            scl <= '1';
        end if;

    end process scl_process;

    read_write_process : process(all) is
    begin

        if (reset = '1') then
            start_read_procedure <= '0';
            start_write_procedure <= '0';
        elsif (rising_edge(clk_100khz)) then
            start_read_procedure <= control(0);
            start_write_procedure <= control(1);
        end if;

    end process read_write_process;

    state_change : process(clk_200khz, clk_100khz, reset) is
    begin

        if (reset = '1') then
            state <= i2c_nop;
            cycles_waited <= to_unsigned(0, cycles_waited'length);
            trig_state_machine <= '0';
        elsif (rising_edge(clk_200khz)) then
            if (clk_100khz = '1') then
                if (i2c_wait = '1' and cycles_waited < num_wait_cycles) then
                    cycles_waited <= cycles_waited + 1;
                    trig_state_machine <= '0';
                else
                    trig_state_machine <= '1';
                    state <= next_state;
                    cycles_waited <= to_unsigned(0, cycles_waited'length);
                end if;
            else
                trig_state_machine <= '0';
            end if;
        end if;

    end process state_change;

    state_machine : process(all) is
    begin

        if (reset = '1') then
            i2c_wait <= '0';
            next_state <= i2c_nop;
            prev_state <= i2c_nop;
            enable_create_start_cond <= '0';
            enable_enable_scl <= '0';
            disable_enable_scl <= '0';
            enable_write_slave_addr_write_mode <= '0';
            enable_check_ack <= '0';
            enable_write_word_addr <= '0';
            enable_write_slave_addr_read_mode <= '0';
            enable_repeated_start <= '0';
            enable_read_data <= '0';
            enable_send_ack <= '0';
            enable_create_stop_cond <= '0';
            enable_write_data <= '0';
            enable_write_to_ram <= '0';
            num_wait_cycles <= to_unsigned(0, num_wait_cycles'length);
            start_idx <= to_unsigned(0, start_idx'length);
            bit_count <= to_unsigned(0, bit_count'length);
        elsif (falling_edge(clk_200khz) and trig_state_machine = '1') then
            case state is
                when i2c_nop =>
                    num_wait_cycles <= to_unsigned(0, num_wait_cycles'length);
                    start_idx <= to_unsigned(0, start_idx'length);
                    bit_count <= to_unsigned(0, bit_count'length);
                    i2c_wait <= '0';
                    enable_create_start_cond <= '0';
                    enable_enable_scl <= '0';
                    disable_enable_scl <= '0';
                    enable_write_slave_addr_write_mode <= '0';
                    enable_check_ack <= '0';
                    enable_write_word_addr <= '0';
                    enable_write_slave_addr_read_mode <= '0';
                    enable_repeated_start <= '0';
                    enable_read_data <= '0';
                    enable_send_ack <= '0';
                    enable_create_stop_cond <= '0';
                    enable_write_data <= '0';
                    enable_write_to_ram <= '0';
                    if (start_read_procedure = '1' or start_write_procedure = '1') then
                        next_state <= create_start_cond;
                        prev_state <= i2c_nop;
                    else
                        next_state <= i2c_nop;
                        prev_state <= i2c_nop;
                    end if;
                when create_start_cond =>
                    num_wait_cycles <= to_unsigned(0, num_wait_cycles'length);
                    start_idx <= to_unsigned(0, start_idx'length);
                    bit_count <= to_unsigned(0, bit_count'length);
                    i2c_wait <= '0';
                    enable_create_start_cond <= '1';
                    enable_enable_scl <= '0';
                    disable_enable_scl <= '0';
                    enable_write_slave_addr_write_mode <= '0';
                    enable_check_ack <= '0';
                    enable_write_word_addr <= '0';
                    enable_write_slave_addr_read_mode <= '0';
                    enable_repeated_start <= '0';
                    enable_read_data <= '0';
                    enable_send_ack <= '0';
                    enable_create_stop_cond <= '0';
                    enable_write_data <= '0';
                    enable_write_to_ram <= '0';
                    if (prev_state = receive_ack) then
                        next_state <= write_slave_addr_read_mode;
                        prev_state <= create_start_cond;
                    elsif (prev_state = i2c_nop) then
                        next_state <= enable_scl;
                        prev_state <= create_start_cond;
                    else
                        next_state <= i2c_nop;
                        prev_state <= i2c_nop;
                    end if;
                when create_repeated_start_cond =>
                    num_wait_cycles <= to_unsigned(0, num_wait_cycles'length);
                    start_idx <= to_unsigned(0, start_idx'length);
                    bit_count <= to_unsigned(0, bit_count'length);
                    i2c_wait <= '0';
                    enable_create_start_cond <= '0';
                    enable_enable_scl <= '0';
                    disable_enable_scl <= '0';
                    enable_write_slave_addr_write_mode <= '0';
                    enable_check_ack <= '0';
                    enable_write_word_addr <= '0';
                    enable_write_slave_addr_read_mode <= '0';
                    enable_repeated_start <= '1';
                    enable_read_data <= '0';
                    enable_send_ack <= '0';
                    enable_create_stop_cond <= '0';
                    enable_write_data <= '0';
                    enable_write_to_ram <= '0';
                    if (is_ack_received = '1') then
                        next_state <= write_slave_addr_read_mode;
                        prev_state <= create_repeated_start_cond;
                    else
                        next_state <= create_stop_cond;
                        prev_state <= create_repeated_start_cond;
                    end if;
                when enable_scl =>
                    num_wait_cycles <= to_unsigned(0, num_wait_cycles'length);
                    start_idx <= to_unsigned(0, start_idx'length);
                    bit_count <= to_unsigned(0, bit_count'length);
                    i2c_wait <= '0';
                    enable_create_start_cond <= '0';
                    enable_enable_scl <= '1';
                    disable_enable_scl <= '0';
                    enable_write_slave_addr_write_mode <= '0';
                    enable_check_ack <= '0';
                    enable_write_word_addr <= '0';
                    enable_write_slave_addr_read_mode <= '0';
                    enable_repeated_start <= '0';
                    enable_read_data <= '0';
                    enable_send_ack <= '0';
                    enable_create_stop_cond <= '0';
                    enable_write_data <= '0';
                    enable_write_to_ram <= '0';
                    next_state <= write_slave_addr_write_mode;
                    prev_state <= enable_scl;
                when write_slave_addr_write_mode =>
                    num_wait_cycles <= to_unsigned(7, num_wait_cycles'length);
                    start_idx <= to_unsigned(7, start_idx'length);
                    bit_count <= to_unsigned(7, bit_count'length);
                    i2c_wait <= '1';
                    enable_create_start_cond <= '0';
                    enable_enable_scl <= '0';
                    disable_enable_scl <= '0';
                    enable_write_slave_addr_write_mode <= '1';
                    enable_check_ack <= '0';
                    enable_write_word_addr <= '0';
                    enable_write_slave_addr_read_mode <= '0';
                    enable_repeated_start <= '0';
                    enable_read_data <= '0';
                    enable_send_ack <= '0';
                    enable_create_stop_cond <= '0';
                    enable_write_data <= '0';
                    enable_write_to_ram <= '0';
                    next_state <= receive_ack;
                    prev_state <= write_slave_addr_write_mode;
                when receive_ack =>
                    num_wait_cycles <= to_unsigned(0, num_wait_cycles'length);
                    start_idx <= to_unsigned(0, start_idx'length);
                    bit_count <= to_unsigned(0, bit_count'length);
                    i2c_wait <= '0';
                    enable_create_start_cond <= '0';
                    enable_enable_scl <= '0';
                    disable_enable_scl <= '0';
                    enable_write_slave_addr_write_mode <= '0';
                    enable_check_ack <= '1';
                    enable_write_word_addr <= '0';
                    enable_write_slave_addr_read_mode <= '0';
                    enable_repeated_start <= '0';
                    enable_read_data <= '0';
                    enable_send_ack <= '0';
                    enable_create_stop_cond <= '0';
                    enable_write_data <= '0';
                    enable_write_to_ram <= '0';
                    if (prev_state = write_slave_addr_write_mode) then
                        next_state <= write_word_addr;
                        prev_state <= receive_ack;
                    elsif (prev_state = write_word_addr) then
                        if (start_read_procedure = '1') then
                            next_state <= create_repeated_start_cond;
                            prev_state <= receive_ack;
                        elsif (start_write_procedure = '1') then
                            next_state <= write_data;
                            prev_state <= receive_ack;
                        else
                            next_state <= i2c_nop;
                            prev_state <= i2c_nop;
                        end if;
                    elsif (prev_state = write_slave_addr_read_mode) then
                        next_state <= read_data;
                        prev_state <= receive_ack;
                    elsif (prev_state = write_data) then
                        next_state <= create_stop_cond;
                        prev_state <= receive_ack;
                    else
                        next_state <= i2c_nop;
                        prev_state <= i2c_nop;
                    end if;
                when write_word_addr =>
                    num_wait_cycles <= to_unsigned(7, num_wait_cycles'length);
                    start_idx <= to_unsigned(7, start_idx'length);
                    bit_count <= to_unsigned(7, bit_count'length);
                    i2c_wait <= '1';
                    enable_create_start_cond <= '0';
                    enable_enable_scl <= '0';
                    disable_enable_scl <= '0';
                    enable_write_slave_addr_write_mode <= '0';
                    enable_check_ack <= '0';
                    enable_write_word_addr <= '1';
                    enable_write_slave_addr_read_mode <= '0';
                    enable_repeated_start <= '0';
                    enable_read_data <= '0';
                    enable_send_ack <= '0';
                    enable_create_stop_cond <= '0';
                    enable_write_data <= '0';
                    enable_write_to_ram <= '0';
                    if (is_ack_received = '1') then
                        next_state <= receive_ack;
                        prev_state <= write_word_addr;
                    else
                        next_state <= create_stop_cond;
                        prev_state <= write_word_addr;
                    end if;
                when write_slave_addr_read_mode =>
                    num_wait_cycles <= to_unsigned(7, num_wait_cycles'length);
                    bit_count <= to_unsigned(7, bit_count'length);
                    start_idx <= to_unsigned(7, start_idx'length);
                    i2c_wait <= '1';
                    enable_create_start_cond <= '0';
                    enable_enable_scl <= '0';
                    disable_enable_scl <= '0';
                    enable_write_slave_addr_write_mode <= '0';
                    enable_check_ack <= '0';
                    enable_write_word_addr <= '0';
                    enable_write_slave_addr_read_mode <= '1';
                    enable_repeated_start <= '0';
                    enable_read_data <= '0';
                    enable_send_ack <= '0';
                    enable_create_stop_cond <= '0';
                    enable_write_data <= '0';
                    enable_write_to_ram <= '0';
                    next_state <= receive_ack;
                    prev_state <= write_slave_addr_read_mode;
                when read_data =>
                    num_wait_cycles <= to_unsigned(7, num_wait_cycles'length);
                    start_idx <= to_unsigned(7, start_idx'length);
                    bit_count <= to_unsigned(7, bit_count'length);
                    i2c_wait <= '1';
                    enable_create_start_cond <= '0';
                    enable_enable_scl <= '0';
                    disable_enable_scl <= '0';
                    enable_write_slave_addr_write_mode <= '0';
                    enable_check_ack <= '0';
                    enable_write_word_addr <= '0';
                    enable_write_slave_addr_read_mode <= '0';
                    enable_repeated_start <= '0';
                    enable_read_data <= '1';
                    enable_send_ack <= '0';
                    enable_create_stop_cond <= '0';
                    enable_write_data <= '0';
                    enable_write_to_ram <= '0';
                    if (is_ack_received = '1') then
                        next_state <= send_ack;
                        prev_state <= read_data;
                    else
                        next_state <= create_stop_cond;
                        prev_state <= read_data;
                    end if;
                when write_data =>
                    num_wait_cycles <= to_unsigned(7, num_wait_cycles'length);
                    start_idx <= to_unsigned(7, start_idx'length);
                    bit_count <= to_unsigned(7, bit_count'length);
                    i2c_wait <= '1';
                    enable_create_start_cond <= '0';
                    enable_enable_scl <= '0';
                    disable_enable_scl <= '0';
                    enable_write_slave_addr_write_mode <= '0';
                    enable_check_ack <= '0';
                    enable_write_word_addr <= '0';
                    enable_write_slave_addr_read_mode <= '0';
                    enable_repeated_start <= '0';
                    enable_read_data <= '0';
                    enable_send_ack <= '0';
                    enable_create_stop_cond <= '0';
                    enable_write_data <= '1';
                    enable_write_to_ram <= '0';
                    if (is_ack_received = '1') then
                        next_state <= receive_ack;
                        prev_state <= write_data;
                    else
                        next_state <= create_stop_cond;
                        prev_state <= write_data;
                    end if;
                when send_ack =>
                    num_wait_cycles <= to_unsigned(0, num_wait_cycles'length);
                    start_idx <= to_unsigned(0, start_idx'length);
                    bit_count <= to_unsigned(0, bit_count'length);
                    i2c_wait <= '0';
                    enable_create_start_cond <= '0';
                    enable_enable_scl <= '0';
                    disable_enable_scl <= '0';
                    enable_write_slave_addr_write_mode <= '0';
                    enable_check_ack <= '0';
                    enable_write_word_addr <= '0';
                    enable_write_slave_addr_read_mode <= '0';
                    enable_repeated_start <= '0';
                    enable_read_data <= '0';
                    enable_send_ack <= '1';
                    enable_create_stop_cond <= '0';
                    enable_write_data <= '0';
                    enable_write_to_ram <= '0';
                    next_state <= create_stop_cond;
                    prev_state <= send_ack;
                when create_stop_cond =>
                    num_wait_cycles <= to_unsigned(0, num_wait_cycles'length);
                    start_idx <= to_unsigned(0, start_idx'length);
                    bit_count <= to_unsigned(0, bit_count'length);
                    i2c_wait <= '0';
                    enable_create_start_cond <= '0';
                    enable_enable_scl <= '0';
                    disable_enable_scl <= '1';
                    enable_write_slave_addr_write_mode <= '0';
                    enable_check_ack <= '0';
                    enable_write_word_addr <= '0';
                    enable_write_slave_addr_read_mode <= '0';
                    enable_repeated_start <= '0';
                    enable_read_data <= '0';
                    enable_send_ack <= '0';
                    enable_create_stop_cond <= '1';
                    enable_write_data <= '0';
                    enable_write_to_ram <= '0';
                    next_state <= write_to_ram;
                    prev_state <= create_stop_cond;
                when write_to_ram =>
                    num_wait_cycles <= to_unsigned(200, num_wait_cycles'length);
                    start_idx <= to_unsigned(0, start_idx'length);
                    bit_count <= to_unsigned(0, bit_count'length);
                    i2c_wait <= '1';
                    enable_create_start_cond <= '0';
                    enable_enable_scl <= '0';
                    disable_enable_scl <= '0';
                    enable_write_slave_addr_write_mode <= '0';
                    enable_check_ack <= '0';
                    enable_write_word_addr <= '0';
                    enable_write_slave_addr_read_mode <= '0';
                    enable_repeated_start <= '0';
                    enable_read_data <= '0';
                    enable_send_ack <= '0';
                    enable_create_stop_cond <= '0';
                    enable_write_data <= '0';
                    enable_write_to_ram <= '1';
                    next_state <= i2c_nop;
                    prev_state <= write_to_ram;
            end case;
        end if;

    end process state_machine;

    write_data_proc : process(all) is
        variable index : natural;
    begin

        if (reset = '1') then
            sda_output <= '1';
            write_bits_used <= to_unsigned(0, write_bits_used'length);
            sda_output_enable <= '1';
            index := 0;
        elsif (rising_edge(clk_200khz)) then
            index := to_integer(start_idx - write_bits_used);
            if (enable_create_start_cond = '1') then
                sda_output_enable <= '1';
                sda_output <= '0';
            elsif (enable_write_slave_addr_write_mode = '1' and clk_100khz = '0') then
                sda_output_enable <= '1';
                sda_output <= slave_address_write(index);
                if (write_bits_used < bit_count) then
                    write_bits_used <= write_bits_used + 1;
                else
                    write_bits_used <= to_unsigned(0, write_bits_used'length);
                end if;
            elsif (enable_write_slave_addr_read_mode = '1' and clk_100khz = '0') then
                sda_output_enable <= '1';
                sda_output <= slave_address_read(index);
                if (write_bits_used < bit_count) then
                    write_bits_used <= write_bits_used + 1;
                else
                    write_bits_used <= to_unsigned(0, write_bits_used'length);
                end if;
            elsif (enable_write_word_addr = '1' and clk_100khz = '0') then
                sda_output_enable <= '1';
                sda_output <= address_in(index);
                if (write_bits_used < bit_count) then
                    write_bits_used <= write_bits_used + 1;
                else
                    write_bits_used <= to_unsigned(0, write_bits_used'length);
                end if;
            elsif (enable_write_data = '1' and clk_100khz = '0') then
                sda_output_enable <= '1';
                sda_output <= data_in(index);
                if (write_bits_used < bit_count) then
                    write_bits_used <= write_bits_used + 1;
                else
                    write_bits_used <= to_unsigned(0, write_bits_used'length);
                end if;
            elsif (enable_check_ack = '1' and clk_100khz = '0') then
                sda_output_enable <= '0';
            elsif (enable_repeated_start = '1' and clk_100khz = '0') then
                sda_output_enable <= '1';
                sda_output <= '1';
            elsif (enable_repeated_start = '1' and clk_100khz = '1') then
                sda_output_enable <= '1';
                sda_output <= '0';
            elsif (enable_send_ack = '1' and clk_100khz = '0') then
                sda_output_enable <= '1';
                sda_output <= '1';
            elsif (enable_create_stop_cond = '1' and clk_100khz = '0') then
                sda_output_enable <= '1';
                sda_output <= '0';
            elsif (enable_create_stop_cond = '1' and clk_100khz = '1') then
                sda_output_enable <= '1';
                sda_output <= '1';
            end if;
        end if;

    end process write_data_proc;

    read_data_proc : process(all) is
        variable index : natural;
    begin

        if (reset = '1') then
            data_out <= (others => '0');
            read_bits_used <= to_unsigned(0, read_bits_used'length);
            index := 0;
        elsif (falling_edge(clk_100khz) and enable_read_data = '1') then
            index := to_integer(start_idx - read_bits_used);
            data_out(index) <= sda_input;
            if (read_bits_used < bit_count) then
                read_bits_used <= read_bits_used + 1;
            else
                read_bits_used <= to_unsigned(0, read_bits_used'length);
            end if;
        end if;

    end process read_data_proc;

    do_enable_scl : process(all) is
    begin

        if (reset = '1') then
            scl_enable <= '0';
        elsif (rising_edge(clk_200khz) and clk_100khz = '1') then
            if (enable_enable_scl = '1') then
                scl_enable <= '1';
            elsif (disable_enable_scl = '1') then
                scl_enable <= '0';
            end if;
        end if;

    end process do_enable_scl;

    do_check_ack : process(all) is
    begin

        if (reset = '1') then
            is_ack_received <= '0';
        elsif (rising_edge(clk_200khz) and clk_100khz = '1') then
            if (enable_check_ack = '1') then
                if (sda_input = '0') then
                    is_ack_received <= '1';
                else
                    is_ack_received <= '0';
                end if;
            else
                is_ack_received <= '0';
            end if;
        end if;

    end process do_check_ack;

end architecture rtl;
