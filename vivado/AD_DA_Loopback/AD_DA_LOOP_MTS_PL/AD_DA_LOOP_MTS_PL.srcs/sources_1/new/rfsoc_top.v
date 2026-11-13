`timescale 1ns / 1ps

module rfsoc_top(
//global clock
input           pl_clk_n,
input           pl_clk_p,
input           pl_sysref_n,
input           pl_sysref_p,

//rf clk mts
input           adc0_clk_n,
input           adc0_clk_p,
input           adc1_clk_n,
input           adc1_clk_p,
input           adc2_clk_n,
input           adc2_clk_p,
input           adc3_clk_n,
input           adc3_clk_p,
input           dac0_clk_n,
input           dac0_clk_p,
input           dac1_clk_n,
input           dac1_clk_p,
input           sysref_in_n,
input           sysref_in_p,

//rf signal
input           vin0_01_n,
input           vin0_01_p,
input           vin0_23_n,
input           vin0_23_p,
input           vin1_01_n,
input           vin1_01_p,
input           vin1_23_n,
input           vin1_23_p,
input           vin2_01_n,
input           vin2_01_p,
input           vin2_23_n,
input           vin2_23_p,
input           vin3_01_n,
input           vin3_01_p,
input           vin3_23_n,
input           vin3_23_p,
output          vout00_n,
output          vout00_p,
output          vout01_n,
output          vout01_p,
output          vout02_n,
output          vout02_p,
output          vout03_n,
output          vout03_p,
output          vout10_n,
output          vout10_p,
output          vout11_n,
output          vout11_p,
output          vout12_n,
output          vout12_p,
output          vout13_n,
output          vout13_p,

//sync
output           lmk_sync,
output [3:0]     led
);


// Local Parameters
localparam C_NUM_CHNL     = 8;


//wires and registers
wire                      fclk_100m;
wire                      data_clk; 
wire                      fclk_reset0;  
wire                      pl_clk;

wire [4:0]                emio_gpio_o;  

wire                      dds_resetn;  
wire [31:0]               phase;
wire                      config_done;

reg                       user_sysref_adc;
reg                       user_sysref_dac;
wire                      pl_sysref;

reg   [63:0]              phase_reg;
wire  [127:0]             dds_data;

wire [63:0] m00_axis_tdata, m01_axis_tdata, m02_axis_tdata, m03_axis_tdata;
wire [63:0] m10_axis_tdata, m11_axis_tdata, m12_axis_tdata, m13_axis_tdata;
wire [63:0] m20_axis_tdata, m21_axis_tdata, m22_axis_tdata, m23_axis_tdata;
wire [63:0] m30_axis_tdata, m31_axis_tdata, m32_axis_tdata, m33_axis_tdata;
wire m00_axis_tready=1'b1, m01_axis_tready=1'b1, m02_axis_tready=1'b1, m03_axis_tready=1'b1;
wire m10_axis_tready=1'b1, m11_axis_tready=1'b1, m12_axis_tready=1'b1, m13_axis_tready=1'b1;
wire m20_axis_tready=1'b1, m21_axis_tready=1'b1, m22_axis_tready=1'b1, m23_axis_tready=1'b1;
wire m30_axis_tready=1'b1, m31_axis_tready=1'b1, m32_axis_tready=1'b1, m33_axis_tready=1'b1;
wire m00_axis_tvalid, m01_axis_tvalid, m02_axis_tvalid, m03_axis_tvalid;
wire m10_axis_tvalid, m11_axis_tvalid, m12_axis_tvalid, m13_axis_tvalid;
wire m20_axis_tvalid, m21_axis_tvalid, m22_axis_tvalid, m23_axis_tvalid;
wire m30_axis_tvalid, m31_axis_tvalid, m32_axis_tvalid, m33_axis_tvalid;

wire [C_NUM_CHNL*2-1:0]   adc_tvalid;
wire [64-1:0]  adc_tdata_1i, adc_tdata_1q, adc_tdata_2i, adc_tdata_2q, adc_tdata_3i, adc_tdata_3q, adc_tdata_4i, adc_tdata_4q, adc_tdata_5i, adc_tdata_5q, adc_tdata_6i, adc_tdata_6q, adc_tdata_7i, adc_tdata_7q, adc_tdata_8i, adc_tdata_8q;

wire [127:0] s00_axis_tdata, s01_axis_tdata, s02_axis_tdata, s03_axis_tdata;
wire [127:0] s10_axis_tdata, s11_axis_tdata, s12_axis_tdata, s13_axis_tdata;
wire s00_axis_tready, s01_axis_tready, s02_axis_tready, s03_axis_tready;
wire s10_axis_tready, s11_axis_tready, s12_axis_tready, s13_axis_tready;
wire s00_axis_tvalid=1'b1, s01_axis_tvalid=1'b1, s02_axis_tvalid=1'b1, s03_axis_tvalid=1'b1;
wire s10_axis_tvalid=1'b1, s11_axis_tvalid=1'b1, s12_axis_tvalid=1'b1, s13_axis_tvalid=1'b1;

wire [C_NUM_CHNL-1:0]     dac_tready;
wire [C_NUM_CHNL-1:0]     dac_tvalid = 'd1;
reg  [C_NUM_CHNL*128-1:0] dac_tdata;


//assignments
assign adc_tdata_1i = m00_axis_tdata;
assign adc_tdata_1q = m01_axis_tdata;
assign adc_tdata_2i = m02_axis_tdata;
assign adc_tdata_2q = m03_axis_tdata;
assign adc_tdata_3i = m10_axis_tdata;
assign adc_tdata_3q = m11_axis_tdata;
assign adc_tdata_4i = m12_axis_tdata;
assign adc_tdata_4q = m13_axis_tdata;
assign adc_tdata_5i = m20_axis_tdata;
assign adc_tdata_5q = m21_axis_tdata;
assign adc_tdata_6i = m22_axis_tdata;
assign adc_tdata_6q = m23_axis_tdata;
assign adc_tdata_7i = m30_axis_tdata;
assign adc_tdata_7q = m31_axis_tdata;
assign adc_tdata_8i = m32_axis_tdata;
assign adc_tdata_8q = m33_axis_tdata;

assign adc_tvalid[ 3:0]  = {m03_axis_tvalid,m02_axis_tvalid,m01_axis_tvalid,m00_axis_tvalid};
assign adc_tvalid[ 7:4]  = {m13_axis_tvalid,m12_axis_tvalid,m11_axis_tvalid,m10_axis_tvalid};
assign adc_tvalid[11:8]  = {m23_axis_tvalid,m22_axis_tvalid,m21_axis_tvalid,m20_axis_tvalid};
assign adc_tvalid[15:12] = {m33_axis_tvalid,m32_axis_tvalid,m31_axis_tvalid,m30_axis_tvalid};

assign s00_axis_tdata = dac_tdata[128*0 +:128];
assign s01_axis_tdata = dac_tdata[128*1 +:128];
assign s02_axis_tdata = dac_tdata[128*2 +:128];
assign s03_axis_tdata = dac_tdata[128*3 +:128];
assign s10_axis_tdata = dac_tdata[128*4 +:128];
assign s11_axis_tdata = dac_tdata[128*5 +:128];
assign s12_axis_tdata = dac_tdata[128*6 +:128];
assign s13_axis_tdata = dac_tdata[128*7 +:128];

assign dac_tready[0] = s00_axis_tready;
assign dac_tready[1] = s01_axis_tready;
assign dac_tready[2] = s02_axis_tready;
assign dac_tready[3] = s03_axis_tready;
assign dac_tready[4] = s10_axis_tready;
assign dac_tready[5] = s11_axis_tready;
assign dac_tready[6] = s12_axis_tready;
assign dac_tready[7] = s13_axis_tready;

assign lmk_sync = emio_gpio_o[0];
assign led = emio_gpio_o[4:1];

//input buffering
IBUFDS pl_clk_i (
    .I          (pl_clk_p),
    .IB         (pl_clk_n),
    .O          (pl_clk));
BUFG pl_clk_o (
  .I(pl_clk),
  .O(data_clk)
);

IBUFDS pl_sysref_i (
  .I          (pl_sysref_p),
  .IB         (pl_sysref_n),
  .O          (pl_sysref));

reg pl_sysref_r;  
always @(posedge pl_clk) begin
  pl_sysref_r <= pl_sysref;
end

always @(posedge pl_clk) begin
  user_sysref_adc <= pl_sysref_r;
end

always @(posedge pl_clk) begin
  user_sysref_dac <= pl_sysref_r;
end
 

//DDS
always @ (posedge data_clk)begin
  if(!dds_resetn)begin
    phase_reg[16*0 +:16]<=16'd0;
    phase_reg[16*1 +:16]<=phase;
    phase_reg[16*2 +:16]<=phase+phase;
    phase_reg[16*3 +:16]<=phase+phase+phase;
  end
  else begin
    phase_reg[16*0 +:16]<=phase_reg[16*0 +:16]+{phase,2'b0};
    phase_reg[16*1 +:16]<=phase_reg[16*1 +:16]+{phase,2'b0};
    phase_reg[16*2 +:16]<=phase_reg[16*2 +:16]+{phase,2'b0};
    phase_reg[16*3 +:16]<=phase_reg[16*3 +:16]+{phase,2'b0};
  end
end

genvar channel;
generate
  for (channel = 0; channel < 4; channel = channel + 1) begin : dds_channel_gen
    dds_compiler_0 dds_compiler_0 (
    .aclk                 (data_clk                 ),// input wire aclk
    .aresetn              (dds_resetn               ),// input wire aresetn
    .s_axis_phase_tvalid  (1'b1                     ),// input wire s_axis_phase_tvalid
    .s_axis_phase_tdata   (phase_reg[16*channel +:16]),// input wire [7 : 0] s_axis_phase_tdata
    .m_axis_data_tvalid   (                         ),// output wire m_axis_data_tvalid
    .m_axis_data_tdata    (dds_data[32*channel +:32])// output wire [31 : 0] m_axis_data_tdata
    );
  end
endgenerate

always @ (posedge data_clk)begin
  if(!dds_resetn)begin
    dac_tdata[128*0 +:128]<=128'd0;
    dac_tdata[128*1 +:128]<=128'd0;
    dac_tdata[128*2 +:128]<=128'd0;
    dac_tdata[128*3 +:128]<=128'd0;
    dac_tdata[128*4 +:128]<=128'd0;
    dac_tdata[128*5 +:128]<=128'd0;
    dac_tdata[128*6 +:128]<=128'd0;
    dac_tdata[128*7 +:128]<=128'd0;
  end 
  else begin
    dac_tdata[128*0 +:128]<=dds_data;
    dac_tdata[128*1 +:128]<=dds_data;
    dac_tdata[128*2 +:128]<=dds_data;
    dac_tdata[128*3 +:128]<=dds_data;
    dac_tdata[128*4 +:128]<=dds_data;
    dac_tdata[128*5 +:128]<=dds_data;
    dac_tdata[128*6 +:128]<=dds_data;
    dac_tdata[128*7 +:128]<=dds_data;
  end
end

//chipscope
ila_0 ila_0 (
.clk(data_clk),
.probe0({
// adc 8 channels I/Q data
adc_tdata_1i[16*0  +:16],
adc_tdata_1q[16*0  +:16],
adc_tdata_2i[16*0  +:16],
adc_tdata_2q[16*0  +:16],
adc_tdata_3i[16*0  +:16],
adc_tdata_3q[16*0  +:16],
adc_tdata_4i[16*0  +:16],
adc_tdata_4q[16*0  +:16],
adc_tdata_5i[16*0  +:16],
adc_tdata_5q[16*0  +:16],
adc_tdata_6i[16*0  +:16],
adc_tdata_6q[16*0  +:16],
adc_tdata_7i[16*0  +:16],
adc_tdata_7q[16*0  +:16],
adc_tdata_8i[16*0  +:16],
adc_tdata_8q[16*0  +:16],
// dac 4 phase data
dds_data[16*0  +:16],
dds_data[16*1  +:16],
dds_data[16*2  +:16],
dds_data[16*3  +:16],
dds_data[16*4  +:16],
dds_data[16*5  +:16],
dds_data[16*6  +:16],
dds_data[16*7  +:16]
})
);  


//block design
system system_i(
    .adc0_clk_clk_n     (adc0_clk_n),
    .adc0_clk_clk_p     (adc0_clk_p),
    .adc1_clk_clk_n     (adc1_clk_n),
    .adc1_clk_clk_p     (adc1_clk_p),
    .adc2_clk_clk_n     (adc2_clk_n),
    .adc2_clk_clk_p     (adc2_clk_p),
    .adc3_clk_clk_n     (adc3_clk_n),
    .adc3_clk_clk_p     (adc3_clk_p),
    .dac0_clk_clk_n     (dac0_clk_n),
    .dac0_clk_clk_p     (dac0_clk_p),
    .dac1_clk_clk_n     (dac1_clk_n),
    .dac1_clk_clk_p     (dac1_clk_p),
    .data_clk           (data_clk),
    .pl_resetn0         (fclk_reset0),
    .m00_axis_tdata     (m00_axis_tdata),
    .m00_axis_tready    (m00_axis_tready),
    .m00_axis_tvalid    (m00_axis_tvalid),
    .m01_axis_tdata     (m01_axis_tdata),
    .m01_axis_tready    (m01_axis_tready),
    .m01_axis_tvalid    (m01_axis_tvalid),
    .m02_axis_tdata     (m02_axis_tdata),
    .m02_axis_tready    (m02_axis_tready),
    .m02_axis_tvalid    (m02_axis_tvalid),
    .m03_axis_tdata     (m03_axis_tdata),
    .m03_axis_tready    (m03_axis_tready),
    .m03_axis_tvalid    (m03_axis_tvalid),
    .m10_axis_tdata     (m10_axis_tdata),
    .m10_axis_tready    (m10_axis_tready),
    .m10_axis_tvalid    (m10_axis_tvalid),
    .m11_axis_tdata     (m11_axis_tdata),
    .m11_axis_tready    (m11_axis_tready),
    .m11_axis_tvalid    (m11_axis_tvalid),
    .m12_axis_tdata     (m12_axis_tdata),
    .m12_axis_tready    (m12_axis_tready),
    .m12_axis_tvalid    (m12_axis_tvalid),
    .m13_axis_tdata     (m13_axis_tdata),
    .m13_axis_tready    (m13_axis_tready),
    .m13_axis_tvalid    (m13_axis_tvalid),
    .m20_axis_tdata     (m20_axis_tdata),
    .m20_axis_tready    (m20_axis_tready),
    .m20_axis_tvalid    (m20_axis_tvalid),
    .m21_axis_tdata     (m21_axis_tdata),
    .m21_axis_tready    (m21_axis_tready),
    .m21_axis_tvalid    (m21_axis_tvalid),
    .m22_axis_tdata     (m22_axis_tdata),
    .m22_axis_tready    (m22_axis_tready),
    .m22_axis_tvalid    (m22_axis_tvalid),
    .m23_axis_tdata     (m23_axis_tdata),
    .m23_axis_tready    (m23_axis_tready),
    .m23_axis_tvalid    (m23_axis_tvalid),
    .m30_axis_tdata     (m30_axis_tdata),
    .m30_axis_tready    (m30_axis_tready),
    .m30_axis_tvalid    (m30_axis_tvalid),
    .m31_axis_tdata     (m31_axis_tdata),
    .m31_axis_tready    (m31_axis_tready),
    .m31_axis_tvalid    (m31_axis_tvalid),
    .m32_axis_tdata     (m32_axis_tdata),
    .m32_axis_tready    (m32_axis_tready),
    .m32_axis_tvalid    (m32_axis_tvalid),
    .m33_axis_tdata     (m33_axis_tdata),
    .m33_axis_tready    (m33_axis_tready),
    .m33_axis_tvalid    (m33_axis_tvalid),
    .s00_axis_tdata     (s00_axis_tdata),
    .s00_axis_tready    (s00_axis_tready),
    .s00_axis_tvalid    (s00_axis_tvalid),
    .s01_axis_tdata     (s01_axis_tdata),
    .s01_axis_tready    (s01_axis_tready),
    .s01_axis_tvalid    (s01_axis_tvalid),
    .s02_axis_tdata     (s02_axis_tdata),
    .s02_axis_tready    (s02_axis_tready),
    .s02_axis_tvalid    (s02_axis_tvalid),
    .s03_axis_tdata     (s03_axis_tdata),
    .s03_axis_tready    (s03_axis_tready),
    .s03_axis_tvalid    (s03_axis_tvalid),
    .s10_axis_tdata     (s10_axis_tdata),
    .s10_axis_tready    (s10_axis_tready),
    .s10_axis_tvalid    (s10_axis_tvalid),
    .s11_axis_tdata     (s11_axis_tdata),
    .s11_axis_tready    (s11_axis_tready),
    .s11_axis_tvalid    (s11_axis_tvalid),
    .s12_axis_tdata     (s12_axis_tdata),
    .s12_axis_tready    (s12_axis_tready),
    .s12_axis_tvalid    (s12_axis_tvalid),
    .s13_axis_tdata     (s13_axis_tdata),
    .s13_axis_tready    (s13_axis_tready),
    .s13_axis_tvalid    (s13_axis_tvalid),
    .sysref_in_diff_n   (sysref_in_n),
    .sysref_in_diff_p   (sysref_in_p),
    .fclk_100m          (fclk_100m),
    .reset_n            (dds_resetn),
    .config_done        (config_done),
    .phase              (phase),
    .emio_gpio_o        (emio_gpio_o),
    .vin0_01_v_n        (vin0_01_n),
    .vin0_01_v_p        (vin0_01_p),
    .vin0_23_v_n        (vin0_23_n),
    .vin0_23_v_p        (vin0_23_p),
    .vin1_01_v_n        (vin1_01_n),
    .vin1_01_v_p        (vin1_01_p),
    .vin1_23_v_n        (vin1_23_n),
    .vin1_23_v_p        (vin1_23_p),
    .vin2_01_v_n        (vin2_01_n),
    .vin2_01_v_p        (vin2_01_p),
    .vin2_23_v_n        (vin2_23_n),
    .vin2_23_v_p        (vin2_23_p),
    .vin3_01_v_n        (vin3_01_n),
    .vin3_01_v_p        (vin3_01_p),
    .vin3_23_v_n        (vin3_23_n),
    .vin3_23_v_p        (vin3_23_p),
    .vout00_v_n         (vout00_n),
    .vout00_v_p         (vout00_p),
    .vout01_v_n         (vout01_n),
    .vout01_v_p         (vout01_p),
    .vout02_v_n         (vout02_n),
    .vout02_v_p         (vout02_p),
    .vout03_v_n         (vout03_n),
    .vout03_v_p         (vout03_p),
    .vout10_v_n         (vout10_n),
    .vout10_v_p         (vout10_p),
    .vout11_v_n         (vout11_n),
    .vout11_v_p         (vout11_p),
    .vout12_v_n         (vout12_n),
    .vout12_v_p         (vout12_p),
    .vout13_v_n         (vout13_n),
    .vout13_v_p         (vout13_p),
    .user_sysref_adc    (user_sysref_adc),
    .user_sysref_dac    (user_sysref_dac)
);

endmodule
