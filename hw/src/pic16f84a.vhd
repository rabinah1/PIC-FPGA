library ieee;
use ieee.std_logic_1164.all;

entity pic16f84a is
    port (
        serial_in            : in    std_logic;
        clk                  : in    std_logic;
        clk_50mhz_in         : in    std_logic;
        reset                : in    std_logic;
        mosi                 : in    std_logic;
        timer_external_input : in    std_logic;
        miso                 : out   std_logic;
        scl                  : out   std_logic;
        alu_output_raspi     : out   std_logic;
        sda                  : inout std_logic;
        memory_mem_a         : out   std_logic_vector(12 downto 0);
        memory_mem_ba        : out   std_logic_vector(2 downto 0);
        memory_mem_ck        : out   std_logic;
        memory_mem_ck_n      : out   std_logic;
        memory_mem_cke       : out   std_logic;
        memory_mem_cs_n      : out   std_logic;
        memory_mem_ras_n     : out   std_logic;
        memory_mem_cas_n     : out   std_logic;
        memory_mem_we_n      : out   std_logic;
        memory_mem_reset_n   : out   std_logic;
        memory_mem_dq        : inout std_logic_vector(7 downto 0);
        memory_mem_dqs       : inout std_logic;
        memory_mem_dqs_n     : inout std_logic;
        memory_mem_odt       : out   std_logic;
        memory_mem_dm        : out   std_logic;
        memory_oct_rzqin     : in    std_logic;
        reset_reset_n        : in    std_logic
    );
end entity pic16f84a;

