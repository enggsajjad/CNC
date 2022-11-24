{$N+}
{$Q-}
program HM2IDROM;
uses Dos;
{$I datarec }
{$I general }
{$I hread}
{$I HPrint}
{$I PCI }
{$I pcibrid}
{$L flat.obj}
{$I parsep }
{$I HM2ID}

var
BaseAddr : longint;
CardNumber : word;

procedure FLAT_install; external;
procedure FLAT_destall; external;

procedure Barfout(es:string);
begin
  writeln;
  writeln(es);
  halt(2);
end;

procedure PrintAsText(d: longint);
begin
  write(char(LongIntByteRec(d).Byte0));
  write(char(LongIntByteRec(d).Byte1));
  write(char(LongIntByteRec(d).Byte2));
  write(char(LongIntByteRec(d).Byte3));
end;

procedure PrintModuleName(gtag,pad:byte);
var
index : byte;
foundit : boolean;
begin
  foundit := false;
  for index := 1 to MaxTags do
  begin
    if FNames[index].FTag = gtag then
    begin
      write(PadWithSpaces(FNames[index].FName,pad));
      foundit := true;
    end;
  end;
  if foundit = false then write(PadWithSpaces('Unknown',pad));
end;

procedure PrintPinName(gtag,n,pad : byte);
var
index,num,chan : byte;
chans : string;
foundit : boolean;

begin
  foundit := false;
  { normal modules are like this }
  num := n and OutputMask;
  for index := 1 to MaxTags do
  begin
    if PNames[index].FTag = gtag then
    begin
      if gtag = SSerialTag then
      begin
        chan := n and $0F;
        str(chan,chans);
        if n and $F0 = $00 then write(PadWithSpaces(PNames[index].Names[1]+chans,pad));
        if n and $F0 = $80 then write(PadWithSpaces(PNames[index].Names[2]+chans,pad));
        if n and $F0 = $90 then write(PadWithSpaces(PNames[index].Names[3]+chans,pad));
        if n = $A1         then write(PadWithSpaces(PNames[index].Names[4],pad));
        foundit := true;
      end;
      if gtag = DAQFIFOTag then
      begin
        chan := n and $1F;
        str(chan,chans);
        if n and $E0 = $00 then write(PadWithSpaces(PNames[index].Names[1]+chans,pad));
        if n = $41 then write(PadWithSpaces(PNames[index].Names[2],pad));
        if n = $81 then write(PadWithSpaces(PNames[index].Names[3],pad));
        foundit := true;
      end;
      if gtag = TwiddlerTag then
      begin
        chan := n and $1F;
        str(chan,chans);
        if n and $C0 = $00 then write(PadWithSpaces(PNames[index].Names[1]+chans,pad));
        if n and $C0 = $C0 then write(PadWithSpaces(PNames[index].Names[2]+chans,pad));
        if n and $C0 = $80 then write(PadWithSpaces(PNames[index].Names[3]+chans,pad));
        foundit := true;
      end;
      if gtag = BinOscTag then
      begin
        chan := n and $1F;
        str(chan,chans);
        if n and $80 = $80 then write(PadWithSpaces(PNames[index].Names[1]+chans,pad));
        foundit := true;
      end;
      if foundit = false then
      begin
        write(PadWithSpaces(PNames[index].Names[num],pad));
        foundit := true;
      end;
    end;
  end;
  if foundit = false then write(PadWithSpaces('Unknown',pad));
end;

procedure PrintConnectorName(bn : longint;p:byte);
var index : byte;
begin
  for index := 1 to Boards do
  begin
    if ConnectorNames[index].BoardName = bn then
    begin
      write(ConnectorNames[index].ConName[p]);
    end;
  end;
end;

