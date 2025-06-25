# LiteX Logistic Regression Accelerator (simplified template)

from migen import *
from litex.soc.interconnect.csr import *
from litex.gen.fhdl.module import LiteXModule

class LogisticRegressionAccelerator(LiteXModule):
    def __init__(self, input_size=16, data_width=8):  # Reduced from 64 to 16 inputs
        self.input_size = input_size
        self.data_width = data_width
        
        # For 8-bit inputs: 8+8=16 bits per multiply, +4 bits for 16 inputs = 20 bits max
        sum_width = 2 * data_width + log2_int(input_size)

        # CSR Interface - pack multiple 8-bit values into 32-bit CSRs to save resources
        self.start = CSR()
        self.done = CSRStatus()
        
        # Pack 4 inputs per 32-bit CSR (4 * 8 = 32 bits)
        inputs_per_csr = 4
        num_input_csrs = (input_size + inputs_per_csr - 1) // inputs_per_csr
        
        self.input_csrs = Array([CSRStorage(32, name=f"inputs_{i}") for i in range(num_input_csrs)])
        self.weight_csrs = Array([CSRStorage(32, name=f"weights_{i}") for i in range(num_input_csrs)])
        self.bias = CSRStorage(8, name="bias")
        self.result = CSRStatus(16, name="result")  # 16-bit result

        # Internal signals
        self.processing = Signal()
        
        # Extract individual 8-bit values from packed CSRs
        inputs = Array([Signal((8, True)) for _ in range(input_size)])
        weights = Array([Signal((8, True)) for _ in range(input_size)])
        
        # Unpack CSRs into individual 8-bit signals
        for i in range(input_size):
            csr_idx = i // inputs_per_csr
            bit_offset = (i % inputs_per_csr) * 8
            self.comb += [
                inputs[i].eq(self.input_csrs[csr_idx].storage[bit_offset:bit_offset+8]),
                weights[i].eq(self.weight_csrs[csr_idx].storage[bit_offset:bit_offset+8])
            ]
        
        # Create multiply terms (8x8=16 bit results)
        multiply_terms = Array([Signal((16, True)) for _ in range(input_size)])
        
        # Parallel multiplication
        for i in range(input_size):
            self.comb += multiply_terms[i].eq(inputs[i] * weights[i])
        
        # Simplified adder tree for small input size
        # For 16 inputs, create a 4-level tree: 16->8->4->2->1
        if input_size <= 16:
            # Level 1: 16 -> 8 (pair-wise addition)
            level1 = Array([Signal((17, True)) for _ in range(8)])
            for i in range(8):
                if 2*i+1 < input_size:
                    self.comb += level1[i].eq(multiply_terms[2*i] + multiply_terms[2*i+1])
                else:
                    self.comb += level1[i].eq(multiply_terms[2*i])
            
            # Level 2: 8 -> 4
            level2 = Array([Signal((18, True)) for _ in range(4)])
            for i in range(4):
                self.comb += level2[i].eq(level1[2*i] + level1[2*i+1])
            
            # Level 3: 4 -> 2
            level3 = Array([Signal((19, True)) for _ in range(2)])
            for i in range(2):
                self.comb += level3[i].eq(level2[2*i] + level2[2*i+1])
            
            # Level 4: 2 -> 1
            mac_sum = Signal((20, True))
            self.comb += mac_sum.eq(level3[0] + level3[1])
        else:
            # Fallback for larger sizes (shouldn't happen with input_size=16)
            mac_sum = Signal((sum_width, True))
            self.comb += mac_sum.eq(sum(multiply_terms))
        
        # Add bias and create final result
        final_result = Signal((20, True))
        self.comb += final_result.eq(mac_sum + self.bias.storage)

        # Simple 2-state FSM
        self.submodules.fsm = fsm = FSM(reset_state="IDLE")
        
        fsm.act("IDLE",
            If(self.start.re,
                NextValue(self.processing, 1),
                NextState("COMPUTE")
            )
        )
        
        fsm.act("COMPUTE",
            # Saturate to 16-bit signed result
            NextValue(self.result.status, final_result[4:20]),  # Take upper 16 bits (scale down by 16)
            NextValue(self.processing, 0),
            NextState("IDLE")
        )
        
        self.comb += self.done.status.eq(~self.processing)
