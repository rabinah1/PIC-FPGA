library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity ALU is
    generic (N : natural := 8);
    port (input_w : in std_logic_vector(N-1 downto 0);
          output_mux : in std_logic_vector(N-1 downto 0);
          opcode : in std_logic_vector(5 downto 0);
          status_in : in std_logic_vector(N-1 downto 0);
          clk : in std_logic;
          reset : in std_logic;
          enable : in std_logic;
          bit_idx : in std_logic_vector(2 downto 0);
          status_out : out std_logic_vector(N-1 downto 0);
          ALU_output : out std_logic_vector(N-1 downto 0));
end ALU;

architecture rtl of ALU is
    signal skipNext : std_logic;

    procedure NOP is
    begin
    end NOP;

    function update_status_z (
        result : in std_logic_vector(N-1 downto 0))
        return std_logic is
        begin
            if (result = "00000000") then
                return '1';
            else
                return '0';
            end if;
        end update_status_z;

begin

    func: process(all) is
        variable decTemp : std_logic_vector(N downto 0);
        variable incTemp : std_logic_vector(N downto 0);
        variable opTemp : std_logic_vector(5 downto 0);
        variable result : std_logic_vector(N downto 0);
        variable temp_mem : std_logic_vector(7 downto 0);
        variable status_carry : std_logic;

    begin

        if (reset = '1') then
            ALU_output <= (others => '0');
            skipNext <= '0';
            status_out <= (others => '0');
            decTemp := (others => '0');
            incTemp := (others => '0');
            opTemp := (others => '0');
            result := (others => '0');
            temp_mem := (others => '0');
            status_carry := '0';

        elsif (rising_edge(clk)) then
            if (skipNext = '1') then
                skipNext <= '0';
                opTemp := (others => '0');
            else
                opTemp := opcode;
            end if;

            if (enable = '1') then
                case opTemp is

                    when "000111" => -- ADDWF
                        result := '0' & input_w + output_mux;
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        status_out(0) <= result(N);
                        ALU_output <= result(N-1 downto 0);

                    when "000101" => -- ANDWF
                        result := '0' & input_w and output_mux;
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        ALU_output <= result(N-1 downto 0);

                    when "010000" => -- BCF
                        temp_mem := output_mux;
                        temp_mem(to_integer(unsigned(bit_idx))) := '0';
                        result := '0' & temp_mem;
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        ALU_output <= result(N-1 downto 0);

                    when "010100" => -- BSF
                        temp_mem := output_mux;
                        temp_mem(to_integer(unsigned(bit_idx))) := '1';
                        result := '0' & temp_mem;
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        ALU_output <= result(N-1 downto 0);

                    when "000001" => -- CLR
                        result := (others => '0');
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        ALU_output <= result(N-1 downto 0);

                    when "001001" => -- COMF
                        result := '0' & not output_mux;
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        ALU_output <= result(N-1 downto 0);

                    when "000011" => -- DECF
                        result := '0' & output_mux - 1;
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        ALU_output <= result(N-1 downto 0);

                    when "001011" => -- DECFSZ
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        decTemp := '0' & output_mux - 1;
                        if (decTemp = "00000000") then
                            skipNext <= '1';
                        end if;
                        ALU_output <= result(N-1 downto 0);

                    when "001010" => -- INCF
                        result := '0' & output_mux + 1;
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        ALU_output <= result(N-1 downto 0);

                    when "001111" => -- INCFSZ
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        incTemp := '0' & output_mux + 1;
                        if (incTemp = "00000000") then
                            skipNext <= '1';
                        end if;
                        ALU_output <= result(N-1 downto 0);

                    when "000100" => -- IORWF
                        result := '0' & input_w or output_mux;
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        ALU_output <= result(N-1 downto 0);

                    when "001000" => -- MOVF
                        result := '0' & output_mux;
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        ALU_output <= result(N-1 downto 0);

                    -- MOVWF: the first six bits the same as with NOP

                    when "001101" => -- RLF
                        status_carry := status_in(7);
                        status_out <= status_in(6 downto 0) & output_mux(7);
                        ALU_output <= output_mux(6 downto 0) & status_carry;

                    when "001100" => -- RRF
                        status_carry := status_in(0);
                        status_out <= output_mux(0) & status_in(7 downto 1);
                        ALU_output <= status_carry & output_mux(7 downto 1);

                    when "000010" => -- SUBWF
                        result := '1' & output_mux - input_w;
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        status_out(0) <= not result(N);
                        ALU_output <= result(N-1 downto 0);

                    when "001110" => -- SWAPF
                        result := '0' & output_mux(3 downto 0) & output_mux(7 downto 4);
                        ALU_output <= result(N-1 downto 0);

                    when "000110" => -- XORWF
                        result := '0' & input_w xor output_mux;
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        ALU_output <= result(N-1 downto 0);

                    when "111110" => -- ADDLW
                        result := '0' & input_w + output_mux;
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        status_out(0) <= result(N);
                        ALU_output <= result(N-1 downto 0);

                    when "111001" => -- ANDLW
                        result := '0' & input_w and output_mux;
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        ALU_output <= result(N-1 downto 0);

                    when "111000" => -- IORLW
                        result := '0' & input_w or output_mux;
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        ALU_output <= result(N-1 downto 0);

                    when "110000" => -- MOVLW
                        result := '0' & output_mux;
                        status_out <= (others => '0');
                        ALU_output <= result(N-1 downto 0);

                    when "111101" => -- SUBLW
                        result := '1' & input_w - output_mux;
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        status_out(0) <= not result(N);
                        ALU_output <= result(N-1 downto 0);

                    when "111010" => -- XORLW
                        result := '0' & input_w xor output_mux;
                        status_out(2) <= update_status_z(result(N-1 downto 0));
                        ALU_output <= result(N-1 downto 0);

                    when "110001" => -- READ_WREG
                        result := '0' & input_w;
                        status_out <= status_in;
                        ALU_output <= result(N-1 downto 0);

                    when "110010" => -- READ_STATUS
                        result := '0' & status_in;
                        status_out <= status_in;
                        ALU_output <= result(N-1 downto 0);

                    when "110011" => -- READ_ADDRESS
                        result := '0' & output_mux;
                        status_out <= status_in;
                        ALU_output <= result(N-1 downto 0);

                    when "000000" => -- NOP
                        NOP;
                        status_out <= (others => '0');

                    when others =>
                        ALU_output <= (others => '0');
                        status_out <= (others => '0');
                end case;
            end if;
        end if;
    end process func;
end architecture rtl;
