{ PC interrupt controller low level stuff }
const

InterruptCont     = $20;                     { to issue EOI to and read IIR commands to}
InterruptMask     = $21;                     { to enable and disable our irq }

Interrupt2Cont    = $A0;                     { to issue EOI to and read IIR commands to}
Interrupt2Mask    = $A1;                     { to enable and disable our irq }

EOICommand          : byte = $20;              { EOI primes 8259 for next int }
SEOICommand         : byte = $60;              { Specific EIO }
RISRCommand         : byte = $0A;              { to read ISR }

type
PtrRec = record
           Ofs : word;
           Seg : word;
         end;

WordRec = record
            Low : byte;
            High : byte;
          end;

procedure DisableInterrupts;
begin
  inline($FA);                                 { CLI }
end;

procedure EnableInterrupts;
begin
  inline($FB);                                 { STI }
end;

function ATIntsQ : boolean;
var
masksave : byte;
xt : boolean;
begin
  DisableInterrupts;
  xt := false;
  masksave := port[Interrupt2Mask];
  port[Interrupt2Mask] := $55;
  if port[Interrupt2Mask] <> $55 then xt := true;
  port[Interrupt2Mask] := $AA;
  if port[Interrupt2Mask] <> $AA then xt := true;
  if xt then port[Interrupt2Mask] := $80     { enable NMI mask if was XT }
  else port[Interrupt2Mask] := masksave;     { otherwise restore mask reg }
  EnableInterrupts;
  AtIntsQ := not xt;
end;

procedure MaskALLInts;
begin
  port[InterruptMask] := $FF;
  port[Interrupt2Mask] := $FF;
end;

function GetIntMask : word;
var
temp : word;
begin
  { save the 2 mask values in upper and lower bytes of a word}
  WordRec(temp).Low := port[InterruptMask];
  WordRec(temp).High := port[Interrupt2Mask];
  GetIntMask := temp;
end;

procedure SetIntMask(mask : Word);
begin
  port[InterruptMask] := WordRec(mask).Low;
  port[Interrupt2Mask] := WordRec(mask).High;
end;

procedure MaskInterrupt( intnum : byte);
var
masksave : byte;
newmask : byte;
begin
  DisableInterrupts;
  if intnum <= 7 then
  begin
    masksave := port[InterruptMask];
    newmask  := 1 shl intnum;
    newmask  := newmask or masksave;
    port[InterruptMask] := newmask;
  end
  else
  begin
    masksave := port[Interrupt2Mask];
    newmask  := 1 shl (intnum-8);
    newmask  := newmask or masksave;
    port[Interrupt2Mask] := newmask;
  end;

  EnableInterrupts;
end; { MaskInterrupt }

function InterruptMaskedQ(intnum : byte) : boolean;
var
imaskr : byte;
imask : byte;
begin
  InterruptMaskedQ := false;
  DisableInterrupts;
  if intnum <= 7 then
  begin
    imaskr := port[InterruptMask];
    imask  := 1 shl intnum;
  end
  else
  begin
    imaskr := port[Interrupt2Mask];
    imask  := 1 shl (intnum-8);
  end;
  if (imask and imaskr) <> 0 then InterruptMaskedQ := true;
  EnableInterrupts;
end; { InterruptMaskedQ }

procedure UnMaskInterrupt(intnum : byte);
var
masksave : byte;
newmask : byte;
begin
  DisableInterrupts;
  if intnum <= 7 then
  begin
    masksave := port[InterruptMask];
    newmask  := 1 shl intnum;
    newmask  := $FF xor newmask;
    newmask  := newmask and masksave;
    port[InterruptMask] := newmask;
  end
  else
  begin
    { first enable IRQ 2 input on primary int controller just in case }
    { we don't do the opposite on MaskInterrupt(x) because we may break }
    { someone else's > 7 irq }

    masksave := port[InterruptMask];
    newmask  := 1 shl 2;
    newmask  := $FF xor newmask;
    newmask  := newmask and masksave;
    port[InterruptMask] := newmask;
    { then do the standard enable interrupt routine }
    masksave := port[Interrupt2Mask];
    newmask  := 1 shl (intnum-8);
    newmask  := $FF xor newmask;
    newmask  := newmask and masksave;
    port[Interrupt2Mask] := newmask;
  end;
  EnableInterrupts;
end; { UnMaskInterrupt }

procedure IssueEOI;
begin
  port[InterruptCont] := EOICommand;
end;

procedure IssueSpecificEOI(i : byte);
begin
  i := i and $07;
  port[InterruptCont] := (SEOICommand or i);
end;

procedure IssueEOI2;
begin
  port[Interrupt2Cont] := EOICommand;
  port[InterruptCont]  := EOICommand;
end;

function ReadIIR : byte;
begin
  DisableInterrupts;
  port[InterruptCont] := RISRCommand;
  ReadIIR := port[InterruptCont];
  EnableInterrupts;
end;

function ReadIIR2 : byte;
begin
  DisableInterrupts;
  port[Interrupt2Cont] := RISRCommand;
  ReadIIR2 := port[Interrupt2Cont];
  EnableInterrupts;
end;

function ReadIER : byte;
begin
  DisableInterrupts;
  ReadIER := port[InterruptMask];
  EnableInterrupts;
end;

function ReadIER2 : byte;
begin
  DisableInterrupts;
  ReadIER2 := port[Interrupt2Mask];
  EnableInterrupts;
end;

function IntNullptrQ(intnum : byte) : boolean;
var
intptr : pointer;
begin
  GetIntVec((intnum), intptr);
  if (PtrRec(intptr).Ofs = 0) and (PtrRec(intptr).Seg = 0) then IntNullPtrQ := true else IntNullPtrQ := false;
end ; { GetHardIntVec }

procedure GetHardIntVec(intnum : byte; var intsave : pointer);
begin
  if (intnum < 8) then intnum := intnum + 8 else intnum := intnum + $68;
  GetIntVec((intnum), intsave);
end ; { GetHardIntVec }


procedure SetHardIntVec(intnum : byte; intvec: pointer);
begin
  if (intnum < 8) then intnum := intnum + 8 else intnum := intnum + $68;
  SetIntVec((intnum), intvec);
end ; { SetHardIntVec }
