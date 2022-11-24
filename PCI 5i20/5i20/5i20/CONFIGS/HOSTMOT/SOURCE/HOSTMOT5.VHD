library IEEE;
use IEEE.std_logic_1164.all;  -- defines std_logic types

entity HostMot5 is
  	port 
  (
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
	
   LAD: inout STD_LOGIC_VECTOR (31 downto 0); 		-- data/address bus
 	LA: in STD_LOGIC_VECTOR (8 downto 2); 				-- non-muxed address bus
	lBE: in STD_LOGIC_VECTOR (3 downto 0); 			-- byte enables

			
	SYNCLK: in STD_LOGIC;
	LClk: in STD_LOGIC;
	-- I/O signals	
	A: in STD_LOGIC_VECTOR (11 downto 0);
	B: in STD_LOGIC_VECTOR (11 downto 0);
	Idx: in STD_LOGIC_VECTOR (11 downto 0);
	PWM: inout STD_LOGIC_VECTOR  (11 downto 0);
	Ena: out STD_LOGIC_VECTOR  (11 downto 0);
	Dir: inout STD_LOGIC_VECTOR  (11 downto 0);

--	Outbits: out STD_LOGIC_VECTOR (11 downto 0);			
--	Inbits: in STD_LOGIC_VECTOR (11 downto 0);	

	-- led bits
	LEDS: out STD_LOGIC_VECTOR(7 downto 0)

	);
end HostMot5;


architecture dataflow of Hostmot5 is

	alias BLE: STD_LOGIC is LBE(0);	-- 16 bit mode
   alias BHE: STD_LOGIC is LBE(3);	-- 16 bit mode
	alias LA1: STD_LOGIC is LBE(1);	-- 8/16 bit mode
	alias LA0: STD_LOGIC is LBE(0);	-- 8 bit mode
-- misc global signals --
	signal D: STD_LOGIC_VECTOR (31 downto 0);							-- internal data bus
	signal LatchedA: STD_LOGIC_VECTOR (15 downto 0);
	signal LatchedLBE: STD_LOGIC_VECTOR (3 downto 0);
	signal PreFastRead: STD_LOGIC;
	signal FastRead: STD_LOGIC;
	-- per channel I/O signals --

	constant counters :integer := 12;

-- misc global signals --
	signal CardSelect: STD_LOGIC; 											-- card select decode
	signal LEDView: STD_LOGIC_VECTOR (7 downto 0); 					-- index register


--	irq related signals
	signal IRQSource: STD_LOGIC; 
	signal IRQLatch: STD_LOGIC;
	signal IRQMask: STD_LOGIC;
	signal MissedIRQ: STD_LOGIC; 
	signal LatchOnIRQ: STD_LOGIC;
	signal StopOnMissedIRQ: STD_LOGIC;
	signal ClearMissedIRQ: STD_LOGIC;
	signal LatchOnInterrupt: STD_LOGIC;

-- LEDView reg signals
	signal LoadLEDViewCmd: STD_LOGIC;
	signal ReadLEDViewCmd: STD_LOGIC;
	signal Enasigs :STD_LOGIC_VECTOR (counters-1 downto 0);
	signal Outbitsigs :STD_LOGIC_VECTOR (counters-1 downto 0);

-- irqdiv reg signals
	signal ReadIRQDivCmd: STD_LOGIC; 
	signal LoadIRQDivCmd: STD_LOGIC; 
	signal ClearIRQCmd: STD_LOGIC;

-- irq sel reg signals

	signal loadGCRCmd: STD_LOGIC;
	signal LoadGMRCmd: STD_LOGIC;
	signal ReadGMRCmd: STD_LOGIC;
	signal WordAccess: STD_LOGIC;

