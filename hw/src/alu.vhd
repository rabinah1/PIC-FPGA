library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;
use work.constants_package.all;

entity alu is
    port (
        clk         : in    std_logic;
        reset       : in    std_logic;
        enable      : in    std_logic;
        input_w_reg : in    std_logic_vector(7 downto 0);
        output_mux  : in    std_logic_vector(7 downto 0);
        opcode      : in    std_logic_vector(5 downto 0);
        status_in   : in    std_logic_vector(7 downto 0);
        bit_idx     : in    std_logic_vector(2 downto 0);
        status_out  : out   std_logic_vector(7 downto 0);
        alu_output  : out   std_logic_vector(7 downto 0)
    );
end entity alu;

architecture rtl of alu is

    signal skip_next : std_logic;

    function update_status_z (
        result : in std_logic_vector(7 downto 0)
    )
    return std_logic is

    begin

        if (result = "00000000") then
            return '1';
        else
            return '0';
        end if;

    end function update_status_z;

begin

    alu : process (all) is

        variable opcode_temp  : std_logic_vector(5 downto 0);
        variable result       : std_logic_vector(8 downto 0);
        variable dc_check     : std_logic_vector(4 downto 0);
        variable temp_data    : std_logic_vector(7 downto 0);
        variable status_carry : std_logic;

    begin

        if (reset = '1') then
            alu_output   <= (others => '0');
            skip_next    <= '0';
            status_out   <= (others => '0');
            opcode_temp  := (others => '0');
            result       := (others => '0');
            dc_check     := (others => '0');
            temp_data    := (others => '0');
            status_carry := '0';
        elsif (rising_edge(clk)) then
            if (enable = '1') then
                if (skip_next = '1') then
                    skip_next   <= '0';
                    opcode_temp := (others => '0');
                else
                    opcode_temp := opcode;
                end if;

                case opcode_temp is

                    when ADDWF =>

                        result        := '0' & input_w_reg + output_mux;
                        dc_check      := '0' & input_w_reg(3 downto 0) + output_mux(3 downto 0);
                        status_out(0) <= result(8);
                        status_out(1) <= dc_check(4);
                        status_out(2) <= update_status_z(result(7 downto 0));
                        alu_output    <= result(7 downto 0);

                    when ANDWF =>

                        result        := '0' & (input_w_reg and output_mux);
                        status_out(2) <= update_status_z(result(7 downto 0));
                        alu_output    <= result(7 downto 0);

                    when BCF =>

                        temp_data                                := output_mux;
                        temp_data(to_integer(unsigned(bit_idx))) := '0';
                        result                                   := '0' & temp_data;
                        status_out(2)                            <= update_status_z(result(7 downto 0));
                        alu_output                               <= result(7 downto 0);

                    when BSF =>

                        temp_data                                := output_mux;
                        temp_data(to_integer(unsigned(bit_idx))) := '1';
                        result                                   := '0' & temp_data;
                        status_out(2)                            <= update_status_z(result(7 downto 0));
                        alu_output                               <= result(7 downto 0);

                    when CLR =>

                        result        := (others => '0');
                        status_out(2) <= update_status_z(result(7 downto 0));
                        alu_output    <= result(7 downto 0);

                    when COMF =>

                        result        := '0' & not output_mux;
                        status_out(2) <= update_status_z(result(7 downto 0));
                        alu_output    <= result(7 downto 0);

                    when DECF =>

                        result        := '0' & output_mux - 1;
                        status_out(2) <= update_status_z(result(7 downto 0));
                        alu_output    <= result(7 downto 0);

                    when DECFSZ =>

                        status_out(2) <= update_status_z(result(7 downto 0));
                        result        := '0' & output_mux - 1;
                        if (result(7 downto 0) = "0000000") then
                            skip_next <= '1';
                        end if;
                        alu_output <= result(7 downto 0);

                    when INCF =>

                        result        := '0' & output_mux + 1;
                        status_out(2) <= update_status_z(result(7 downto 0));
                        alu_output    <= result(7 downto 0);

                    when INCFSZ =>

                        status_out(2) <= update_status_z(result(7 downto 0));
                        result        := '0' & output_mux + 1;
                        if (result(7 downto 0) = "0000000") then
                            skip_next <= '1';
                        end if;
                        alu_output <= result(7 downto 0);

                    when IORWF =>

                        result        := '0' & (input_w_reg or output_mux);
                        status_out(2) <= update_status_z(result(7 downto 0));
                        alu_output    <= result(7 downto 0);

                    when MOVF =>

                        result        := '0' & output_mux;
                        status_out(2) <= update_status_z(result(7 downto 0));
                        alu_output    <= result(7 downto 0);

                    when RLF =>

                        status_carry := status_in(0);
                        status_out   <= status_in(7 downto 1) & output_mux(7);
                        alu_output   <= output_mux(6 downto 0) & status_carry;

                    when RRF =>

                        status_carry := status_in(0);
                        status_out   <= status_in(7 downto 1) & output_mux(0);
                        alu_output   <= status_carry & output_mux(7 downto 1);

                    when SUBWF =>

                        result        := '0' & output_mux + (not input_w_reg + 1);
                        dc_check      := '0' & output_mux(3 downto 0) + (not input_w_reg(3 downto 0) + 1);
                        status_out(0) <= result(8);
                        status_out(1) <= dc_check(4);
                        status_out(2) <= update_status_z(result(7 downto 0));
                        alu_output    <= result(7 downto 0);

                    when SWAPF =>

                        result     := '0' & output_mux(3 downto 0) & output_mux(7 downto 4);
                        alu_output <= result(7 downto 0);

                    when XORWF =>

                        result        := '0' & (input_w_reg xor output_mux);
                        status_out(2) <= update_status_z(result(7 downto 0));
                        alu_output    <= result(7 downto 0);

                    when ADDLW =>

                        result        := '0' & input_w_reg + output_mux;
                        dc_check      := '0' & input_w_reg(3 downto 0) + output_mux(3 downto 0);
                        status_out(0) <= result(8);
                        status_out(1) <= dc_check(4);
                        status_out(2) <= update_status_z(result(7 downto 0));
                        alu_output    <= result(7 downto 0);

                    when ANDLW =>

                        result        := '0' & (input_w_reg and output_mux);
                        status_out(2) <= update_status_z(result(7 downto 0));
                        alu_output    <= result(7 downto 0);

                    when IORLW =>

                        result        := '0' & (input_w_reg or output_mux);
                        status_out(2) <= update_status_z(result(7 downto 0));
                        alu_output    <= result(7 downto 0);

                    when MOVLW =>

                        result     := '0' & output_mux;
                        status_out <= (others => '0');
                        alu_output <= result(7 downto 0);

                    when SUBLW =>

                        result        := '0' & output_mux + (not input_w_reg + 1);
                        dc_check      := '0' & output_mux(3 downto 0) + (not input_w_reg(3 downto 0) + 1);
                        status_out(0) <= result(8);
                        status_out(1) <= dc_check(4);
                        status_out(2) <= update_status_z(result(7 downto 0));
                        alu_output    <= result(7 downto 0);

                    when XORLW =>

                        result        := '0' & (input_w_reg xor output_mux);
                        status_out(2) <= update_status_z(result(7 downto 0));
                        alu_output    <= result(7 downto 0);

                    when READ_WREG =>

                        result     := '0' & input_w_reg;
                        status_out <= status_in;
                        alu_output <= result(7 downto 0);

                    when READ_STATUS =>

                        result     := '0' & status_in;
                        status_out <= status_in;
                        alu_output <= result(7 downto 0);

                    when READ_ADDRESS =>

                        result     := '0' & output_mux;
                        status_out <= status_in;
                        alu_output <= result(7 downto 0);

                    when "000000" =>

                        status_out <= (others => '0');

                    when others =>

                        alu_output <= (others => '0');
                        status_out <= (others => '0');

                end case;

            end if;
        end if;

    end process alu;

end architecture rtl;
