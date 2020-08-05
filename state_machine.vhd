library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;
use work.states_package.all;

entity state_machine is
	port (clk : in std_logic;
			reset : in std_logic;
			trig_state_machine : in std_logic;
			instruction_type : in std_logic_vector(1 downto 0);
			ram_read_enable : out std_logic;
			alu_input_mux_enable : out std_logic;
			alu_enable : out std_logic;
			wreg_enable : out std_logic;
			ram_write_enable : out std_logic;
			result_enable : out std_logic);
end state_machine;

architecture rtl of state_machine is

	signal state : t_state;
	signal next_state : t_state;
	
begin

	state_change : process(clk, reset) is
	begin

		if (reset = '1') then
			state <= do_nop;
		elsif (falling_edge(clk)) then
			state <= next_state;
		end if;

	end process state_change;

	func : process(all) is
	begin

		if (reset = '1') then
			ram_read_enable <= '0';
			ram_write_enable <= '0';
			alu_input_mux_enable <= '0';
			alu_enable <= '0';
			wreg_enable <= '0';
			result_enable <= '0';
			next_state <= do_nop;
			
		elsif (rising_edge(clk)) then

			case state is
				when do_nop =>
					ram_read_enable <= '0';
					alu_input_mux_enable <= '0';
					alu_enable <= '0';
					wreg_enable <= '0';
					ram_write_enable <= '0';
					result_enable <= '0';

					if (trig_state_machine = '0') then
						next_state <= do_nop;
					else
						if (instruction_type = "00") then
							next_state <= do_alu_input_sel;
						elsif (instruction_type = "01" or instruction_type = "10") then
							next_state <= do_ram_read;
						else
							next_state <= do_nop;
						end if;
					end if;

				when do_ram_read =>
					ram_read_enable <= '1';
					alu_input_mux_enable <= '0';
					alu_enable <= '0';
					wreg_enable <= '0';
					ram_write_enable <= '0';
					result_enable <= '0';
					next_state <= do_alu_input_sel;

				when do_alu_input_sel =>
					ram_read_enable <= '0';
					alu_input_mux_enable <= '1';
					alu_enable <= '0';
					wreg_enable <= '0';
					ram_write_enable <= '0';
					result_enable <= '0';
					next_state <= do_alu;

				when do_alu =>
					ram_read_enable <= '0';
					alu_input_mux_enable <= '0';
					alu_enable <= '1';
					wreg_enable <= '0';
					ram_write_enable <= '0';
					result_enable <= '0';
					if (instruction_type = "00" or instruction_type = "01") then
						next_state <= do_wreg;
					elsif (instruction_type = "10") then
						next_state <= do_ram_write;
					else
						next_state <= do_nop;
					end if;

				when do_wreg =>
					ram_read_enable <= '0';
					alu_input_mux_enable <= '0';
					alu_enable <= '0';
					wreg_enable <= '1';
					ram_write_enable <= '0';
					result_enable <= '0';
					next_state <= do_result;

				when do_ram_write =>
					ram_read_enable <= '0';
					alu_input_mux_enable <= '0';
					alu_enable <= '0';
					wreg_enable <= '0';
					ram_write_enable <= '1';
					result_enable <= '0';
					next_state <= do_result;

				when do_result =>
					ram_read_enable <= '0';
					alu_input_mux_enable <= '0';
					alu_enable <= '0';
					wreg_enable <= '0';
					ram_write_enable <= '0';
					result_enable <= '1';
					next_state <= do_nop;
			end case;
		end if;
	end process func;
end architecture rtl;