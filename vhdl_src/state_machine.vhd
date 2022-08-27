library ieee;
use ieee.std_logic_1164.all;
use work.states_package.all;

entity state_machine is
    port (
        clk                    : in    std_logic;
        reset                  : in    std_logic;
        trig_state_machine     : in    std_logic;
        instruction_type       : in    std_logic_vector(2 downto 0);
        ram_read_enable        : out   std_logic;
        alu_input_mux_enable   : out   std_logic;
        alu_enable             : out   std_logic;
        wreg_enable            : out   std_logic;
        ram_write_enable       : out   std_logic;
        mem_dump_enable        : out   std_logic;
        status_write_enable    : out   std_logic;
        result_enable          : out   std_logic;
        timer_write_enable     : out   std_logic;
        result_enable_mem_dump : out   std_logic
    );
end entity state_machine;

architecture rtl of state_machine is

    signal state      : pic_state;
    signal next_state : pic_state;

begin

    state_change : process (clk, reset) is
    begin

        if (reset = '1') then
            state <= do_nop;
        elsif (falling_edge(clk)) then
            state <= next_state;
        end if;

    end process state_change;

    func : process (all) is
    begin

        if (reset = '1') then
            ram_read_enable        <= '0';
            ram_write_enable       <= '0';
            mem_dump_enable        <= '0';
            alu_input_mux_enable   <= '0';
            alu_enable             <= '0';
            wreg_enable            <= '0';
            result_enable          <= '0';
            result_enable_mem_dump <= '0';
            status_write_enable    <= '0';
            timer_write_enable     <= '0';
            next_state             <= do_nop;
        elsif (rising_edge(clk)) then

            case state is

                when do_nop =>

                    mem_dump_enable        <= '0';
                    ram_read_enable        <= '0';
                    alu_input_mux_enable   <= '0';
                    alu_enable             <= '0';
                    wreg_enable            <= '0';
                    ram_write_enable       <= '0';
                    mem_dump_enable        <= '0';
                    result_enable          <= '0';
                    result_enable_mem_dump <= '0';
                    status_write_enable    <= '0';
                    timer_write_enable     <= '0';
                    if (trig_state_machine = '0') then
                        next_state <= do_nop;
                    else
                        if (instruction_type = "101") then
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

                    ram_read_enable        <= '0';
                    mem_dump_enable        <= '0';
                    alu_input_mux_enable   <= '0';
                    alu_enable             <= '0';
                    wreg_enable            <= '0';
                    ram_write_enable       <= '0';
                    result_enable          <= '0';
                    result_enable_mem_dump <= '0';
                    status_write_enable    <= '0';
                    timer_write_enable     <= '0';
                    next_state             <= do_result;

                when do_ram_read =>

                    ram_read_enable        <= '1';
                    mem_dump_enable        <= '0';
                    alu_input_mux_enable   <= '0';
                    alu_enable             <= '0';
                    wreg_enable            <= '0';
                    ram_write_enable       <= '0';
                    result_enable          <= '0';
                    result_enable_mem_dump <= '0';
                    status_write_enable    <= '0';
                    timer_write_enable     <= '0';
                    next_state             <= do_alu_input_sel;

                when do_ram_dump =>

                    ram_read_enable        <= '0';
                    mem_dump_enable        <= '1';
                    alu_input_mux_enable   <= '0';
                    alu_enable             <= '0';
                    wreg_enable            <= '0';
                    ram_write_enable       <= '0';
                    result_enable          <= '0';
                    result_enable_mem_dump <= '0';
                    status_write_enable    <= '0';
                    timer_write_enable     <= '0';
                    next_state             <= do_wait;

                when do_alu_input_sel =>

                    ram_read_enable        <= '0';
                    mem_dump_enable        <= '0';
                    alu_input_mux_enable   <= '1';
                    alu_enable             <= '0';
                    wreg_enable            <= '0';
                    ram_write_enable       <= '0';
                    result_enable          <= '0';
                    result_enable_mem_dump <= '0';
                    status_write_enable    <= '0';
                    timer_write_enable     <= '0';
                    next_state             <= do_alu;

                when do_alu =>

                    ram_read_enable        <= '0';
                    mem_dump_enable        <= '0';
                    alu_input_mux_enable   <= '0';
                    alu_enable             <= '1';
                    wreg_enable            <= '0';
                    ram_write_enable       <= '0';
                    result_enable          <= '0';
                    result_enable_mem_dump <= '0';
                    timer_write_enable     <= '0';
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

                    ram_read_enable        <= '0';
                    mem_dump_enable        <= '0';
                    alu_input_mux_enable   <= '0';
                    alu_enable             <= '0';
                    wreg_enable            <= '1';
                    ram_write_enable       <= '0';
                    result_enable          <= '0';
                    result_enable_mem_dump <= '0';
                    status_write_enable    <= '1';
                    timer_write_enable     <= '1';
                    next_state             <= do_nop;

                when do_ram_write =>

                    ram_read_enable        <= '0';
                    mem_dump_enable        <= '0';
                    alu_input_mux_enable   <= '0';
                    alu_enable             <= '0';
                    wreg_enable            <= '0';
                    ram_write_enable       <= '1';
                    result_enable          <= '0';
                    result_enable_mem_dump <= '0';
                    status_write_enable    <= '1';
                    timer_write_enable     <= '1';
                    next_state             <= do_nop;

                when do_result =>

                    if (instruction_type = "101") then
                        ram_read_enable        <= '0';
                        mem_dump_enable        <= '0';
                        alu_input_mux_enable   <= '0';
                        alu_enable             <= '0';
                        wreg_enable            <= '0';
                        ram_write_enable       <= '0';
                        result_enable          <= '0';
                        result_enable_mem_dump <= '1';
                        status_write_enable    <= '0';
                        timer_write_enable     <= '1';
                        next_state             <= do_nop;
                    else
                        ram_read_enable        <= '0';
                        mem_dump_enable        <= '0';
                        alu_input_mux_enable   <= '0';
                        alu_enable             <= '0';
                        wreg_enable            <= '0';
                        ram_write_enable       <= '0';
                        result_enable          <= '1';
                        result_enable_mem_dump <= '0';
                        status_write_enable    <= '0';
                        timer_write_enable     <= '1';
                        next_state             <= do_nop;
                    end if;

            end case;

        end if;

    end process func;

end architecture rtl;
