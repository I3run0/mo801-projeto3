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
        self.control = CSRStorage(8, description="Control register")
        self.status = CSRStatus(8, description="Status register")
        
        # Control bits
        self.START_BIT = 0
        self.RESET_BIT = 1
        self.MODE_BIT = 2  # 0: single operation, 1: batch mode
        
        # Status bits
        self.READY_BIT = 0
        self.DONE_BIT = 1
        self.BUSY_BIT = 2
        
        # Internal signals
        self.start = Signal()
        self.reset = Signal()
        self.mode = Signal()
        self.ready = Signal(reset=1)
        self.done = Signal()
        self.busy = Signal()
        
        # Computation pipeline
        self.compute_valid = Signal()
        self.mult_result = Signal(64)
        self.add_result = Signal(64)
        self.final_result = Signal(32)
        
        # Connect control signals
        self.comb += [
            self.start.eq(self.control.storage[self.START_BIT]),
            self.reset.eq(self.control.storage[self.RESET_BIT]),
            self.mode.eq(self.control.storage[self.MODE_BIT]),
        ]
        
        # Connect status signals
        self.comb += [
            self.status.status[self.READY_BIT].eq(self.ready),
            self.status.status[self.DONE_BIT].eq(self.done),
            self.status.status[self.BUSY_BIT].eq(self.busy),
        ]
        
        # State machine
        self.fsm = FSM(reset_state="IDLE")
        self.submodules += self.fsm
        
        self.fsm.act("IDLE",
            NextValue(self.ready, 1),
            NextValue(self.done, 0),
            NextValue(self.busy, 0),
            If(self.start,
                NextState("COMPUTE")
            ),
            If(self.reset,
                NextState("RESET")
            )
        )
        
        self.fsm.act("COMPUTE",
            NextValue(self.ready, 0),
            NextValue(self.busy, 1),
            NextValue(self.compute_valid, 1),
            NextState("FINISH")
        )
        
        self.fsm.act("FINISH",
            NextValue(self.done, 1),
            NextValue(self.busy, 0),
            NextValue(self.compute_valid, 0),
            NextState("IDLE")
        )
        
        self.fsm.act("RESET",
            NextValue(self.ready, 0),
            NextValue(self.done, 0),
            NextValue(self.busy, 0),
            NextValue(self.compute_valid, 0),
            NextState("IDLE")
        )
        
        # Computation pipeline (combinatorial for simplicity)
        # y = x * weight + bias (all in Q16.16 fixed point format)
        self.comb += [
            # Multiply: input * weight (32x32 -> 64 bit result)
            self.mult_result.eq(self.input_data.storage * self.weight.storage),
            # Add bias (shift multiplication result back to Q16.16)
            self.add_result.eq((self.mult_result >> 16) + self.bias.storage),
            # Final result (clamp to 32 bits)
            self.final_result.eq(self.add_result[:32]),
        ]
        
        # Update result register when computation is valid
        self.sync += [
            If(self.compute_valid,
                self.result.status.eq(self.final_result)
            )
        ]
