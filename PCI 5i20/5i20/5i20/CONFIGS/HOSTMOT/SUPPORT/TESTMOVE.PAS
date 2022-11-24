program TESTMove;  { for 4I65/5I20}
uses DOS,crt;


{$R-}    {Range checking off }
{$I-}    {No I/O checking }
{$S-}    {No stack checking}

{$M 8192,0,32768}              { use minimum memory, but a large stack...      }
                               { don't make the heap any smaller or you'll be in heap big trouble }

{$I 8259.pas}

{$I HEXREAD.PAS}
{$I HPRINT.PAS}
{$I BEEPY.PAS}
{$I pci}
{$I 9030}
{$I general }

type


LongIntIntRec = record
  LowWord  : word;
  HighWord : Integer;
end;

Motorx = record
      CurPos : longint;
      DesPos : longint;
      NewPos : longint;
      FinalPos : longint;
      DesFrac : word;
      LastPos : longint;
      PosErr : longint;
      MaxPosErr : longint;
      MinPosErr : longint;
      PosErrForStop : longint;
      IntegralErr : longint;
      Accel : longint;
      MaxVel : longint;
      SignedMaxVel : longint;
      Velocity : longint;
      DesVel : longint;
      ActVel : Integer;
      HalfPos : longint;
      KP : longint;
      KD : longint;
      KI : longint;
      IL : longint;
      KF : longint;
      DeadZone : longint;
      RunProfile : boolean;
      PIDOn : boolean;
      EXPE : boolean;
      Drive : longint;
  end;

MotorP = ^MotorX;

const
LastMotor = 7;
LastActiveMotor = 3;
{$I hostmlow.pas}
{$I parsep.pas}
{$I motrec.pas}

var

ConFigFileName : string;
ConFigFile : text;
OldVector : pointer;
Foo : string;
TheInterrupt : byte;
Motors : array [0..lastmotor] of Motorx;
CardNum : word;
ConfigBase : word;
IOBase16 : word;
IOBase32 : word;
IRQDiv : byte;
FooKey : word;
CurChan,CurMot : byte;
{ interrupt variables }
dummy : byte;
count : longint;
longdrive : longint;
tdespos,tdesvel : longint;
signfix,axt,bxt,cxt,tdesfrac : word;
SSSave : word;
SPSave : word;
NewSS : word;
NewSP : word;
foostr : string;
DemoVel : longint;
DemoAcc : longint;
EncCounts : longint;

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

procedure BumOut(s: string);
begin
  writeln;
  writeln(chr(7),s);
  halt(2);
end;

function OpenForRead(var fp: text; name: string): boolean;
begin
  Assign(fp,Name);
  {$I-}
  Reset(fp);
  {$I+}
  OpenForRead := IOResult = 0;
end { Open };

procedure FBumout(ftoken : string);
var
funcidx : byte;
begin
  writeln;
  writeln(ftoken,' is not a valid parameter');
  writeln(chr(7));
  writeln('Valid parameters are:');
  writeln;
  for funcidx := 1 to MaxTokens do
  begin
    writeln('        ',TokenPile[funcidx].Token);
  end;
  halt(2);
end;

procedure VBumout(ftoken,vtoken : string);
var
valuidx,funcidx : byte;
begin
  for funcidx := 1 to MaxTokens do
  begin
    if UpString(ftoken) = UpString(TokenPile[funcidx].Token) then
    begin
      writeln;
      writeln(vtoken,' is not a valid value for ',TokenPile[funcidx].Token);
      writeln(chr(7));
    end;
  end;
  halt(2);
end;

procedure AssignVars(ftoken : string;val : longint);
var ft : string;
begin
  ft := UpString(ftoken);
  if ft = 'MOTOR' then CurMot := val;
  if ft = 'KP' then Motors[CurMot].KP := word(val);
  if ft = 'KI' then Motors[CurMot].KI := val;
  if ft = 'KD' then Motors[CurMot].KD := word(val);
  if ft = 'IL' then LongIntRec(Motors[CurMot].IL).HiWord := val;
  if ft = 'KF' then Motors[CurMot].KF := word(val);
  if ft = 'POSERRORFORSTOP' then Motors[CurMot].PosErrForStop := val;
  if ft = 'SLEWVEL' then DemoVel:= val;
  if ft = 'ACC' then DemoAcc := word(val);
