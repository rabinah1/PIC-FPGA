library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity PIC16F84A is
	generic (N : natural := 8);
	port (input_mux : in std_logic_vector(N-1 downto 0);
			operation : in std_logic_vector(5 downto 0);
			status_in : in std_logic_vector(N-1 downto 0);
			status_out : out std_logic_vector(N-1 downto 0);
			clk : in std_logic;
			reset : in std_logic);
end PIC16F84A;

architecture struct of PIC16F84A is
	signal ALU_output_int : std_logic_vector(7 downto 0);
	signal W_output_int : std_logic_vector(7 downto 0);
	
	component ALU is
		generic (N : natural := 8);
		port (input_W : in std_logic_vector(N-1 downto 0);
				input_mux : in std_logic_vector(N-1 downto 0);
				operation : in std_logic_vector(5 downto 0);
				status_in : in std_logic_vector(N-1 downto 0);
				status_out : out std_logic_vector(N-1 downto 0);
				ALU_output : out std_logic_vector(N-1 downto 0);
				clk : in std_logic;
				reset : in std_logic);
	end component ALU;
	
	component W_register is
		generic (N : natural := 8);
		port (data_in : in std_logic_vector(N-1 downto 0);
				data_out : out std_logic_vector(N-1 downto 0);
				clk : in std_logic;
				reset : in std_logic);
	end component W_register;
	
	begin
		ALU_unit : component ALU
			generic map (N => 8)
			port map (input_W => W_output_int,
						input_mux => input_mux,
						operation => operation,
						status_in => status_in,
						status_out => status_out,
						ALU_output => ALU_output_int,
						clk => clk,
						reset => reset);
		
		W_register_unit : component W_register
			generic map (N => 8)
			port map (data_in => ALU_output_int,
						 data_out => W_output_int,
						 clk => clk,
						 reset => reset);
end architecture struct;