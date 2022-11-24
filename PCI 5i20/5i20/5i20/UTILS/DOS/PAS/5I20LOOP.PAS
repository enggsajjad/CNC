program I20LOOP;
uses dos;

{$I HEXREAD.PAS}
{$I HPRINT.PAS}
{$I BEEPY.PAS}
{$I pci}
{$I 9030}
{$I general }
const
AllIn  : longint = $000000;
AllOut : longint = $FFFFFF;
PortAOff = $00;
PortBOff = $04;
PortCOff = $08;
TestPortOff = $0C;
DDRPortOff = $10;

var
ConfigBase,BasePort : word;
PortA,PortB,PortC,TestPort : word;
CardNum : word;

procedure BadBeeps;
begin
  BeepOn(800);
  BeepDelay(20);
  BeepOn(1500);
  BeepDelay(10);
  BeepOff;
end;

procedure HappyBeeps;
begin
  BeepOn(1000);
  BeepDelay(6);
  BeepOn(1200);
  BeepDelay(3);
  BeepOn(1000);
  BeepDelay(6);
  BeepOn(1200);
  BeepDelay(3);
  BeepOn(1000);
  BeepDelay(6);
  BeepOn(1200);
  BeepDelay(3);
  BeepOff;
end;

procedure GetCardNum;
var
retcode : integer;
begin
  if ParamCount > 0 then
  begin
    val(ParamStr(1),CardNum,retcode);
    if (retcode <> 0) or (CardNum <0) or (CardNum >15) then
    begin
      writeln;
      writeln('Invalid 5I20 card #');
      halt(2);
    end;
  end;
end;

procedure SetupPorts;
var busnum,devnum: byte;
begin
  if PCIBIOSPresent then
  begin
    writeln('PCI BIOS is present!');
    if FindPCIDevice(N030DeviceID,N030VendorID,CardNum,busnum,devnum) then
    begin
      writeln('found 9030 at bus: ',busnum,' device: ',devnum);
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress1,ConfigBase) then
      begin
        ConfigBase := ConfigBase and $FFFE;
        write('PLX 9030 Configuration IO base address is: ');
        HexPrint(ConFigBase,4);
        writeln;
      end;
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress2,ConfigBase) then
      begin
        ConfigBase := ConfigBase and $FFFE;
        write('PLX 9030 LOCAL 16 bit IO base address is: ');
        HexPrint(ConFigBase,4);
        writeln;
      end;
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress3,ConfigBase) then
      begin
        ConfigBase := ConfigBase and $FFFE;
        write('PLX 9030 LOCAL 32 bit IO base address is: ');
        HexPrint(ConFigBase,4);
        writeln;
        BasePort := ConfigBase;
      end;
    end
    else
    begin
      writeln('Didn''t find PLX9030 at card number ',CardNum);
      halt(2);
    end;
  end
  else
  begin
    writeln('No PCI? How can this be?');
    halt(2);
  end;
  PortA := BasePort+PortAOff;
  PortB := BasePort+PortBOff;
  PortC := BasePort+PortCOff;
  TestPort := BasePort+TestPortOff;
end;

procedure SetPPMode(theport: word; themode : longint);
var
conport : word;
begin
  conport := theport + DDRPortOff;
  WritePort32(conport,themode);
end;

procedure OutOneZero(theport : word; thebit : word);
var
dword : longint;
begin
  { set one bit in the 24 bit output field of 5I20 low }
  WritePort32(baseport,$FFFFFF);
  { first set all bits high }
  dword := 1;
  dword := dword shl (thebit);
  dword := dword xor $FFFFFF;
  WritePort32(theport,dword);
end;

procedure OutOneOne(theport : word; thebit : word);
var
dword : longint;
begin
  { set one bit in the 24 bit output field of 5I20 low }
  WritePort32(baseport,$000000);
  { first set all bits low }
  dword := 1;
  dword := dword shl (thebit);
  WritePort32(theport,dword);
end;