end;

procedure ParseTokens(ftoken,vtoken : string);
var
funcidx,valuidx : byte;
tokenfound,valufound : boolean;
value : longint;
retcode: integer;
begin
  Tokenfound := false;
  for funcidx := 1 to MaxTokens do
  begin
    if UpString(ftoken) = UpString(TokenPile[funcidx].Token) then
    begin
      tokenfound := true;
      with TokenPile[funcidx] do
      begin
        valufound:= false;
        if length(vtoken) > 0 then
        begin
          val(vtoken,value,retcode);
          if retcode = 0 then
          begin
            if (value <= Max) and (value >= Min) then
            begin
              valufound := true;
              AssignVars(ftoken,value);
            end;
          end;
        end;
      end;
    end;
  end;
  if not tokenfound then FBumout(ftoken);
  if not valufound then VBumout(ftoken,vtoken);
end;

procedure ReadCFile;
var
cstring,ftoken,vtoken : string;
begin
  ConfigFileName := 'MOTORS.TXT';
  if OpenForRead(ConfigFile,ConfigFileName) then
  begin
    writeln('Reading ',ConfigFileName);
    while not EOF(ConfigFile) do
    begin
      readln(ConfigFile,cstring);
      if length(cstring) > 0 then
      begin
        StripLeadingBlanks(cstring);
        StripTrailingBlanks(cstring);
        vtoken :='';
        ftoken :='';
        if length(cstring) > 0 then
        begin
          ftoken := strip(cstring);
          if ftoken <>';' then
          begin
            vtoken := strip(cstring);
            ParseTokens(ftoken,vtoken);
          end;
        end;
      end;
    end;
  end
  else Bumout('Can''t open '+ConfigFileName);
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
        IOBase16 := ConfigBase and $FFFE;
        write('PLX 9030 LOCAL 16 bit IO base address is: ');
        HexPrint(IOBase16,4);
        writeln;
      end;
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress3,ConfigBase) then
      begin
        IOBase32 := ConfigBase and $FFFE;
        write('PLX 9030 LOCAL 32 bit IO base address is: ');
        HexPrint(IOBase32,4);
        writeln;
      end;
      if ReadPCIConfigWord(busnum,devnum,N030PCIIntRegister,ConfigBase) then
      begin
        TheInterrupt := ConfigBase and $000F;
        writeln('PCI Interrupt is: ',TheInterrupt);
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
end;

procedure startup;
begin
  for CurChan := 0 to LastMotor do
  begin
    with Motors[CurChan] do
    begin
      DesPos := 0;
      CurPos := 0;
      Velocity := 0;
      Accel := 0;
      MaxVel := 0;
      IntegralErr := 0;
      RunProfile := false;
      DeadZone := 1;
      PIDOn := false;
      EXPE := false;
      { do per channel setup stuff}
      portw[IOBase16 + CCRegBase + CurChan*2] := CCRCountEnable+ CCRIndex; { local Count enable + clear counter }
      portw[IOBase16 + PCRegBase + CurChan*2] := $01; { local PWM enable}
    end;
  end;
  ReadCFile;
  portw[IOBase16 + GModeReg] := GMLOI + GMPWMEN + GMCTREN; { global PWM enable and Count enable and latch on IRQ }
  portw[IOBase16 + LEDPtrReg] := $01; { set LED reg to point to channel 1 }
  portw[IOBase16 + RateReg] := 40265; {for 20KHz PWM}
  IrqDiv := 1; { divides PWM by (IRQDiv+1) for 10KHz sample rate }
  portw[IOBase16 + IRQDivReg] := IRQDiv; {for 10 KHz sample rate}
  NewSS := SSeg;
  NewSP := SPtr - 4096;               { we will use the current stack later... }
end;

