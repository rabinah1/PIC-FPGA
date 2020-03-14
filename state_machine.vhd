library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity state_machine is
	port (clk : in std_logic;
			reset : in std_logic;
			trig_state_machine : in std_logic;
			alu_enable : out std_logic;
			wreg_enable : out std_logic;
			result_enable : out std_logic);
end state_machine;

architecture rtl of state_machine is

	type t_state is (do_alu, do_wreg, do_result);
	signal state : t_state;
	
begin

	func : process(all) is
		variable counter : integer := 0;
	begin

		if (reset = '1') then
			alu_enable <= '0';
			wreg_enable <= '0';
			result_enable <= '0';
			state <= do_alu;
			counter := 0;
			
		elsif (rising_edge(clk)) then
			case state is

				when do_alu =>
					alu_enable <= '1';
					if (trig_state_machine = '1') then
						counter := counter + 1;
					end if;
					if (counter = 100) then
						counter := 0;
						alu_enable <= '0';
						state <= do_wreg;
					end if;
					
				when do_wreg =>
					wreg_enable <= '1';
					counter := counter + 1;
					if (counter = 100) then
						counter := 0;
						wreg_enable <= '0';
						state <= do_result;
					end if;
				
				when do_result =>
					result_enable <= '1';
					counter := counter + 1;
					if (counter = 100) then
						counter := 0;
						result_enable <= '0';
						state <= do_alu;
					end if;

			end case;
		end if;
	end process func;
end architecture rtl;