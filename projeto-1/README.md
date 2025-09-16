# compilando o projeto

apenas execute o script bash

```sh
./test.sh
```


# executando o projeto manualmente
ou, se preferir, compile manualmente com o comando:
```sh 
make
```

e execute com o comando:
```sh
Usage:
./jacobi --input_path <file_path> --schedule <scheduler_method> --chunk <chunk_size> --threads <num_threads>
./jacobi --input_path <file_path> --seq

Additional arguments for parallel mode:
--num_threads      Run parallel version with specified number of threads
--schedule         Scheduling method (static, dynamic, guided)
--chunk_size       Chunk size for scheduling (optional)
```

