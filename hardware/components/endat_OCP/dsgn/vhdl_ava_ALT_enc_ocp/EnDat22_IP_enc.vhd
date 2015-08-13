--******************************************************************************
--* MAZeT. All rights reserved. This document and all informations             *
--* contained therein is considered confidential and proprietary of MAZeT.     *
--*----------------------------------------------------------------------------*
--*                                                                            *
--* Project     : EnDat22-IP                                                   *
--*                                                                            *
--* File name   : EnDat22_IP.vhd                                               *
--* Author      : Frank Schurz                                                 *
--* Description : Wrapper fuer SOPC Builder                                    *
--* Remarks     :                                                              *
--* History     :                                                              *
--*                                                                            *
--* Version  | Date       | Name      | Description                            *
--*----------+------------+-----------+----------------------------------------*
--* ver. 1.0 | 05.08.2010 | F. Schurz | initial version                        *
--*----------+------------+-----------+----------------------------------------*
--* ver. 1.1 | 25.11.2010 | S. Go"tz  | keine parameter                        *
--*----------+------------+-----------+----------------------------------------*
library ieee;
use     ieee.std_logic_1164.all;
use     ieee.std_logic_arith.all;
use     ieee.std_logic_signed.all;

--------------------------------------------------------------------------------
entity endat22_ip is
  port
  (
        clk               : in  std_logic;
        n_rs              : in  std_logic;

        ------------------------------------
        -- Avalon Slave Interface Port 1
        ------------------------------------
        AVS_S0_ADDRESS    : in  std_logic_vector(7 downto 0);     -- Avalonbus Address
        AVS_S0_READ       : in  std_logic;                                -- Avalonbus Read
        AVS_S0_WRITE      : in  std_logic;                                -- Avalonbus Write
        AVS_S0_WRITEDATA  : in  std_logic_vector(31 downto 0);  -- Avalonbus WriteData

        AVS_S0_READDATA   : out std_logic_vector(31 downto 0);  -- Avalonbus ReadData

        -- Interrupt Request Micro Controller
        n_int1 :             out   std_logic;                     -- Interrupt Request1 to Micro Controller

        -- Mess System
        data_rc :            in    std_logic;
        data_dv :            out   std_logic;
        tclk :               out   std_logic;
        de :                 out   std_logic;
        tclk_de :            out   std_logic;                     -- Drive enable fuer TCLK Tranceiver

        nstr :               in    std_logic;
        ntimer :             out   std_logic;

        n_int6, n_int7 :     in    std_logic;
        clk2:                out   std_logic;
        dui    :             out   std_logic;
        tst_out_pin :        out   std_logic;
        n_si   :             out   std_logic
  );
end entity endat22_ip;
--------------------------------------------------------------------------------
architecture endat22_ip_rtl of endat22_ip is
--------------------------------------------------------------------------------
--******************************************************************************
-- EnDat22 core
--******************************************************************************
  component ENDAT22_S is
      port(
        clk      :           in    std_logic;
        n_rs     :           in    std_logic;

        ------------------------------------
        -- Avalon Slave Interface Port 1
        ------------------------------------
        AVS_S0_ADDRESS    : in  std_logic_vector(7 downto 0);     -- Avalonbus Address
        AVS_S0_READ       : in  std_logic;                                -- Avalonbus Read
        AVS_S0_WRITE      : in  std_logic;                                -- Avalonbus Write
        AVS_S0_WRITEDATA  : in  std_logic_vector(31 downto 0);  -- Avalonbus WriteData

        AVS_S0_READDATA   : out std_logic_vector(31 downto 0);  -- Avalonbus ReadData

        -- Interrupt Request Micro Controller
        n_int1 :             out   std_logic;                     -- Interrupt Request1 to Micro Controller

        -- Mess System
        data_rc :            in    std_logic;
        data_dv :            out   std_logic;
        tclk :               out   std_logic;
        de :                 out   std_logic;
        tclk_de :            out   std_logic;                     -- Drive enable fuer TCLK Tranceiver

        nstr :               in    std_logic;
        ntimer :             out   std_logic;

        n_int6, n_int7 :     in    std_logic;
        clk2:                out   std_logic;
        dui    :             out   std_logic;
        tst_out_pin :        out   std_logic;
        n_si   :             out   std_logic

       );
  end component;
--------------------------------------------------------------------------------
begin

--------------------------------------------------------------------------------
-- EnDat22 Master
--------------------------------------------------------------------------------

  I_EnDat22 : ENDAT22_S
      port map(
        clk           => clk,
        n_rs          => n_rs,

        ------------------------------------
        -- Avalon Slave Interface Port 1
        ------------------------------------
        AVS_S0_ADDRESS    => AVS_S0_ADDRESS,
        AVS_S0_READ       => AVS_S0_READ,
        AVS_S0_WRITE      => AVS_S0_WRITE,
        AVS_S0_WRITEDATA  => AVS_S0_WRITEDATA,

        AVS_S0_READDATA   => AVS_S0_READDATA,

        -- Interrupt Request Micro Controller
        n_int1        => n_int1,          -- Interrupt Request1 to Micro Controller

        -- Mess System
        data_rc       => data_rc,         -- Data from Encoder
        data_dv       => data_dv,         -- Data to  Encoder
        tclk          => tclk,
        de            => de,
        tclk_de       => tclk_de,         -- Drive enable fuer TCLK Tranceiver

        nstr          => nstr,
        ntimer        => ntimer,
        n_int6        => n_int6,
        n_int7        => n_int7,
        clk2          => clk2,
        dui           => dui,
        tst_out_pin   => tst_out_pin,
        n_si          => n_si
    );
--------------------------------------------------------------------------------
END endat22_ip_rtl;
