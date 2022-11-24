const

IDROMSize = 256;
IDROMHeaderSize = 16;
MaxModules = 32;
MaxPins = 144;
IDROMStyle0 = 2;
IDROMStyle1 = 3;
Boards = 8;
MaxConns = 6;

type

IDROMAsArrayType = Array[0 ..IDROMSize-1] of longint;
IDROMHeaderAsArrayType = Array[0 ..IDROMHeaderSize-1] of longint;

ModulesAsArrayType = array[0.. MaxModules*3-1] of longint;
ModuleType = array[1 .. MaxModules] of record
  GTag : byte;
  Version : byte;
  Clock : byte;
  NumInstances : byte;
  BaseAddr : word;
  NumRegisters : byte;
  Strides : byte;
  MultRegs : longint;
end;

PinDescType = array[1..MaxPins] of record
  PNumber : byte;
  GTag : byte;
  Chan : byte;
  PTag : byte;
end;

PinDescAsArrayType = array[0..MaxPins-1] of longint;

FNameType = record
  FName : string[40];
  FTag  : byte;
end;

PinNameType = record
  FTag : byte;
  Names : array[1..10] of string[20];
end;

IDROMHeaderType = record
  IDROMType : longint;
  ModulePointer : longint;
  PinDescPointer : longint;
  BoardNameLow : longint;
  BoardNameHigh : longint;
  FPGASize : longint;
  FPGAPins : longint;
  IOPorts : longint;
  IOWidth : longint;
  PortWidth : longint;
  ClockLow : longint;
  ClockHigh : longint;
  InstStride0 : longint;
  InstStride1 : longint;
  RegStride0 : longint;
  RegStride1 : longint;
end;

ConnectorNamesType = array[1..Boards] of record
  BoardName : longint;
  ConName : array[1..MaxConns] of string[20];
end;

const
HM2Cookie : longint = $55AACAFE;
HostMotName = 'HOSTMOT2';

IDOffset = $100;
HM2CookieOffset = IDOffset + 0;
HostMotNameLowOffset  = IDOffset+4;
HostMotNameHighOffset = IDOffset+8;
IDROMPointer =  IDOffset+ 12;

ClockLowTag= $01;
ClockHighTag= $02;

MaxTags = 28;
NullTag = $00;
IRQLogicTag = $01;
WatchDogTag = $02;
IOPortTag = $03;
QCountTag = $04;
StepGenTag = $05;
PWMTag = $06;
SPITag = $07;
SSITag = $08;
UARTTXTag = $09;
UARTRXTag = $0A;
AddrXTag = $0B;
MuxedQCountTag = $0C;
MuxedQCountSelTag =$0D;
BSPITag = $0E;
DBSPITag = $0F;
DPLLTag = $10;
MuxedQCountMIMTag = $11;
MuxedQCountSelMIMTag =$12;
TPPWMTag = $13;
WaveGenTag = $14;
DAQFIFOTag =$15;
BinOscTag =$16;
DMDMATag =$17;
ResModTag = $C0;
SSerialTag = $C1;
TwiddlerTag = $C2;

LEDTag = $80;

BoardName4I65 = $35364934;		{  4I65 }
BoardName4I68 = $38364934;		{  4I68 }
BoardName5I20 = $30324935;		{  5I20 }
BoardName5I22 = $32324935;		{  5I22 }
BoardName5I23 = $33324935;		{  5I23 }
BoardName7I43 = $33344937;		{  7I43 }
BoardName7I60 = $30364937;		{  7I60 }
BoardName3X20 = $30325833;		{  3X20 }

OutputMarker = $80;
GlobalMarker = $80;
OutputMask = $7F;

