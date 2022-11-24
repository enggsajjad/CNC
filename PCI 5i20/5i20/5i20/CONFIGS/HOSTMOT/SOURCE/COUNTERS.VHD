library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;


entity counter is
   port ( 
	obus: out STD_LOGIC_VECTOR (31 downto 0);
	ibus: in STD_LOGIC_VECTOR (15 downto 0);
	quada: in STD_LOGIC;
	quadb: in STD_LOGIC;
	index: in STD_LOGIC;
	inbit: in STD_LOGIC;
	outbit: out STD_LOGIC;
	ccrloadcmd: in STD_LOGIC;
	ccrreadcmd: in STD_LOGIC;
	countoutreadcmd: in STD_LOGIC;
	countlatchcmd: in STD_LOGIC;
	countclearcmd: in STD_LOGIC;
	countenable: in STD_LOGIC;
	clk: in STD_LOGIC
	);
end counter;

architecture behavioral of counter is

signal count: STD_LOGIC_VECTOR (31 downto 0);
signal countoutlatch: STD_LOGIC_VECTOR (31 downto 0);
signal outlatchdel1: STD_LOGIC;
signal outlatchdel2: STD_LOGIC;
signal quada1: STD_LOGIC;
signal quada2: STD_LOGIC;
signal quadacnt: STD_LOGIC_VECTOR (2 downto 0);
signal quadafilt: STD_LOGIC;
signal quadb1: STD_LOGIC;
signal quadb2: STD_LOGIC;
signal quadbcnt: STD_LOGIC_VECTOR (2 downto 0);
signal quadbfilt: STD_LOGIC;
signal indexcnt: STD_LOGIC_VECTOR (2 downto 0);
signal indexfilt: STD_LOGIC;
signal qcountup: STD_LOGIC; 
signal qcountdown: STD_LOGIC;
signal udcountup: STD_LOGIC; 
signal udcountdown: STD_LOGIC;
signal index1: STD_LOGIC;
signal index2: STD_LOGIC;
signal doclear: STD_LOGIC;
signal clearonindex: STD_LOGIC;	-- ccr register bits...
signal latchedindex: STD_LOGIC;
signal quadfilter: STD_LOGIC;
signal countermode: STD_LOGIC;
signal indexpol: STD_LOGIC;
signal localenable: STD_LOGIC;
signal localclear: STD_LOGIC;
signal divisor: STD_LOGIC;


