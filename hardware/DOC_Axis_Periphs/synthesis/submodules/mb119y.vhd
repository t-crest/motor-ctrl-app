-- iC-Haus
-- Projekt    : MB
-- Block      : 119Y
-- Revision   : 4
-- Bearbeiter : Sz
-- Datum      : 04.02.2011
-- Name       : QSYS compatible BiSS-Master
-- ~VP        : VDD local
-- ~VN        : GND local

library ieee;
use ieee.std_logic_1164.all;

entity mb119y is
  generic(
    Slaves   : integer := 8;
    NumRegs  : integer := 64
  );
  port (
-- System ----------------------------------------------------------------------
    csi_clk_clk       : in std_logic;                     -- Systemtakt (50 MHz)
    csi_reset_reset_n : in std_logic;          -- asynchroner Reset (low active)
-- Avalon-MM slave -------------------------------------------------------------
    avs_s1_write_n      : in  std_logic;                -- NotWrite (low active)
    avs_s1_read_n       : in  std_logic;                 -- NotRead (low active)
    avs_s1_chipselect_n : in  std_logic;           -- NotChipSelect (low active)
    avs_s1_address      : in  std_logic_vector(5 downto 0);         -- Adressbus
    avs_s1_writedata    : in  std_logic_vector(31 downto 0);     -- Schreibdaten
    avs_s1_readdata     : out std_logic_vector(31 downto 0);        -- Lesedaten
-- BiSS ------------------------------------------------------------------------
    coe_ch1_MA : out std_logic;                                  -- Clock Outout
    coe_ch1_MO : out std_logic;                                   -- Data Output
    coe_ch1_SL : in  std_logic;                                    -- Data Input
    coe_ch1_EOT : out  std_logic;                         -- End Of Transmission
-- GetSens Strobe --------------------------------------------------------------
    get_sens : in  std_logic;    -- Strobe for sensor data reading (high active)
-- Status ----------------------------------------------------------------------
    ins_irq0_irq  : out std_logic                         -- End of Transmission
  );
end mb119y;

library mb;
use mb.mb_vcomponents.all;

architecture synth of mb119y is
  signal NER_O : std_logic;
  signal EOT : std_logic;
begin

coe_ch1_EOT <= EOT;
ins_irq0_irq <= EOT;

X001: mb0 -- MB0
  generic map(
    BusWidth      => 32,
    Slaves        => Slaves,
    Channels      => 1,
    AddressBus    => 2, -- Adressbus ohne LAtch
    UseLatch      => 0, -- FlipFlops fuer Register verwenden
    UseTristate   => 0, -- Getrennten Lese- und Schreibbus aufbauen
    NumRegs       => NumRegs,
    UseSPI        => 0, -- Serielle (SPI) Schnittstelle vorhanden
    UseARes       => 0, -- Speicher ueber NRES zuruecksetzen
    RamWidth      => 32, -- 32-Bit-Bus
    BankCount     => 2, -- 2 Baenke fuer SensorData
    UseFpgaRam    => 1, -- FPGA-RAM (EABs) verwenden
    ClkdivSensLen => 5, -- wie MB3
    ClkdivAgsLen  => 8, -- wie MB3
    ClkDivRegLen  => 7, -- wie MB3
    BusyCountLen  => 12, -- wie MB3
    UseReg245     => 1, -- Testregister verwenden
    Revision      => 4, -- Revision Y1
    Version       => 83 -- MB119
  )
  port map (
    NWR_E    => avs_s1_write_n,
    NRD_RNW  => avs_s1_read_n,
    NCS      => avs_s1_chipselect_n,
    ADR0_ALE => '0',
    ADR1     => '0',
    ADR2     => avs_s1_address(0),
    ADR3     => avs_s1_address(1),
    ADR4     => avs_s1_address(2),
    ADR5     => avs_s1_address(3),
    ADR6     => avs_s1_address(4),
    ADR7     => avs_s1_address(5),
    INT_NMOT => '1',
    CFGDB    => '0',
    CFGADR   => '0',
    CFGSPI   => '0',
    GETSENS  => get_sens,
    NRES     => csi_reset_reset_n,
    CLK      => csi_clk_clk,
    SL1      => coe_ch1_SL,
    SL2      => '1',
    SL3      => '1',
    SL4      => '1',
    SL5      => '1',
    SL6      => '1',
    SL7      => '1',
    SL8      => '1',
    GND1     => '0',
    GND2     => '0',
    GND3     => '0',
    GND4     => '0',
    GND5     => '0',
    GND6     => '0',
    VDD1     => '1',
    VDD2     => '1',
    VDD3     => '1',
    VDD4     => '1',
    VDD5     => '1',
    DBIN     => avs_s1_writedata,
    DBOUT    => avs_s1_readdata,
    DB0      => open,
    DB1      => open,
    DB2      => open,
    DB3      => open,
    DB4      => open,
    DB5      => open,
    DB6      => open,
    DB7      => open,
    DB8      => open,
    DB9      => open,
    DB10     => open,
    DB11     => open,
    DB12     => open,
    DB13     => open,
    DB14     => open,
    DB15     => open,
    DB16     => open,
    DB17     => open,
    DB18     => open,
    DB19     => open,
    DB20     => open,
    DB21     => open,
    DB22     => open,
    DB23     => open,
    DB24     => open,
    DB25     => open,
    DB26     => open,
    DB27     => open,
    DB28     => open,
    DB29     => open,
    DB30     => open,
    DB31     => open,
    MA1      => coe_ch1_MA,
    MA2      => open,
    MA3      => open,
    MA4      => open,
    MA5      => open,
    MA6      => open,
    MA7      => open,
    MA8      => open,
    MO1      => coe_ch1_MO,
    MO2      => open,
    MO3      => open,
    MO4      => open,
    MO5      => open,
    MO6      => open,
    MO7      => open,
    MO8      => open,
    EOT      => EOT,
    NER      => NER_O,
    CLKOUT   => open
  );

end synth;
