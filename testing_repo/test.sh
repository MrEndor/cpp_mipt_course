#! /usr/bin/env bash

if [[ $1 == "master" ]]
then 
    echo "No testing on master branch"
elif [[ $1 == "main" ]]
then
    echo "No testing on main branch"
else 
    cp -R testing_repo/$1/. $1/
    cp testing_repo/banned_words_checker.py $1/banned_words_checker.py
    cp testing_repo/valgrind_parser.py $1/valgrind_parser.py
    cd $1
    bash test.sh $1
fi
