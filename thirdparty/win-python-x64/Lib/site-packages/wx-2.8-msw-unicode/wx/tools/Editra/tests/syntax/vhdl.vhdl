-- Syntax Highlighting Test File for VHDL
-- Comments are like this
-- Hello World in VHDL

entity hello_world is
end;
  
 architecture hello_world of hello_world is
 begin
    stimulus : process
    begin
      assert false report "Hello World By Deepak"
      severity note;
      wait;
    end process stimulus;
end hello_world;

-- A simple counter
library ieee ;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
   
entity counter is 
port(  clk:  in std_logic;
   reset:  in std_logic;
   enable:  in std_logic;
   count:  out std_logic_vector(3 downto 0)
);
end counter;
  
architecture behav of counter is         
  signal pre_count: std_logic_vector(3 downto 0);
  begin
    process(clk, enable, reset)
    begin
      if reset = '1' then
        pre_count <= "0000";
      elsif (clk='1' and clk'event) then
        if enable = '1' then
          pre_count <= pre_count + "1";
        end if;
      end if;
    end process;  
    count <= pre_count;
end behav;
