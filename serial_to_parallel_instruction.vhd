library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;
use work.states_package.all;

entity serial_to_parallel_instruction is
    port (clk : in std_logic;
          reset : in std_logic;
          enable : in std_logic;
          binary_string : in std_logic_vector(13 downto 0);
          instruction_type : out std_logic_vector(2 downto 0);
          sel_alu_input_mux : out std_logic;
          d : out std_logic;
          trig_state_machine : out std_logic;
          transfer_to_sw : out std_logic;
          literal_out : out std_logic_vector(7 downto 0);
          address_out : out std_logic_vector(6 downto 0);
          bit_idx_out : out std_logic_vector(2 downto 0);
          opcode_out : out std_logic_vector(5 downto 0));
end serial_to_parallel_instruction;

architecture rtl of serial_to_parallel_instruction is
begin

    -- Incoming data type: <6 bit opcode> + <7 bit address or operand>
    func: process(all) is
        variable opcode_var : std_logic_vector(5 downto 0);
    begin

        if (reset = '1') then
            literal_out <= (others => '0');
            address_out <= (others => '0');
            opcode_out <= (others => '0');
            bit_idx_out <= (others => '0');
            sel_alu_input_mux <= '0';
            d <= '0';
            trig_state_machine <= '0';
            transfer_to_sw <= '0';
            instruction_type <= (others => '0');
            opcode_var := (others => '0');
        elsif (rising_edge(clk)) then
            if (enable = '1') then
                if (binary_string(13 downto 12) = "01") then -- bit-oriented instruction
                    opcode_out <= binary_string(13 downto 10) & "00";
                    bit_idx_out <= binary_string(9 downto 7);
                    address_out <= binary_string(6 downto 0);
                    literal_out <= (others => '0');
                    sel_alu_input_mux <= '1';
                    d <= '1';
                    transfer_to_sw <= '0';
                    instruction_type <= "010";
                    trig_state_machine <= '1';
                else
                    opcode_var := binary_string(13 downto 8);
                    if (opcode_var = "101000") then -- dump_mem
                        opcode_out <= binary_string(13 downto 8);
                        bit_idx_out <= (others => '0');
                        address_out <= (others => '0');
                        literal_out <= (others => '0');
                        sel_alu_input_mux <= '0';
                        d <= '0';
                        transfer_to_sw <= '0';
                        instruction_type <= "101";
                        trig_state_machine <= '1';
                    elsif (opcode_var = "110011") then
                        opcode_out <= binary_string(13 downto 8);
                        bit_idx_out <= (others => '0');
                        address_out <= binary_string(6 downto 0);
                        literal_out <= (others => '0');
                        sel_alu_input_mux <= '1';
                        d <= '0';
                        transfer_to_sw <= '1';
                        instruction_type <= "100";
                        trig_state_machine <= '1';
                    elsif (opcode_var = "110001" or opcode_var = "110010") then
                        opcode_out <= binary_string(13 downto 8);
                        bit_idx_out <= (others => '0');
                        address_out <= (others => '0');
                        literal_out <= (others => '0');
                        sel_alu_input_mux <= '0';
                        d <= '0';
                        transfer_to_sw <= '1';
                        instruction_type <= "011";
                        trig_state_machine <= '1';
                    elsif (opcode_var = "000111" or opcode_var = "000101" or opcode_var = "001001" or
                            opcode_var = "000011" or opcode_var = "001011" or opcode_var = "001010" or
                            opcode_var = "001111" or opcode_var = "000100" or opcode_var = "001000" or
                            opcode_var = "000010" or opcode_var = "000110" or opcode_var = "001101" or
                            opcode_var = "001100" or opcode_var = "000001" or opcode_var = "001110") then
                        opcode_out <= binary_string(13 downto 8);
                        bit_idx_out <= (others => '0');
                        address_out <= binary_string(6 downto 0);
                        literal_out <= (others => '0');
                        sel_alu_input_mux <= '1'; -- input to ALU will be from memory
                        d <= binary_string(7); -- d = 0 --> data stored to wreg, d = 1 --> data stored to ram
                        transfer_to_sw <= '0';
                        if (binary_string(7) = '0') then
                            instruction_type <= "001";
                        else
                            instruction_type <= "010";
                        end if;
                        trig_state_machine <= '1';
                    else
                        opcode_out <= binary_string(13 downto 8);
                        bit_idx_out <= (others => '0');
                        address_out <= (others => '0');
                        literal_out <= binary_string(7 downto 0);
                        sel_alu_input_mux <= '0'; -- input to ALU will be from literal
                        d <= '0';
                        transfer_to_sw <= '0';
                        instruction_type <= "000";
                        trig_state_machine <= '1';
                    end if;
                end if;
            else
                trig_state_machine <= '0';
            end if;
        end if;
    end process func;
end architecture rtl;
