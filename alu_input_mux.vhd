library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity alu_input_mux is
	port (clk : in std_logic;
			reset : in std_logic;
			enable : in std_logic;
			input_ram : in std_logic_vector(7 downto 0);
			input_literal : in std_logic_vector(7 downto 0);
			sel : in std_logic;
			data_out : out std_logic_vector(7 downto 0));
end alu_input_mux;

architecture rtl of alu_input_mux is
begin

	func : process(all) is
	begin

		if (reset = '1') then
			data_out <= (others => '0');

		elsif (rising_edge(clk)) then
			if (enable = '1') then
				if (sel = '0') then
					data_out <= input_literal;
				elsif (sel = '1') then
					data_out <= input_ram;
				end if;
			end if;
		end if;
	end process func;
end architecture rtl;