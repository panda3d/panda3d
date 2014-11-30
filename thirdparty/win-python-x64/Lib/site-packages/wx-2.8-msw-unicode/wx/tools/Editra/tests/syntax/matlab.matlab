% Syntax Highlight Test File for MatLab
% Some comments about this file

% HelloWorld in MatLab
disp('Hello World');

% And now some other randomness to test different color regions
for j=1:4,
   j
end

A = 1;   B = [];
if(A|B) disp 'The statement is true',  end;

% Plotting Polynomials
x=[27.7 28 29 30];
a=[4.1 4.3 4.1];
b=[0.749 0.503 -0.781];
c=[0.0 -0.819 -0.470];
d=[-0.910 0.116 0.157];

for i=1:3
   ['p_' num2str(i) '(x) = ' num2str(a(i)) ' + ' ...
         num2str(b(i)) ' (x - ' num2str(x(i)) ') + ' ...
         num2str(c(i)) ' (x - ' num2str(x(i)) ')^2 + ' ...
         num2str(d(i)) ' (x - ' num2str(x(i)) ')^3']
end;

%---------------------------------------------------------------------
function y = nev(xx,n,x,Q)
% NEV   Neville's algorithm as a function
%       y= nev(xx,n,x,Q)
%
% inputs:
%    n = order of interpolation (n+1 = # of points)
%    x(1),...,x(n+1)    x coords
%    Q(1),...,Q(n+1)    y coords
%    xx=evaluation point for interpolating polynomial p
%
% output:  p(xx)
for i = n:-1:1
   for j = 1:i
      Q(j) = (xx-x(j))*Q(j+1) - (xx-x(j+n+1-i))*Q(j);
      Q(j) = Q(j)/(x(j+n+1-i)-x(j));
   end
end

y = Q(1);

%---------------------------------------------------------------------
function ssum = geom(a,N)
  n=0:N;
  ssum = sum(a.^n);
end
