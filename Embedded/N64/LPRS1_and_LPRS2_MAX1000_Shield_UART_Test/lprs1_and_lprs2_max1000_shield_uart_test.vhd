
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

library work;

entity lprs1_and_lprs2_max1000_shield_uart_test is
	generic(
		-- Default frequency used in synthesis.
		constant CLK_FREQ : positive := 12000000
	);
	port(
		-- On MAX1000.
		-- System signals.
		i_clk                      :  in std_logic; -- 12MHz clock.
		in_rst                     :  in std_logic; -- Active low reset.
		
		
		
		-- On both LPRS1_MAX1000_Shield and LPRS2_MAX1000_Shield.
		-- UART.
		o_uart_tx                  : out std_logic;
		o_uart_rx                  :  in std_logic
	);
end entity;

architecture lprs1_and_lprs2_max1000_shield_uart_test_arch of 
	lprs1_and_lprs2_max1000_shield_uart_test is
	
	
begin
	
	process(i_clk, in_rst)
	begin
		if in_rst = '0' then
			o_uart_tx <= '0';
		elsif rising_edge(i_clk) then
			o_uart_tx <= o_uart_rx;
		end if;
	end process;
	
	
end architecture;
