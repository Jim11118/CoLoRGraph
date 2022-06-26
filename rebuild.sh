#!/bin/bash


ps -ef|grep FourParty|grep -v grep|cut -c 9-15|xargs kill -9

