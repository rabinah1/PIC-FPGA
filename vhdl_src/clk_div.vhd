library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity clk_div is
    port (clk_in : in std_logic;
          reset : in std_logic;
          clk_100khz : out std_logic;
          clk_200khz : out std_logic);
end clk_div;

architecture rtl of clk_div is
    signal counter_100khz : unsigned(7 downto 0);
    signal counter_200khz : unsigned(7 downto 0);
    signal clk_int_100khz : std_logic;
    signal clk_int_200khz : std_logic;
begin

    clock_200 : process(all) is
    begin

        if (reset = '1') then
            clk_200khz <= '0';
            clk_int_200khz <= '0';
            counter_200khz <= to_unsigned(0, counter_200khz'length);
        elsif (rising_edge(clk_in)) then
            if (counter_200khz = 50) then
                clk_int_200khz <= not clk_int_200khz;
                counter_200khz <= counter_200khz + 1;
            elsif (counter_200khz < 124) then
                counter_200khz <= counter_200khz + 1;
            elsif (counter_200khz = 124) then
                clk_200khz <= clk_int_200khz;
                counter_200khz <= to_unsigned(0, counter_200khz'length);
            end if;
        end if;

    end process clock_200;

    clock_100 : process(all) is
    begin

        if (reset = '1') then
            clk_100khz <= '0';
            clk_int_100khz <= '0';
            counter_100khz <= to_unsigned(0, counter_100khz'length);
        elsif (rising_edge(clk_in)) then
            if (counter_100khz = 100) then
                clk_int_100khz <= not clk_int_100khz;
                counter_100khz <= counter_100khz + 1;
            elsif (counter_100khz < 249) then
                counter_100khz <= counter_100khz + 1;
            elsif (counter_100khz = 249) then
                clk_100khz <= clk_int_100khz;
                counter_100khz <= to_unsigned(0, counter_100khz'length);
            end if;
        end if;

    end process clock_100;

end architecture rtl;
