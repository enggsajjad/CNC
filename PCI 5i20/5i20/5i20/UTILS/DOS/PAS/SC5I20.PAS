program sendconf;

uses dos;

{$I pci}
{$I hprint}
{$I hexread}
{$I 9030}


type configfile = file;

const

BufferSize = 8192;
BaseAddressMask = $FFFC;


NCSBit           = 1;
NProgramBit      = 2;
NWriteBit        = 4;
LedBit           = 8;

var

Cardnum : word;
FileName : string;
CFile : configfile;
ConfigBase : word;
MConfigBase : longint;
IntReg : word;
ConfigPort16,ConfigPort32 : word;
StatusPortHigh : word;
StatusPortLow : word;
NeedsBitRev : boolean;

BBuf: array[1..BufferSize] of byte;

function OpenForRead(var fp: configfile; name: string): boolean;
begin
  Assign(fp,Name);
  {$I-}
  Reset(fp);
  {$I+}
  OpenForRead := IOResult = 0;
end { Open };

procedure Usage;
begin
  writeln;
  writeln(' SC5I20 -- Writes config data to 5I20');
  writeln;
  writeln(' USAGE - SENDCONF file [card#] ');
  writeln(' Where file is a FPGA configuration file');
  writeln(' and card# is optional card #');
  writeln;
  halt(2);
end;

function Reverse(inbyte : byte) : byte;
const
revtable : array[0..$FF] of byte =

         ($00,$80,$40,$C0,$20,$A0,$60,$E0,$10,$90,$50,$D0,$30,$B0,$70,$F0,
          $08,$88,$48,$C8,$28,$A8,$68,$E8,$18,$98,$58,$D8,$38,$B8,$78,$F8,
          $04,$84,$44,$C4,$24,$A4,$64,$E4,$14,$94,$54,$D4,$34,$B4,$74,$F4,
          $0C,$8C,$4C,$CC,$2C,$AC,$6C,$EC,$1C,$9C,$5C,$DC,$3C,$BC,$7C,$FC,
          $02,$82,$42,$C2,$22,$A2,$62,$E2,$12,$92,$52,$D2,$32,$B2,$72,$F2,
          $0A,$8A,$4A,$CA,$2A,$AA,$6A,$EA,$1A,$9A,$5A,$DA,$3A,$BA,$7A,$FA,
          $06,$86,$46,$C6,$26,$A6,$66,$E6,$16,$96,$56,$D6,$36,$B6,$76,$F6,
          $0E,$8E,$4E,$CE,$2E,$AE,$6E,$EE,$1E,$9E,$5E,$DE,$3E,$BE,$7E,$FE,
          $01,$81,$41,$C1,$21,$A1,$61,$E1,$11,$91,$51,$D1,$31,$B1,$71,$F1,
          $09,$89,$49,$C9,$29,$A9,$69,$E9,$19,$99,$59,$D9,$39,$B9,$79,$F9,
          $05,$85,$45,$C5,$25,$A5,$65,$E5,$15,$95,$55,$D5,$35,$B5,$75,$F5,
          $0D,$8D,$4D,$CD,$2D,$AD,$6D,$ED,$1D,$9D,$5D,$DD,$3D,$BD,$7D,$FD,
          $03,$83,$43,$C3,$23,$A3,$63,$E3,$13,$93,$53,$D3,$33,$B3,$73,$F3,
          $0B,$8B,$4B,$CB,$2B,$AB,$6B,$EB,$1B,$9B,$5B,$DB,$3B,$BB,$7B,$FB,
          $07,$87,$47,$C7,$27,$A7,$67,$E7,$17,$97,$57,$D7,$37,$B7,$77,$F7,
          $0F,$8F,$4F,$CF,$2F,$AF,$6F,$EF,$1F,$9F,$5F,$DF,$3F,$BF,$7F,$FF);
begin
  Reverse := revtable[inbyte];
end;

function ReadString(var idx: word) : string;
var
sleng,count,index : word;
tstring : string;
begin
  index := idx;
  sleng := BBuf[index] * 256 + BBuf[index+1];
  index := index +2; {skip over string length}
  if sleng >255 then sleng := 255;
  tstring := '';
  for count := 1 to sleng do
  begin
    if BBuf[index] <> 0 then tstring := tstring + chr(BBuf[index]);
    index := index +1;
  end;
  idx := index;
  ReadString := tstring;