-- counter signals --
	signal CounterRead: STD_LOGIC_VECTOR  (counters-1 downto 0);		-- read counter
	signal GlobalCounterEnable: STD_LOGIC;										-- enable counting
	signal GlobalCountLatchcmd: STD_LOGIC;					-- command to latch counter value	
	signal GlobalCountLatch: STD_LOGIC;						-- command + irq generated latch count
	signal CountLatchEdge1: STD_LOGIC;
	signal CountLatchEdge2: STD_LOGIC;
	signal CCRLoadCmds: STD_LOGIC_VECTOR (counters-1 downto 0);			-- counter control reg loads
	signal CCRReadCmds:	STD_LOGIC_VECTOR (counters-1 downto 0);		-- counter control reg reads
	signal GlobalCounterClear: STD_LOGIC;										-- clear counter

-- pwm generator signals --
	signal RefCountBus: STD_LOGIC_VECTOR (10 downto 0);
	signal LoadPWM: STD_LOGIC_VECTOR  (counters-1 downto 0);
	signal ReadPWM: STD_LOGIC_VECTOR  (counters-1 downto 0);
	signal PCRLoadCmds: STD_LOGIC_VECTOR  (counters-1 downto 0);
	signal PCRReadCmds: STD_LOGIC_VECTOR  (counters-1 downto 0);
	signal GlobalPWMEnable: STD_LOGIC;
	signal GlobalClearPWM: STD_LOGIC;
	signal GlobalClearPWMCmd: STD_LOGIC;
	signal StopPWM: STD_LOGIC;

-- misc i/o signals
	signal LoadPortCmdA: STD_LOGIC;
	signal LoadDDRCmdA: STD_LOGIC;
	signal ReadDDRCmdA: STD_LOGIC;
	signal ReadPortCmdA: STD_LOGIC;
	signal MiscIOASigs: STD_LOGIC_VECTOR (15 downto 0); 

	signal LoadPortCmdB: STD_LOGIC;
	signal LoadDDRCmdB: STD_LOGIC;
	signal ReadDDRCmdB: STD_LOGIC;
	signal ReadPortCmdB: STD_LOGIC;
	signal MiscIOBSigs: STD_LOGIC_VECTOR (15 downto 0); 