architecture struct of pic16f84a is

    signal trig_state_machine_int             : std_logic;
    signal ram_read_enable_int                : std_logic;
    signal status_write_enable_int            : std_logic;
    signal alu_input_mux_enable_int           : std_logic;
    signal alu_enable_int                     : std_logic;
    signal wreg_enable_int                    : std_logic;
    signal result_enable_int                  : std_logic;
    signal sel_alu_input_mux_int              : std_logic;
    signal sel_alu_demux_int                  : std_logic;
    signal trig_instruction_process_int       : std_logic;
    signal ram_write_enable_int               : std_logic;
    signal timer_write_enable_int             : std_logic;
    signal transfer_to_sw_int                 : std_logic;
    signal ram_dump_enable_int                : std_logic;
    signal eeprom_dump_enable_int             : std_logic;
    signal result_enable_ram_dump_int         : std_logic;
    signal result_enable_eeprom_dump_int      : std_logic;
    signal edge_trigger_int                   : std_logic;
    signal clk_100khz_int                     : std_logic;
    signal clk_200khz_int                     : std_logic;
    signal write_to_ram_trig                  : std_logic;
    signal eeprom_write_completed             : std_logic;
    signal output_mux                         : std_logic_vector(7 downto 0);
    signal alu_output_int                     : std_logic_vector(7 downto 0);
    signal w_output_int                       : std_logic_vector(7 downto 0);
    signal literal_input_mux                  : std_logic_vector(7 downto 0);
    signal opcode                             : std_logic_vector(5 downto 0);
    signal timer_external_counter_falling_int : std_logic_vector(7 downto 0);
    signal timer_external_counter_rising_int  : std_logic_vector(7 downto 0);
    signal data_from_eeprom                   : std_logic_vector(7 downto 0);
    signal eedata_int                         : std_logic_vector(7 downto 0);
    signal eeadr_int                          : std_logic_vector(7 downto 0);
    signal eecon_int                          : std_logic_vector(7 downto 0);
    signal binary_string_int                  : std_logic_vector(13 downto 0);
    signal bit_idx_out_int                    : std_logic_vector(2 downto 0);
    signal data_to_sw_int                     : std_logic_vector(7 downto 0);
    signal ram_dump_int                       : std_logic_vector(1015 downto 0);
    signal eeprom_dump_int                    : std_logic_vector(2047 downto 0);
    signal instruction_type_int               : std_logic_vector(2 downto 0);
    signal status_out_alu                     : std_logic_vector(7 downto 0);
    signal status_in_alu                      : std_logic_vector(7 downto 0);
    signal from_ram_to_alu                    : std_logic_vector(7 downto 0);
    signal ram_address_int                    : std_logic_vector(6 downto 0);
    signal from_alu_to_wreg                   : std_logic_vector(7 downto 0);
    signal from_alu_to_ram                    : std_logic_vector(7 downto 0);

    component hps_adder_qsys is
        port (
            clk_clk            : in    std_logic;
            memory_mem_a       : out   std_logic_vector(12 downto 0);
            memory_mem_ba      : out   std_logic_vector(2 downto 0);
            memory_mem_ck      : out   std_logic;
            memory_mem_ck_n    : out   std_logic;
            memory_mem_cke     : out   std_logic;
            memory_mem_cs_n    : out   std_logic;
            memory_mem_ras_n   : out   std_logic;
            memory_mem_cas_n   : out   std_logic;
            memory_mem_we_n    : out   std_logic;
            memory_mem_reset_n : out   std_logic;
            memory_mem_dq      : inout std_logic_vector(7 downto 0);
            memory_mem_dqs     : inout std_logic;
            memory_mem_dqs_n   : inout std_logic;
            memory_mem_odt     : out   std_logic;
            memory_mem_dm      : out   std_logic;
            memory_oct_rzqin   : in    std_logic;
            reset_reset_n      : in    std_logic
        );
    end component hps_adder_qsys;

    component alu_output_demux is
        port (
            clk            : in    std_logic;
            reset          : in    std_logic;
            sel            : in    std_logic;
            transfer_to_sw : in    std_logic;
            input_data     : in    std_logic_vector(7 downto 0);
            data_to_ram    : out   std_logic_vector(7 downto 0);
            data_to_sw     : out   std_logic_vector(7 downto 0);
            data_to_wreg   : out   std_logic_vector(7 downto 0)
        );
    end component alu_output_demux;

    component alu_input_mux is
        port (
            clk           : in    std_logic;
            reset         : in    std_logic;
            enable        : in    std_logic;
            sel           : in    std_logic;
            input_ram     : in    std_logic_vector(7 downto 0);
            input_literal : in    std_logic_vector(7 downto 0);
            data_out      : out   std_logic_vector(7 downto 0)
        );
    end component alu_input_mux;

    component ram is
        port (
            clk                            : in    std_logic;
            reset                          : in    std_logic;
            write_enable                   : in    std_logic;
            read_enable                    : in    std_logic;
            mem_dump_enable                : in    std_logic;
            eeprom_dump_enable             : in    std_logic;
            status_write_enable            : in    std_logic;
            timer_write_enable             : in    std_logic;
            eeprom_read                    : in    std_logic;
            eeprom_write_completed         : in    std_logic;
            timer_external_counter_falling : in    std_logic_vector(7 downto 0);
            timer_external_counter_rising  : in    std_logic_vector(7 downto 0);
            eeprom_data_in                 : in    std_logic_vector(7 downto 0);
            data                           : in    std_logic_vector(7 downto 0);
            status_in                      : in    std_logic_vector(7 downto 0);
            address                        : in    std_logic_vector(6 downto 0);
            edge_trigger                   : out   std_logic;
            mem_dump                       : out   std_logic_vector(1015 downto 0);
            eeprom_dump                    : out   std_logic_vector(2047 downto 0);
            data_out                       : out   std_logic_vector(7 downto 0);
            status_out                     : out   std_logic_vector(7 downto 0);
            eedata_out                     : out   std_logic_vector(7 downto 0);
            eeadr_out                      : out   std_logic_vector(7 downto 0);
            eecon_out                      : out   std_logic_vector(7 downto 0)
        );
    end component ram;

    component state_machine is
        port (
            clk                       : in    std_logic;
            reset                     : in    std_logic;
            trig_state_machine        : in    std_logic;
            instruction_type          : in    std_logic_vector(2 downto 0);
            ram_read_enable           : out   std_logic;
            alu_input_mux_enable      : out   std_logic;
            alu_enable                : out   std_logic;
            wreg_enable               : out   std_logic;
            ram_write_enable          : out   std_logic;
            ram_dump_enable           : out   std_logic;
            eeprom_dump_enable        : out   std_logic;
            status_write_enable       : out   std_logic;
            result_enable             : out   std_logic;
            timer_write_enable        : out   std_logic;
            result_enable_ram_dump    : out   std_logic;
            result_enable_eeprom_dump : out   std_logic
        );
    end component state_machine;

    component serial_to_parallel_instruction is
        port (
            clk                : in    std_logic;
            reset              : in    std_logic;
            enable             : in    std_logic;
            binary_string      : in    std_logic_vector(13 downto 0);
            sel_alu_input_mux  : out   std_logic;
            sel_alu_demux      : out   std_logic;
            trig_state_machine : out   std_logic;
            transfer_to_sw     : out   std_logic;
            instruction_type   : out   std_logic_vector(2 downto 0);
            literal_out        : out   std_logic_vector(7 downto 0);
            address_out        : out   std_logic_vector(6 downto 0);
            bit_idx_out        : out   std_logic_vector(2 downto 0);
            opcode_out         : out   std_logic_vector(5 downto 0)
        );
    end component serial_to_parallel_instruction;

    component input_receive is
        port (
            clk                      : in    std_logic;
            reset                    : in    std_logic;
            serial_in                : in    std_logic;
            mosi                     : in    std_logic;
            trig_instruction_process : out   std_logic;
            binary_string            : out   std_logic_vector(13 downto 0)
        );
    end component input_receive;

    component alu is
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
    end component alu;

    component w_register is
        port (
            clk      : in    std_logic;
            enable   : in    std_logic;
            reset    : in    std_logic;
            data_in  : in    std_logic_vector(7 downto 0);
            data_out : out   std_logic_vector(7 downto 0)
        );
    end component w_register;

    component parallel_to_serial_output is
        port (
            clk                       : in    std_logic;
            reset                     : in    std_logic;
            enable                    : in    std_logic;
            result_enable_ram_dump    : in    std_logic;
            result_enable_eeprom_dump : in    std_logic;
            data_to_sw                : in    std_logic_vector(7 downto 0);
            ram_dump_to_sw            : in    std_logic_vector(1015 downto 0);
            eeprom_dump_to_sw         : in    std_logic_vector(2047 downto 0);
            miso                      : out   std_logic;
            serial_out                : out   std_logic
        );
    end component parallel_to_serial_output;

    component timer is
        port (
            trigger          : in    std_logic;
            reset            : in    std_logic;
            edge_trigger     : in    std_logic;
            data_out_falling : out   std_logic_vector(7 downto 0);
            data_out_rising  : out   std_logic_vector(7 downto 0)
        );
    end component timer;

    component clk_div is
        port (
            clk_in     : in    std_logic;
            reset      : in    std_logic;
            clk_100khz : out   std_logic;
            clk_200khz : out   std_logic
        );
    end component clk_div;

    component i2c is
        port (
            reset               : in    std_logic;
            clk_100khz          : in    std_logic;
            clk_200khz          : in    std_logic;
            control             : in    std_logic_vector(7 downto 0);
            data_in             : in    std_logic_vector(7 downto 0);
            address_in          : in    std_logic_vector(7 downto 0);
            scl                 : out   std_logic;
            enable_write_to_ram : out   std_logic;
            write_completed     : out   std_logic;
            data_out            : out   std_logic_vector(7 downto 0);
            sda                 : inout std_logic
        );
    end component i2c;

