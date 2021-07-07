library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity status_register is
    port (data_in : in std_logic_vector(7 downto 0);
          data_out_to_ALU : out std_logic_vector(7 downto 0);
          clk : in std_logic;
          enable : in std_logic;
          reset : in std_logic);
end status_register;

architecture rtl of status_register is
begin

    operation : process(all) is
    begin
        if (reset = '1') then
            data_out_to_ALU <= (others => '0');
        elsif falling_edge(clk) then
            if (enable = '1') then
                data_out_to_ALU <= data_in;
            end if;
        end if;
    end process operation;
end architecture rtl;
