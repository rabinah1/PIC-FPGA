library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity PIC16F84A is
	generic (N : natural := 8);
	port (serial_in_literal : in std_logic;
			serial_in_opcode : in std_logic;
			--status_in : in std_logic_vector(N-1 downto 0);
			--status_out : out std_logic_vector(N-1 downto 0);
			clk : in std_logic;
			reset : in std_logic;
			enable_opcode : in std_logic;
			enable_literal : in std_logic;
			enable_ALU : in std_logic;
			enable_w_register : in std_logic;
			enable_wreg_out : in std_logic;
			--ALU_output_raspi : out std_logic_vector(N-1 downto 0);
			ALU_output_raspi : out std_logic);
end PIC16F84A;

architecture struct of PIC16F84A is
	signal ALU_output_int : std_logic_vector(7 downto 0);
	signal W_output_int : std_logic_vector(7 downto 0);
	signal input_mux : std_logic_vector(7 downto 0);
	signal opcode : std_logic_vector(5 downto 0);
	signal raspi_input_int : std_logic_vector(7 downto 0);
	
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
	
	component serial_to_parallel_literal is
		generic (N : natural := 8);
		port (clk : in std_logic;
				reset : in std_logic;
				enable : in std_logic;
				serial_in : in std_logic;
				parallel_out : out std_logic_vector(N-1 downto 0));
	end component serial_to_parallel_literal;
	
	component serial_to_parallel_opcode is
		generic (N : natural := 6);
		port (clk : in std_logic;
				reset : in std_logic;
				enable : in std_logic;
				serial_in : in std_logic;
				parallel_out : out std_logic_vector(N-1 downto 0));
	end component serial_to_parallel_opcode;
	
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
						enable => enable_ALU,
						reset => reset);
						
		serial_to_parallel_literal_unit : component serial_to_parallel_literal
			generic map (N => 8)
			port map (clk => clk,
						 reset => reset,
						 enable => enable_literal,
						 serial_in => serial_in_literal,
						 parallel_out => input_mux);
						 
		parallel_to_serial_wreg_unit : component parallel_to_serial_wreg
			generic map (N => 8)
			port map (clk => clk,
						 reset => reset,
						 enable => enable_wreg_out,
						 parallel_in => raspi_input_int,
						 serial_out => ALU_output_raspi);

		serial_to_parallel_opcode_unit : component serial_to_parallel_opcode
			generic map(N => 6)
			port map (clk => clk,
						 reset => reset,
						 enable => enable_opcode,
						 serial_in => serial_in_opcode,
						 parallel_out => opcode);
		
		W_register_unit : component W_register
			generic map (N => 8)
			port map (data_in => ALU_output_int,
						 data_out => W_output_int,
						 --ALU_output_raspi => ALU_output_raspi,
						 ALU_output_raspi => raspi_input_int,
						 clk => clk,
						 enable => enable_w_register,
						 reset => reset);
end architecture struct;