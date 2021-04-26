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
			mem_dump_enable : in std_logic;
			mem_dump : out std_logic_vector(1015 downto 0);
			data_out : out std_logic_vector(7 downto 0));
end ram;

architecture rtl of ram is
	type memory is array(126 downto 0) of std_logic_vector(7 downto 0);
	signal ram_block : memory;
	
	function ram_to_linear(memory_data : in memory)
								  return std_logic_vector is
	variable linear_data : std_logic_vector(1015 downto 0);
	begin
		linear_data := (others => '0');
		for i in 0 to (memory_data'length)-1 loop
			linear_data(i*8 + 7 downto i*8) := memory_data(i);
		end loop;
		return linear_data;
	end ram_to_linear;

begin

	func : process(all) is
	begin

		if (reset = '1') then
			ram_block <= (others => (others => '0'));
			data_out <= (others => '0');
			mem_dump <= (others => '0');
		elsif (rising_edge(clk)) then
			if (mem_dump_enable = '1') then
				mem_dump <= ram_to_linear(ram_block);
			end if;
			if (read_enable = '1' and mem_dump_enable = '0') then
				data_out <= ram_block(to_integer(unsigned(address(6 downto 0))));
			end if;
			if (write_enable = '1' and mem_dump_enable = '0') then
				ram_block(to_integer(unsigned(address(6 downto 0)))) <= data;
			end if;
		end if;
	end process func;
end architecture rtl;