function CheckOneZero(theport : word; thebit : byte): boolean;
var
dword : longint;
pword : longint;
portok : boolean;
begin                  { check for one low bit only in the 24 bit output field of our 5I20 }
  portok := true;
  dword := 1;
  dword := dword shl (thebit);
  dword := dword xor $FFFFFF;
  pword := ReadPort32(theport);
  pword := pword and $FFFFFF;
    if pword <> dword then
    begin
      portok := false;
      write('Error during rotating zero test, expected ');
      HexPrint(dword,8);
      write(' but found ');
      HexPrint(pword,8);
      write(' at port ');
      HexPrint(theport,4);
      writeln;
    end;
  CheckOneZero := portok;
end;

function CheckOneOne(theport : word; thebit : byte): boolean;
var
dword : longint;
pword : longint;
portok : boolean;
begin                  { check for one low bit only in the 24 bit output field of our 5I20 }
  portok := true;
  dword := 1;
  dword := dword shl (thebit);
  pword := ReadPort32(theport);
  pword := pword and $FFFFFF;
    if pword <> dword then
    begin
      portok := false;
      write('Error during rotating one test, expected ');
      HexPrint(dword,8);
      write(' but found ');
      HexPrint(pword,8);
      write(' at port ');
      HexPrint(theport,4);
      writeln;
    end;
  CheckOneOne := portok;
end;

procedure BumOut(s: string);
begin
  BadBeeps;
  writeln(s);
  halt(2);
end;

procedure DoRotBitTest(outport,inport : word);
var
testbit : word;
times : word;

begin

  write('Rotating bit test, output port = ');
  HexPrint(outport,4);
  write(' and input port = ');
  HexPrint(inport,4);
  writeln;
  SetPPMode(inport,AllIn);
  SetPPMode(outport,AllOut);
  for times := 1 to 100 do
  begin
    for testbit := 0 to 23 do
    begin
      OutOneOne(outport,testbit);
      if not CheckOneOne(inport,testbit) then BumOut('5I20 LOOPBACK FAILURE!');
      OutOneZero(outport,testbit);
      if not CheckOneZero(inport,testbit) then BumOut('5I20 LOOPBACK FAILURE!');
    end;
  end;
  { avoid conflicts..}
  SetPPMode(inport,AllIn);
  SetPPMode(outport,AllIn);

end;

procedure CheckTestPort;
begin
  WritePort32(TestPort,$5555AAAA);
  if ReadPort32(TestPort) <> $5555AAAA then Bumout('TestPort readback failed!');
  WritePort32(TestPort,$AAAA5555);
  if ReadPort32(TestPort) <> $AAAA5555 then Bumout('TestPort readback failed!');
  WritePort32(TestPort,$00000000);
  if ReadPort32(TestPort) <> $00000000 then Bumout('TestPort readback failed!');
  WritePort32(TestPort,$FFFFFFFF);
  if ReadPort32(TestPort) <> $FFFFFFFF then Bumout('TestPort readback failed!');
  WritePort32(TestPort,$12345678);
  if ReadPort32(TestPort) <> $12345678 then Bumout('TestPort readback failed!');
  writeln('32 bit TestPort readback OK');
end;

procedure TestPorts;
begin
  {set all ports to input mode in case they were not}
  SetPPMode(PortA,AllIn);
  SetPPMode(PortB,AllIn);
  SetPPMode(PortB,AllIn);
  CheckTestPort;
  DoRotBitTest(PortA,PortB);
  DoRotBitTest(PortA,PortC);
  DoRotBitTest(PortB,PortA);
  DoRotBitTest(PortB,PortC);
  DoRotBitTest(PortC,PortA);
  DoRotBitTest(PortC,PortB);
end;

procedure PrintUsage;
begin
  writeln;
  writeln('5I20LOOP -- Test 5I20 type parallel ports via loopback cable');
  writeln;
  writeln('USAGE -- 5I20LOOP [CardNum]');
  writeln;
  writeln('Requires 5I20 with IOPR24 config and all three connectors');
  writeln('looped back with 50 conductor flat cable');
  writeln;
  writeln('CAUTION: make sure all VIO jumpers are set for 3.3V or 5V');
  writeln('or you may damage the 5I20 card');
end;

begin
  PrintUsage;
  GetCardNum;
  SetUpPorts;
  TestPorts;
  HappyBeeps;
  Writeln('5I20 PORT TESTS OK!');
end.
