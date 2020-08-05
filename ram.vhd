library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity ram is
	port (clk : in std_logic;
			reset : in std_logic;
			data : in std_logic_vector(7 downto 0);
			address : in std_logic_vector(7 downto 0);
			write_enable : in std_logic;
			read_enable : in std_logic;
			data_out : out std_logic_vector(7 downto 0));
end ram;

architecture rtl of ram is
	type memory is array(67 downto 0) of std_logic_vector(7 downto 0);
	signal ram_block : memory;
begin

	func : process(all) is
	begin
	
		if (reset = '1') then
			ram_block <= (others => (others => '0'));
			data_out <= (others => '0');
		
		elsif (rising_edge(clk)) then
			if (read_enable = '1') then
				data_out <= ram_block(to_integer(unsigned(address(6 downto 0))));
			end if;
			if (write_enable = '1') then
				ram_block(to_integer(unsigned(address(6 downto 0)))) <= data;
			end if;
		end if;
	end process func;
end architecture rtl;