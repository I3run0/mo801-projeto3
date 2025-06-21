from migen import *
from litex.gen import *
from litex.soc.integration.soc_core import *
from litex.soc.integration.soc import SoCRegion
from litex.soc.interconnect.csr import CSRStatus, CSRStorage
from litex.gen.fhdl.module import LiteXModule

class InferenceAccelerator(LiteXModule):
    """
    Hardware accelerator for linear inference: y = x * weight + bias
    Supports both floating point and fixed point operations
    """
    def __init__(self, data_width=32):
        self.data_width = data_width
        
        # CSR Registers
        self.input_data = CSRStorage(data_width, description="Input data (Q16.16 fixed point)")
        self.weight = CSRStorage(data_width, description="Weight coefficient (Q16.16 fixed point)")
        self.bias = CSRStorage(data_width, description="Bias value (Q16.16 fixed point)")
        self.result = CSRStatus(data_width, description="Result output (Q16.16 fixed point)")
        
        # Computation pipeline
        self.mult_result = Signal(64)
        self.add_result = Signal(64)        
        
        # Computation pipeline (combinatorial for simplicity)
        # y = x * weight + bias (all in Q16.16 fixed point format)
        self.comb += [
            # Multiply: input * weight (32x32 -> 64 bit result)
            self.mult_result.eq(self.input_data.storage * self.weight.storage),
            # Add bias (shift multiplication result back to Q16.16)
            self.add_result.eq((self.mult_result >> 16) + self.bias.storage),
            # Final result (clamp to 32 bits)
            self.result.status.eq(self.add_result[:32]),
        ]
        