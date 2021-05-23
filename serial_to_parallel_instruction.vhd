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
			mosi : in std_logic;
			trig_state_machine : out std_logic;
			instruction_type : out std_logic_vector(2 downto 0);
			sel_alu_input_mux : out std_logic;
			d : out std_logic;
			transfer_to_sw : out std_logic;
			operand_out : out std_logic_vector(N-1 downto 0);
			address_out : out std_logic_vector(N-1 downto 0);
			opcode_out : out std_logic_vector(M-1 downto 0));
end serial_to_parallel_instruction;

architecture rtl of serial_to_parallel_instruction is
	signal counter : integer := 0;
	signal dump_mem : std_logic;
begin

	-- Incoming data type: <6 bit opcode> + <7 bit address or operand>
	func: process(all) is
	begin

		if (reset = '1') then
			operand_out <= (others => '0');
			address_out <= (others => '0');
			opcode_out <= (others => '0');
			counter <= 0;
			sel_alu_input_mux <= '0';
			d <= '0';
			transfer_to_sw <= '0';
			instruction_type <= (others => '0');
			trig_state_machine <= '0';
			dump_mem <= '0';
		elsif (rising_edge(clk)) then
			if (counter >= N + M) then
				if (dump_mem = '1') then
					instruction_type <= "101";
				elsif (sel_alu_input_mux = '0' and d = '0' and transfer_to_sw = '0') then -- input from literal, output to wreg
					instruction_type <= "000";
				elsif (sel_alu_input_mux = '1' and d = '0' and transfer_to_sw = '0') then -- input from ram, output to wreg
					instruction_type <= "001";
				elsif (sel_alu_input_mux = '1' and d = '1' and transfer_to_sw = '0') then -- input from ram, output to ram
					instruction_type <= "010";
				elsif (transfer_to_sw = '1' and sel_alu_input_mux = '0') then
					instruction_type <= "011";
				elsif (transfer_to_sw = '1' and sel_alu_input_mux = '1') then
					instruction_type <= "100";
				else
					instruction_type <= "111";
				end if;

				if (counter <= N + M + 1) then
					counter <= counter + 1;
					trig_state_machine <= '1';
				elsif (counter <= N + M + 5) then
					counter <= counter + 1;
					trig_state_machine <= '0';
				else
					trig_state_machine <= '0';
					counter <= 0;
				end if;

			-- Read the address or operand after opcode was found
			elsif (counter >= M and counter <= N + M - 1) then
				if (counter = M) then
					if (opcode_out = "101000") then
						dump_mem <= '1';
						sel_alu_input_mux <= '0';
						d <= '0';
						transfer_to_sw <= '0';
					elsif (opcode_out = "110011") then
						dump_mem <= '0';
						sel_alu_input_mux <= '1';
						d <= '0';
						transfer_to_sw <= '1'; -- Output ALU is transferred to raspberry pi
					elsif (opcode_out = "110001" or opcode_out = "110010") then
						dump_mem <= '0';
						sel_alu_input_mux <= '0';
						d <= '0'; -- This does not matter when transfer_to_sw = 1
						transfer_to_sw <= '1';
					elsif (opcode_out = "000111" or opcode_out = "000101" or opcode_out = "001001" or
							 opcode_out = "000011" or opcode_out = "001011" or opcode_out = "001010" or
							 opcode_out = "001111" or opcode_out = "000100" or opcode_out = "001000" or
							 opcode_out = "000010" or opcode_out = "000110" or opcode_out = "001101" or
							 opcode_out = "001100" or opcode_out = "000001" or opcode_out = "001110") then
						dump_mem <= '0';
						sel_alu_input_mux <= '1'; -- input to ALU will be from memory
						d <= serial_in; -- d = 0 --> data stored to wreg, d = 1 --> data stored to ram
						transfer_to_sw <= '0';
					else
						dump_mem <= '0';
						sel_alu_input_mux <= '0'; -- input to ALU will be from literal
						d <= '0';
						transfer_to_sw <= '0';
					end if;
				end if;
				operand_out <= operand_out(N-2 downto 0) & serial_in;
				address_out <= operand_out(N-2 downto 0) & serial_in;
				counter <= counter + 1;

			-- Read the opcode after mosi was set
			elsif (counter <= M - 1 and mosi = '1') then
				opcode_out <= opcode_out(M-2 downto 0) & serial_in;
				operand_out <= (others => '0');
				counter <= counter + 1;

			elsif (mosi = '0') then
				opcode_out <= (others => '0');
				operand_out <= (others => '0');

			end if;
		end if;
	end process func;
end architecture rtl;
