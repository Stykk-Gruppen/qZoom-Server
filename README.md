# qZoom-Server README
The server side of the qZoom project. It is responsible for routing the data to the correct users and handles SQL queries.
# Dependencies
* `Qt 5.*`
* `libqt5sql5-mysql`
* `mysql-server`

# Installation
We assume you know how to setup a MySQL database.
```
sudo apt install qt5-default
sudo apt install libqt5sql5-mysql
git clone https://github.com/Feqzz/qZoom-Server/
cd qZoom-Server
qmake
make
sudo make install
```

## Configuration
You have to fill in the config file with your database credentials.
```
cd /usr/local/qZoom-Server/config
sudo vim qZoom-Server.conf
```
```
hostname = localhost
databasename =
username =
password =
```


# Documentation
The online documentation is available on this [website](https://tarves.no/docs/qZoom-Server/). Information about the process and more can be found in the client's
[wiki](https://github.com/Feqzz/qZoom-Client/wiki).

# Client
The client can be found [here](https://github.com/Feqzz/qZoom-Client).
