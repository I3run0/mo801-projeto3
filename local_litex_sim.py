import sys
import argparse

from migen import *

from litex.build.generic_platform import *
from litex.build.sim              import SimPlatform
from litex.build.sim.config       import SimConfig

from litex.soc.integration.common   import *
from litex.soc.integration.soc_core import *
from litex.soc.integration.builder  import *
from litex.soc.integration.soc      import *
from litex.soc.cores.bitbang import *

from litedram.modules   import parse_spd_hexdump

from litex.tools.litex_sim  import sim_args
from litex.tools.litex_sim  import SimSoC
from litex.tools.litex_sim import generate_gtkw_savefile

from inference_accelerator import InferenceAccelerator

class LocalSimSoc(SimSoC):
    def __init__(self,
        with_sdram             = False,
        with_sdram_bist        = False,
        with_ethernet          = False,
        ethernet_phy_model     = "sim",
        ethernet_local_ip      = "192.168.1.50",
        ethernet_remote_ip     = "192.168.1.100",
        with_etherbone         = False,
        with_analyzer          = False,
        sdram_module           = "MT48LC16M16",
        sdram_init             = [],
        sdram_data_width       = 32,
        sdram_spd_data         = None,
        sdram_verbosity        = 0,
        with_i2c               = False,
        with_sdcard            = False,
        with_spi_flash         = False,
        spi_flash_init         = [],
        with_gpio              = False,
        with_video_framebuffer = False,
        with_video_terminal    = False,
        with_video_colorbars   = False,
        sim_debug              = False,
        trace_reset_on         = False,
        with_jtag              = False,
        **kwargs):
        SimSoC.__init__(self,
            with_sdram,
            with_sdram_bist,
            with_ethernet,
            ethernet_phy_model,
            ethernet_local_ip,
            ethernet_remote_ip,
            with_etherbone,
            with_analyzer,
            sdram_module,
            sdram_init,
            sdram_data_width,
            sdram_spd_data,
            sdram_verbosity,
            with_i2c,
            with_sdcard,
            with_spi_flash,
            spi_flash_init,
            with_gpio,
            with_video_framebuffer,
            with_video_terminal,
            with_video_colorbars,
            sim_debug,
            trace_reset_on,
            with_jtag,
            **kwargs
        )

        self.inference_accel = InferenceAccelerator()


