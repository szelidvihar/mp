#########################################
## pow_03_pow5.mod
#########################################

var x >= -10, <= -2;
var y;

s.t. Pow01: y <= x^5;

s.t. Lin01: x+y <= -3;

maximize Obj01: 0.005*x+y;