begin
	acounter: process 	(clk, 
	 							count,
								countoutlatch,
								ibus,
								countenable,
								quada,quada1,quada2,quadacnt,quadafilt,
							   quadb,quadb1,quadb2,quadbcnt,quadbfilt,
								index,index1,index2,indexcnt,indexfilt, 
								clearonindex,
								inbit,
								indexpol,
								countclearcmd,  
								countoutreadcmd,
								countlatchcmd,
								outlatchdel1,
								ccrloadcmd,
								ccrreadcmd,
								doclear,
								localclear,
								localenable,
								quadfilter,
								countermode,
								qcountup,
								qcountdown,
								udcountup,
								udcountdown,
								divisor)

	begin
		if clk'event and clk = '1' then				
			outlatchdel1 <= countlatchcmd;
			outlatchdel2 <= outlatchdel1;

			if quadfilter = '1' then 
				quada1 <= quadafilt;
			else quada1 <= quada;
			end if;
			quada2 <= quada1;

			if quadfilter = '1' then 
				quadb1 <= quadbfilt;
			else quadb1 <= quadb;
			end if;
			quadb2 <= quadb1;		

			if quadfilter = '1' then 
				index1 <= indexfilt;
			else index1 <= index;
			end if;
			index2 <= index1;		

				-- deadended counter for a input filter --
			if (quada = '1') and (quadacnt /= 7) then 
			   quadacnt <= quadacnt + 1;
			end if;
			if (quada = '0') and (quadacnt /= 0) then 
			   quadacnt <= quadacnt -1;
			end if;
			if quadacnt = 7 then
			  quadafilt<= '1';
			end if;
			if quadacnt = 0 then
			  quadafilt<= '0';
			end if;
			
			-- deadended counter for b input filter --
			if (quadb = '1') and (quadbcnt /= 7) then 
			   quadbcnt <= quadbcnt + 1;
			end if;
			if (quadb = '0') and (quadbcnt /= 0) then 
				quadbcnt <= quadbcnt -1;
			end if;
			if quadbcnt = 7 then
				quadbfilt<= '1';
			end if;
			if quadbcnt = 0 then
				quadbfilt<= '0';
			end if;	

			-- deadended counter for index input filter --
			if (index = '1') and (indexcnt /= 7) then 
			   indexcnt <= indexcnt + 1;
			end if;
			if (index = '0') and (indexcnt /= 0) then 
			   indexcnt <= indexcnt -1;
			end if;
			if indexcnt = 7 then
			  indexfilt<= '1';
			end if;
			if indexcnt = 0 then
			  indexfilt<= '0';
			end if;

 		   if ((index1 = '1') and (index2 = '0') and (indexpol = '1')) or	-- rising edge of index
			((index1 = '0') and (index2 = '1') and (indexpol = '0')) then -- falling edge of index
				latchedindex <= '1';	
			end if;
			if (countclearcmd = '1') or (localclear = '1') or 
			((clearonindex = '1')  and (index1 = '1') and (index2 = '0') and (indexpol = '1')) or		-- rising edge of index
			((clearonindex = '1')  and (index1 = '0') and (index2 = '1') and (indexpol = '0')) then	-- falling edge of index
				doclear <= '1';
			else
				doclear <= '0';
			end if;		
			if (outlatchdel2 = '0') and (outlatchdel1 = '1') then		-- leading edge of countoutlatchcmd
				countoutlatch <= count;
			end if;
			if countermode = '0' and localenable = '1' and countenable = '1' and doclear = '0' and (  
			(quada2 = '0' and quada1 = '1' and quadb2 = '0' and quadb1 = '0')  or
			(quada2 = '0' and quada1 = '0' and quadb2 = '1' and quadb1 = '0')  or
			(quada2 = '1' and quada1 = '1' and quadb2 = '0' and quadb1 = '1')  or
			(quada2 = '1' and quada1 = '0' and quadb2 = '1' and quadb1 = '1')) then		   
				qcountup <= '1';
			else 
				qcountup <= '0';
			end if;			

			if (countermode = '1'  and localenable = '1' and countenable = '1' and doclear = '0' and 
			quadb2 = '1' and quada2 = '0' and quada1 = '1') then
 				udcountup <= '1';
			else
				udcountup <= '0';
			end if;
			
			if countermode = '0' and localenable = '1' and countenable = '1' and doclear = '0' and ( 
			(quada2 = '0' and quada1 = '0' and quadb2 = '0' and quadb1 = '1')  or
			(quada2 = '0' and quada1 = '1' and quadb2 = '1' and quadb1 = '1')  or
			(quada2 = '1' and quada1 = '0' and quadb2 = '0' and quadb1 = '0')  or
			(quada2 = '1' and quada1 = '1' and quadb2 = '1' and quadb1 = '0')) then
				qcountdown <= '1';
			else
				qcountdown <= '0';
			end if;		
	
			if (countermode = '1' and localenable = '1' and countenable = '1' and doclear = '0' and  
			quadb2 = '0' and quada2 = '0' and quada1 = '1') then
				udcountdown <= '1';
			else
				udcountdown <= '0';
			end if;		

			if (qcountup = '1' or udcountup = '1') and doclear = '0' then
				count <= count + 1; 
			end if;
			if (qcountdown = '1' or udcountdown = '1') and doclear = '0' then
				count <= count - 1;
			end if;	
			if doclear = '1' then
				count <= x"00000000";
			end if;
			if ccrloadcmd = '1' then		
				countermode <= ibus(9);
				quadfilter <= ibus(8);
				outbit <= ibus(7);
				localenable <= ibus(6);
				clearonindex <= ibus(5);
				latchedindex <= ibus(4);
				indexpol <= ibus(3);
				localclear <= ibus(2);
			end if;
			if localclear = '1' then											-- once were done clearing,, dont stick around
				localclear <= '0';
			end if;
		end if; --(clock edge)
			
		obus <= "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ";
		if (countoutreadcmd = '1') and (ccrreadcmd = '0') then
			obus <= countoutlatch;
		end if;		
		if (ccrreadcmd = '1') and (countoutreadcmd = '0') then
				obus(9) <= countermode;
				obus(8) <= quadfilter;
				obus(7) <= inbit;		
				obus(6) <= localenable;			
				obus(5) <= clearonindex;
				obus(4) <= latchedindex;
				obus(3) <= indexpol;
				obus(2) <= index;
				obus(1) <= quada1;
				obus(0) <= quadb1;
				obus(31 downto 10) <= "ZZZZZZZZZZZZZZZZZZZZZZ";
		end if;
	end process;
end behavioral;
