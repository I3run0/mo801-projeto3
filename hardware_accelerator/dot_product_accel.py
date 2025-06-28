from migen import *
from litex.soc.interconnect.csr import *
from litex.gen.fhdl.module import LiteXModule
import math


class DotProductAccelerator(LiteXModule):
    def __init__(self, input_size=4, data_width=32):
        """
        A simple hardware accelerator for logistic regression dot product:
        output = sum(input_i * weight_i)
        """
        # CSR definitions
        self.input = CSRStorage(fields=[
            CSRField(f"value_{i}", size=data_width) for i in range(input_size)
        ])
        self.weight = CSRStorage(fields=[
            CSRField(f"weight_{i}", size=data_width) for i in range(input_size)
        ])
        self.result = CSRStatus(data_width, name="result")
        
        # Access fields as arrays
        input_array = Array(self.input.fields.fields)
        weight_array = Array(self.weight.fields.fields)

        # Element-wise multiply inputs and weights
        products = [
            Signal(2*data_width, name=f"product_{i}")
            for i in range(input_size)
        ]
        for i in range(input_size):
            self.comb += products[i].eq(input_array[i] * weight_array[i])

        # Build a reduction tree to sum all the products
        def pairwise_sum_level(signals, level):
            next_level = []
            for i in range(0, len(signals), 2):
                s = Signal(data_width*2, name=f"sum_l{level}_{i//2}")
                self.comb += s.eq(signals[i] + signals[i+1])
                next_level.append(s)
            return next_level

        sum_level = products
        level = 0
        while len(sum_level) > 1:
            sum_level = pairwise_sum_level(sum_level, level)
            level += 1

        # Assign the final sum to the output register
        # self.sync += self.result.status.eq(sum_level[0]) TODO: Maybe this is better
        self.comb += self.result.status.eq(sum_level[0][:32])
