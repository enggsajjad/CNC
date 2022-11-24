{ 93C66 (16 bit mode) serial eeprom access stuff }
const

NumRegs      = 256;
ComLength    = 3;
AddLength    = 8;
DataLength   = 16;

ReadCom      = 6;
WriteCom     = 5;
EraseCom     = 7;
WrEnDsCom    = 4;

ComMask      = $04;
AddMask      = $80;
DataMask     = $8000;


{
procedure SetCSHigh;
begin
end;

procedure SetCSLow;
begin
end;

procedure SetDinHigh;
begin
end;

procedure SetDinLow;
begin
end;

procedure SetClockHigh;
begin
end;

procedure SetClockLow;
begin
end;

function DataHighQ : boolean;
begin
end;
}

procedure LDisableInterrupts;
begin
  inline($FA);                                 { CLI }
end;

procedure LEnableInterrupts;
begin
  inline($FB);                                 { STI }
end;

function ReadSE(reg: byte; var data: word) : boolean;
var
bit : byte;
mask : word;
tdata : word;
begin
  ReadSE := false;
  SetCSLow;
  SetDinLow;
  SetClockLow;
  SetCSHigh;
  { send command first }
  mask := ComMask;
  for bit := 1 to ComLength do
  begin
    if (mask and ReadCom) = 0 then SetDinLow else SetDinHigh;
    mask := mask shr 1;
    SetClockLow;
    SetClockHigh;
  end;
  { then send address }
  mask := AddMask;
  for bit :=  1 to AddLength do
  begin
    if (mask and reg) = 0 then SetDinLow else SetDinHigh;
    mask := mask shr 1;
    SetClockLow;
    SetClockHigh;
  end;
  { read dummy 0 bit, if zero assume ok }
  if not DataHighQ then ReadSE := true;
  mask := DataMask;
  tdata := 0;
  for bit := 1 to DataLength do
  begin
    SetClockLow;
    SetClockHigh;
    if DataHighQ then tdata := tdata or mask;
    mask := mask shr 1;
  end;
  data := tdata;
  SetCSLow;
  SetDinLow;
  SetClockLow;
end;

procedure LLWriteSerialEEPROM(reg: byte; data: word);
var
bit : byte;
mask : word;
bailout : word;
begin
  SetCSLow;
  SetDinLow;
  SetClockHigh;
  SetCSHigh;
  { send command first }
  mask := ComMask;
  for bit := 1 to ComLength do
  begin
    if (mask and WriteCom) = 0 then SetDinLow else SetDinHigh;
    mask := mask shr 1;
    SetClockLow;
    SetClockHigh;
  end;
  { then send address }
  mask := AddMask;
  for bit :=  1 to AddLength do
  begin
    if (mask and reg) = 0 then SetDinLow else SetDinHigh;
    mask := mask shr 1;
    SetClockLow;
    SetClockHigh;
  end;
  mask := DataMask;
  bailout := 0;
  LDisableInterrupts;
  for bit := 1 to DataLength do
  begin
    if (mask and data) = 0 then SetDinLow else SetDinHigh;
    mask := mask shr 1;
    SetClockLow;
    SetClockHigh;
  end;
  SetCSlow;
  SetCSHigh;
  if not DataHighQ then  { if busy (or tristate) then wait a bit }
  begin
    LEnableInterrupts;
    while (bailout < 2000) and (not DataHighQ) do
    begin
      SetClockHigh;          { dont really need to do anything but this }
      bailout := bailout +1; { keeps my timing loop within bounds }
    end;
  end;
  SetCSLow;
  SetDinLow;
  SetClockLow;
end;

procedure LLEraseSerialEEPROM(reg: byte);
var
bit : byte;
mask : word;
bailout : word;
begin
  SetCSLow;
  SetDinLow;
  SetClockHigh;
  SetCSHigh;
  { send command first }
  mask := ComMask;
  for bit := 1 to ComLength do
  begin
    if (mask and EraseCom) = 0 then SetDinLow else SetDinHigh;
    mask := mask shr 1;
    SetClockLow;
    SetClockHigh;
  end;
  { then send address }
  bailout := 0;
  mask := AddMask;
  LDisableInterrupts;
  for bit :=  1 to AddLength do
  begin
    if (mask and reg) = 0 then SetDinLow else SetDinHigh;
    mask := mask shr 1;
    SetClockLow;
    SetClockHigh;
  end;
  SetCSlow;
  SetCSHigh;
  if not DataHighQ then  { if still busy then... }
  begin
     LEnableInterrupts;
    while (bailout < 2000) and (not DataHighQ) do
    begin
      SetClockHigh;            { dont really need to do anything but this }
      bailout := bailout +1;   { keeps my timing loop within bounds }
    end;
  end;
  SetCSLow;
  SetDinLow;
  SetClockLow;
end;

procedure LLWriteAccessSerialEEPROM(wen : boolean);
var
bit : byte;
mask : word;
bailout : word;
begin
  SetCSLow;
  SetDinLow;
  SetClockHigh;
  SetCSHigh;
  { send command first }
  mask := ComMask;
  for bit := 1 to ComLength do
  begin
    if (mask and WrEnDsCom) = 0 then SetDinLow else SetDinHigh;
    mask := mask shr 1;
    SetClockLow;
    SetClockHigh;
  end;
  { then send enable or disable bits }
  for bit :=  1 to AddLength do
  begin
    if wen then SetDinhigh else SetDinlow;
    SetClockLow;
    SetClockHigh;
  end;
  SetCSlow;
  SetDinLow;
  SetClockLow;
end;

function WriteSE(thereg : byte; thedata : word): boolean;
var
readdata : word;

begin
  WriteSE := false;
  LLWriteAccessSerialEEPROM(true);
  LLEraseSerialEEPROM(thereg);
{  WriteDataShadow(0); {this is one that matters!}
  LLWriteSerialEEPROM(thereg,thedata);
  if ReadSE(thereg,readdata) then
  begin
    if (readdata = thedata) then
    begin
      WriteSE := true;
    end
    else
    begin
      {writeln('EEPROM write failure...');
      write('sb: ');
      HexPrint(thedata,4);
      write(' but is: ');
      HexPrint(readdata,4);
      writeln;}
    end;
  end;
  LLWriteAccessSerialEEPROM(false);
end;