-- decodes --
	signal LEDViewSel: STD_LOGIC;
	signal IndexSel: STD_LOGIC;	
	signal GCRSel: STD_LOGIC;	
	signal GMRSel: STD_LOGIC;
	signal CCRSel: STD_LOGIC;
	signal PCRSel: STD_LOGIC;	
	signal IRQDIVSel: STD_LOGIC;	
	signal PWMValSel: STD_LOGIC;
	signal CounterSel: STD_LOGIC;	

  function OneOfTwelveDecode(ena : std_logic; dec : std_logic_vector(3 downto 0)) return std_logic_vector is
    variable result : std_logic_vector(11 downto 0);
  begin
    if ena = '1' then
      case dec is
        when "0000" => result := "000000000001";
        when "0001" => result := "000000000010";
        when "0010" => result := "000000000100";
        when "0011" => result := "000000001000";
        when "0100" => result := "000000010000";
        when "0101" => result := "000000100000";
        when "0110" => result := "000001000000";
        when "0111" => result := "000010000000";
        when "1000" => result := "000100000000";
        when "1001" => result := "001000000000";
        when "1010" => result := "010000000000";
        when "1011" => result := "100000000000";
        when others => result := "000000000000";
      end case;
    else
      result := "000000000000";
    end if;
    return result;
  end OneOfTwelveDecode;

  	function OneOfTwelveMux(sel: std_logic_vector (3 downto 0); input: std_logic_vector(11 downto 0)) return std_logic is
 	variable result : std_logic;
  	begin
		case sel is
        when "0000" => result := input(0);
        when "0001" => result := input(1);
		  when "0010" => result := input(2);
		  when "0011" => result := input(3);
		  when "0100" => result := input(4);
		  when "0101" => result := input(5);
		  when "0110" => result := input(6);
		  when "0111" => result := input(7);
		  when "1000" => result := input(8);
		  when "1001" => result := input(9);
		  when "1010" => result := input(10);
		  when "1011" => result := input(11);
		  when others => result := '0';
      end case;
    return result;
  end OneOfTwelveMux;

	component indexreg 
   	port (
		clk: in STD_LOGIC;
		ibus: in STD_LOGIC_VECTOR (15 downto 0);
		obus: out STD_LOGIC_VECTOR (15 downto 0);
		loadindex: in STD_LOGIC;
		readindex: in STD_LOGIC;
		index: out STD_LOGIC_VECTOR (7 downto 0)
		);
	end component;
		
	component counter
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
	end component;

	component pwmgen 
		port (
		clk: in STD_LOGIC;
		refcount: in STD_LOGIC_VECTOR (10 downto 0);
		ibus: in STD_LOGIC_VECTOR (15 downto 0);
		obus: out STD_LOGIC_VECTOR (15 downto 0);
		loadpwmval: in STD_LOGIC;
		readpwmval: in STD_LOGIC;
		clearpwmval: in STD_LOGIC;
		pcrloadcmd: STD_LOGIC;
		pcrreadcmd: STD_LOGIC;
		pwmout: out STD_LOGIC;
		dirout: out STD_LOGIC;
		enablein: in STD_LOGIC;
		enableout: out STD_LOGIC
		);
	end component pwmgen;
		
	component pwmref is
   	port (
		clk: in STD_LOGIC;
		refcount: out STD_LOGIC_VECTOR (10 downto 0);
		irqgen: out STD_LOGIC;
		ibus: in STD_LOGIC_VECTOR (15 downto 0);
		obus: out STD_LOGIC_VECTOR (15 downto 0);
		irqdivload: in STD_LOGIC;
		irqdivread: in STD_LOGIC
		);
	end component pwmref;

 	
	component globalcontrolreg is
	   port (
		clk: in STD_LOGIC;
		ibus: in STD_LOGIC_VECTOR (15 downto 0);
		reset: in STD_LOGIC;
		loadgcr: in STD_LOGIC;
		ctrclear: out STD_LOGIC;
		ctrlatch: out STD_LOGIC;
		pwmclear: out STD_LOGIC;
		irqclear: out STD_LOGIC
	);
	end component globalcontrolreg;

	component globalmodereg is
   	port (
		clk: in STD_LOGIC;
		ibus: in STD_LOGIC_VECTOR (15 downto 0);
		obus: out STD_LOGIC_VECTOR (15 downto 0);
		reset: in STD_LOGIC;
		loadglobalmode: in STD_LOGIC;
		readglobalmode: in STD_LOGIC;
		ctrena: out STD_LOGIC;
		pwmena: out STD_LOGIC;
		clearpwmena: in STD_LOGIC;
		loi: out STD_LOGIC;
		som: out STD_LOGIC;
		miout: out STD_LOGIC;
		miin: in STD_LOGIC;
   	irqmask: out STD_LOGIC;
   	irqstatus: in STD_LOGIC		
		);
	end component globalmodereg;

	component WordPort is 
		port (
		clear: in STD_LOGIC;
		clk: in STD_LOGIC;
		ibus: in STD_LOGIC_VECTOR (15 downto 0);
		obus: out STD_LOGIC_VECTOR (15 downto 0);
		loadport: in STD_LOGIC;
		loadddr: in STD_LOGIC;
		readddr: in STD_LOGIC;
		portdata: out STD_LOGIC_VECTOR (15 downto 0)
		);
	end component WordPort;

	component WordIORB is
    port (			
	 		obus: out STD_LOGIC_VECTOR (15 downto 0);
			readport: in STD_LOGIC;
			portdata: in STD_LOGIC_VECTOR (15 downto 0) );
	end component WordIORB;

	
	begin

 

	makecounters: for i in 0 to (counters -1) generate
		counterx: counter port map ( 		
		obus => D,
		ibus => LAD(15 downto 0),
		quada => A(i),
		quadb => B(i),		
		index => Idx(i),
		inbit => '0', 
--		outbit =>
		ccrloadcmd => CCRLoadCmds(i),
		ccrreadcmd =>	CCRReadCmds(i),
		countoutreadcmd  => CounterRead(i),
		countlatchcmd => GlobalCountLatch,	
		countclearcmd => GlobalCounterClear,
		countenable => GlobalCounterEnable,
		clk => LClk
		);	
	end generate;

	makepwmgen: for i in 0 to (counters -1) generate
		pwmgenx: pwmgen port map (
		clk => LClk,
		refcount => RefCountBus,
		ibus => LAD(15 downto 0),
		obus => D(15 downto 0),
		loadpwmval => LoadPWM(i),
		readpwmval => ReadPWM(i),
		clearpwmval => GlobalClearPWM,
		pcrloadcmd => PCRLoadCmds(i),
		pcrreadcmd => PCRReadCmds(i),
		pwmout => PWM(i),
		dirout => Dir(i),
		enablein => GlobalPWMEnable,
		enableout =>EnaSigs(i)
	);
	end generate;

	pwmrefcount: pwmref port map (
		clk => LClk,
		refcount => RefCountBus,
		irqgen => IRQSource,
		ibus => LAD(15 downto 0),
		obus => D(15 downto 0),
		irqdivload => LoadIRQDivCmd,
		irqdivread => ReadIRQDivCmd
		);


	gLedreg: indexreg port map (			
		clk => LClk,
		ibus => LAD(15 downto 0),
		obus => D(15 downto 0),
		loadindex => LoadLEDViewCmd,
		readindex => ReadLEDViewCmd,
		index => LEDView
		);

	ggcontrolreg: globalcontrolreg port map (
		clk => LClk,
		ibus => LAD(15 downto 0),
		reset => '0',
		loadgcr => LoadGCRCmd,
		ctrclear => GlobalCounterClear,
		ctrlatch => GlobalCountLatchCmd,
		pwmclear => GlobalClearPWMCmd,
		irqclear => ClearIRQCmd
		);

	gglobalmodereg: globalmodereg port map (
		clk => LClk,
		ibus => LAD(15 downto 0),
		obus => D(15 downto 0),
		reset => '0',
		loadglobalmode => loadGMRCmd,
		readglobalmode => ReadGMRCmd,
		ctrena => GlobalCounterEnable,
		pwmena => GlobalPWMEnable,
		clearpwmena => StopPWM,
		loi => LatchOnInterrupt,
		som => StopOnMissedIRQ,
		miout	=> ClearMissedIRQ,
		miin => MissedIRQ,
		irqmask => IRQMask,
		irqstatus => IRQLatch
		);

		
	miscoporta: WordPort port map (
		clear => '0',
		clk => LClk,
		ibus => LAD(15 downto 0),
		obus => D(15 downto 0),
		loadport => LoadPortCmdA,
		loadddr => LoadDDRCmdA,
		readddr => ReadDDRCmdA,
		portdata => MiscIOASigs 
		);	
	
	misciporta: WordIORB port map (
		obus => D(15 downto 0),
		readport => ReadPortCmdA,
		portdata => MiscIOASigs
		);	

	miscoportb: WordPort port map (
		clear => '0',
		clk => LClk,
		ibus => LAD(15 downto 0),
		obus => D(15 downto 0),
		loadport => LoadPortCmdB,
		loadddr => LoadDDRCmdB,
		readddr => ReadDDRCmdB,
		portdata => MiscIOBSigs 
		);	
	
	misciportb: WordIORB port map (
		obus => D(15 downto 0),
		readport => ReadPortCmdB,
		portdata => MiscIOBSigs
		);		



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
	  			LatchedA <= LAD(15 downto 0);
				LatchedLBE <= LBE;
			end if;
		end if;
	end process AddressLatch;


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

		
	Decode: process (LatchedA) 
	begin 
	

		if LatchedA(7 downto 6) = "00" then 	 -- 32 bit access
			CounterSel <= '1'; 
		else 
			CounterSel <= '0';
		end if;		
		
		if LatchedA(7 downto 5) = "010" then    -- 16 bit access
			CCRSel <= '1'; 
		else 
			CCRSel <= '0';
		end if;			

 		if LatchedA(7 downto 5) = "011" then 		 -- 16 bit access
			PWMValSel <= '1'; 
		else 
			PWMValSel <= '0';
		end if;				

 		if LatchedA(7 downto 5) = "100" then 		-- 16 bit access
			PCRSel <= '1'; 
		else 
			PCRSel <= '0';
		end if;		
 
 		if LatchedA(7 downto 1) = "1100000"  then 
			GCRSel <= '1'; 
		else 
			GCRSel <= '0';
		end if;
			
		if LatchedA(7 downto 1) = "1100001" then 
			GMRSel <= '1'; 
		else 
			GMRSel <= '0';
		end if;			
			
		if LatchedA(7 downto 1) = "1100010"then 
			IRQDivSel <= '1'; 
		else 
			IRQDivSel <= '0';
		end if;		

		if LatchedA(7 downto 1) = "1100011" then 
			LEDViewSel <= '1'; 
		else 
			LEDViewSel <= '0';
		end if;					
	
	end process;		

								

	SigsOut: process (EnaSigs)
	begin
