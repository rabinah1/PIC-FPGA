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
	signal delay : integer := 10;
	
begin

	func : process(all) is
	begin

		if (reset = '1') then
			alu_enable <= '0';
			wreg_enable <= '0';
			result_enable <= '0';
			state <= do_alu;
			delay <= 10;
			
		elsif (rising_edge(clk)) then

			if delay > 0 then
				delay <= delay - 1;
			end if;

			case state is
				when do_alu =>
					if (trig_state_machine = '1') then
						if delay = 0 then
							delay <= 10;
							alu_enable <= '1';
							wreg_enable <= '0';
							result_enable <= '0';
							state <= do_wreg;
						end if;
					end if;

				when do_wreg =>
					if delay = 0 then
						delay <= 10;
						alu_enable <= '0';
						wreg_enable <= '1';
						result_enable <= '0';
						state <= do_result;
					end if;
				
				when do_result =>
					if delay = 0 then
						delay <= 10;
						alu_enable <= '0';
						wreg_enable <= '0';
						result_enable <= '1';
						state <= do_alu;
					end if;

			end case;
		end if;
	end process func;
end architecture rtl;