// System Verilog Syntax Highlighting Test File
// Comments are like this

package oop;

class BaseScoreboard;
  string name;
  bit[3:0] srce, dest;
  int run_for_n_packets;  // max number of packets to transmit before giving up on coverage goals
  static int pkts_checked = 0;
  static int sent_pkt_count = 0;
  static int recvd_pkt_count = 0;
  Packet refPkts[$];

  function new(string name = "class", int run_for_n_packets ); 
    if (TRACE_ON) $display("@%0d: %s.new() started", $time, name);
    this.name = name;
    this.run_for_n_packets = run_for_n_packets;
  endfunction : new

  task report();
    if (TRACE_ON) $display("@%0d: %s.report() started", $time, name);
    $display("%0d packets sent, %0d packets sampled, %0d packets checked\n", 
              sent_pkt_count, recvd_pkt_count, pkts_checked);
  endtask : report

endclass : BaseScoreboard


class Scoreboard extends BaseScoreboard;

  covergroup router_cvg;
    coverpoint srce;
    coverpoint dest;
    cross srce, dest;
    option.at_least = 1;
    option.auto_bin_max = 256;
  endgroup

  function new(string name = "class", int run_for_n_packets ); 
    super.new(name, run_for_n_packets); 
    router_cvg = new();
  endfunction : new

  task check(Packet pktrecvd);
    int    index;
    int    status;
    string diff;
    Packet pktsent;
    if (TRACE_ON) $display("@%0d: %s.check() started", $time, name);
  endtask


endclass : Scoreboard

endpackage : oop

/*****************************************************************************/

program test(io_if dutif, input bit clk);

import oop::*;

bit[3:0] srce, dest;
reg[7:0] payload[$], pkt2cmp_payload[$];

Scoreboard sb;

initial begin
  DONE <= 0;
  sb = new("sb", 2500); 
  pkt2send = new();
  pkt2send.pt_mode = 1;
  do begin
       fork
         begin send(); end
         begin recv(); end
       join
     end
  repeat(10) @(posedge clk);
end

task automatic recv();
  static int pkts_recvd = 0;
  int i;
  pktrecvd = new($psprintf("Pkt_recvd[%0d]", pkts_recvd++));
  pktrecvd.payload = new[pkt2cmp_payload.size()];
  for (i=0; i<pkt2cmp_payload.size(); i++)
    pktrecvd.payload[i] = pkt2cmp_payload[i];
  pktrecvd.dest = dest;
endtask

task automatic send();
  int i;
  payload.delete();
  for (i=0; i<pkt2send.payload.size(); i++)
    payload.push_back(pkt2send.payload[i]);
  srce = pkt2send.srce;
  dest = pkt2send.dest;
endtask

endprogram

interface io(input clock, input bit[15:0] din, frame_n);
bit [15:0] passthru;

sequence s_pass_thru_0 ;
  frame_n[ 0] ##1 !frame_n[ 0] ##0 din[ 0] == 0 [*4];  // 0000
endsequence

property p_pass_thru_0 ;
  @(posedge clk) fr_valid |-> s_pass_thru_0;
endproperty

assert property (p_pass_thru_0) $info("%m  OK"); 
  else $error("%m Problem");
endinterface

sequence s_pass_thru_1 ;
  frame_n[ 1] ##1 !frame_n[ 1] ##0 din[ 1] == 0 [*4];  // 0000
endsequence

property p_pass_thru_1 ;
  @(posedge clk) fr_valid |-> s_pass_thru_1;
endproperty

assert property (p_pass_thru_1) $info("%m  OK"); 
  else $error("%m Problem");
endinterface

module top;
bit clk;
bit[15:0] din;
bit frame_n;

io IF1 (.clk, .din, .frame_n);
test TB1 (.clk, io_if(IF1) );

endmodule
