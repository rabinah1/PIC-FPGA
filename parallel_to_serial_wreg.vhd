library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity parallel_to_serial_wreg is
	generic (N : natural := 8);
	port (clk : in std_logic;
			reset : in std_logic;
			enable : in std_logic;
			parallel_in : in std_logic_vector(N-1 downto 0);
			serial_out : out std_logic);
end parallel_to_serial_wreg;

architecture rtl of parallel_to_serial_wreg is
	signal data_to_send : std_logic_vector(13 downto 0);
begin

	func: process(all) is
		variable idx : integer := 13;
	begin
	
		if (reset = '1') then
			serial_out <= '0';
			data_to_send <= (others => '0');
			idx := 13;
		
		elsif (rising_edge(clk)) then
			if (enable = '1' and idx >= 0) then
				serial_out <= data_to_send(idx);
				idx := idx - 1;
			elsif (enable = '1') then
				serial_out <= '0';
			elsif (enable = '0') then
				data_to_send <= "000101" & parallel_in;
				idx := 13;
			end if;
		end if;
	end process func;
end architecture rtl;
