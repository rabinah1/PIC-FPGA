library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity timer is
    port (trigger : in std_logic;
          reset : in std_logic;
          edge_trigger : in std_logic;
          data_out_falling : out std_logic_vector(7 downto 0);
          data_out_rising : out std_logic_vector(7 downto 0));
end timer;

architecture rtl of timer is
begin

    func_rising : process(all) is
    begin

        if (reset = '1') then
            data_out_rising <= (others => '0');
        elsif (rising_edge(trigger) and edge_trigger = '0') then
            data_out_rising <= std_logic_vector(unsigned(data_out_rising) + 1);
        end if;

    end process func_rising;

    func_falling : process(all) is
    begin

        if (reset = '1') then
            data_out_falling <= (others => '0');
        elsif (falling_edge(trigger) and edge_trigger = '1') then
            data_out_falling <= std_logic_vector(unsigned(data_out_falling) + 1);
        end if;

    end process func_falling;
    
end architecture rtl;
