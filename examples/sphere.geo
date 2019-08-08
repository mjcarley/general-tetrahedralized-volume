lc = 0.05 ;
R = 1.0003 ;
Point(1) = {0,0,0,lc};
Point(2) = {0,0,R,lc};
Point(3) = {0,0,-R,lc};
Point(4) = {0,R,0,lc};
Circle(2) = {2,1,4};
Circle(3) = {4,1,3};
Line(4) = {3,2};
Line Loop(5) = {2,3,4};
Plane Surface(6) = {5};
Extrude {{0,0,1}, {0,0,0}, Pi/2} {
  Surface{6};
}
Extrude {{0,0,1}, {0,0,0}, Pi/2} {
  Surface{18};
}
Extrude {{0,0,1}, {0,0,0}, Pi/2} {
  Surface{30};
}
Extrude {{0,0,1}, {0,0,0}, Pi/2} {
  Surface{42};
}
