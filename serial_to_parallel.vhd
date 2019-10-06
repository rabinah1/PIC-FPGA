library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity serial_to_parallel is
	generic (N : natural := 8);
	port (clk : in std_logic;
			reset : in std_logic;
			enable : in std_logic;
			serial_in : in std_logic;
			parallel_out : out std_logic_vector(N-1 downto 0));
end serial_to_parallel;

architecture rtl of serial_to_parallel is
begin

	func: process(all) is
		variable counter : integer;
	begin

		if (reset = '1') then
			parallel_out <= (others => '0');
			counter := 0;

		elsif (falling_edge(clk)) then
			if (enable = '1' and counter <= N-1) then
				parallel_out(counter) <= serial_in;
				counter := counter + 1;
			elsif (enable = '0') then
				counter := 0;
			end if;
		end if;
	end process func;
end architecture rtl;