end;

function PrintBitFileHeader : word;
var
index,sleng : word;
tstring : string;
conflength : longint;
begin
  NeedsBitRev := false;
  index := 1;
  if (BBuf[index] = 0) and (BBuf[index+1] = 9) then
  begin
    writeln('Looks like a BIT file');
    index := index+2 +9; {skip over 00 09 and 9 header bytes}
    if (BBuf[index] = 0) and (BBuf[index+1] = 1) and (BBuf[index+2] = $61) then
    begin
      index := index + 3; { skip over 00 01 61 = 'a'}
      writeln('Design name: ',ReadString(index));
      index := index +1; {Skip over 'b'}
      writeln('Part name: ',ReadString(index));
      index := index +1; {Skip over 'c'}
      writeln('Design date: ',ReadString(index));
      index := index +1; {Skip over 'd'}
      writeln('Design time: ',ReadString(index));
      index := index +1; {Skip over 'e'}
      conflength := longint(BBuf[index])*16777216;
      conflength := conflength +longint(BBuf[index+1])*65536;
      conflength := conflength +longint(BBuf[index+2])*256;
      conflength := conflength +longint(BBuf[index+3]);
      writeln('Config Length: ',conflength);
      NeedsBitRev := true;
    end {valid bitfile header}
    else
    begin
      writeln('Invalid bitfile header!');
      halt(2);
    end;
  end {looks like a bit file }
  else
  begin
    if (BBuf[index] = $FF) and (BBuf[index+1] = $FF) and (BBuf[index+2] = $FF) and (BBuf[index+3] = $FF)  then
    begin
      if (BBuf[index+4] = $55) and (BBuf[index+5] = $99) and (BBuf[index+6] = $AA) and (BBuf[index+7] = $66)  then
      begin
        writeln('Looks like a PROM file');
      end
      else
      begin
        writeln('Invalid PROM file: No sync');
        halt(2);
      end;
    end
    else
    begin
      writeln('Invalid PROM file: no FF FF FF FF');
      halt(2);
    end;
  end;
  PrintBitFileHeader := index;
end;

procedure GetParms;
var
theport : word;
thefile : string;
retcode : integer;
  begin
  if ParamCount < 1 then Usage;
  begin
    thefile := paramStr(1);
  end;
  FileName := thefile;
  if not OpenForRead(CFile,FileName) then
  begin
    writeln('Can''t open configuration file');
    halt(2);
  end;
  CardNum := 0;
  if ParamCount > 1 then
  begin
    val(ParamStr(2),CardNum,retcode);
    if (retcode <> 0) or (CardNum <0) or (CardNum >15) then
    begin
      writeln;
      writeln('Invalid 5I20 card #');
      halt(2);
    end;
  end;
end;

procedure SetupPorts;
var
busnum,devnum: byte;
tword1,tword2 : word;
begin
  if PCIBIOSPresent then
  begin
    writeln('PCI BIOS is present!');
    if FindPCIDevice(N030DeviceID,N030VendorID,CardNum,busnum,devnum) then
    begin
      writeln('found 9030 at bus: ',busnum,' device: ',devnum);
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress1,ConfigBase) then
      begin
        ConfigBase := ConfigBase and BaseAddressMask;
        write('PLX 9030 Configuration IO base address is: ');
        HexPrint(ConFigBase,4);
        writeln;
        StatusPortLow := ConfigBase + N030GPIOControlReg;
        StatusPortHigh := ConfigBase + N030GPIOControlRegTop;
      end;
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress2,ConfigBase) then
      begin
        ConfigBase := ConfigBase and BaseAddressMask;
        write('PLX 9030 LOCAL 16 bit IO base address is: ');
        HexPrint(ConFigBase,4);
        writeln;
        ConfigPort16 := ConfigBase;
      end;
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress3,ConfigBase) then
      begin
        ConfigBase := ConfigBase and BaseAddressMask;
        write('PLX 9030 LOCAL 32 bit IO base address is: ');
        HexPrint(ConFigBase,4);
        writeln;
        ConfigPort32 := ConfigBase;
      end;
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress4,tword1) then
      begin
        tword1 := tword1 and BaseAddressMask;
        if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress4+2,tword2) then
        begin
          write('PLX 9030 LOCAL 16 bit Memory base address is: ');
          HexPrint(tword2,4);
          HexPrint(tword1,4);
          writeln;
        end;
      end;
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress5,tword1) then
      begin
        tword1 := tword1 and BaseAddressMask;
        if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress5+2,tword2) then
        begin
          write('PLX 9030 LOCAL 32 bit Memory base address is: ');
          HexPrint(tword2,4);
          HexPrint(tword1,4);
          writeln;
        end;
      end;
      if ReadPCIConfigWord(busnum,devnum,N030PCIIntRegister,IntReg) then
      begin
        IntReg := IntReg and $FF;
        write('PLX 9030 Interrupt is: ');
        writeln(IntReg);
      end;
    end
    else
    begin
      writeln('Didn''t find PLX9030 at card number ',cardnum);
      halt(2);
    end;
  end
  else
  begin
    writeln('No PCI? How can this be?');
    halt(2);
  end;
