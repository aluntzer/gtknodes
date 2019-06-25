
# building

run make


# C

You can find a sample layout for the C example in  c_binary_adder.xml

It is an 8-bit binary adder with a 1-bit overflow indicator. A pulse generator
functions as a clock driver and causes iteration over two range steppers, which
are essentially a representation of a for-loop. The floating output of the
steppers is converted to an 8-bit output and decoded into its individual bits
for input into the adder. The first attached encoder converts the lower
8 bit-portion of the added result back into a decimal number, the second
encoder represents the status of the carry/overflow bit of the last binary adder,
making the output effectively 9 bits wide.


# Python

A set of nodes using NumPy, SciPy and Matplotlib representing an image
filter/manipulation chain is found in img.py and py_img_proc.xml
respectively. Note that in the python example, the load/store file argument
is hardcoded, because I'm a lazy bum.

Note: if you did not install the library earlier, you will have to
setup LD_LIBARY_PATH and GI_TYPELIB_PATHS yourself.
