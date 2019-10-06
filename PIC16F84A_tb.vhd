library ieee;
library std;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;
use std.textio.all;
use ieee.std_logic_textio.all;

entity PIC16F84A_tb is
end PIC16F84A_tb;

architecture behavior of PIC16F84A_tb is
	signal input_mux : std_logic_vector(7 downto 0) := "00000000";
	signal operation : std_logic_vector(5 downto 0) := "000000";
	--signal status_in : std_logic_vector(7 downto 0) := "00000000";
	--signal status_out : std_logic_vector(7 downto 0) := "00000000";
	
	signal clk : std_logic := '0';
	signal reset : std_logic := '0';
	constant clk_period : time := 1 us;
	signal check : natural := 0;
	
	component PIC16F84A is
		generic (N : natural := 8);
		port (input_mux : in std_logic_vector(N-1 downto 0);
				operation : in std_logic_vector(5 downto 0);
				--status_in : in std_logic_vector(N-1 downto 0);
				--status_out : out std_logic_vector(N-1 downto 0);
				clk : in std_logic;
				reset : in std_logic);
	end component;
	
	begin
		dut : PIC16F84A
			port map(input_mux => input_mux, operation => operation,
			clk => clk, reset => reset);
			
		clk_process : process is
			begin
				clk <= '0';
				wait for clk_period/2;
				clk <= '1';
				wait for clk_period/2;
				if (check = 1) then
					wait;
				end if;
		end process clk_process;
			
		stimulus : process is
			file stimulus_file : text open read_mode is "C:\Users\henry\Documents\PIC-FPGA\Input.txt";
			variable data_in_mux : std_logic_vector(7 downto 0);
			variable data_in_oper : std_logic_vector(5 downto 0);
			variable comma : character;
			variable linein : line;
			
			begin
				reset <= '0';
				wait for 2 us;
				reset <= '1';
				wait for 10.5 us;
				reset <= '0';
				while (not endfile(stimulus_file)) loop
					wait until falling_edge(clk);
					readline(stimulus_file, linein);
					read(linein, data_in_mux);
					read(linein, comma);
					read(linein, data_in_oper);
					input_mux <= data_in_mux;
					operation <= data_in_oper;
				end loop;
				file_close(stimulus_file);
				check <= 1;
				wait;
		end process stimulus;
	end architecture behavior;