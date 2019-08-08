lc = 0.1 ;
Point(1) = {-1,-1,-1,lc};
Point(2) = {1,-1,-1,lc};
Point(3) = {1,1,-1,lc};
Point(4) = {-1,1,-1,lc};
Line(1) = {4,3};
Line(2) = {2,2};
Line(3) = {2,3};
Line(4) = {4,1};
Line(5) = {1,2};
Line Loop(6) = {4,5,3,-1};
Plane Surface(7) = {6};
Extrude {0,0,2} {
  Surface{7};
}