def main():
    from litex.build.parser import LiteXArgumentParser
    parser = LiteXArgumentParser(description="LiteX SoC Simulation utility")
    parser.set_platform(SimPlatform)
    sim_args(parser)
    args = parser.parse_args()

    soc_kwargs = soc_core_argdict(args)

    sys_clk_freq = int(1e6)
    sim_config   = SimConfig()
    sim_config.add_clocker("sys_clk", freq_hz=sys_clk_freq)

    # Configuration --------------------------------------------------------------------------------

    # UART.
    if soc_kwargs["uart_name"] == "serial":
        soc_kwargs["uart_name"] = "sim"
        sim_config.add_module("serial2console", "serial")

    # Create config SoC that will be used to prepare/configure real one.
    conf_soc = SimSoC(**soc_kwargs)

    # ROM.
    if args.rom_init:
        soc_kwargs["integrated_rom_init"] = get_mem_data(args.rom_init,
            data_width = conf_soc.bus.data_width,
            endianness = conf_soc.cpu.endianness
        )

    # RAM / SDRAM.
    ram_boot_address = None
    soc_kwargs["integrated_main_ram_size"] = args.integrated_main_ram_size
    if args.integrated_main_ram_size:
        if args.ram_init is not None:
            soc_kwargs["integrated_main_ram_init"] = get_mem_data(args.ram_init,
                data_width = conf_soc.bus.data_width,
                endianness = conf_soc.cpu.endianness,
                offset     = conf_soc.mem_map["main_ram"]
            )
            ram_boot_address = get_boot_address(args.ram_init)
    elif args.with_sdram:
        assert args.ram_init is None
        soc_kwargs["sdram_module"]     = args.sdram_module
        soc_kwargs["sdram_data_width"] = int(args.sdram_data_width)
        soc_kwargs["sdram_verbosity"]  = int(args.sdram_verbosity)
        if args.sdram_from_spd_dump:
            soc_kwargs["sdram_spd_data"] = parse_spd_hexdump(args.sdram_from_spd_dump)
        if args.sdram_init is not None:
            soc_kwargs["sdram_init"] = get_mem_data(args.sdram_init,
                data_width = conf_soc.bus.data_width,
                endianness = conf_soc.cpu.endianness,
                offset     = conf_soc.mem_map["main_ram"]
            )
            ram_boot_address = get_boot_address(args.sdram_init)

    # Ethernet.
    if args.with_ethernet or args.with_etherbone:
        if args.ethernet_phy_model == "sim":
            sim_config.add_module("ethernet", "eth", args={"interface": "tap0", "ip": args.remote_ip})
        elif args.ethernet_phy_model == "xgmii":
            sim_config.add_module("xgmii_ethernet", "xgmii_eth", args={"interface": "tap0", "ip": args.remote_ip})
        elif args.ethernet_phy_model == "gmii":
            sim_config.add_module("gmii_ethernet", "gmii_eth", args={"interface": "tap0", "ip": args.remote_ip})
        else:
            raise ValueError("Unknown Ethernet PHY model: " + args.ethernet_phy_model)

    # I2C.
    if args.with_i2c:
        sim_config.add_module("spdeeprom", "i2c")

    # JTAG
    if args.with_jtagremote:
        sim_config.add_module("jtagremote", "jtag", args={'port': 44853})

    # Video.
    if args.with_video_framebuffer or args.with_video_terminal or args.with_video_colorbars:
        sim_config.add_module("video", "vga", args={"render_on_vsync": args.video_vsync})

    # SoC ------------------------------------------------------------------------------------------
    soc = LocalSimSoc(
        with_sdram             = args.with_sdram,
        with_sdram_bist        = args.with_sdram_bist,
        with_ethernet          = args.with_ethernet,
        ethernet_phy_model     = args.ethernet_phy_model,
        ethernet_local_ip      = args.local_ip,
        ethernet_remote_ip     = args.remote_ip,
        with_etherbone         = args.with_etherbone,
        with_analyzer          = args.with_analyzer,
        with_i2c               = args.with_i2c,
        with_jtag              = args.with_jtagremote,
        with_sdcard            = args.with_sdcard,
        with_spi_flash         = args.with_spi_flash,
        with_gpio              = args.with_gpio,
        with_video_framebuffer = args.with_video_framebuffer,
        with_video_terminal    = args.with_video_terminal,
        with_video_colorbars   = args.with_video_colorbars,
        sim_debug              = args.sim_debug,
        trace_reset_on         = int(float(args.trace_start)) > 0 or int(float(args.trace_end)) > 0,
        spi_flash_init         = None if args.spi_flash_init is None else get_mem_data(args.spi_flash_init, endianness="big"),
        **soc_kwargs)
    if ram_boot_address is not None:
        if ram_boot_address == 0:
            ram_boot_address = conf_soc.mem_map["main_ram"]
        soc.add_constant("ROM_BOOT_ADDRESS", ram_boot_address)
    if args.with_ethernet and (not args.with_etherbone): # FIXME: Remove.
        for i in range(4):
            soc.add_constant("LOCALIP{}".format(i+1), int(args.local_ip.split(".")[i]))
        for i in range(4):
            soc.add_constant("REMOTEIP{}".format(i+1), int(args.remote_ip.split(".")[i]))

    # Build/Run ------------------------------------------------------------------------------------
    def pre_run_callback(vns):
        if args.trace:
            generate_gtkw_savefile(builder, vns, args.trace_fst)

    builder = Builder(soc, **parser.builder_argdict)
    builder.build(
        sim_config       = sim_config,
        interactive      = not args.non_interactive,
        video            = args.with_video_framebuffer or args.with_video_terminal or args.with_video_colorbars,
        pre_run_callback = pre_run_callback,
        **parser.toolchain_argdict,
    )

if __name__ == "__main__":
    main()