FNames : array[1..MaxTags] of FNameType =
(
(FName : 'Null'; Ftag : NullTag),
(FName : 'IRQLogic'; Ftag : IRQLogicTag),
(FName : 'WatchDog'; Ftag : WatchDogTag),
(FName : 'IOPort'; Ftag : IOPortTag),
(FName : 'QCount'; Ftag : QCountTag),
(FName : 'StepGen'; Ftag : StepGenTag),
(FName : 'PWM'; Ftag : PWMTag),
(FName : 'SPI'; Ftag : SPITag),
(FName : 'SSI'; Ftag : SSITag),
(FName : 'UARTTX'; Ftag : UARTTXTag),
(FName : 'UARTRX'; Ftag : UARTRXTag),
(FName : 'AddrX'; Ftag : AddrXTag),
(FName : 'MuxQCnt'; Ftag : MuxedQCountTag),
(FName : 'MuxQSel'; Ftag : MuxedQCountSelTag),
(FName : 'BufSPI'; Ftag : BSPITag),
(FName : 'DBufSPI'; Ftag : DBSPITag),
(FName : 'DPLL'; Ftag : DPLLTag),
(FName : 'MuxQCntM'; Ftag : MuxedQCountMIMTag),
(FName : 'MuxQSelM'; Ftag : MuxedQCountSelMIMTag),
(FName : 'TPPWM'; Ftag : TPPWMTag),
(FName : 'WaveGen'; Ftag : WaveGenTag),
(FName : 'DAQFIFO'; Ftag : DAQFIFOTag),
(FName : 'BinOsc'; Ftag : BinOscTag),
(FName : 'DMDMA'; Ftag : DMDMATag),
(FName : 'ResolverMod'; Ftag : ResModTag),
(FName : 'SSerial'; Ftag : SSerialTag),
(FName : 'Twiddler'; Ftag : TwiddlerTag),
(FName : 'LED'; Ftag : LEDTag)
);

