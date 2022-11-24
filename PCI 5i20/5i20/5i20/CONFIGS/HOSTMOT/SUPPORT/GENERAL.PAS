{ This replaces Turbo Pascals keypressed function   }
{ which requires the CRT unit... The CRT unit bombs }
{ when there is no video card in the system!        }
type
LongIntRec =
record
  LoWord : word;
  HiWord : word;
end;

function KeyPressed : boolean;
var flag : boolean;
begin
  asm
    mov ah,01
    int $16
    mov flag,false
    jz @nokey
    mov flag,true
    @nokey:
  end;
  KeyPressed := flag;
end;

function ReadPort32(add : word): longint;
var
retlow,rethigh : word;
tlong : longint;
begin
  asm
    mov dx,add
    db $66,$ed          {in eax,dx}
    mov retlow,ax
    db $66,$c1,$e8,$10  {shr eax,16}
    mov rethigh,ax
  end;
  LongIntRec(tlong).hiword := rethigh;
  LongIntRec(tlong).loword := retlow;
  ReadPort32 := tlong;
end;

procedure WritePort32(add : word; data : longint);
var
datalow,datahigh : word;
begin
  datahigh := longintrec(data).hiword;
  datalow :=  longintrec(data).loword;
  asm
    mov dx,add
    mov ax,datahigh
    db $66,$c1,$e0,$10  {shl eax,16}
    mov ax,datalow
    db $66,$ef          {out dx,eax}
  end;
end;

function ReadKey : word;
var kdat : word;
begin
  asm
    mov ah,00
    int $16
    mov kdat,ax
  end;
  ReadKey := kdat;
end;

function GetCPUType : string;
const
CPUIdentSeg = $FFF0;
CPUIdentOfs = $0000;
var
index : byte;
tempstr : string;
begin
  tempstr := '';
  index := 0;
  while mem[CPUIdentSeg:CPUIdentOfs+word(index)] <> 0 do
  begin
    tempstr := tempstr +char(mem[CPUIdentSeg:CPUIdentOfs+word(index)]);
    index := index +1;
  end;
  GetCPUType := tempstr;
end;

function GetBIOSRev : string;
const
BIOSRevSeg = $FFF0;
BIOSRevOfs = $12;
var
tempstr : string;
begin
  tempstr := '';
  tempstr := tempstr + char(mem[BIOSRevSeg:BIOSRevOfs+0]+$30);
  tempstr := tempstr + '.';
  tempstr := tempstr + char(mem[BIOSRevSeg:BIOSRevOfs+1]+$30);
  tempstr := tempstr + char(mem[BIOSRevSeg:BIOSRevOfs+2]+$61);
  GetBIOSRev := tempstr;
end;

procedure FixCRT;
{ Fix re-direction when using CRT unit }
begin
  assign(input,'');
  reset(input);
  assign(output,'');
  rewrite(output);
end;
