library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity alu_output_demux is
	port (clk : in std_logic;
			reset : in std_logic;
			d : in std_logic;
			input_data : in std_logic_vector(7 downto 0);
			data_to_ram : out std_logic_vector(7 downto 0);
			data_to_wreg : out std_logic_vector(7 downto 0));
end alu_output_demux;

architecture rtl of alu_output_demux is
begin

	func : process(all) is
	begin

		if (reset = '1') then
			data_to_ram <= (others => '0');
			data_to_wreg <= (others => '0');

		else
			if (d = '0') then
				data_to_wreg <= input_data;
				data_to_ram <= (others => '0');
			else
				data_to_ram <= input_data;
				data_to_wreg <= (others => '0');
			end if;
		end if;
	end process func;
end architecture rtl;