library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity serial_to_parallel_instruction is
	generic (N : natural := 8;
				M : natural := 6);
	port (clk : in std_logic;
			reset : in std_logic;
			serial_in : in std_logic;
			trig_state_machine : out std_logic;
			operand_out : out std_logic_vector(N-1 downto 0);
			opcode_out : out std_logic_vector(M-1 downto 0));
end serial_to_parallel_instruction;

architecture rtl of serial_to_parallel_instruction is
	signal code_word : std_logic_vector(5 downto 0);
begin

	func: process(all) is
		variable counter : integer := 0;
		variable state_machine_counter : integer := 0;
	begin

		if (reset = '1') then
			operand_out <= (others => '0');
			opcode_out <= (others => '0');
			code_word <= (others => '0');
			trig_state_machine <= '0';
			counter := 0;
			state_machine_counter := 0;

		elsif (rising_edge(clk)) then

			if (counter >= N + M) then
				if (state_machine_counter >= 120) then
					trig_state_machine <= '0';
					counter := 0;
					code_word <= (others => '0');
				end if;
				state_machine_counter := state_machine_counter + 1;

			elsif (counter >= M and counter <= N + M - 1 and code_word = "000110") then
				operand_out <= operand_out(N-2 downto 0) & serial_in;
				trig_state_machine <= '1';
				counter := counter + 1;

			elsif (counter <= M - 1 and code_word = "000110") then
				opcode_out <= opcode_out(M-2 downto 0) & serial_in;
				operand_out <= (others => '0');
				trig_state_machine <= '0';
				counter := counter + 1;

			elsif (counter = 0) then
				opcode_out <= (others => '0');
				operand_out <= (others => '0');
				trig_state_machine <= '0';
				state_machine_counter := 0;
				code_word <= code_word(4 downto 0) & serial_in;

			end if;
		end if;
	end process func;
end architecture rtl;
