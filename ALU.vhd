library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity ALU is
	generic (N : natural := 8);
	port (input_W : in std_logic_vector(N-1 downto 0);
			input_mux : in std_logic_vector(N-1 downto 0);
			operation : in std_logic_vector(5 downto 0);
			status_in : in std_logic_vector(7 downto 0);
			status_out : out std_logic_vector(7 downto 0);
			ALU_output : out std_logic_vector(N-1 downto 0);
			clk : in std_logic;
			reset : in std_logic);
end ALU;

architecture rtl of ALU is
	signal skipNext : std_logic;
	
	procedure NOP is
	begin
	end NOP;
	
begin

	func: process(all) is
		variable decTemp : std_logic_vector(N-1 downto 0);
		variable incTemp : std_logic_vector(N-1 downto 0);
		variable opTemp : std_logic_vector(5 downto 0);
		variable status_carry : std_logic;
		
	begin
	
		if (reset = '1') then
			ALU_output <= (others => '0');
			decTemp := (others => '0');
			incTemp := (others => '0');
			skipNext <= '0';
			opTemp := (others => '0');
			status_carry := '0';
			status_out <= (others => '0');
			
		elsif (rising_edge(clk)) then
			if (skipNext = '1') then
				skipNext <= '0';
				opTemp := (others => '0');
			else
				opTemp := operation;
			end if;
				
			case opTemp is
		
				when "000111" => -- ADDWF
					ALU_output <= input_W + input_mux;
				
				when "000101" => -- ANDWF
					ALU_output <= input_W and input_mux;
					
				when "000001" => -- CLRF / CLRW
					ALU_output <= (others => '0');
			
				when "001001" => -- COMF
					ALU_output <= not input_mux;
			
				when "000011" => -- DECF
					ALU_output <= input_mux - 1;
					
				when "001011" => -- DECFSZ
					decTemp := input_mux - 1;
					if (decTemp = "00000000") then
						skipNext <= '1';
					end if;
					ALU_output <= decTemp;
			
				when "001010" => -- INCF
					ALU_output <= input_mux + 1;
					
				when "001111" => -- INCFSZ
					incTemp := input_mux + 1;
					if (incTemp = "00000000") then
						skipNext <= '1';
					end if;
					ALU_output <= incTemp;
					
				when "000100" => -- IORWF
					ALU_output <= input_W or input_mux;
					
				when "001000" => -- MOVF
					ALU_output <= input_mux;
					
				-- MOVWF: the first six bits the same as with NOP
				
				when "001101" => -- RLF
					status_carry := status_in(0);
					status_out <= status_in(7 downto 1) & input_mux(7);
					ALU_output <= input_mux(6 downto 0) & status_carry;
				
				when "001100" => -- RRF
					status_carry := status_in(0);
					status_out <= status_in(7 downto 1) & input_mux(0);
					ALU_output <= status_carry & input_mux(7 downto 1);
				
				when "000010" => -- SUBWF
					ALU_output <= input_mux - input_W;
				
				when "111110" => -- ADDLW
					ALU_output <= input_W + input_mux;
				
				when "111001" => -- ANDLW
					ALU_output <= input_W and input_mux;
				
				when "111101" => -- SUBLW
					ALU_output <= input_mux - input_W;
					
				when "000000" => -- NOP
					NOP;
				
				when others =>
					ALU_output <= (others => '0');
			end case;
		end if;
	end process func;
end architecture rtl;