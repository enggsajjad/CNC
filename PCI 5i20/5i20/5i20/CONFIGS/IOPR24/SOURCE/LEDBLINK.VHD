library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity ledblink is
    Port (
	 		clk: in STD_LOGIC;
			ledx: out STD_LOGIC_VECTOR (7 downto 0)
			);
end ledblink;

architecture behavioral of ledblink is

signal count: STD_LOGIC_VECTOR (28 downto 0);

begin
	aledbliker: process  (clk,
								count
								) 
	begin
		if clk'event and clk = '1' then
			count <= count + 1;			
		end if;
	end process;
	ledx(0) <= not count(21);
	ledx(1) <= not count(22);
	ledx(2) <= not count(23);
	ledx(3) <= not count(24);
	ledx(4) <= not count(25);	
	ledx(5) <= not count(26);
	ledx(6) <= not count(27);
	ledx(7) <= not count(28);
end behavioral;

