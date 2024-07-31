# pylint: disable=missing-docstring
from pathlib import Path
from vunit import VUnit

root = Path(__file__).resolve().parent
dut_path = root / "src"
test_path = root / "testbench" / "component"

VU = VUnit.from_argv()
VU.add_vhdl_builtins()
VU.enable_location_preprocessing()

design_lib = VU.add_library("design_lib")
design_lib.add_source_files([dut_path / "*.vhd"])

tb_lib = VU.add_library("tb_lib")
tb_lib.add_source_files([test_path / "*.vhd"])

VU.main()
