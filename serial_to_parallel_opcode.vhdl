library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity serial_to_parallel_opcode is
	generic (N : natural := 6);
	port (clk : in std_logic;
			reset : in std_logic;
			enable : in std_logic;
			serial_in : in std_logic;
			parallel_out : out std_logic_vector(N-1 downto 0));
end serial_to_parallel_opcode;

architecture rtl of serial_to_parallel_opcode is
	signal code_word : std_logic_vector(5 downto 0);
begin

	func: process(all) is
		variable counter : integer;
	begin

		if (reset = '1') then
			parallel_out <= (others => '0');
			code_word <= (others => '0');
			counter := 0;

		elsif (rising_edge(clk)) then
			if (enable = '1' and counter <= N-1 and code_word = "110011") then
				--parallel_out(counter) <= serial_in;
				parallel_out <= parallel_out(N-2 downto 0) & serial_in;
				counter := counter + 1;
			elsif (enable = '1') then
				parallel_out <= (others => '0');
				code_word <= code_word(4 downto 0) & serial_in;
			elsif (enable = '0') then
				code_word <= (others => '0');
				counter := 0;
			end if;
		end if;
	end process func;
end architecture rtl;