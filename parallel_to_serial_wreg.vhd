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
			status_in : in std_logic_vector(N-1 downto 0);
			serial_out : out std_logic);
end parallel_to_serial_wreg;

architecture rtl of parallel_to_serial_wreg is
	signal data_to_send : std_logic_vector(21 downto 0);
	--signal idx : integer := 0;
begin

	func: process(all) is
		variable idx : integer := 0;
	begin
	
		if (reset = '1') then
			serial_out <= '0';
			data_to_send <= (others => '0');
			idx := 0;

		elsif (rising_edge(clk)) then
			if (idx > 0) then
				serial_out <= data_to_send(idx-1);
				idx := idx - 1;
			elsif (enable = '1') then
				serial_out <= '0';
				idx := 22;
			elsif (enable = '0') then
				data_to_send <= "000101" & parallel_in & status_in;
				idx := 0;
				--idx := 13;
			end if;
		end if;
	end process func;
end architecture rtl;
