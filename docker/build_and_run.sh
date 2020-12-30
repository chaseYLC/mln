#!/bin/bash

sudo docker build --no-cache --pull -t mlnserver_image .
sudo docker run -d -p 28282:28282 --name mlnserver mlnserver_image

