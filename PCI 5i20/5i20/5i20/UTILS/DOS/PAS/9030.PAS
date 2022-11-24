const
N030VendorID = $10B5;
N030DeviceID = $9030;

N030PCIBaseAddress0 = $10;  { memory access to local config regs }
N030PCIBaseAddress1 = $14;  { I/O access to local config regs }
N030PCIBaseAddress2 = $18;  { local address space 0 }
N030PCIBaseAddress3 = $1C;  { local address space 1 }
N030PCIBaseAddress4 = $20;  { local address space 2 }
N030PCIBaseAddress5 = $24;  { local address space 3 }
N030InitControlReg = $50;
N030InitControlRegTop = $52;
N030GPIOControlReg = $54;
N030GPIOControlRegTop = $56;

N030EECLKMask : word = $0100;
N030EECSMask : word = $0200;
N030EEDIMask : word = $0400;
N030EEDOMask : word = $0800;

{ 5I20 Specific masks for access to PCI bridge GPIO bits... }

DoneMask : word = $0800; { Read at N030GPIOControlReg }
InitMask : word = $4000; { Read at N030GPIOControlReg }
LEDMask : word = $0002; { Written at N030GPIOControlRegTop}
WriteMask : word = $0080; { Written at N030GPIOControlRegTop}
ProgramMask: word = $0400; { Written at N030GPIOControlRegTop}

