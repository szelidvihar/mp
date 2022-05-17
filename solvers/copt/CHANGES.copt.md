Summary of recent updates to COPT for AMPL
==========================================

## 20220511
- *Complementarity constraints: also quadratics*
    Complementarity constraints now handle quadratics.

- *Branch develop is used for new code*
    The active development branch is now *develop*.

- *Convert quadratic range constraints to QuadCon(LE/EQ/GE)*
    COPT does not support quadratic range constraints.
    Conversion of linear range constraints into one-side rhs
    constraints has been generalized for any algebraic ones.
    

### 20220411
- First mp-based release, linked with COPT libraries 4.0.5