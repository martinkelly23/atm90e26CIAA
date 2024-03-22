# Library for ATM90E26 
With this library you will be able of:
- Initialice the comunication an registers of the IC with: InitEnergyIC()
- Get the system status: GetSysStatus()
- Get the meter status: GetMeterStatus();
- Get the line voltage with: GetLineVoltage()
- Get the line current with GetLineCurrent()
- Get the active power with GetActivePower()
- Get the frequency with: GetFrequency()
- Get the power factor with: GetPowerFactor()
- Get the import energy with: GetImportEnergy()
- Get the export energy with: GetExportEnergy()

### Single-Phase High-Performance Wide-Span Energy Metering IC
Microchip reference: https://www.microchip.com/en-us/product/atm90e26#document-table

### Schematic 
<img width="1311" alt="Captura de pantalla 2024-03-22 a las 22 03 07" src="https://github.com/martinkelly23/atm90e26CIAA/assets/21344092/fb6611f7-9860-4e15-b295-6276e3524fc6">

### How it work?
Based on the UART communication protocol, and making use of the registers explained in the dastasheet.
