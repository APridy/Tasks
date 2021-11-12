#!/bin/bash

content=(one two three four five)
mkdir -p ./testdir
mkdir -p ./testdir/5
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/5/6.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/5/\ 50.txt
mkdir -p ./testdir/3
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/3/4.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/3/3.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/3/5.txt
mkdir -p ./testdir/1.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/1.txt/4.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/1.txt/1.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/1.txt/3.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/1.txt/5.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/1.txt/6.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/1.txt/8.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/1.txt/2.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/1.txt/7.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/1.txt/9.txt
mkdir -p ./testdir/4
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/4/1\'.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/4/9\ 1.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/4/\ 50.txt
mkdir -p ./testdir/7
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/7/8.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/7/7.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/7/9.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/7/1\'.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/7/9\ 1.txt
mkdir -p ./testdir/6
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/6/3.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/6/5.txt
mkdir -p ./testdir/2
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/2/3.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/2/2.txt
mkdir -p ./testdir/2\ 1
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/2\ 1/3.txt
echo ${content[$(( $RANDOM % ${#content[*]} ))]} > ./testdir/2\ 1/2.txt
