from ctypes import cdll
from pathlib import Path

def demo(filename):
    filepath = Path(__file__).parent
    filepath = filepath / "models" / filename
    filepath_str = (str(filepath)).encode()
    
    lib = cdll.LoadLibrary('build/libnumerical_analysis_library.so')

    lib.run_demo(filepath_str)

demo('car.frame')