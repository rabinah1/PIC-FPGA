library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity PIC16F84A is
    port (serial_in : in std_logic;
          clk : in std_logic;
          clk_50mhz_in : in std_logic;
          reset : in std_logic;
          mosi : in std_logic;
          timer_external_input : in std_logic;
          sda : inout std_logic;
          miso : out std_logic;
          scl : out std_logic;
          alu_output_raspi : out std_logic);
end PIC16F84A;

architecture struct of PIC16F84A is
    signal output_mux : std_logic_vector(7 downto 0);
    signal alu_output_int : std_logic_vector(7 downto 0);
    signal w_output_int : std_logic_vector(7 downto 0);
    signal literal_input_mux : std_logic_vector(7 downto 0);
    signal opcode : std_logic_vector(5 downto 0);
    signal trig_state_machine_int : std_logic;
    signal ram_read_enable_int : std_logic;
    signal status_write_enable_int : std_logic;
    signal alu_input_mux_enable_int : std_logic;
    signal alu_enable_int : std_logic;
    signal wreg_enable_int : std_logic;
    signal result_enable_int : std_logic;
    signal sel_alu_input_mux_int : std_logic;
    signal d_int : std_logic;
    signal trig_instruction_process_int : std_logic;
    signal from_ram_to_alu : std_logic_vector(7 downto 0);
    signal ram_address_int : std_logic_vector(6 downto 0);
    signal from_alu_to_wreg : std_logic_vector(7 downto 0);
    signal from_alu_to_ram : std_logic_vector(7 downto 0);
    signal ram_write_enable_int : std_logic;
    signal timer_write_enable_int : std_logic;
    signal instruction_type_int : std_logic_vector(2 downto 0);
    signal status_out_alu : std_logic_vector(7 downto 0);
    signal status_in_alu : std_logic_vector(7 downto 0);
    signal transfer_to_sw_int : std_logic;
    signal data_to_sw_int : std_logic_vector(7 downto 0);
    signal mem_dump_int : std_logic_vector(1015 downto 0);
    signal mem_dump_enable_int : std_logic;
    signal result_enable_mem_dump_int : std_logic;
    signal binary_string_int : std_logic_vector(13 downto 0);
    signal bit_idx_out_int : std_logic_vector(2 downto 0);
    signal edge_trigger_int : std_logic;
    signal clk_100khz_int : std_logic;
    signal clk_200khz_int : std_logic;
    signal write_to_ram_trig : std_logic;
    signal timer_external_counter_falling_int : std_logic_vector(7 downto 0);
    signal timer_external_counter_rising_int : std_logic_vector(7 downto 0);
    signal data_from_eeprom : std_logic_vector(7 downto 0);
    signal eedata_int : std_logic_vector(7 downto 0);
    signal eeadr_int : std_logic_vector(7 downto 0);
    signal eecon_int : std_logic_vector(7 downto 0);

    component alu_output_demux is
        port (clk : in std_logic;
              reset : in std_logic;
              d : in std_logic;
              transfer_to_sw : in std_logic;
              input_data : in std_logic_vector(7 downto 0);
              data_to_ram : out std_logic_vector(7 downto 0);
              data_to_sw : out std_logic_vector(7 downto 0);
              data_to_wreg : out std_logic_vector(7 downto 0));
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
              status_in : in std_logic_vector(7 downto 0);
              address : in std_logic_vector(6 downto 0);
              write_enable : in std_logic;
              read_enable : in std_logic;
              mem_dump_enable : in std_logic;
              status_write_enable : in std_logic;
              timer_write_enable : in std_logic;
              eeprom_read : in std_logic;
              timer_external_counter_falling : in std_logic_vector(7 downto 0);
              timer_external_counter_rising : in std_logic_vector(7 downto 0);
              eeprom_data_in : in std_logic_vector(7 downto 0);
              edge_trigger : out std_logic;
              mem_dump : out std_logic_vector(1015 downto 0);
              data_out : out std_logic_vector(7 downto 0);
              status_out : out std_logic_vector(7 downto 0);
              eedata_out : out std_logic_vector(7 downto 0);
              eeadr_out : out std_logic_vector(7 downto 0);
              eecon_out : out std_logic_vector(7 downto 0));
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
              status_write_enable : out std_logic;
              result_enable : out std_logic;
              timer_write_enable : out std_logic;
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
        port (input_w : in std_logic_vector(7 downto 0);
              output_mux : in std_logic_vector(7 downto 0);
              opcode : in std_logic_vector(5 downto 0);
              status_in : in std_logic_vector(7 downto 0);
              clk : in std_logic;
              reset : in std_logic;
              enable : in std_logic;
              bit_idx : in std_logic_vector(2 downto 0);
              status_out : out std_logic_vector(7 downto 0);
              alu_output : out std_logic_vector(7 downto 0));
    end component ALU;

    component W_register is
        port (data_in : in std_logic_vector(7 downto 0);
              clk : in std_logic;
              enable : in std_logic;
              reset : in std_logic;
              data_out : out std_logic_vector(7 downto 0));
    end component W_register;

    component parallel_to_serial_output is
        port (clk : in std_logic;
              reset : in std_logic;
              enable : in std_logic;
              result_enable_mem_dump : in std_logic;
              data_to_sw : in std_logic_vector(7 downto 0);
              mem_dump_to_sw : in std_logic_vector(1015 downto 0);
              miso : out std_logic;
              serial_out : out std_logic);
    end component parallel_to_serial_output;

    component timer is
        port (trigger : in std_logic;
              reset : in std_logic;
              edge_trigger : in std_logic;
              data_out_falling : out std_logic_vector(7 downto 0);
              data_out_rising : out std_logic_vector(7 downto 0));
    end component timer;

    component clk_div is
        port (clk_in : in std_logic;
              reset : in std_logic;
              clk_100khz : out std_logic;
              clk_200khz : out std_logic);
    end component clk_div;

    component i2c is
        port (reset : in std_logic;
              clk_100khz : in std_logic;
              clk_200khz : in std_logic;
              control : in std_logic_vector(7 downto 0);
              data_in : in std_logic_vector(7 downto 0);
              address_in : in std_logic_vector(7 downto 0);
              sda : inout std_logic;
              scl : out std_logic;
              enable_write_to_ram : out std_logic;
              data_out : out std_logic_vector(7 downto 0));
    end component i2c;
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
                  input_data => alu_output_int,
                  data_to_ram => from_alu_to_ram,
                  data_to_sw => data_to_sw_int,
                  data_to_wreg => from_alu_to_wreg);

    ram_unit : component ram
        port map (clk => clk,
                  reset => reset,
                  data => from_alu_to_ram,
                  status_in => status_out_alu,
                  address => ram_address_int,
                  write_enable => ram_write_enable_int,
                  read_enable => ram_read_enable_int,
                  mem_dump_enable => mem_dump_enable_int,
                  status_write_enable => status_write_enable_int,
                  timer_write_enable => timer_write_enable_int,
                  eeprom_read => write_to_ram_trig,
                  timer_external_counter_falling => timer_external_counter_falling_int,
                  timer_external_counter_rising => timer_external_counter_rising_int,
                  eeprom_data_in => data_from_eeprom,
                  edge_trigger => edge_trigger_int,
                  mem_dump => mem_dump_int,
                  data_out => from_ram_to_alu,
                  status_out => status_in_alu,
                  eedata_out => eedata_int,
                  eeadr_out => eeadr_int,
                  eecon_out => eecon_int);

    ALU_unit : component ALU
        port map (input_w => w_output_int,
                  output_mux => output_mux,
                  opcode => opcode,
                  status_in => status_in_alu,
                  clk => clk,
                  reset => reset,
                  enable => alu_enable_int,
                  bit_idx => bit_idx_out_int,
                  status_out => status_out_alu,
                  alu_output => alu_output_int);

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
                  status_write_enable => status_write_enable_int,
                  result_enable => result_enable_int,
                  timer_write_enable => timer_write_enable_int,
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
        port map (clk => clk,
                  reset => reset,
                  enable => result_enable_int,
                  result_enable_mem_dump => result_enable_mem_dump_int,
                  data_to_sw => data_to_sw_int,
                  mem_dump_to_sw => mem_dump_int,
                  miso => miso,
                  serial_out => alu_output_raspi);

    W_register_unit : component W_register
        port map (data_in => from_alu_to_wreg,
                  clk => clk,
                  enable => wreg_enable_int,
                  reset => reset,
                  data_out => w_output_int);

    timer_unit : component timer
        port map (trigger => timer_external_input,
                  reset => reset,
                  edge_trigger => edge_trigger_int,
                  data_out_falling => timer_external_counter_falling_int,
                  data_out_rising => timer_external_counter_rising_int);

    clk_div_unit : component clk_div
        port map (clk_in => clk_50mhz_in,
                  reset => reset,
                  clk_100khz => clk_100khz_int,
                  clk_200khz => clk_200khz_int);

    i2c_unit : component i2c
        port map (reset => reset,
                  clk_100khz => clk_100khz_int,
                  clk_200khz => clk_200khz_int,
                  control => eecon_int,
                  data_in => eedata_int,
                  address_in => eeadr_int,
                  sda => sda,
                  scl => scl,
                  enable_write_to_ram => write_to_ram_trig,
                  data_out => data_from_eeprom);

end architecture struct;
