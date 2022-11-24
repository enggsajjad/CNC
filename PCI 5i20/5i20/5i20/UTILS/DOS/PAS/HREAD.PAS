function HexWordRead(hexstring : string; var hexnumber : word): boolean;
var
hindex : byte;
charVal : integer;
{placeVal : integer;}
validhex : boolean;
const
hexChars : string = '0123456789ABCDEF';
begin
  hexNumber :=0;
  validhex := true;
  for hindex := 1 to length(hexstring) do
  begin
    if validhex then hexNumber := HexNumber * $10;
    charval := pos(upcase(hexstring[hindex]),hexChars) -1;
    if charval < 0 then
    begin
      validhex := false;
    end
    else
    begin
      hexnumber := hexnumber + charVal;
    end;
  end;
  HexWordRead := validhex;
end;

function HexByteRead(hexstring : string; var hexnumber : byte): boolean;
var
hindex : byte;
charVal : integer;
{placeVal : integer;}
validhex : boolean;
const
hexChars : string = '0123456789ABCDEF';
begin
  hexNumber :=0;
  validhex := true;
  for hindex := 1 to length(hexstring) do
  begin
    if validhex then hexNumber := HexNumber * $10;
    charval := pos(upcase(hexstring[hindex]),hexChars) -1;
    if charval < 0 then
    begin
      validhex := false;
    end
    else
    begin
      hexnumber := hexnumber + charVal;
    end;
  end;
  HexByteRead := validhex;
end;

function HexLongRead(hexstring : string; var hexnumber : longint): boolean;
var
hindex : byte;
charVal : integer;
validhex : boolean;
const
hexChars : string = '0123456789ABCDEF';
begin
  hexNumber :=0;
  validhex := true;
  for hindex := 1 to length(hexstring) do
  begin
    hexNumber := HexNumber * $10;
    charval := pos(upcase(hexstring[hindex]),hexChars) -1;
    if charval < 0 then
    begin
      validhex := false;
    end
    else
    begin
      hexnumber := hexnumber + charVal;
    end;
  end;
  HexLongRead := validhex;
end;
{$IFDEF COPROC}
function FPHexDoubleRead(hexstring : string; var hexnumber : comp): boolean;
var
hindex : byte;
charVal : integer;
validhex : boolean;
const
hexChars : string = '0123456789ABCDEF';
begin
  hexNumber :=0;
  validhex := true;
  for hindex := 1 to length(hexstring) do
  begin
    hexNumber := hexnumber * $10;
    charval := pos(upcase(hexstring[hindex]),hexChars) -1;
    if charval < 0 then
    begin
      validhex := false;
    end
    else
    begin
      hexnumber := hexnumber + charVal;
    end;
  end;

  FPHexDoubleRead := validhex;
end;

function HexDoubleRead(hexstring : string; var hexnumber : comp): boolean;
var
hindex : byte;
charVal : integer;
validhex : boolean;
const
hexChars : string = '0123456789ABCDEF';
begin
  hexNumber :=0;
  validhex := true;
  for hindex := 1 to length(hexstring) do
  begin
    hexNumber := HexNumber * $10;
    charval := pos(upcase(hexstring[hindex]),hexChars) -1;
    if charval < 0 then
    begin
      validhex := false;
    end
    else
    begin
      hexnumber := hexnumber + charVal;
    end;
  end;
  HexDoubleRead := validhex;
end;
{$ENDIF}