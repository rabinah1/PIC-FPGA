library ieee;
use ieee.std_logic_1164.all;

entity alu_input_mux is
    port (
        clk           : in    std_logic;
        reset         : in    std_logic;
        enable        : in    std_logic;
        sel           : in    std_logic;
        input_ram     : in    std_logic_vector(7 downto 0);
        input_literal : in    std_logic_vector(7 downto 0);
        data_out      : out   std_logic_vector(7 downto 0)
    );
end entity alu_input_mux;

architecture rtl of alu_input_mux is

begin

    alu_input_mux : process (all) is
    begin

        if (reset = '1') then
            data_out <= (others => '0');
        elsif (rising_edge(clk)) then
            if (enable = '1') then
                if (sel = '0') then
                    data_out <= input_literal;
                else
                    data_out <= input_ram;
                end if;
            end if;
        end if;

    end process alu_input_mux;

end architecture rtl;
