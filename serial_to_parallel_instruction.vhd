library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;
use work.states_package.all;

entity serial_to_parallel_instruction is
	generic (N : natural := 8;
				M : natural := 6);
	port (clk : in std_logic;
			reset : in std_logic;
			serial_in : in std_logic;
			trig_state_machine : out std_logic;
			instruction_type : out std_logic_vector(1 downto 0);
			sel_alu_input_mux : out std_logic;
			d : out std_logic;
			operand_out : out std_logic_vector(N-1 downto 0);
			address_out : out std_logic_vector(N-1 downto 0);
			opcode_out : out std_logic_vector(M-1 downto 0));
end serial_to_parallel_instruction;

architecture rtl of serial_to_parallel_instruction is
	signal code_word : std_logic_vector(5 downto 0);
	signal counter : integer := 0;
begin

	func: process(all) is
	begin

		if (reset = '1') then
			operand_out <= (others => '0');
			address_out <= (others => '0');
			opcode_out <= (others => '0');
			code_word <= (others => '0');
			counter <= 0;
			sel_alu_input_mux <= '0';
			d <= '0';
			instruction_type <= (others => '0');
			trig_state_machine <= '0';

		elsif (rising_edge(clk)) then

			if (counter >= N + M) then
				if (sel_alu_input_mux = '0' and d = '0') then
					instruction_type <= "00";
				elsif (sel_alu_input_mux = '1' and d = '0') then
					instruction_type <= "01";
				elsif (sel_alu_input_mux = '1' and d = '1') then
					instruction_type <= "10";
				else
					instruction_type <= "11";
				end if;

				if (counter <= N + M + 1) then
					counter <= counter + 1;
					trig_state_machine <= '1';
				elsif (counter <= N + M + 5) then
					counter <= counter + 1;
					trig_state_machine <= '0';
				else
					trig_state_machine <= '0';
					code_word <= (others => '0');
					counter <= 0;
				end if;

			elsif (counter >= M and counter <= N + M - 1 and code_word = "000110") then
				if (counter = M) then
					if (opcode_out = "000111" or opcode_out = "000101" or opcode_out = "001001" or
						 opcode_out = "000011" or opcode_out = "001011" or opcode_out = "001010" or
						 opcode_out = "001111" or opcode_out = "000100" or opcode_out = "001000" or
						 opcode_out = "000010" or opcode_out = "000110") then
						sel_alu_input_mux <= '1';
						d <= serial_in;
					else
						sel_alu_input_mux <= '0';
						d <= '0';
					end if;
				end if;
				operand_out <= operand_out(N-2 downto 0) & serial_in;
				address_out <= operand_out(N-2 downto 0) & serial_in;
				counter <= counter + 1;

			elsif (counter <= M - 1 and code_word = "000110") then
				opcode_out <= opcode_out(M-2 downto 0) & serial_in;
				operand_out <= (others => '0');
				counter <= counter + 1;

			elsif (counter = 0) then
				opcode_out <= (others => '0');
				operand_out <= (others => '0');
				code_word <= code_word(4 downto 0) & serial_in;

			end if;
		end if;
	end process func;
end architecture rtl;
