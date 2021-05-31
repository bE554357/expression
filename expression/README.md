# About

C++ code for differentiation of tensor-argument functions.

## Details

All variable are distinguished only by a symbol, but not a type (only 256 different variables), i.e.: in

	variable<matrix, 'A'> var1;
	variable<matrix, 'A'> var2;
  
`var1` and `var2` are equivalent, because both are 'A'.

diff is a GATEAUX differential:
`d.diff(b, c)` is the same like (\partial d)/(\partial b):db (with db = c)

Now, you cant write like this: 3.141*some_expression. You shall write like this: constant<double>{3.141}*some_expression

All available at this moment operations are presented in binary_expression.h and unary_expression.h