--		OutBits <= OutBitSigs;
		Ena <= EnaSigs;
	end process;
	
	CounterDecode: process (CounterSel, Fastread, LatchedA) 
	begin 
		if FastRead = '1' then
			CounterRead <= OneOfTwelveDecode(CounterSel,LatchedA(5 downto 2));
 		else
			CounterRead <= (others => '0');
		end if;
	end process;	

	CCRegs: process (CCRSel, FastRead, LWR, LatchedA) 
	begin 
 		if FastRead = '1' then
			CCRReadCmds <= OneOfTwelveDecode(CCRSel,LatchedA(4 downto 1));
 		else
			CCRReadCmds <= (others => '0');
		end if;
		if LWR = '0' then
			CCRLoadCmds <= OneOfTwelveDecode(CCRSel,LatchedA(4 downto 1));
 		else
			CCRLoadCmds <= (others => '0');
		end if;
	end process;	

	PWMdecode: process (PWMValSel,Fastread, LWR, LatchedA) 
	begin 
		if FastRead = '1' then
			ReadPWM <= OneOfTwelveDecode(PWMValSel,LatchedA(4 downto 1));
 		else
			ReadPWM <= (others => '0');
		end if; 			
		if LWR = '0' then
			LoadPWM <= OneOfTwelveDecode(PWMValSel,LatchedA(4 downto 1));
 		else
			LoadPWM <= (others => '0');
		end if; 		
	end process;	

 	PCRegs: process (PCRSel,Fastread, LWR, LatchedA) 
	begin 
		if FastRead = '1' then
			PCRReadCmds <= OneOfTwelveDecode(PCRSel,LatchedA(4 downto 1));
 		else
			PCRReadCmds <= (others => '0');
		end if; 			
		if LWR = '0' then
			PCRLoadCmds <= OneOfTwelveDecode(PCRSel,LatchedA(4 downto 1));
 		else
			PCRLoadCmds <= (others => '0');
		end if; 		
	end process;	

	GCRDecode: process (GCRSel,LWR)
	begin
		if GCRSel = '1' and LWR = '0' then 
			LoadGCRCmd <= '1';
		else 
			LoadGCRCmd <= '0';
		end if;
	end process GCRDecode;

	GMRDecode: process (GMRSel,FastRead,LWR)
	begin
		if GMRSel = '1' and LWR = '0' then 
			LoadGMRCmd <= '1';
		else 
			LoadGMRCmd <= '0';
		end if;
		if GMRSel = '1' and FastRead = '1' then 
			ReadGMRCmd <= '1';
		else 
			ReadGMRCmd <= '0';
		end if;
	end process GMRDecode;

	
	LEDViewDecode: process (LedViewSel,FastRead,LWR)
	begin
		if LEDViewSel = '1' and LWR = '0' then 
			LoadLEDViewCmd <= '1';
		else 
			LoadLEDViewCmd <= '0';
		end if;
		if LEDViewSel = '1' and FastRead= '1' then 
			ReadLEDViewCmd <= '1';
		else 
			ReadLEDViewCmd <= '0';
		end if;
	end process LEDViewDecode;

	
	IRQDivDecode: process (IRQDivSel,FastRead,LWR)
	begin
		if IRQDivSel = '1' and LWR = '0' then 
			LoadIRQDivCmd <= '1';
		else 
			LoadIRQDivCmd <= '0';
		end if;
		if IRQDivSel = '1' and FastRead = '1' then 
			ReadIRQDivCmd <= '1';
		else 
			ReadIRQDivCmd <= '0';
		end if;
	end process IrqDivDecode;


	irqlogic: process (CardSelect,
							IRQSource, 
							IrqLatch, 
							ClearMissedIRQ,
							MissedIRQ,
							StopOnMissedIRQ,
							LatchOnInterrupt,
							GlobalCountLatchCmd,
							GlobalClearPWMCmd,
							ClearIRQCmd)
	begin		
 		if IrqSource'event and IRQsource = '1' then
			IRQLatch <= IRQMask;
			if IRQLatch = '1' then 	-- if IRQLatch is set and we get the next interrupt
				MissedIRQ <= '1';		-- set Missed IRQ latch	
			end if;			
		end if;

		if LClk'event and LClk = '1' then
			if ((IRQLatch = '1') and (LatchOnInterrupt = '1')) or (GlobalCountLatchCmd = '1') then
				CountLatchEdge1 <= '1';
			else
				CountLatchEdge1 <= '0';
			end if;
			CountLatchEdge2 <= 	CountLatchEdge1;
			if  CountLatchEdge2 = '0' and	CountLatchEdge1 = '1' then
				GlobalCountLatch <= '1';
			else
				GlobalCountLatch <= '0';
		 	end if;
		end if;

		if ClearMissedIRQ = '1' then 
			MissedIRQ <= '0';
		end if;

		if (MissedIRQ = '1') and (StopOnMissedIRQ = '1') then
			StopPWM <= '1';
		else 
			StopPWM <= '0';
		end if;
		
		if (StopPWM = '1') or (GlobalClearPWMCmd = '1') then  -- either stop on pwm or global clear reset pwm gens
			GlobalClearPWM <= '1';
		else
			GlobalClearPWM <= '0';
		end if;
			
		if ClearIRQCmd = '1' then  --  clear IRQ
 	   	IRQLatch <= '0';
		end if;
		
		Int <=  not IRQLatch;	-- drive our (active low) interrupt pin
		Ready <= '0';				-- We're always ready
		
	end process;


	LEDDrive: process (A,B,Idx,Dir,PWM,IRQLatch,LedView) 
	begin 
	
		LEDS(7) <= not IRQLatch;
		LEDS(6) <= not OneOfTwelveMux(LEDView(3 downto 0),A);
		LEDS(5) <= not OneOfTwelveMux(LEDView(3 downto 0),B);
		LEDS(4) <= not OneOfTwelveMux(LEDView(3 downto 0),Idx);
		LEDS(3) <= not OneOfTwelveMux(LEDView(3 downto 0),Dir);
		LEDS(2) <= not OneOfTwelveMux(LEDView(3 downto 0),PWM);
		LEDS(1) <= OneOfTwelveMux(LEDView(3 downto 0),EnaSigs);
		LEDS(0) <= not OutbitSigs(0);
		
	end process leddrive;

end dataflow;

  