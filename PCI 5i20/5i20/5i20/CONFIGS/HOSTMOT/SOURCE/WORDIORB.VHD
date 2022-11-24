library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity wordiorb is
    Port (			
	 		obus: out STD_LOGIC_VECTOR (15 downto 0);
			readport: in STD_LOGIC;
			portdata: in STD_LOGIC_VECTOR (15 downto 0) );
end wordiorb;

architecture behavioral of wordiorb is

begin
	awordiorb: process (portdata,readport)
	begin
		if readport = '1' then
			obus <= portdata;
 	   else
			obus <= "ZZZZZZZZZZZZZZZZ";
		end if;
	end process;

end behavioral;
