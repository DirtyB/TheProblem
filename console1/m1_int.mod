param n;
param p;
param r {i in 1..n};
param w {i in 1..n};

param d:=n*p+(max{i in 1..n} r[i]);

var x {i in 1..n , k in 1..d }, binary;
var y {i in 1..n , k in 1..1 };

minimize f: sum{ i in 1..n } w[i]*y[i,1];

s.t. OneInRow {k in 1..d} : sum{ i in 1..n} x[i,k] <=1;
s.t. WorkDone {i in 1..n} : sum{ k in 1..n*p} x[i,k] = p;
s.t. Wait     {i in 1..n, k in 1..r[i]} : x[i,k] = 0;
s.t. Profit   {i in 1..n, k in 1..d}    : y[i,1] >= k*x[i,k] ;

