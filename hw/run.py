# pylint: disable=missing-docstring
import os
from vunit import VUnit

script_dir = os.path.dirname(__file__)
src_dir = f"{script_dir}/src"  # pylint: disable=C0103
test_dir = f"{script_dir}/testbench/component"  # pylint: disable=C0103

VU = VUnit.from_argv()
VU.add_vhdl_builtins()
VU.enable_location_preprocessing()

design_lib = VU.add_library("design_lib")
design_lib.add_source_files([os.path.join(src_dir, "*.vhd")])

tb_lib = VU.add_library("tb_lib")
tb_lib.add_source_files([os.path.join(test_dir, "*.vhd")])

VU.main()
