{$I 9030}
{$I 9054}
{$I 9056}

function SetupPCIWithMessage(var base : word; var message : string) : boolean;
var
busnum,devnum: byte;
configbase : word;
FoundPCIBridge : boolean;
hstring : string;
begin
  FoundPCIBridge := false;
  if PCIBIOSPresent then
  begin
    if FindPCIDevice(N030DeviceID,N030VendorID,0,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress1,ConfigBase) then
      begin
        ConfigBase := ConfigBase and $FFFE;
      end;
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress2,ConfigBase) then
      begin
        ConfigBase := ConfigBase and $FFFE;
        base := ConfigBase;
        FoundPCIBridge := true;
        hstring := HexString(base,4);
        message := 'Found 9030 based PCI card -- base address is: 0x'+hstring;
      end;
    end;
  end;
  if FindPCIDevice(N054DeviceID,N054VendorID,0,busnum,devnum) then
  begin
    if ReadPCIConfigWord(busnum,devnum,N054PCIBaseAddress1,ConfigBase) then
    begin
      ConfigBase := ConfigBase and $FFFE;
    end;
    if ReadPCIConfigWord(busnum,devnum,N054PCIBaseAddress2,ConfigBase) then
    begin
      ConfigBase := ConfigBase and $FFFE;
      base := ConfigBase;
      FoundPCIBridge := true;
      hstring := HexString(base,4);
      message := 'Found 9054 based PCI card -- base address is: 0x'+hstring;
    end;
  end;
  if FindPCIDevice(N056DeviceID,N056VendorID,0,busnum,devnum) then
  begin
    if ReadPCIConfigWord(busnum,devnum,N056PCIBaseAddress1,ConfigBase) then
    begin
      ConfigBase := ConfigBase and $FFFE;
    end;
    if ReadPCIConfigWord(busnum,devnum,N056PCIBaseAddress2,ConfigBase) then
    begin
      ConfigBase := ConfigBase and $FFFE;
      base := ConfigBase;
      FoundPCIBridge := true;
      hstring := HexString(base,4);
      message := 'Found 9056 based PCI card -- base address is: 0x'+hstring;
    end;
  end;
  if FoundPCIBridge = false then message := 'Didn''t find any PCI cards';
  SetupPCIWithMessage := FoundPCIBridge;
  { we really should look for the secondary ID for 5I20, 4I65, 5I21, 4I68 etc}
end;

function SetupPCI(var base : word) : boolean;
var
busnum,devnum: byte;
configbase : word;
FoundPCIBridge : boolean;
hstring : string;
begin
  FoundPCIBridge := false;
  if PCIBIOSPresent then
  begin
    if FindPCIDevice(N030DeviceID,N030VendorID,0,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress1,ConfigBase) then
      begin
        ConfigBase := ConfigBase and $FFFE;
      end;
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress2,ConfigBase) then
      begin
        ConfigBase := ConfigBase and $FFFE;
        base := ConfigBase;
        FoundPCIBridge := true;
        hstring := HexString(base,4);
        Writeln('Found 9030 based PCI card -- base address is: 0x'+hstring);
      end;
    end;
    if FindPCIDevice(N054DeviceID,N054VendorID,0,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,N054PCIBaseAddress1,ConfigBase) then
      begin
        ConfigBase := ConfigBase and $FFFE;
      end;
      if ReadPCIConfigWord(busnum,devnum,N054PCIBaseAddress2,ConfigBase) then
      begin
        ConfigBase := ConfigBase and $FFFE;
        base := ConfigBase;
        FoundPCIBridge := true;
        hstring := HexString(base,4);
        Writeln('Found 9054 based PCI card -- base address is: 0x'+hstring);
      end;
    end;
    if FindPCIDevice(N056DeviceID,N056VendorID,0,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,N056PCIBaseAddress1,ConfigBase) then
      begin
        ConfigBase := ConfigBase and $FFFE;
      end;
      if ReadPCIConfigWord(busnum,devnum,N056PCIBaseAddress2,ConfigBase) then
      begin
        ConfigBase := ConfigBase and $FFFE;
        base := ConfigBase;
        FoundPCIBridge := true;
        hstring := HexString(base,4);
        Writeln('Found 9056 based PCI card -- base address is: 0x'+hstring);
      end;
    end;
  end;
  if FoundPCIBridge = false then Writeln('Didn''t find any PCI cards');
  SetupPCI := FoundPCIBridge;
  { we really should look for the secondary ID for 5I20, 4I65, 5I21, 4I68 etc}
