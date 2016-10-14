
exename=PKin-local

populationpath=juv_salmon_large
numthreads=4
numlogicalthreads=4
stacksizekb=1
reconalg=1
lociclusters=4
outfile=results.gms

./$exename $populationpath $numthreads $numlogicalthreads $stacksizekb $reconalg $lociclusters $outfile

populationpath=juv_salmon_large        
numthreads=3
numlogicalthreads=4
stacksizekb=1
reconalg=1
lociclusters=4
outfile=results.gms         

./$exename $populationpath $numthreads $numlogicalthreads $stacksizekb $reconalg $lociclusters $outfile

populationpath=juv_salmon_large        
numthreads=2
numlogicalthreads=4
stacksizekb=1
reconalg=1
lociclusters=4
outfile=results.gms         

./$exename $populationpath $numthreads $numlogicalthreads $stacksizekb $reconalg $lociclusters $outfile  

populationpath=juv_salmon_large        
numthreads=1
numlogicalthreads=4
stacksizekb=1
reconalg=1
lociclusters=4
outfile=results.gms         

./$exename $populationpath $numthreads $numlogicalthreads $stacksizekb $reconalg $lociclusters $outfile
