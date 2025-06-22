Defina as variaveis de ambiente

Para medir o tempo gasto pela aplicação, foi seguido os passos descritos em: 

    https://github.com/BrunoLevy/learn-fpga/discussions/105

O timer utilizado é definido no arquivo `/home/luis/Estudos/Faculdade/mo801/projeto2/litex/litex/soc/cores/timer.py`. O código utilizado para medir o tempo foi inspirado no código do arquivo `/home/luis/Estudos/Faculdade/mo801/projeto2/litex/litex/soc/software/libbase/memtest.c`. Note que o timer funciona como um countdown, sendo necessário substrair o start do end.

TODO:

[] Descrever processo de build.
[] Escrever como o tempo vai ser medido.
[] Escolher um modelo para acelerar.
[] Medir o tempo sem aceleração.
[] Explicar como o acelerador funciona.
[] Explicar como buildar o SoC com o acelerador.
[] Medir o tempo da aplicação com o acelerador.
[] Comparar o resultado.
