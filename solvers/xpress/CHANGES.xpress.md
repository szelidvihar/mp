Summary of recent updates to Xpress for AMPL
============================================


## 20230227
- Eliminated warning message when a non feasible solution is added as a starting
  point for the MIP search


## 20230207
- *Changes in the MP library*


## 20221228
- Changes in MP


## 20221222
- Bug fixes in MP


## 20221211
- *Changes in MP: added the ==> else operator*
   Implemented implication with 'else': *constr1* ==> *constr2* [else *constr3*]   

- *Changes in MP: PLApproxRelTol, PLApproxDomain*
   Parameters to control piecewise-linear approximation.
   cvt:plapprox:reltol default value changed from 1e-5 to 0.01.


## 20221208
- First release of the MP-based driver for Xpress (solver version 9.0)
