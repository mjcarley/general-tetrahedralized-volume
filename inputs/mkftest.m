function mkftest(n,file)

fid = fopen(file, 'w') ; 

x = rand(n,3)-0.5 ;

fprintf(fid, '%d 0 0 0\n', n) ;
fprintf(fid, '%f %f %f\n', x') ;

fclose(fid) ;