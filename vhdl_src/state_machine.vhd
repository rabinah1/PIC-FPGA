library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;
use work.states_package.all;

entity state_machine is
    port (
        clk                       : in    std_logic;
        reset                     : in    std_logic;
        trig_state_machine        : in    std_logic;
        instruction_type          : in    std_logic_vector(2 downto 0);
        ram_read_enable           : out   std_logic;
        alu_input_mux_enable      : out   std_logic;
        alu_enable                : out   std_logic;
        wreg_enable               : out   std_logic;
        ram_write_enable          : out   std_logic;
        ram_dump_enable           : out   std_logic;
        eeprom_dump_enable        : out   std_logic;
        status_write_enable       : out   std_logic;
        result_enable             : out   std_logic;
        timer_write_enable        : out   std_logic;
        result_enable_ram_dump    : out   std_logic;
        result_enable_eeprom_dump : out   std_logic
    );
end entity state_machine;

architecture rtl of state_machine is

    signal state                 : pic_state;
    signal next_state            : pic_state;
    signal wait_for_state_change : std_logic;
    signal num_wait_cycles       : unsigned(11 downto 0);
    signal cycles_waited         : unsigned(11 downto 0);

begin

    state_change : process (clk, reset) is
    begin

        if (reset = '1') then
            state         <= do_nop;
            cycles_waited <= to_unsigned(0, cycles_waited'length);
        elsif (falling_edge(clk)) then
            if (wait_for_state_change = '1' and cycles_waited < num_wait_cycles) then
                cycles_waited <= cycles_waited + 1;
            else
                cycles_waited <= to_unsigned(0, cycles_waited'length);
                state         <= next_state;
            end if;
        end if;

    end process state_change;

    state_machine : process (all) is
    begin

        if (reset = '1') then
            wait_for_state_change     <= '0';
            num_wait_cycles           <= to_unsigned(0, num_wait_cycles'length);
            ram_read_enable           <= '0';
            ram_write_enable          <= '0';
            ram_dump_enable           <= '0';
            eeprom_dump_enable        <= '0';
            alu_input_mux_enable      <= '0';
            alu_enable                <= '0';
            wreg_enable               <= '0';
            result_enable             <= '0';
            result_enable_ram_dump    <= '0';
            result_enable_eeprom_dump <= '0';
            status_write_enable       <= '0';
            timer_write_enable        <= '0';
            next_state                <= do_nop;
        elsif (rising_edge(clk)) then

            case state is

                when do_nop =>

                    wait_for_state_change     <= '0';
                    num_wait_cycles           <= to_unsigned(0, num_wait_cycles'length);
                    ram_read_enable           <= '0';
                    alu_input_mux_enable      <= '0';
                    alu_enable                <= '0';
                    wreg_enable               <= '0';
                    ram_write_enable          <= '0';
                    ram_dump_enable           <= '0';
                    eeprom_dump_enable        <= '0';
                    result_enable             <= '0';
                    result_enable_ram_dump    <= '0';
                    result_enable_eeprom_dump <= '0';
                    status_write_enable       <= '0';
                    timer_write_enable        <= '0';
                    if (trig_state_machine = '0') then
                        next_state <= do_nop;
                    else
                        if (instruction_type = "110") then
                            next_state <= do_eeprom_dump;
                        elsif (instruction_type = "101") then
                            next_state <= do_ram_dump;
                        elsif (instruction_type = "000") then
                            next_state <= do_alu_input_sel;
                        elsif (instruction_type = "001" or instruction_type = "010" or instruction_type = "100") then
                            next_state <= do_ram_read;
                        elsif (instruction_type = "011") then
                            next_state <= do_alu;
                        else
                            next_state <= do_nop;
                        end if;
                    end if;

                when do_wait =>

                    wait_for_state_change     <= '0';
                    num_wait_cycles           <= to_unsigned(0, num_wait_cycles'length);
                    ram_read_enable           <= '0';
                    ram_dump_enable           <= '0';
                    eeprom_dump_enable        <= '0';
                    alu_input_mux_enable      <= '0';
                    alu_enable                <= '0';
                    wreg_enable               <= '0';
                    ram_write_enable          <= '0';
                    result_enable             <= '0';
                    result_enable_ram_dump    <= '0';
                    result_enable_eeprom_dump <= '0';
                    status_write_enable       <= '0';
                    timer_write_enable        <= '0';
                    next_state                <= do_result;

                when do_ram_read =>

                    wait_for_state_change     <= '0';
                    num_wait_cycles           <= to_unsigned(0, num_wait_cycles'length);
                    ram_read_enable           <= '1';
                    ram_dump_enable           <= '0';
                    eeprom_dump_enable        <= '0';
                    alu_input_mux_enable      <= '0';
                    alu_enable                <= '0';
                    wreg_enable               <= '0';
                    ram_write_enable          <= '0';
                    result_enable             <= '0';
                    result_enable_ram_dump    <= '0';
                    result_enable_eeprom_dump <= '0';
                    status_write_enable       <= '0';
                    timer_write_enable        <= '0';
                    next_state                <= do_alu_input_sel;

                when do_ram_dump =>

                    wait_for_state_change     <= '0';
                    num_wait_cycles           <= to_unsigned(0, num_wait_cycles'length);
                    ram_read_enable           <= '0';
                    ram_dump_enable           <= '1';
                    eeprom_dump_enable        <= '0';
                    alu_input_mux_enable      <= '0';
                    alu_enable                <= '0';
                    wreg_enable               <= '0';
                    ram_write_enable          <= '0';
                    result_enable             <= '0';
                    result_enable_ram_dump    <= '0';
                    result_enable_eeprom_dump <= '0';
                    status_write_enable       <= '0';
                    timer_write_enable        <= '0';
                    next_state                <= do_wait;

                when do_eeprom_dump =>

                    wait_for_state_change     <= '1';
                    num_wait_cycles           <= to_unsigned(1792, num_wait_cycles'length);
                    ram_read_enable           <= '0';
                    ram_dump_enable           <= '0';
                    eeprom_dump_enable        <= '1';
                    alu_input_mux_enable      <= '0';
                    alu_enable                <= '0';
                    wreg_enable               <= '0';
                    ram_write_enable          <= '0';
                    result_enable             <= '0';
                    result_enable_ram_dump    <= '0';
                    result_enable_eeprom_dump <= '0';
                    status_write_enable       <= '0';
                    timer_write_enable        <= '0';
                    next_state                <= do_wait;

                when do_alu_input_sel =>

                    wait_for_state_change     <= '0';
                    num_wait_cycles           <= to_unsigned(0, num_wait_cycles'length);
                    ram_read_enable           <= '0';
                    ram_dump_enable           <= '0';
                    eeprom_dump_enable        <= '0';
                    alu_input_mux_enable      <= '1';
                    alu_enable                <= '0';
                    wreg_enable               <= '0';
                    ram_write_enable          <= '0';
                    result_enable             <= '0';
                    result_enable_ram_dump    <= '0';
                    result_enable_eeprom_dump <= '0';
                    status_write_enable       <= '0';
                    timer_write_enable        <= '0';
                    next_state                <= do_alu;

                when do_alu =>

                    wait_for_state_change     <= '0';
                    num_wait_cycles           <= to_unsigned(0, num_wait_cycles'length);
                    ram_read_enable           <= '0';
                    ram_dump_enable           <= '0';
                    eeprom_dump_enable        <= '0';
                    alu_input_mux_enable      <= '0';
                    alu_enable                <= '1';
                    wreg_enable               <= '0';
                    ram_write_enable          <= '0';
                    result_enable             <= '0';
                    result_enable_ram_dump    <= '0';
                    result_enable_eeprom_dump <= '0';
                    timer_write_enable        <= '0';
                    if (instruction_type = "000" or instruction_type = "001") then
                        next_state <= do_wreg;
                    elsif (instruction_type = "010") then
                        next_state <= do_ram_write;
                    elsif (instruction_type = "100" or instruction_type = "011") then
                        next_state <= do_wait;
                    else
                        next_state <= do_nop;
                    end if;

                when do_wreg =>

                    wait_for_state_change     <= '0';
                    num_wait_cycles           <= to_unsigned(0, num_wait_cycles'length);
                    ram_read_enable           <= '0';
                    ram_dump_enable           <= '0';
                    eeprom_dump_enable        <= '0';
                    alu_input_mux_enable      <= '0';
                    alu_enable                <= '0';
                    wreg_enable               <= '1';
                    ram_write_enable          <= '0';
                    result_enable             <= '0';
                    result_enable_ram_dump    <= '0';
                    result_enable_eeprom_dump <= '0';
                    status_write_enable       <= '1';
                    timer_write_enable        <= '1';
                    next_state                <= do_nop;

                when do_ram_write =>

                    wait_for_state_change     <= '0';
                    num_wait_cycles           <= to_unsigned(0, num_wait_cycles'length);
                    ram_read_enable           <= '0';
                    ram_dump_enable           <= '0';
                    eeprom_dump_enable        <= '0';
                    alu_input_mux_enable      <= '0';
                    alu_enable                <= '0';
                    wreg_enable               <= '0';
                    ram_write_enable          <= '1';
                    result_enable             <= '0';
                    result_enable_ram_dump    <= '0';
                    result_enable_eeprom_dump <= '0';
                    status_write_enable       <= '1';
                    timer_write_enable        <= '1';
                    next_state                <= do_nop;

                when do_result =>

                    if (instruction_type = "110") then
                        wait_for_state_change     <= '0';
                        num_wait_cycles           <= to_unsigned(0, num_wait_cycles'length);
                        ram_read_enable           <= '0';
                        ram_dump_enable           <= '0';
                        eeprom_dump_enable        <= '0';
                        alu_input_mux_enable      <= '0';
                        alu_enable                <= '0';
                        wreg_enable               <= '0';
                        ram_write_enable          <= '0';
                        result_enable             <= '0';
                        result_enable_ram_dump    <= '0';
                        result_enable_eeprom_dump <= '1';
                        status_write_enable       <= '0';
                        timer_write_enable        <= '1';
                        next_state                <= do_nop;
                    elsif (instruction_type = "101") then
                        wait_for_state_change     <= '0';
                        num_wait_cycles           <= to_unsigned(0, num_wait_cycles'length);
                        ram_read_enable           <= '0';
                        ram_dump_enable           <= '0';
                        eeprom_dump_enable        <= '0';
                        alu_input_mux_enable      <= '0';
                        alu_enable                <= '0';
                        wreg_enable               <= '0';
                        ram_write_enable          <= '0';
                        result_enable             <= '0';
                        result_enable_ram_dump    <= '1';
                        result_enable_eeprom_dump <= '0';
                        status_write_enable       <= '0';
                        timer_write_enable        <= '1';
                        next_state                <= do_nop;
                    else
                        wait_for_state_change     <= '0';
                        num_wait_cycles           <= to_unsigned(0, num_wait_cycles'length);
                        ram_read_enable           <= '0';
                        ram_dump_enable           <= '0';
                        eeprom_dump_enable        <= '0';
                        alu_input_mux_enable      <= '0';
                        alu_enable                <= '0';
                        wreg_enable               <= '0';
                        ram_write_enable          <= '0';
                        result_enable             <= '1';
                        result_enable_ram_dump    <= '0';
                        result_enable_eeprom_dump <= '0';
                        status_write_enable       <= '0';
                        timer_write_enable        <= '1';
                        next_state                <= do_nop;
                    end if;

            end case;

        end if;

    end process state_machine;

end architecture rtl;
