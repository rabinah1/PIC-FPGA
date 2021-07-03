library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity PIC16F84A is
	port (serial_in : in std_logic;
			clk : in std_logic;
			reset : in std_logic;
			miso : out std_logic;
			mosi : in std_logic;
			ALU_output_raspi : out std_logic);
end PIC16F84A;

architecture struct of PIC16F84A is
	signal output_mux : std_logic_vector(7 downto 0);
	signal ALU_output_int : std_logic_vector(7 downto 0);
	signal w_output_int : std_logic_vector(7 downto 0);
	signal literal_input_mux : std_logic_vector(7 downto 0);
	signal opcode : std_logic_vector(5 downto 0);
	signal trig_state_machine_int : std_logic;
	signal ram_read_enable_int : std_logic;
	signal alu_input_mux_enable_int : std_logic;
	signal alu_enable_int : std_logic;
	signal wreg_enable_int : std_logic;
	signal result_enable_int : std_logic;
	signal status_enable_int : std_logic;
	signal sel_alu_input_mux_int : std_logic;
	signal d_int : std_logic;
	signal trig_instruction_process_int : std_logic;
	signal from_ram_to_alu : std_logic_vector(7 downto 0);
	signal ram_address_int : std_logic_vector(6 downto 0);
	signal from_alu_to_wreg : std_logic_vector(7 downto 0);
	signal from_alu_to_ram : std_logic_vector(7 downto 0);
	signal ram_write_enable_int : std_logic;
	signal instruction_type_int : std_logic_vector(2 downto 0);
	signal status_out_int : std_logic_vector(7 downto 0);
	signal status_in_ALU : std_logic_vector(7 downto 0);
	signal transfer_to_sw_int : std_logic;
	signal data_to_sw_int : std_logic_vector(7 downto 0);
	signal mem_dump_int : std_logic_vector(1015 downto 0);
	signal mem_dump_enable_int : std_logic;
	signal result_enable_mem_dump_int : std_logic;
	signal binary_string_int : std_logic_vector(13 downto 0);
	signal bit_idx_out_int : std_logic_vector(2 downto 0);

	component alu_output_demux is
		port (clk : in std_logic;
				reset : in std_logic;
				d : in std_logic;
				transfer_to_sw : in std_logic;
				input_data : in std_logic_vector(7 downto 0);
				data_to_ram : out std_logic_vector(7 downto 0);
				data_to_wreg : out std_logic_vector(7 downto 0);
				data_to_sw : out std_logic_vector(7 downto 0));
	end component alu_output_demux;
	
	component alu_input_mux is
		port (clk : in std_logic;
				reset : in std_logic;
				enable : in std_logic;
				input_ram : in std_logic_vector(7 downto 0);
				input_literal : in std_logic_vector(7 downto 0);
				sel : in std_logic;
				data_out : out std_logic_vector(7 downto 0));
	end component alu_input_mux;

	component ram is
		port (clk : in std_logic;
				reset : in std_logic;
				data : in std_logic_vector(7 downto 0);
				address : in std_logic_vector(6 downto 0);
				write_enable : in std_logic;
				read_enable : in std_logic;
				mem_dump_enable : in std_logic;
				mem_dump : out std_logic_vector(1015 downto 0);
				data_out : out std_logic_vector(7 downto 0));
	end component ram;

	component state_machine is
		port (clk : in std_logic;
				reset : in std_logic;
				trig_state_machine : in std_logic;
				instruction_type : in std_logic_vector(2 downto 0);
				ram_read_enable : out std_logic;
				alu_input_mux_enable : out std_logic;
				alu_enable : out std_logic;
				wreg_enable : out std_logic;
				ram_write_enable : out std_logic;
				mem_dump_enable : out std_logic;
				status_enable : out std_logic;
				result_enable : out std_logic;
				result_enable_mem_dump : out std_logic);
	end component state_machine;

	component serial_to_parallel_instruction is
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
	end component serial_to_parallel_instruction;
	
	component input_receive is
		port (clk : in std_logic;
				reset : in std_logic;
				serial_in : std_logic;
				mosi : in std_logic;
				trig_instruction_process : out std_logic;
				binary_string : out std_logic_vector(13 downto 0));
	end component input_receive;

	component ALU is
		generic (N : natural := 8);
		port (input_w : in std_logic_vector(N-1 downto 0);
				output_mux : in std_logic_vector(N-1 downto 0);
				opcode : in std_logic_vector(5 downto 0);
				status_in : in std_logic_vector(N-1 downto 0);
				status_out : out std_logic_vector(N-1 downto 0);
				ALU_output : out std_logic_vector(N-1 downto 0);
				clk : in std_logic;
				enable : in std_logic;
				bit_idx : in std_logic_vector(2 downto 0);
				reset : in std_logic);
	end component ALU;

	component W_register is
		generic (N : natural := 8);
		port (data_in : in std_logic_vector(N-1 downto 0);
				data_out : out std_logic_vector(N-1 downto 0);
				clk : in std_logic;
				enable : in std_logic;
				reset : in std_logic);
	end component W_register;
	
	component status_register is
		port (data_in : in std_logic_vector(7 downto 0);
				data_out_to_ALU : out std_logic_vector(7 downto 0);
				clk : in std_logic;
				enable : in std_logic;
				reset : in std_logic);
	end component status_register;
	
	component parallel_to_serial_output is
		generic (N : natural := 8);
		port (clk : in std_logic;
				reset : in std_logic;
				enable : in std_logic;
				result_enable_mem_dump : in std_logic;
				data_to_sw : in std_logic_vector(N-1 downto 0);
				mem_dump_to_sw : in std_logic_vector(1015 downto 0);
				miso : out std_logic;
				serial_out : out std_logic);
	end component parallel_to_serial_output;
	
	begin

		alu_input_mux_unit : component alu_input_mux
			port map (clk => clk,
						 reset => reset,
						 enable => alu_input_mux_enable_int,
						 input_ram => from_ram_to_alu,
						 input_literal => literal_input_mux,
						 sel => sel_alu_input_mux_int,
						 data_out => output_mux);

		alu_output_demux_unit : component alu_output_demux
			port map (clk => clk,
						 reset => reset,
						 d => d_int,
						 transfer_to_sw => transfer_to_sw_int,
						 input_data => ALU_output_int,
						 data_to_ram => from_alu_to_ram,
						 data_to_sw => data_to_sw_int,
						 data_to_wreg => from_alu_to_wreg);

		ram_unit : component ram
			port map (clk => clk,
						 reset => reset,
						 data => from_alu_to_ram,
						 address => ram_address_int,
						 write_enable => ram_write_enable_int,
						 read_enable => ram_read_enable_int,
						 mem_dump_enable => mem_dump_enable_int,
						 mem_dump => mem_dump_int,
						 data_out => from_ram_to_alu);

		ALU_unit : component ALU
			generic map (N => 8)
			port map (input_w => w_output_int,
						 output_mux => output_mux,
						 opcode => opcode,
						 status_in => status_in_ALU,
						 status_out => status_out_int,
						 ALU_output => ALU_output_int,
						 clk => clk,
						 enable => alu_enable_int,
						 bit_idx => bit_idx_out_int,
						 reset => reset);

		state_machine_unit : component state_machine
			port map (clk => clk,
						 reset => reset,
						 trig_state_machine => trig_state_machine_int,
						 instruction_type => instruction_type_int,
						 ram_read_enable => ram_read_enable_int,
						 alu_input_mux_enable => alu_input_mux_enable_int,
						 alu_enable => alu_enable_int,
						 wreg_enable => wreg_enable_int,
						 ram_write_enable => ram_write_enable_int,
						 mem_dump_enable => mem_dump_enable_int,
						 status_enable => status_enable_int,
						 result_enable => result_enable_int,
						 result_enable_mem_dump => result_enable_mem_dump_int);

		serial_to_parallel_instruction_unit : component serial_to_parallel_instruction
			port map (clk => clk,
						 reset => reset,
						 enable => trig_instruction_process_int,
						 binary_string => binary_string_int,
						 instruction_type => instruction_type_int,
						 sel_alu_input_mux => sel_alu_input_mux_int,
						 d => d_int,
						 trig_state_machine => trig_state_machine_int,
						 transfer_to_sw => transfer_to_sw_int,
						 literal_out => literal_input_mux,
						 address_out => ram_address_int, 
						 bit_idx_out => bit_idx_out_int,
						 opcode_out => opcode);
						 
		input_receive_unit : component input_receive
			port map (clk => clk,
						 reset => reset,
						 serial_in => serial_in,
						 mosi => mosi,
						 trig_instruction_process => trig_instruction_process_int,
						 binary_string => binary_string_int);

		parallel_to_serial_output_unit : component parallel_to_serial_output
			generic map (N => 8)
			port map (clk => clk,
						 reset => reset,
						 enable => result_enable_int,
						 result_enable_mem_dump => result_enable_mem_dump_int,
						 data_to_sw => data_to_sw_int,
						 mem_dump_to_sw => mem_dump_int,
						 miso => miso,
						 serial_out => ALU_output_raspi);

		W_register_unit : component W_register
			generic map (N => 8)
			port map (data_in => from_alu_to_wreg,
						 data_out => w_output_int,
						 clk => clk,
						 enable => wreg_enable_int,
						 reset => reset);

		status_register_unit : component status_register
			port map (data_in => status_out_int,
						 data_out_to_ALU => status_in_ALU,
						 clk => clk,
						 enable => status_enable_int,
						 reset => reset);
end architecture struct;