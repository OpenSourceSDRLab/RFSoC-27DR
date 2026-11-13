set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]
set_property -dict {PACKAGE_PIN E10 IOSTANDARD LVDS_25} [get_ports pl_clk_p]
set_property -dict {PACKAGE_PIN E9 IOSTANDARD  LVDS_25} [get_ports pl_clk_n]
set_property -dict {PACKAGE_PIN E11 IOSTANDARD LVDS_25} [get_ports pl_sysref_p]
set_property -dict {PACKAGE_PIN D11 IOSTANDARD LVDS_25} [get_ports pl_sysref_n]

set_property -dict {PACKAGE_PIN J12 IOSTANDARD LVCMOS25} [get_ports lmk_sync]
set_property -dict {PACKAGE_PIN K12 IOSTANDARD LVCMOS25} [get_ports led[0]]
set_property -dict {PACKAGE_PIN H11 IOSTANDARD LVCMOS25} [get_ports led[1]]
set_property -dict {PACKAGE_PIN J11 IOSTANDARD LVCMOS25} [get_ports led[2]]
set_property -dict {PACKAGE_PIN K10 IOSTANDARD LVCMOS25} [get_ports led[3]]

create_clock -period 5.000 -name pl_clk_1_p -waveform {0.000 2.500} [get_ports pl_clk_1_p]
create_clock -period 5.000 -name pl_clk_2_p -waveform {0.000 2.500} [get_ports pl_clk_2_p]



