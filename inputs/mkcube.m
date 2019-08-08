function mkcube(n,file)

fid = fopen(file, 'w') ; 

[x,y,z] = meshgrid(linspace(-1,1,n)) ;

x = [x(:) y(:) z(:)] ;

[s,i] = sort(rand(n^3,1)) ; i = i(:) ;
x = x(i,:) ;

fprintf(fid, '%d 0 0 0\n', n^3) ;
fprintf(fid, '%f %f %f\n', x') ;

fclose(fid) ;