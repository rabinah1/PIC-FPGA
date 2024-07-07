set_time_format -unit ns -decimal_places 3
create_clock -name {clk} -period 10000.000 -waveform {0.000 5000.000} [get_ports {clk}]
create_clock -name {timer_external_input} -period 10000.000 -waveform {0.000 5000.000} [get_ports {timer_external_input}]
create_clock -name {clk_50mhz_in} -period 20.000 -waveform {0.000 10.000} [get_ports {clk_50mhz_in}]
create_generated_clock -name clk_100khz -divide_by 500 -source [get_ports {clk_50mhz_in}] clk_div:clk_div_unit|clk_100khz|q
create_generated_clock -name clk_200khz -divide_by 250 -source [get_ports {clk_50mhz_in}] clk_div:clk_div_unit|clk_200khz|q
derive_pll_clocks -create_base_clocks
derive_clock_uncertainty