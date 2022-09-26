#########################################
## asin_01.mod
#########################################

param pi := 4*atan(1);  ## 3.14159265358979;

var x >= -5, <= 1;
var y;

s.t. Sin01: y == atan(x);

s.t. Lin01: -x-y <= 1;

maximize SumKXY: -13*x-y;
