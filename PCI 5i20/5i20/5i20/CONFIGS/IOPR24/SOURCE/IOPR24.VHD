library IEEE;
use IEEE.std_logic_1164.all;  -- defines std_logic types
 
entity IOPR24 is
  	port 
  (
     -- bus interface signals --
	LRD: in STD_LOGIC; 
	LWR: in STD_LOGIC; 
	LW_R: in STD_LOGIC; 
	ALE: in STD_LOGIC; 
	ADS: in STD_LOGIC; 
	BLAST: in STD_LOGIC; 
	WAITO: in STD_LOGIC;
	LOCKO: in STD_LOGIC;
	CS0: in STD_LOGIC;
	CS1: in STD_LOGIC;
	READY: out STD_LOGIC; 
	INT: out STD_LOGIC;
	
    LAD: inout STD_LOGIC_VECTOR (31 downto 0); 			-- data/address bus
 	LA: in STD_LOGIC_VECTOR (8 downto 2); 				-- non-muxed address bus
	lBE: in STD_LOGIC_VECTOR (3 downto 0); 				-- byte enables

	IOBits: inout STD_LOGIC_VECTOR (71 downto 0);			
	SYNCLK: in STD_LOGIC;
	LClk: in STD_LOGIC;

	-- led bits
	LEDS: out STD_LOGIC_VECTOR(7 downto 0)

	);
end IOPR24;

architecture dataflow of IOPR24 is

	alias BLE: STD_LOGIC is LBE(0);	-- 16 bit mode
    alias BHE: STD_LOGIC is LBE(3);	-- 16 bit mode
	alias LA1: STD_LOGIC is LBE(1);	-- 8/16 bit mode
	alias LA0: STD_LOGIC is LBE(0);	-- 8 bit mode
-- misc global signals --
	signal D: STD_LOGIC_VECTOR (31 downto 0);							-- internal data bus
	signal A: STD_LOGIC_VECTOR (15 downto 0);
	signal PreFastRead: STD_LOGIC;
	signal FastRead: STD_LOGIC;
	signal test32: STD_LOGIC_VECTOR (31 downto 0);

-- decodes --
	signal WordSel: STD_LOGIC_VECTOR(3 downto 0);	
	signal LoadPortCmd: STD_LOGIC_VECTOR(3 downto 0);
	signal ReadPortCmd: STD_LOGIC_VECTOR(3 downto 0);
	signal DDRSel: STD_LOGIC_VECTOR(2 downto 0);	
	signal LoadDDRCmd: STD_LOGIC_VECTOR(2 downto 0);
	signal ReadDDRCmd: STD_LOGIC_VECTOR(2 downto 0);	

-- components --	
	component WordPR24 is 
		port (
		clear: in STD_LOGIC;
		clk: in STD_LOGIC;
		ibus: in STD_LOGIC_VECTOR (23 downto 0);
		obus: out STD_LOGIC_VECTOR (23 downto 0);
		loadport: in STD_LOGIC;
		loadddr: in STD_LOGIC;
		readddr: in STD_LOGIC;
		portdata: out STD_LOGIC_VECTOR (23 downto 0)
		);
	end component WordPR24;

	component Word24RB is
    Port (			
	 		obus: out STD_LOGIC_VECTOR (23 downto 0);
			readport: in STD_LOGIC;
			portdata: in STD_LOGIC_VECTOR (23 downto 0) );
	end component Word24RB;

	component ledblink is
   	port (
		clk: in STD_LOGIC;
		ledx: out STD_LOGIC_VECTOR (7 downto 0)
		);
	end component ledblink;

	
	begin
 
	gledblink: ledblink port map (
		clk => SynClk,
		ledx => LEDS
		);	
	
	makeoports: for i in 0 to 2 generate
		oportx: WordPR24 port map (
		clear => '0',
		clk => LClk,
		ibus => LAD(23 downto 0),
		obus => D(23 downto 0),
		loadport => LoadPortCmd(i),
		loadddr => LoadDDRCmd(i),
		readddr => ReadDDRCmd(i),
		portdata => IOBits((((i+1)*24) -1) downto (i*24)) 
		);	
	end generate;

	makeiports: for i in 0 to 2 generate
		iportx: Word24RB port map (
		obus => D(23 downto 0),
		readport => ReadPortCmd(i),
		portdata => IOBits((((i+1)*24) -1) downto (i*24)) 
		);	
	end generate;

	
	Decode: process(A) 
	begin
				
		case A(4 downto 2)is
			when "000" => wordsel <= "0001";
			when "001" => wordsel <= "0010";
			when "010" => wordsel <= "0100";
			when "011" => wordsel <= "1000";
			when others => wordsel <= "0000";
		end case;
		case A(4 downto 2)is
			when "100" => DDRsel <= "001";
			when "101" => DDRsel <= "010";
			when "110" => DDRsel <= "100";
			when others => DDRsel <= "000";
		end case;
		
	end process;


	
	PortDecode: process (WordSel,DDRSel,FastRead,LWR)
	begin
		for i in 0 to 3 loop                                                                                                                                                                                                                                                                                                                                                                                                                
			if WordSel(i) = '1' and LWR = '0' then 
    			LoadPortCmd(i) <= '1';
			else 
				LoadPortCmd(i) <= '0';
			end if;
			if WordSel(i) = '1' and FastRead = '1' then 
				ReadPortCmd(i) <= '1';
			else 
				ReadPortCmd(i) <= '0';
			end if;
		end loop;
	
		for i in 0 to 2 loop                                                                                                                                                                                                                                                                                                                                                                                                                
			if DDRSel(i) = '1' and LWR = '0' then 
    			LoadDDRCmd(i) <= '1';
			else 
				LoadDDRCmd(i) <= '0';
			end if;
			if DDRSel(i) = '1' and FastRead = '1' then 
				ReadDDRCmd(i) <= '1';
			else 
				ReadDDRCmd(i) <= '0';
			end if;
		end loop;	
	end process PortDecode;


	LADDrivers: process (D,FastRead)
	begin 
		if FastRead ='1' then	
			LAD <= D;
		else
			LAD <= "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ";			
		end if;
	end process LADDrivers;

	AddressLatch: process (lclk)
	begin
		if lclk'event and LClk = '1' then
	  		if ADS = '0' then
	  			A <= LAD(15 downto 0);
			end if;
		end if;
	end process AddressLatch;

	TestLatch: process(lclk,ReadPortCmd)
	begin
		if lclk'event and LClk = '1' then
			if LoadPortCmd(3) = '1' then
				test32 <= LAD;
			end if;
		end if;
		if ReadPortCmd(3) = '1' then
			D <= test32;
		else
			D <= "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ";
		end if;
	end process;


	-- we generate an early read from ADS and LR_W
	-- since the 10 nS LRD delay and 5 nS setup time
 	-- only give us 15 nS to provide data to the PLX chip
	
	MakeFastRead: process (lclk,PreFastread,LRD)
	begin
		if lclk'event and LClk = '1' then
			if ADS = '0' and LW_R = '0'then
				PreFastRead <= '1';
			else 
				PreFastRead <= '0'; 
			end if;
		end if;
		FastRead <= PreFastRead or (not LRD);
	end process MakeFastRead;


end dataflow;

  