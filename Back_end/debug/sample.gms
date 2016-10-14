$ONEMPTY 
 
SET i   sibling   /s1*s4/; 
SET j   sibset    /t1*t10/; 
 
TABLE A(j,i)    sets covered by node i 
     	 s1	 s2	 s3	 s4 
t1	 0	 1	 1	 0 
t2	 1	 0	 0	 1 
t3	 1	 1	 0	 0 
t4	 0	 1	 0	 1 
t5	 1	 0	 1	 0 
t6	 0	 0	 1	 1 
t7	 1	 0	 0	 0 
t8	 0	 1	 0	 0 
t9	 0	 0	 1	 0 
t10	 0	 0	 0	 1 
; 
 
$OFFLISTING 
***** Lst File Options ***** 
OPTION solprint=off; 
OPTION sysout=off; 
OPTION limrow=0; 
OPTION limcol=0; 
 
BINARY VARIABLES 
        xset(j)    sibling j 
; 
EQUATIONS 
        setcov(i) 
	      objective 
; 
VARIABLE TF; 
 
setcov(i).. 
sum(j, A(j,i)*xset(j)) =g= 1; 
 
* Objective Function 
objective.. 
TF =e= sum(j, xset(j)); 
 
***** Model Definition ****** 
MODEL setcover / ALL /; 
***** Solution Options ******* 
OPTION lp=cplex; 
OPTION mip=cplex; 
OPTION optcr=0.01; 
OPTION optca=0.1; 
OPTION iterlim=5000; 
OPTION reslim=50; 
option rmip = cplex; 
SOLVE setcover using mip minimizing TF; 
 
DISPLAY xset.l; 
