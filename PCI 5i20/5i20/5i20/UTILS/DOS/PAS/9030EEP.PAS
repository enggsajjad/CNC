program N030EEP;

{$I pci}
{$I hprint}
{$I hexread}
{$I 9030}

var
BusNum,DevNum : byte;
Configbase : word;
EEPort : word;
TheEreg : byte;
TheEdata : word;
DoWrite,DoZero : Boolean;
Index : byte;

procedure Usage;
begin
  writeln('USAGE: 9030EEP REG [DATA]');
  writeln('Read or Writes 9030 PCI setup EEPROM');

  halt(2);
end;
procedure FooDelay;
var
 a,b : word;
begin
  for a := 1 to 500 do b := portw[EEPort];
  { theory here is that a PCI read must take an absolute minumum of 30 ns }
  { so slowest eeprom clock is 250 khz = 4 usec so we will wait about 4 usec }
  { every time we do anything just to make sure...}
end;

procedure GetParms;
var tempword : word;
begin
  DoZero := false;
  DoWrite := false;
  if ParamCount < 1 then Usage;
  if ParamStr(1) = 'z' then DoZero := true;
  if ParamStr(1) = 'Z' then DoZero := true;
  if DoZero = false then
  begin
    if not HexWordRead(ParamStr(1),tempword) then Usage;
    if ParamCount > 1 then
    begin
      if not HexWordRead(ParamStr(2),TheEdata) then Usage;
      DoWrite := true;
    end;
    if tempword > $1FE then Usage;
    TheEreg := byte(tempword div 2); { change to word address for EEPROM }
  end;

end;  { GetParameter }


procedure SetCSHigh;
var temp: word;
begin
  temp :=  portw[EEPort];
  temp := temp or N030EECSMask;
  portw[EEPort] := temp;
  FooDelay;
end;

procedure SetCSLow;
var temp : word;
begin
  temp :=  portw[EEPort];
  temp := temp and (not N030EECSMask);
  portw[EEPort] := temp;
  FooDelay;
end;

procedure SetDinHigh;
var temp : word;
begin
  temp :=  portw[EEPort];
  temp := temp or N030EEDIMask;
  portw[EEPort] := temp;
  FooDelay;
end;

procedure SetDinLow;
var temp : word;
begin
  temp :=  portw[EEPort];
  temp := temp and (not N030EEDIMask);
  portw[EEPort] := temp;
  FooDelay;
end;

procedure SetClockHigh;
var temp : word;
begin
  temp :=  portw[EEPort];
  temp := temp or N030EECLKMask;
  portw[EEPort] := temp;
  FooDelay;
end;

procedure SetClockLow;
var temp : word;
begin
  temp :=  portw[EEPort];
  temp := temp and (not N030EECLKMask);
  portw[EEPort] := temp;
  FooDelay;
end;

function DataHighQ : boolean;
begin
  FooDelay;
  if (portw[EEPORT] and N030EEDOMask) <> 0 then DataHighQ := True else DataHighQ:= false;
end;

{$I 9366LOW}

begin
  GetParms;
  if PCIBIOSPresent then
  begin
    writeln('PCI BIOS is present!');
    if FindPCIDevice(N030DeviceID,N030VendorID,0,busnum,devnum) then
    begin
      writeln('found 9030 at bus: ',busnum,' device: ',devnum);
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress1,ConFigBase) then
      begin
        ConfigBase := ConfigBase and $FFFE;
        write('PLX 9030 IO base address is: ');
        HexPrint(ConFigBase,4);
        writeln;
        EEPort := ConfigBase + N030InitControlRegTop;
        if DoZero then
        begin
          writeln('Zeroing entire EEPROM...');
          for Index := 0 to 255 do
          begin
            if not WriteSE(Index,0) then
            begin
              write('Zeroing failed at address: ');
              HexPrint(Index,2);
              halt(2);
            end;
          end;
          writeln('9030 EEPROM zeroed');
        end
        else
        begin
          if DoWrite then
          begin
            if WriteSE(TheEReg,TheEData) then
            begin
              writeln('Data written successfully');
            end
            else
            begin
              writeln('EEPROM data write failed!');
              halt(2);
            end;
          end
          else
          begin  { do EEPROM READ }
            write('EEPROM data at ');
            Hexprint(TheEREG*2,3);
            write(' is: ');
            if not ReadSE(TheEreg,TheEdata) then
            begin
              writeln('EEPROM read failed!');
            end
            else
            begin
              HexPrint(TheEdata,4);
              writeln;
            end;
          end;
        end;
      end
      else
      begin
        writeln('Read config data failed');
        halt(2)
      end;
    end
    else
    begin
      writeln('Didn''t find PLX9030');
      halt(2);
    end;
  end
  else
  begin
    writeln('No PCI? How can this be?');
    halt(2);
  end;
end.
