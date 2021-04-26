library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

package states_package is
	type t_state is (do_nop, do_wait, do_ram_read, do_ram_dump, do_alu_input_sel, do_alu, do_wreg, do_ram_write, do_result);
end states_package;