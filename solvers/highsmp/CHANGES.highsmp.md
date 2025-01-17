Summary of recent updates to HiGHS for AMPL
===========================================


## 20230227
- Fixed a problem when retrieving basis status
- Implemented retrieval of MIP gap so that if optimality is reached via presolving,
  the returned gap is 0


## 20230224
- *Updated* to HiGHS 1.5.1; now returning number of LP iterations in a MIP solve


## 20230222
- Fixed: now returning the correct dual values for the constraints


## 20230209
- *Updated* to HiGHS 1.4.2
- Fix for error encountered when passing variables status if variables with unknown 
  basis status are present.


## 20230207
- *Changes in the MP library*


## 20221228
- Changes in MP


## 20221222
- *Updates for HiGHS 1.4.1*
    - The *run_crossover* option has now values "on", "off" and "choose". The latter 
      results in crossover being run if the result of IPM without crossover is imprecise
    - Bug fixes

## 20221211
- *Changes in MP: added the ==> else operator*
   Implemented implication with 'else': *constr1* ==> *constr2* [else *constr3*]   

- *Changes in MP: PLApproxRelTol, PLApproxDomain*
   Parameters to control piecewise-linear approximation.
   cvt:plapprox:reltol default value changed from 1e-5 to 0.01.


## 20221012
- *Piecewise-linear approximation of quadratics*
    HiGHS accepts quadratic objectives.
    For nonconvex ones, set cvt:quadobj=0.
    Recognizing x^2 for stronger univariate approximation


## 20220928
- *Changes in MP*: piecewise-linear approximations of nonlinear functions,
    default value of big-M


## 20220603
- Fixed an issue with passing/retrieving basis information


## 20220524
- First release of HiGHS for AMPL
