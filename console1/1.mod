param n;
param p;
param r {i in 1..n};
param w {i in 1..n};

param d:=n*p+(max{i in 1..n} r[i]);

var x {i in 1..n , k in 1..d } >=0;
var y {i in 1..n , k in 1..d } >=0;

minimize f: sum{ i in 1..n } (  w[i] * (1 + sum {k in 1..d} y[i,k]) );

s.t. WorkDone  {i in 1..n} : sum{ k in 1..n*p } x[i,k] = p;
s.t. OneInRow  {k in 1..d} : sum{ i in 1..n } x[i,k] <=1;
s.t. Wait      {i in 1..n, k in 1..r[i]} : x[i,k] = 0;
s.t. cn_y1     {i in 1..n, k in 1..d-1} : sum{ l in k+1..d } x[i,l] <= p*y[i,k];
s.t. cn_y2     {i in 1..n, k in 1..d-1} : sum{ l in k+1..d } x[i,l] >= y[i,k];