procedure PID;
begin
  { do all motors }
  for CurChan := LastActiveMotor downto 0 do
  begin
    with Motors[CurChan] do
    begin
      CurPos := ReadPort32(IOBase32 + CounterBase + CurChan*4); {read current position}
      PosErr := CurPos-DesPos;
      If PosErr > MaxPosErr then MaxPosErr := PosErr;
      If PosErr < MinPosErr then MinPosErr := PosErr;
      ActVel := LastPos-CurPos;
      LastPos := CurPos;
      if abs(PosErr) > PosErrForStop then
      begin
        PIDOn := false;
        EXPE := true;
      end;
      if PIDOn then
      begin
        IntegralErr := IntegralErr + PosErr*KI;
        if IntegralErr >  IL then IntegralErr :=  IL;             { bound IntegralErr by IL }
        if IntegralErr < -IL then IntegralErr := -IL;
        { this is the actual PID loop calculation: }
        longdrive := KP*PosErr                                    { Proportional term }
                   + LongIntIntRec(IntegralErr).HighWord          { Integral term }
                   - KD*(ActVel{ - (DesVel div 65536)})             { Derivative term }
                   - KF*(DesVel div 256);           { Velocity feed forward term }
        if longdrive >  32767 then longdrive :=  32767;
        if longdrive < -32768 then longdrive := -32768;
        {if (Velocity = 0) and (abs(longdrive) < DeadZone) then Longdrive := 0;}
        portw[IOBase16+ PWMBase +CurChan*2] := word(longdrive);
      end
      else portw[IOBase16 + PWMBase +CurChan*2] := 0;
      Drive := Longdrive;
      if RunProfile = true then
      begin
        if DesPos < HalfPos then
          Velocity := Velocity + Accel
        else
          Velocity := Velocity - Accel;
        if Velocity = 0 then
        begin
          FinalPos := DesPos;
          RunProfile := false;
          Accel := 0;
          DesPos := NewPos;
          Velocity := 0;
          DesVel := 0;
        end;
        if abs(Velocity) <= MaxVel then DesVel := Velocity else DesVel := SignedMaxVel;
        if DesVel < 0 then signfix := $FFFF else signfix := $0000;
        tdespos := DesPos;
        tdesfrac := DesFrac;
        tdesvel := DesVel;
        asm
          { get three words}
          mov ax,tdesfrac
          mov bx,LongIntRec(tdespos).LoWord
          mov cx,LongIntRec(tdespos).HiWord
          { do the add }
          add ax,LongIntRec(tdesvel).LoWord
          adc bx,LongIntRec(tdesvel).HiWord
          { last add fixes sign }
          adc cx,signfix
          { restore data (why the casts above dont work here I do not know...}
          mov axt,ax
          mov bxt,bx
          mov cxt,cx
        end;
        { restore values in motor record }
        { doesnt work in asm for some reason }
        DesFrac := axt;
        LongIntRec(DesPos).LoWord := bxt;
        LongIntRec(DesPos).HiWord := cxt;
      end;
   end;
   portw[IOBase16 + GContReg] := GCClrIRQ; { clear interrupt }
  end;
  IssueEOI2;
end;

procedure ISR;
interrupt;
begin
  PID;
end; { InterruptServiceRoutine }

procedure NEWISR;
interrupt;
{ this is the actual hardware interrupt routine }
{ all it does is create a local stack for us to so we  }
{ do not have to depend on the kindness of strangers...}

begin
  asm
    { save old SS and SP }
    MOV SSSave,SS
    MOV SPSave,SP
    MOV SS,NewSS
    MOV SP,NewSP
  end;
  PID;
  asm
    { restore old SS and SP (we hope) }
    MOV SS,SSSave
    MOV SP,SPSave
  end;
end; { InterruptServiceRoutine }

procedure Move(chan : byte;nextpos : longint; acc : longint; maxv : longint);
begin
  writeln('Chan ',chan,' Move to ',nextpos);
  with Motors[chan] do
  begin
    DisableInterrupts;
    MaxVel := maxv;
    if (nextpos-DesPos) < 0 then
    begin
      SignedMaxVel := -maxv;
    end
    else
    begin
      SignedMaxVel := maxv;
    end;
    HalfPos := (DesPos + nextpos) div 2;
    NewPos := NextPos;
    LastPos := CurPos;
    Accel := acc;
    Velocity := 0;
    DesVel := 0;
    DesFrac := 0;
{    IntegralErr := 0;}
    MaxPosErr := 0;
    MinPosErr := 0;
    RunProfile := true;
    EXPE := false;
    EnableInterrupts;
  end;
end;

function ProfileRunning(theMotor : byte) : boolean;
begin
  ProfileRunning := Motors[TheMotor].RunProfile;
end;

function MotorEXPE(theMotor : byte) : boolean;
begin
  MotorEXPE := Motors[TheMotor].EXPE;
end;

procedure PIDOn(theMotor : byte);
begin
  Motors[TheMotor].PIDOn := true;
end;

procedure StartInterrupts;
begin
  DisableInterrupts;
  GetHardIntVec(TheInterrupt,OldVector);
  SetHardIntVec(TheInterrupt,@NEWISR);
  portw[IOBase16 + GModeReg] := GMLOI + GMPWMEN + GMCTREN +GMIRQMask; { global PWM enable and Count enable and latch on IRQ }
  UnmaskInterrupt(TheInterrupt);
  EnableInterrupts;
  writeln('Control loop running');
end;

procedure StopInterrupts;
begin
  DisableInterrupts;
  portw[IOBase16 + GModeReg] := $00; {turn off PWMs, disable IRQs}
  SetHardIntVec(TheInterrupt,OldVector);
  UnmaskInterrupt(TheInterrupt);
  EnableInterrupts;
  writeln('Control loop shut down');
end;

procedure shutdown;
begin
  StopInterrupts;
  halt(0);
end;

procedure WaitForKey;
begin
  while not keypressed do;
  FooKey := ReadKey;
  if upcase(chr(FooKey)) = 'Q' then Shutdown;
end;

procedure WaitForMotors;
begin
  while (ProfileRunning(0) or ProfileRunning(1)) do
  begin
    with Motors[0] do
    begin
      writeln('PE= ',PosErr,
      ' AV= ',ActVel,
      ' DV= ',DesVel,
      ' DR= ',Drive,
      ' DP= ',DesPos);
      if abs(PosErr) >30 then WaitForKey;
    end;
    if MotorEXPE(0) or MotorEXPE(1) then
    begin
      writeln('Shut down due to excessive position error');
      StopInterrupts;
      halt(2);
    end;
  end;
  writeln('Got to (0,1) ',Motors[0].DesPos,',',Motors[1].DesPos);
  writeln('DDA Got to (0,1) ',Motors[0].FinalPos,',',Motors[1].FinalPos);
  writeln('MaxPosErr = ',Motors[0].MaxPosErr,' MinPosErr = ',Motors[0].MinPosErr);
end;



procedure WaitAndShow;
begin
  ClrScr;
  while not keypressed do
  begin
    for CurMot := 0 to LastActiveMotor do
    begin
      with Motors[CurMot] do
      begin
        writeln('Motor: ',CurMot,' CurPos= ',CurPos,' PosErr= ',PosErr,' Drive= ',Drive,'          ');
      end;
    end;
    GotoXY(1,1);
  end;
  writeln;
  fookey := ReadKey;
end;

begin
  GetCardNum;
  SetupPorts;
  Startup;
  StartInterrupts;
  for CurMot := 0 to LastActiveMotor do
  begin
    PIDOn(CurMot);
  end;
  writeln(' PID loop started; press a key to start moves');
  WaitForKey;
  while true do
  begin
    WaitAndShow;
    EncCounts := 500;
    writeln ('Move to 0');
    move(0,0,DemoAcc,DemoVel);
    move(1,0,DemoAcc,DemoVel);
    WaitForMotors;
    writeln('Move 6 turns + -- key to continue');
    WaitForKey;
    move(0, EncCounts*6,DemoAcc,DemoVel);
    move(1, EncCounts*6,DemoAcc,DemoVel);
    WaitForMotors;
    writeln('Move 6 turns + -- key to continue');
    WaitForKey;
    move(0, EncCounts*12,DemoAcc,DemoVel);
    move(1, EncCounts*12,DemoAcc,DemoVel);
    WaitForMotors;
    writeln('Move 6 turns - -- key to continue');
    WaitForKey;
    move(0,EncCounts*6,DemoAcc,DemoVel);
    move(1,EncCounts*6,DemoAcc,DemoVel);
    WaitForMotors;
    writeln('Move 6 turns - -- key to continue');
    WaitForKey;
    move(0,0,DemoAcc,DemoVel);
    move(1,0,DemoAcc,DemoVel);
    WaitForMotors;
  end;
end.
