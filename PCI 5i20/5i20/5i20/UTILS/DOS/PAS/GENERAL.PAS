{ This replaces Turbo Pascals keypressed function   }
{ which requires the CRT unit... The CRT unit bombs }
{ when there is no video card in the system!        }

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

procedure badWriteMem32(add,data : longint);
begin
  asm
    push bp
    mov bp,sp
    push ds
    db $66,$33,$c0      { xor eax,eax }
    db $66,$8e,$d8      { mov ds,eax }
    db $36              { ss: }
    db $66,$8b,$46,$06  { mov eax,[BP+06] get data }
    db $66,$8b,$4e,$0a  { mov ecx,[BP+10] get address}
    db $3e              { ds: }
    db $67,$66,$89,$01  { mov [ecx],eax write data to address}
    pop ds
    pop bp
  end;
end;

procedure WriteMem32(add,data : longint);
var
datahigh,datalow,addhigh,addlow: word;

begin
  datahigh := longintrec(data).HighWord;
  datalow := longintrec(data).LowWord;
  addhigh := longintrec(add).HighWord;
  addlow  := longintrec(add).LowWord;
  asm
    push ds
    db $66,$33,$c0      { xor eax,eax }
    db $66,$8e,$d8      { mov ds,eax }
    mov cx,addhigh
    db $66,$c1,$e1,$10  {shl ecx 16}
    mov cx,addlow
    mov ax,datahigh
    db $66,$c1,$e0,$10  {shl eax,16}
    mov ax,datalow
    db $3e              { ds: }
    db $67,$66,$89,$01  { mov [ecx],eax write data to address}
    pop ds
  end;
end;

procedure ReadMem32(add : longint;var data : longint);
var
datahigh,datalow,addhigh,addlow: word;

begin
  addhigh := longintrec(add).HighWord;
  addlow  := longintrec(add).LowWord;
  asm
    push ds
    db $66,$33,$c0      { xor eax,eax }
    db $66,$8e,$d8      { mov ds,eax }
    mov cx,addhigh
    db $66,$c1,$e1,$10  {shl ecx 16}
    mov cx,addlow
    db $3e              { ds: }
    db $67,$66,$8b,$01  { mov eax,[ecx]   get data}
    mov datalow,ax
    db $66,$c1,$e8,$10  {shr eax,16}
    mov datahigh,ax
    pop ds
  end;
  LongIntRec(data).LowWord := datalow;
  LongIntRec(data).HighWord := datahigh;


end;

procedure badReadMem32(add,data : longint);
begin
  asm
    push bp
    mov bp,sp
    db $36              { ss: }
    db $66,$8b,$4e,$0a  { mov ecx,[bp+10] get address}
    db $67,$66,$8b,$01  { mov eax,[eax]   get data}
    db $66,$89,$46,$06  { mov [bp+6],eax return data }
    pop bp
  end;
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
  LongIntRec(tlong).HighWord := rethigh;
  LongIntRec(tlong).LowWord := retlow;
  ReadPort32 := tlong;
end;

procedure WritePort32(add : word; data : longint);
var
datalow,datahigh : word;
begin
  datahigh := longintrec(data).HighWord;
  datalow :=  longintrec(data).LowWord;
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
