library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity wordport is
    Port (		
	   	clear: in STD_LOGIC;
			clk: in STD_LOGIC;
			ibus: in STD_LOGIC_VECTOR (15 downto 0);
			obus: out STD_LOGIC_VECTOR (15 downto 0);
			loadport: in STD_LOGIC;
			loadddr: in STD_LOGIC;
			readddr: in STD_LOGIC;
			portdata: out STD_LOGIC_VECTOR (15 downto 0)
 			);
end wordport;

architecture behavioral of wordport is

signal outreg: STD_LOGIC_VECTOR (15 downto 0);
signal ddrreg: STD_LOGIC_VECTOR (15 downto 0);
signal loadport1: STD_LOGIC;
signal loadport2: STD_LOGIC;
signal loadddr1: STD_LOGIC;
signal loadddr2: STD_LOGIC;
signal tsoutreg: STD_LOGIC_VECTOR (15 downto 0);

begin
	awordioport: process (
								clk,
								ibus,
								loadport,loadport1,loadport2,
								loadddr,loadddr1,loadddr2,
								readddr,
								outreg,ddrreg)
	begin
		if clk'event and clk = '1' then
			loadport1 <= loadport;
			loadport2 <= loadport1;
			loadddr1 <= loadddr;
			loadddr2 <= loadddr1;
			if loadport1 = '0' and loadport2 = '1' then
				outreg <= ibus;
			end if; 
			if loadddr1 = '0' and loadddr2 = '1' then
				ddrreg <= ibus;
			end if;
			if clear = '1' then ddrreg <= x"0000"; end if;
		end if; -- clk

		for i in 0 to 15 loop
			if ddrreg(i) = '1' then 
				tsoutreg(i) <= outreg(i);
			else
				tsoutreg(i) <= 'Z';
			end if;
		end loop;
		
		portdata <= tsoutreg;
		
		if readddr = '1' then
			obus <= ddrreg;
 	   else
			obus <= "ZZZZZZZZZZZZZZZZ";
		end if;

	end process;
end behavioral;
