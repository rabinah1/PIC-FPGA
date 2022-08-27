library ieee;
use ieee.std_logic_1164.all;

entity w_register is
    port (
        data_in  : in    std_logic_vector(7 downto 0);
        clk      : in    std_logic;
        enable   : in    std_logic;
        reset    : in    std_logic;
        data_out : out   std_logic_vector(7 downto 0)
    );
end entity w_register;

architecture rtl of w_register is

begin

    operation : process (all) is
    begin

        if (reset = '1') then
            data_out <= (others => '0');
        elsif rising_edge(clk) then
            if (enable = '1') then
                data_out <= data_in;
            end if;
        end if;

    end process operation;

end architecture rtl;