end;

function QuietSetupPCI(var base : word) : boolean;
var
busnum,devnum: byte;
configbase : word;
FoundPCIBridge : boolean;
{hstring : string;}
begin
  FoundPCIBridge := false;
  if PCIBIOSPresent then
  begin
    if FindPCIDevice(N030DeviceID,N030VendorID,0,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress2,ConfigBase) then
      begin
        ConfigBase := ConfigBase and $FFFE;
        base := ConfigBase;
        FoundPCIBridge := true;
        {hstring := HexString(base,4);}
      end;
    end;
    if FindPCIDevice(N054DeviceID,N054VendorID,0,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,N054PCIBaseAddress2,ConfigBase) then
      begin
        ConfigBase := ConfigBase and $FFFE;
        base := ConfigBase;
        FoundPCIBridge := true;
        {hstring := HexString(base,4);}
      end;
    end;
    if FindPCIDevice(N056DeviceID,N056VendorID,0,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,N056PCIBaseAddress2,ConfigBase) then
      begin
        ConfigBase := ConfigBase and $FFFE;
        base := ConfigBase;
        FoundPCIBridge := true;
        {hstring := HexString(base,4);}
      end;
    end;
  end;
  QuietSetupPCI := FoundPCIBridge;
  { we really should look for the secondary ID for 5I20, 4I65, 5I21, 4I68 etc}
end;

function SetupPCIM(var base : longint): boolean;
var
busnum,devnum: byte;
configbase : longint;
FoundPCIBridge : boolean;
hstring : string;
begin
  FoundPCIBridge := false;
  if PCIBIOSPresent then
  begin
    if FindPCIDevice(N030DeviceID,N030VendorID,0,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress5,LongIntRec(configbase).LowWord) then
      begin
        if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress5+2,LongIntRec(configbase).HighWord) then
        begin
          base := ConfigBase;
          FoundPCIBridge := true;
          hstring := HexString(base,8);
          Writeln('Found 9030 based PCI card -- memory base address is: 0x'+hstring);
        end;
      end;
    end;
    if FindPCIDevice(N054DeviceID,N054VendorID,0,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,N054PCIBaseAddress3,LongIntRec(ConfigBase).LowWord) then
      begin
        if ReadPCIConfigWord(busnum,devnum,N054PCIBaseAddress3+2,LongIntRec(ConfigBase).HighWord) then
        begin
          base := ConfigBase;
          FoundPCIBridge := true;
          hstring := HexString(base,8);
          Writeln('Found 9054 based PCI card -- memory base address is: 0x'+hstring);
        end;
      end;
    end;
    if FindPCIDevice(N056DeviceID,N056VendorID,0,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,N056PCIBaseAddress3,LongIntRec(ConfigBase).LowWord) then
      begin
        if ReadPCIConfigWord(busnum,devnum,N056PCIBaseAddress3+2,LongIntRec(ConfigBase).HighWord) then
        begin
          base := ConfigBase;
          FoundPCIBridge := true;
          hstring := HexString(base,8);
          Writeln('Found 9056 based PCI card -- memory base address is: 0x'+hstring);
        end;
      end;
    end;
  end;
  if FoundPCIBridge = false then Writeln('Didn''t find any PCI cards');
  SetupPCIM := FoundPCIBridge;
  { we really should look for the secondary ID for 5I20, 4I65, 5I21, 4I68 etc}
end;

function QuietSetupPCIM(cardnum: word;var base : longint) : boolean;
var
busnum,devnum: byte;
configbase : longint;
FoundPCIBridge : boolean;
begin
  FoundPCIBridge := false;
  if PCIBIOSPresent then
  begin
    if FindPCIDevice(N030DeviceID,N030VendorID,cardnum,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress5,LongIntRec(configbase).LowWord) then
      begin
        if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress5+2,LongIntRec(configbase).HighWord) then
        begin
          base := ConfigBase;
          FoundPCIBridge := true;
        end;
      end;
    end;
    if FindPCIDevice(N054DeviceID,N054VendorID,cardnum,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,N054PCIBaseAddress3,LongIntRec(ConfigBase).LowWord) then
      begin
        if ReadPCIConfigWord(busnum,devnum,N054PCIBaseAddress3+2,LongIntRec(ConfigBase).HighWord) then
        begin
          base := ConfigBase;
          FoundPCIBridge := true;
        end;
      end;
    end;
    if FindPCIDevice(N056DeviceID,N056VendorID,cardnum,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,N056PCIBaseAddress3,LongIntRec(ConfigBase).LowWord) then
      begin
        if ReadPCIConfigWord(busnum,devnum,N056PCIBaseAddress3+2,LongIntRec(ConfigBase).HighWord) then
        begin
          base := ConfigBase;
          FoundPCIBridge := true;
        end;
      end;
    end;
  end;
  QuietSetupPCIM := FoundPCIBridge;
  { we really should look for the secondary ID for 5I20, 4I65, 5I21, 4I68 etc}