end;

procedure SetProgramHigh;
var temp : word;
begin
  temp := portw[StatusPortHigh];
  portw[StatusPortHigh] := temp or ProgramMask;
end;

procedure SetProgramLow;
var temp : word;
begin
  temp := portw[StatusPortHigh];
  portw[StatusPortHigh] := temp and (not ProgramMask);
end;

procedure SetWriteHigh;
var temp : word;
begin
  temp := portw[StatusPortHigh];
  portw[StatusPortHigh] := temp or WriteMask;
end;

procedure SetWriteLow;
var temp : word;
begin
  temp := portw[StatusPortHigh];
  portw[StatusPortHigh] := temp and (not WriteMask);
end;

procedure LEDOff;
var temp : word;
begin
  temp := portw[StatusPortHigh];
  portw[StatusPortHigh] := temp or LEDMask;
end;

procedure LEDOn;
var temp : word;
begin
  temp := portw[StatusPortHigh];
  portw[StatusPortHigh] := temp and (not LEDMask);
end;

function DoneHigh : boolean;
begin
  if (portw[StatusPortLow] and DoneMask) <> 0 then DoneHigh := true else DoneHigh := false;
end;

procedure StartProgramming;
begin
  SetProgramLow;
  LedOn;
  SetWriteHigh;
  { Turn on LED, set program low, no CS or write }

  if DoneHigh then
  begin
    writeln('Problem: done not low at beginning...');
    halt(2);
  end;
  SetWriteLow;
  SetProgramHigh;
  LEDOn;
  { Leave led on, set program high, enable write and CS }
end;

procedure DoneProgramming;
var
cclks : byte;
wait: longint;
dummy : byte;
begin
  SetWriteHigh;
  { Leave LED on, no program, CS or write }
  if not DoneHigh then
  begin
    writeln('Problem: done not high at end...');
    halt(2);
  end;
  LEDOff;
  { Turn LED off, no program, CS or write }
  for cclks := 1 to 24 do
  begin
    portw[ConfigPort16] := $FFFF;
  end;
end;


procedure DoIt;
var
cbyte : byte;
bytesread,index,start : word;
bytecount : longint;
firstblock : boolean;
begin
  StartProgramming;
  writeln;
  if OpenForRead(CFile,FileName) then
  begin
    reset(CFile,1);
    firstblock := true;
    while not EOF(CFile) do
    begin
      bytesread := 0;
      blockread(CFile,BBuf,BufferSize,bytesread);
      if firstblock then start := PrintBitFileHeader else start := 1;
      firstblock := false;
      write('* ');
      if bytesread > 0 then
      begin
        if NeedsBitRev then
        begin
          for index := start to bytesread do
          begin
            port[ConfigPort16] := Reverse(BBuf[index]);
          end;
        end
        else
        begin
          for index := start to bytesread do
          begin
            port[ConfigPort16] := BBuf[index];
          end;
        end;
      end;
    end;
  end
  else
  begin
    writeln('Can''t open configuration file');
    halt(2);
  end;
  writeln;
  DoneProgramming;
end;

begin
  GetParms;
  SetUpPorts;
  DoIt;
  writeln('Configuration complete!');
  halt(0);
end.
