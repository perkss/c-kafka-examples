# C Kafka Examples


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

TODO fix this now multiple files
Then compile the consumer to an executable name `myconsumer`
```shell script
#FIXME
gcc -Wall -g -c src/consumer.c -o consumer.o -I include/
gcc -Wall -g -c src/UppercaseTopology.c -o UppercaseTopology.o -I include/
ar ruv mylib.a consumer.o UppercaseTopology.o
gcc -Wall -std=c11 -LLibProject/ -lmy -lrdkafka MainProject/src/main.c -o myconsumer
gcc -Wall -std=c11 -LLibProject/ -lmy -lrdkafka MainProject/src/main.c -o myconsumer -I LibProject/include
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
./myconsumer localhost:9091 my-lowercase-consumer lowercase-topic
```

```shell script
docker exec -it kafka-1 kafka-console-producer --broker-list kafka-1:29091 --topic lowercase-topic --property "parse.key=true" --property "key.separator=:"
```

## Useful Docker
Stop and remove all running containers.
```shell script
docker stop $(docker ps -a -q)
docker rm $(docker ps -a -q)
```