PNames : array[1..MaxTags] of PinNameType =
(
(FTag : NullTag; Names : ('Null1','Null2','Null3','Null4','Null5','Null6','Null7','Null8','Null9','Null10')),
(FTag : IRQLogicTag; Names : ('Null1','Null2','Null3','Null4','Null5','Null6','Null7','Null8','Null9','Null10')),
(FTag : WatchdogTag; Names : ('Null1','Null2','Null3','Null4','Null5','Null6','Null7','Null8','Null9','Null10')),
(FTag : IOPortTag; Names : ('Null1','Null2','Null3','Null4','Null5','Null6','Null7','Null8','Null9','Null10')),
(FTag : QCountTag; Names : ('Quad-A','Quad-B','Quad-IDX','Quad-IDXM','Quad-Probe','Null6','Null7','Null8','Null9','Null10')),
(FTag : MuxedQCountTag; Names : ('MuxQ-A','MuxQ-B','MuxQ-IDX','MuxQ-IDXM','Quad-ProbeM','Null6','Null7','Null8'
,'Null9','Null10')),
(FTag : MuxedQCountSelTag; Names : ('MuxSel0','MuxSel1','Null3','Null4','Null5','Null6','Null7','Null8','Null9','Null10')),
(FTag : MuxedQCountMIMTag; Names : ('MuxQ-A','MuxQ-B','MuxQ-IDX','MuxQ-IDXM','Quad-ProbeM','Null6','Null7','Null8'
,'Null9','Null10')),
(FTag : MuxedQCountSelMIMTag; Names : ('MuxSel0','MuxSel1','Null3','Null4','Null5','Null6','Null7','Null8'
,'Null9','Null10')),
(FTag : StepGenTag; Names : ('Step/Table1','Dir/Table2','Table3','Table4','Table5','Table6','SGindex','SGProbe'
,'Null9','Null10')),
(FTag : PWMTag; Names :     ('PWM','Dir','/Enable','Null4','Null5','Null6','Null7','Null8','Null9','Null10')),
(FTag : TPPWMTag; Names :   ('PWMA','PWMB','PWMC','NPWMA','NPWMB','NPWMC','/ENA','FAULT','Null9','Null10')),
(FTag : WaveGenTag; Names : ('PDMA','PDMB','Trig0','Trig1','Trig2','Trig3','Null7','Null8','Null9','Null10')),
(FTag : DAQFIFOTag; Names : ('Data','Strobe','Full','Null4','Null5','Null6','Null7','Null8','Null9','Null10')),
(FTag : BinOscTag; Names : ('OscOut','Null2','Null3','Null4','Null5','Null6','Null7','Null8','Null9','Null10')),
(FTag : DMDMATag; Names : ('Null1','Null2','Null3','Null4','Null5','Null6','Null7','Null8','Null9','Null10')),
(FTag : ResModTag; Names : ('PwrEn','PDMP','PDMM','ADChan0','ADChan1','ADChan2','SPICS','SPIClk','SPIDI0','SPIDI1')),
(FTag : SSerialTag; Names : ('RXData','TXData','TXEn','TestPin','Null5','Null6','Null7','Null8','Null9','Null10')),
(FTag : TwiddlerTag; Names : ('InBit','IOBit','OutBit','Null4','Null5','Null6','Null7','Null8','Null9','Null10')),
(FTag : SPITag; Names :     ('/Frame','DOut','SClk','DIn','Null5','Null6','Null7','Null8','Null9','Null10')),
(FTag : BSPITag; Names :    ('/Frame','DOut','SClk','DIn','CS0','CS1','CS2','CS3','Null9','Null10')),
(FTag : DBSPITag; Names :   ('Null1','DOut','SClk','DIn','/CS-FRM0','/CS-FRM1','/CS-FRM2','/CS-FRM3','Null9','Null10')),
(FTag : DPLLTag; Names :    ('Sync','DDSMSB','FOut','PostOut','SyncToggle','Null6','Null7','Null8','Null9','Null10')),
(FTag : SSITag; Names :     ('SClk','DIn','Null3','Null4','Null5','Null6','Null7','Null8','Null9','Null10')),
(FTag : UARTTXTag; Names :  ('TXData','TXEna','Null3','Null4','Null5','Null6','Null7','Null8','Null9','Null10')),
(FTag : UARTRXTag; Names :  ('RXData','Null2','Null3','Null4','Null5','Null6','Null7','Null8','Null9','Null10')),
(FTag : AddrxTag; Names :   ('Null1','Null2','Null3','Null4','Null5','Null6','Null7','Null8','Null9','Null10')),
(FTag : LEDTag; Names :     ('Null1','Null2','Null3','Null4','Null5','Null6','Null7','Null8','Null9','Null10')));

ConnectorNames : ConnectorNamesType =
(
(BoardName : BoardName4I65; ConName : ('P1','P3','P4','Null1','Null2','Null3')),
(BoardName : BoardName4I68; ConName : ('P1','P2','P3','Null1','Null2','Null3')),
(BoardName : BoardName5I20; ConName : ('P2','P3','P4','Null1','Null2','Null3')),
(BoardName : BoardName5I22; ConName : ('P2','P3','P4','P5','Null1','Null2')),
(BoardName : BoardName5I23; ConName : ('P2','P3','P4','Null1','Null2','Null3')),
(BoardName : BoardName7I43; ConName : ('P4','P3','Null1','Null2','Null3','Null4')),
(BoardName : BoardName7I60; ConName : ('P7','P8','P5','P4','Null1','Null3')),
(BoardName : BoardName3X20; ConName : ('P4','P5','P6','P9','P8','P7')));

var
Modules : ModuleType;
PinDescs : PinDescType;
PinDescsAsArray : PinDescAsArrayType;
IDROMOffset : longint;
ModuleOffset : longint;
PinDescOffset : longint;
IDROMAsArray : IDROMAsArrayType;
IDROMHeaderAsArray : IDROMHeaderAsArrayType;
ModulesAsArray : ModulesAsArrayType;
IDROMHeader : IDROMHeaderType;
ClockLow : longint;
ClockHigh : longint;
