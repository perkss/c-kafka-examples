# C Kafka Examples

Mixing the old with the new. Kafka examples with C using [RDKafka](https://github.com/edenhill/librdkafka).

### MAC OS Installation
1) Install the dependent library for Kafka:
```shell script
brew install librdkafka
```
This will install into the `/usr/local/include` and `/usr/local/lib` which we will include in CMake to find the header files.

Check the headers are installed
```shell script
pkg-config --cflags rdkafka
```
Check the libs are intalled
```shell script
pkg-config --libs rdkafka
```

### Option 1 CMAKE Build
```shell script
mkdir build && cd build   
cmake ..
make
```

```shell script
 ./MainProject/src/MainProject localhost:9091 my-lowercase-consumer lowercase-topic
```

### Option 2 Command Build

[Lib File Mac Ref](https://stackoverflow.com/questions/3532589/how-to-build-a-dylib-from-several-o-in-mac-os-x-using-gcc)
[Header File Library](https://stackoverflow.com/questions/58334781/mac-dylib-linking-cannot-find-header)

```shell script
# Create object files for the Library files
gcc -Wall -g -c src/consumer.c -o consumer.o -I include/
gcc -Wall -g -c src/UppercaseTopology.c -o UppercaseTopology.o -I include/
# Todo UNIX?
ar ruv mylib.a consumer.o UppercaseTopology.o
# Create Mac Shared Lib
gcc -dynamiclib -undefined suppress -flat_namespace consumer.o UppercaseTopology.o -o libmyProject.dylib

# Need to be in the directory where the .dylib file is 
gcc -Wall -std=c11 -L/Users/Stuart/Documents/Programming/C_Programming/c-kafka-examples/LibProject/ -lmyProject -lrdkafka MainProject/src/main.c -o myconsumer -I LibProject/include

```
Note we found the `rdkafka` library and linked as its on the usual path we can search for it using 

```shell script
pkg-config --libs rdkafka
```

3) Start up Kafka.

```shell script
docker exec kafka-1 kafka-topics --create --zookeeper zookeeper-1:22181 --replication-factor 1 --partitions 1 --topic lowercase-topic
```

```shell script
docker exec kafka-1 kafka-topics --zookeeper zookeeper-1:22181 --list
```

```shell script
docker exec kafka-1 kafka-topics --create --zookeeper zookeeper-1:22181 --replication-factor 1 --partitions 1 --topic uppercase-topic
```

```shell script
./myconsumer localhost:9091 my-lowercase-consumer lowercase-topic
```

```shell script
docker exec -it kafka-1 kafka-console-producer --broker-list kafka-1:29091 --topic lowercase-topic --property "parse.key=true" --property "key.separator=:"
```

```shell script
docker exec kafka-1 kafka-console-consumer --bootstrap-server kafka-1:29091 --topic uppercase-topic --property print.key=true --property key.separator="-" --from-beginning
```


## Useful Docker
Stop and remove all running containers.
```shell script
docker stop $(docker ps -a -q)
docker rm $(docker ps -a -q)
```

## TODO
* Avro example

