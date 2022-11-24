const
{ PC speaker timer port addresses and data }
TimerCounterCont    = $43;                     { control register for PC's 8253 }
TimerCounter2       = $42;                     { address of speaker timer }
SquareWaveLeastMost = $B6;                     { data to set counter 2 to square wave - least most mode }

{ PC PIA port addresses and data ( for beeper ) }
ParPortB           = $61;                      { port for speaker gate and timer 2 gate }
BeepEnable         = $03;                      { timer and speaker gate on ( or'ed in )}
BeepDisable        = $FC;                      { timer an speaker gate off ( and'ed in)}
FooPort            = $40;
FPWXPlatchLoc      = $50;
FPWXBeepEnable     = $80;
FPWXBeepDisable    = $7F;

procedure BeepOn(fdivisor : word);
var
rsave : byte;
begin
  if fdivisor <> 0 then
  begin
    port[TimerCounterCont] := SquareWaveLeastMost;               { set timer mode to insure byte order }
    port[TimerCounter2]    := lo(fdivisor);      { write least significant byte of divisor }
    port[TimerCounter2]    := hi(fdivisor);      { write most significant byte of divisor }
    rsave := port[ParPortB];
    port[ParPortB] := rsave or BeepEnable;
  end
  else
  begin
    rsave := port[ParPortB];
    port[ParPortB] := rsave and BeepDisable;
  end;
end;

procedure BeepOnFPWX(fdivisor : word);
var
rsave : byte;
begin
  if fdivisor <> 0 then
  begin
    port[TimerCounterCont] := SquareWaveLeastMost;               { set timer mode to insure byte order }
    port[TimerCounter2]    := lo(fdivisor);      { write least significant byte of divisor }
    port[TimerCounter2]    := hi(fdivisor);      { write most significant byte of divisor }
    rsave := port[FPWXPlatchLoc];
    port[FPWXPLatchLoc] := rsave or FPWXBeepEnable;
  end
  else
  begin
    rsave := port[ParPortB];
    port[ParPortB] := rsave and BeepDisable;
  end;
end;

procedure BeepOff;
var
rsave : byte;
begin
  rsave := port[ParPortB];
  port[ParPortB] := rsave and BeepDisable;
end;

procedure BeepOffFPWX;
var
rsave : byte;
begin
  rsave := port[FPWXPlatchLoc];
  port[FPWXPlatchLoc] := rsave and FPWXBeepDisable;
end;

procedure SetBeepFreq( fdivisor : word);
begin
  port[TimerCounterCont] := SquareWaveLeastMost;               { set timer mode to insure byte order }
  port[TimerCounter2]    := lo(fdivisor);      { write least significant byte of divisor }
  port[TimerCounter2]    := hi(fdivisor);      { write most significant byte of divisor }
end;

procedure BeepDelay(d: word);
var
iloop : word;
oloop : word;
foo : byte;
begin
  for oloop := 0 to d do
  begin
    for iloop := 1 to 6 do
    begin
      { lazy attempt to make beep duration machine independent }
      { XT or AT input operations tend to take about }
      { the same amount of time on most machines }
      asm
        mov dx,FooPort;
        mov cx,$40
        @wloop:
        in al,dx
        in al,dx
        in al,dx
        in al,dx
        in al,dx
        loop  @wloop
      end;
    end;
  end;
end;

procedure DoBeep(p,d : word);
begin
  BeepOn(p);
  BeepDelay(d);
  BeepOff;
end;

procedure DoBeepFPWX(p,d : word);
begin
  BeepOnFPWX(p);
  BeepDelay(d);
  BeepOffFPWX;
end;
