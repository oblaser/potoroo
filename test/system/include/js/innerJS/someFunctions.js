/*

author         Oliver Blaser
date           06.04.2021
copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

// //#p include 'someFunctions.js'
// //#p include "someFunctions.js"

// //#p include 'a.js'
//#p include "a.js"

function func1()
{
    //#p rmn 2
    alert('someFunctions.js not preprocessed!');

    return 'this was func1 and ' + a_func();
}

function addTwo(x)
{
    return x + 2;
}

// to test errors in included file:
// //#p rm
