function HexWordRead(hexstring : string; var hexnumber : word): boolean;
var
hindex : byte;
charVal : integer;
placeVal : integer;
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
placeVal : integer;
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
