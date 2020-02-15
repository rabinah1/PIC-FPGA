set_time_format -unit ns -decimal_places 3
create_clock -name {clk} -period 10000.000 -waveform {0.000 5000.000} [get_ports {clk}]
derive_pll_clocks -create_base_clocks
derive_clock_uncertainty