end;

function QuietSetupPCIMC(cardnum: word;var base : longint) : boolean;
var
busnum,devnum: byte;
configbase : longint;
FoundPCIBridge : boolean;
begin
  FoundPCIBridge := false;
  if PCIBIOSPresent then
  begin
    if FindPCIDevice(N030DeviceID,N030VendorID,cardnum,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress0,LongIntRec(configbase).LowWord) then
      begin
        if ReadPCIConfigWord(busnum,devnum,N030PCIBaseAddress0+2,LongIntRec(configbase).HighWord) then
        begin
          base := ConfigBase;
          FoundPCIBridge := true;
        end;
      end;
    end;
    if FindPCIDevice(N054DeviceID,N054VendorID,cardnum,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,N054PCIBaseAddress0,LongIntRec(ConfigBase).LowWord) then
      begin
        if ReadPCIConfigWord(busnum,devnum,N054PCIBaseAddress0+2,LongIntRec(ConfigBase).HighWord) then
        begin
          base := ConfigBase;
          FoundPCIBridge := true;
        end;
      end;
    end;
    if FindPCIDevice(N056DeviceID,N056VendorID,cardnum,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,N056PCIBaseAddress0,LongIntRec(ConfigBase).LowWord) then
      begin
        if ReadPCIConfigWord(busnum,devnum,N056PCIBaseAddress0+2,LongIntRec(ConfigBase).HighWord) then
        begin
          base := ConfigBase;
          FoundPCIBridge := true;
        end;
      end;
    end;
  end;
  QuietSetupPCIMC := FoundPCIBridge;
  { we really should look for the secondary ID for 5I20, 4I65, 5I21, 4I68 etc}
end;

function ReadBridgeConfigWord(regnum : word; var data : word): boolean;
var
busnum,devnum: byte;
tword,treg : word;
FoundPCIBridge : boolean;
begin
  treg := regnum;
  FoundPCIBridge := false;
  if PCIBIOSPresent then
  begin
    if FindPCIDevice(N030DeviceID,N030VendorID,0,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,treg,tword) then
      begin
        FoundPCIBridge := true;
      end;
    end;
    if FindPCIDevice(N054DeviceID,N054VendorID,0,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,treg,tword) then
      begin
        FoundPCIBridge := true;
      end;
    end;
    if FindPCIDevice(N056DeviceID,N056VendorID,0,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,treg,tword) then
      begin
        FoundPCIBridge := true;
      end;
    end;
  end;
  data := tword;
  ReadBridgeConfigWord := FoundPCIBridge;
end;

function WriteBridgeConfigWord(regnum : word; data : word): boolean;
var
busnum,devnum: byte;
tword,treg : word;
FoundPCIBridge : boolean;
begin
  tword := data;
  treg := regnum;
  FoundPCIBridge := false;
  if PCIBIOSPresent then
  begin
    if FindPCIDevice(N030DeviceID,N030VendorID,0,busnum,devnum) then
    begin
      if WritePCIConfigWord(busnum,devnum,treg,tword) then
      begin
        FoundPCIBridge := true;
      end;
    end;
    if FindPCIDevice(N054DeviceID,N054VendorID,0,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,treg,tword) then
      begin
        FoundPCIBridge := true;
      end;
    end;
    if FindPCIDevice(N056DeviceID,N056VendorID,0,busnum,devnum) then
    begin
      if ReadPCIConfigWord(busnum,devnum,treg,tword) then
      begin
        FoundPCIBridge := true;
      end;
    end;
  end;
  WriteBridgeConfigWord := FoundPCIBridge;
end;
