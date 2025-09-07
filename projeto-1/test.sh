./jacobi --input_path sistlinear2k.dat --seq
./jacobi --input_path sistlinear2k.dat --num_threads 2 --schedule static --chunk 0
./jacobi --input_path sistlinear2k.dat --num_threads 4 --schedule static --chunk 0
./jacobi --input_path sistlinear2k.dat --num_threads 8 --schedule static --chunk 0
./jacobi --input_path sistlinear2k.dat --num_threads 2 --schedule static --chunk 25
./jacobi --input_path sistlinear2k.dat --num_threads 4 --schedule static --chunk 25
./jacobi --input_path sistlinear2k.dat --num_threads 8 --schedule static --chunk 25
./jacobi --input_path sistlinear2k.dat --num_threads 2 --schedule dynamic --chunk 25
./jacobi --input_path sistlinear2k.dat --num_threads 4 --schedule dynamic --chunk 25
./jacobi --input_path sistlinear2k.dat --num_threads 8 --schedule dynamic --chunk 25
./jacobi --input_path sistlinear2k.dat --num_threads 2 --schedule guided --chunk 25
./jacobi --input_path sistlinear2k.dat --num_threads 4 --schedule guided --chunk 25
./jacobi --input_path sistlinear2k.dat --num_threads 8 --schedule guided --chunk 25
