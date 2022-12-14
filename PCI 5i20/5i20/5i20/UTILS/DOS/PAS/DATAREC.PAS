type LongIntRec = record
  LowWord  : word;
  HighWord : word;
end;

type LongIntByteRec = record
  Byte0 : byte;
  Byte1 : byte;
  Byte2 : byte;
  Byte3 : byte;
end;

type DoubleIntRec = record
  Word0  : word;
  Word1  : word;
  Word2  : word;
  Word3  : word;
end;

type DoublelongRec = record
  long0  : longint;
  long1  : longint;
end;

type WordRec = record
  LowByte : byte;
  HighByte : byte;
end;

