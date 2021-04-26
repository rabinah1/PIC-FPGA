library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity parallel_to_serial_output is
	generic (N : natural := 8);
	port (clk : in std_logic;
			reset : in std_logic;
			enable : in std_logic;
			result_enable_mem_dump : in std_logic;
			data_to_sw : in std_logic_vector(N-1 downto 0);
			mem_dump_to_sw : in std_logic_vector(1015 downto 0);
			serial_out : out std_logic);
end parallel_to_serial_output;

architecture rtl of parallel_to_serial_output is
	signal data_to_send : std_logic_vector(13 downto 0);
	signal mem_to_send : std_logic_vector(1021 downto 0);
begin

	func: process(all) is
		variable idx : integer := 0;
		variable mem_dump_process : std_logic := '0';
		variable data_send_process : std_logic := '0';
	begin

		if (reset = '1') then
			serial_out <= '0';
			data_to_send <= (others => '0');
			mem_to_send <= (others => '0');
			idx := 0;
			mem_dump_process := '0';
			data_send_process := '0';

		elsif (rising_edge(clk)) then
			if (idx > 0) then
				if (data_send_process = '1') then
					serial_out <= data_to_send(idx-1);
				elsif (mem_dump_process = '1') then
					serial_out <= mem_to_send(idx-1);
				end if;
				idx := idx - 1;
			elsif (result_enable_mem_dump = '1') then
				mem_dump_process := '1';
				serial_out <= '0';
				idx := 1022;
			elsif (enable = '1') then
				data_send_process := '1';
				serial_out <= '0';
				idx := 14;
			elsif (enable = '0') then
				data_to_send <= "000101" & data_to_sw;
				mem_to_send <= "000101" & mem_dump_to_sw;
				idx := 0;
				data_send_process := '0';
				mem_dump_process := '0';
			end if;
		end if;
	end process func;
end architecture rtl;