procedure ScanParms;
var
data,index : longint;
connector,pin : byte;
begin
  FLAT_install;
  ReadMem32(BaseAddr+HM2CookieOffset,data);
  if data <> HM2Cookie then BarfOut('No HM2 Hardware Found');
  ReadMem32(BaseAddr+HostMotNameLowOffset,data);
  write('Configuration Name: ');
  PrintAsText(data);
  ReadMem32(BaseAddr+HostMotNameHighOffset,data);
  PrintAsText(data);
  writeln;
  ReadMem32(BaseAddr+IDROMPointer,data);
  IDROMOffset := data;
  for index := 0 to IDROMSize-1 do
  begin
    ReadMem32(BaseAddr+IDROMOffset+index*4,data);
    IDROMAsArray[index] := data
  end;
  for index := 0 to IDROMHeaderSize-1 do
  begin
    IDROMHeaderAsArray[index] := IDROMAsArray[index];
  end;
  IDROMHeader := IDROMHeaderType(IDROMHeaderAsArray);
  with IDROMHeader do
  begin
    if (IDROMType <> IDROMStyle0) and (IDROMType <> IDROMStyle1) then Barfout('Wrong IDROM Format!');
    ModuleOffset := ModulePointer div 4;
    PinDescOffset := PinDescPointer div 4;
    writeln;
    writeln('General configuration information:');
    writeln;
    write('  BoardName : ');
    PrintAsText(BoardNameLow);
    PrintAsText(BoardNameHigh);
    writeln;
    writeln('  FPGA Size: ',FPGASize,' KGates');
    writeln('  FPGA Pins: ',FPGAPins);
    writeln('  Number of IO Ports: ',IOPorts);
    writeln('  Width of one I/O port: ',PortWidth);
    writeln('  Clock Low frequency: ',ClockLow/1e6:3:4,' MHz') ;
    writeln('  Clock High frequency: ',ClockHigh/1e6:3:4,' MHz') ;
    writeln('  IDROM Type: ',IDROMType) ;
  end;

  for index := 0 to MaxModules*3 -1 do
  begin
    ModulesAsArray[index] := IDROMAsArray[index+ModuleOffset];
  end;
  Modules := ModuleType(ModulesAsArray);
  index := 1;
  writeln;
  writeln('Modules in configuration: ');
  repeat
    with Modules[index] do
    begin
      writeln;
      write('  Module: ');
      PrintModuleName(GTag,0);
      writeln;
      write('  There are ',NumInstances,' of ');
      PrintModuleName(GTag,0);
      writeln(' in configuration');
      writeln('  Version: ',Version);
      writeln('  Registers: ',NumRegisters);
      write('  BaseAddress: ');
      HexPrint(BaseAddr,4);
      writeln;
      if Clock = ClockLowTag then
        writeln('  ClockFrequency: ',IDROMHeader.ClockLow/1e6:3:3,' MHz')
      else
        writeln('  ClockFrequency: ',IDROMHeader.ClockHigh/1e6:3:3,' MHz');
      index := index+1;

    end;
  until Modules[index].GTag = 0;
  for index := 0 to MaxPins-1 do
  begin
    PinDescsAsArray[index] := IDROMAsArray[index+PinDescOffset];
  end;
  PinDescs := PinDescType(PinDescsAsArray);
  writeln;
  writeln('Configuration pin-out: ');
  for index := 1 to IDROMHeader.IOWidth do
  begin
    with PinDescs[index] do
    begin
      if ((index-1) mod IDROMHeader.PortWidth) = 0 then
      begin
        writeln;
        write('    IO Connections for ');
        PrintConnectorName(IDROMHeader.BoardNameHigh,((index -1) div IDROMHeader.PortWidth+1));
        writeln;
        writeln('    Pin#    I/O     Pri. func    Sec. func    Chan      Pin func        Pin Dir');
        writeln;
      end;
      write('    ',((index-1) mod IDROMHeader.PortWidth)*2 +1:2);
      write('      ',index-1:3,'     ');
      PrintModuleName(PTag,8);
      write('     ');
      if GTag <> $00 then
      begin
        PrintModuleName(Gtag,8);
        if (Chan and GlobalMarker) <> 0 then
        begin
          write('     Global    ');
        end
        else
        begin
          write('     ',Chan:2,'        ');
        end;
        PrintPinName(Gtag,PNumber,12);
        if PNumber and OutputMarker <> 0 then writeln('    (Out)') else writeln('    (In)');
      end
      else writeln('None');
    end;
  end;

  FLAT_destall;
end;
procedure GetParm;
var retcode : integer;
begin
  if ParamCount >1 then
  begin
    val(ParamStr(1),CardNumber,retcode);
    if retcode <> 0 then BarfOut('Invalid card #');
  end;
end;

begin
  GetParm;
  if not QuietSetupPCIM(CardNumber,BaseAddr) then Barfout('No PCI Bridge chip found');
  ScanParms;
end.