begin

    hps_adder_qsys_unit : component hps_adder_qsys
        port map (
            clk_clk            => clk_50mhz_in,
            memory_mem_a       => memory_mem_a,
            memory_mem_ba      => memory_mem_ba,
            memory_mem_ck      => memory_mem_ck,
            memory_mem_ck_n    => memory_mem_ck_n,
            memory_mem_cke     => memory_mem_cke,
            memory_mem_cs_n    => memory_mem_cs_n,
            memory_mem_ras_n   => memory_mem_ras_n,
            memory_mem_cas_n   => memory_mem_cas_n,
            memory_mem_we_n    => memory_mem_we_n,
            memory_mem_reset_n => memory_mem_reset_n,
            memory_mem_dq      => memory_mem_dq,
            memory_mem_dqs     => memory_mem_dqs,
            memory_mem_dqs_n   => memory_mem_dqs_n,
            memory_mem_odt     => memory_mem_odt,
            memory_mem_dm      => memory_mem_dm,
            memory_oct_rzqin   => memory_oct_rzqin,
            reset_reset_n      => reset_reset_n
        );

    alu_input_mux_unit : component alu_input_mux
        port map (
            clk           => clk,
            reset         => reset,
            enable        => alu_input_mux_enable_int,
            sel           => sel_alu_input_mux_int,
            input_ram     => from_ram_to_alu,
            input_literal => literal_input_mux,
            data_out      => output_mux
        );

    alu_output_demux_unit : component alu_output_demux
        port map (
            clk            => clk,
            reset          => reset,
            sel            => sel_alu_demux_int,
            transfer_to_sw => transfer_to_sw_int,
            input_data     => alu_output_int,
            data_to_ram    => from_alu_to_ram,
            data_to_sw     => data_to_sw_int,
            data_to_wreg   => from_alu_to_wreg
        );

    ram_unit : component ram
        port map (
            clk                            => clk,
            reset                          => reset,
            write_enable                   => ram_write_enable_int,
            read_enable                    => ram_read_enable_int,
            mem_dump_enable                => ram_dump_enable_int,
            eeprom_dump_enable             => eeprom_dump_enable_int,
            status_write_enable            => status_write_enable_int,
            timer_write_enable             => timer_write_enable_int,
            eeprom_read                    => write_to_ram_trig,
            eeprom_write_completed         => eeprom_write_completed,
            timer_external_counter_falling => timer_external_counter_falling_int,
            timer_external_counter_rising  => timer_external_counter_rising_int,
            eeprom_data_in                 => data_from_eeprom,
            data                           => from_alu_to_ram,
            status_in                      => status_out_alu,
            address                        => ram_address_int,
            edge_trigger                   => edge_trigger_int,
            mem_dump                       => ram_dump_int,
            eeprom_dump                    => eeprom_dump_int,
            data_out                       => from_ram_to_alu,
            status_out                     => status_in_alu,
            eedata_out                     => eedata_int,
            eeadr_out                      => eeadr_int,
            eecon_out                      => eecon_int
        );

    alu_unit : component alu
        port map (
            clk         => clk,
            reset       => reset,
            enable      => alu_enable_int,
            input_w_reg => w_output_int,
            output_mux  => output_mux,
            opcode      => opcode,
            status_in   => status_in_alu,
            bit_idx     => bit_idx_out_int,
            status_out  => status_out_alu,
            alu_output  => alu_output_int
        );

    state_machine_unit : component state_machine
        port map (
            clk                       => clk,
            reset                     => reset,
            trig_state_machine        => trig_state_machine_int,
            instruction_type          => instruction_type_int,
            ram_read_enable           => ram_read_enable_int,
            alu_input_mux_enable      => alu_input_mux_enable_int,
            alu_enable                => alu_enable_int,
            wreg_enable               => wreg_enable_int,
            ram_write_enable          => ram_write_enable_int,
            ram_dump_enable           => ram_dump_enable_int,
            eeprom_dump_enable        => eeprom_dump_enable_int,
            status_write_enable       => status_write_enable_int,
            result_enable             => result_enable_int,
            timer_write_enable        => timer_write_enable_int,
            result_enable_ram_dump    => result_enable_ram_dump_int,
            result_enable_eeprom_dump => result_enable_eeprom_dump_int
        );

    serial_to_parallel_instruction_unit : component serial_to_parallel_instruction
        port map (
            clk                => clk,
            reset              => reset,
            enable             => trig_instruction_process_int,
            binary_string      => binary_string_int,
            sel_alu_input_mux  => sel_alu_input_mux_int,
            sel_alu_demux      => sel_alu_demux_int,
            trig_state_machine => trig_state_machine_int,
            transfer_to_sw     => transfer_to_sw_int,
            instruction_type   => instruction_type_int,
            literal_out        => literal_input_mux,
            address_out        => ram_address_int,
            bit_idx_out        => bit_idx_out_int,
            opcode_out         => opcode
        );

    input_receive_unit : component input_receive
        port map (
            clk                      => clk,
            reset                    => reset,
            serial_in                => serial_in,
            mosi                     => mosi,
            trig_instruction_process => trig_instruction_process_int,
            binary_string            => binary_string_int
        );

    parallel_to_serial_output_unit : component parallel_to_serial_output
        port map (
            clk                       => clk,
            reset                     => reset,
            enable                    => result_enable_int,
            result_enable_ram_dump    => result_enable_ram_dump_int,
            result_enable_eeprom_dump => result_enable_eeprom_dump_int,
            data_to_sw                => data_to_sw_int,
            ram_dump_to_sw            => ram_dump_int,
            eeprom_dump_to_sw         => eeprom_dump_int,
            miso                      => miso,
            serial_out                => alu_output_raspi
        );

    w_register_unit : component w_register
        port map (
            clk      => clk,
            enable   => wreg_enable_int,
            reset    => reset,
            data_in  => from_alu_to_wreg,
            data_out => w_output_int
        );

    timer_unit : component timer
        port map (
            trigger          => timer_external_input,
            reset            => reset,
            edge_trigger     => edge_trigger_int,
            data_out_falling => timer_external_counter_falling_int,
            data_out_rising  => timer_external_counter_rising_int
        );

    clk_div_unit : component clk_div
        port map (
            clk_in     => clk_50mhz_in,
            reset      => reset,
            clk_100khz => clk_100khz_int,
            clk_200khz => clk_200khz_int
        );

    i2c_unit : component i2c
        port map (
            reset               => reset,
            clk_100khz          => clk_100khz_int,
            clk_200khz          => clk_200khz_int,
            control             => eecon_int,
            data_in             => eedata_int,
            address_in          => eeadr_int,
            scl                 => scl,
            enable_write_to_ram => write_to_ram_trig,
            write_completed     => eeprom_write_completed,
            data_out            => data_from_eeprom,
            sda                 => sda
        );

end architecture struct;
