library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity W_register is
    generic (N : natural := 8);
    port (data_in : in std_logic_vector(N-1 downto 0);
          data_out : out std_logic_vector(N-1 downto 0);
          clk : in std_logic;
          enable : in std_logic;
          reset : in std_logic);
end W_register;

architecture rtl of W_register is
begin

    operation : process(all) is
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