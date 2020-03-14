library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity PIC16F84A is
	generic (N : natural := 8);
	port (serial_in : in std_logic;
			--status_in : in std_logic_vector(N-1 downto 0);
			--status_out : out std_logic_vector(N-1 downto 0);
			clk : in std_logic;
			reset : in std_logic;
			ALU_output_raspi : out std_logic);
end PIC16F84A;

architecture struct of PIC16F84A is
	signal ALU_output_int : std_logic_vector(7 downto 0);
	signal W_output_int : std_logic_vector(7 downto 0);
	signal input_mux : std_logic_vector(7 downto 0);
	signal opcode : std_logic_vector(5 downto 0);
	signal raspi_input_int : std_logic_vector(7 downto 0);
	signal trig_state_machine_int : std_logic;
	signal alu_enable_int : std_logic;
	signal wreg_enable_int : std_logic;
	signal result_enable_int : std_logic;

	component state_machine is
		port (clk : in std_logic;
				reset : in std_logic;
				trig_state_machine : in std_logic;
				alu_enable : out std_logic;
				wreg_enable : out std_logic;
				result_enable : out std_logic);
	end component state_machine;

	component serial_to_parallel_instruction is
		generic (N : natural := 8;
					M : natural := 6);
		port (clk : in std_logic;
				reset : in std_logic;
				serial_in : in std_logic;
				trig_state_machine : out std_logic;
				operand_out : out std_logic_vector(N-1 downto 0);
				opcode_out : out std_logic_vector(M-1 downto 0));
	end component serial_to_parallel_instruction;

	component ALU is
		generic (N : natural := 8);
		port (input_W : in std_logic_vector(N-1 downto 0);
				input_mux : in std_logic_vector(N-1 downto 0);
				opcode : in std_logic_vector(5 downto 0);
				--status_in : in std_logic_vector(N-1 downto 0);
				--status_out : out std_logic_vector(N-1 downto 0);
				ALU_output : out std_logic_vector(N-1 downto 0);
				clk : in std_logic;
				enable : in std_logic;
				reset : in std_logic);
	end component ALU;

	component W_register is
		generic (N : natural := 8);
		port (data_in : in std_logic_vector(N-1 downto 0);
				data_out : out std_logic_vector(N-1 downto 0);
				ALU_output_raspi : out std_logic_vector(N-1 downto 0);
				clk : in std_logic;
				enable : in std_logic;
				reset : in std_logic);
	end component W_register;
	
	component parallel_to_serial_wreg is
		generic (N : natural := 8);
		port (clk : in std_logic;
				reset : in std_logic;
				enable : in std_logic;
				parallel_in : in std_logic_vector(N-1 downto 0);
				serial_out : out std_logic);
	end component parallel_to_serial_wreg;
	
	begin
		ALU_unit : component ALU
			generic map (N => 8)
			port map (input_W => W_output_int,
						 input_mux => input_mux,
						 opcode => opcode,
						 --status_in => status_in,
						 --status_out => status_out,
						 ALU_output => ALU_output_int,
						 clk => clk,
						 enable => alu_enable_int,
						 reset => reset);

		state_machine_unit : component state_machine
			port map (clk => clk,
						 reset => reset,
						 trig_state_machine => trig_state_machine_int,
						 alu_enable => alu_enable_int,
						 wreg_enable => wreg_enable_int,
						 result_enable => result_enable_int);

		serial_to_parallel_instruction_unit : component serial_to_parallel_instruction
			generic map (N => 8,
							 M => 6)
			port map (clk => clk,
						 reset => reset,
						 serial_in => serial_in,
						 trig_state_machine => trig_state_machine_int,
						 operand_out => input_mux,
						 opcode_out => opcode);

		parallel_to_serial_wreg_unit : component parallel_to_serial_wreg
			generic map (N => 8)
			port map (clk => clk,
						 reset => reset,
						 enable => result_enable_int,
						 parallel_in => raspi_input_int,
						 serial_out => ALU_output_raspi);

		W_register_unit : component W_register
			generic map (N => 8)
			port map (data_in => ALU_output_int,
						 data_out => W_output_int,
						 ALU_output_raspi => raspi_input_int,
						 clk => clk,
						 enable => wreg_enable_int,
						 reset => reset);
end architecture struct;