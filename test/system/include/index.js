/*

author         Oliver Blaser
date           06.04.2021
copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

//#p include [../processor/js/jQuery.js]
//#p include [./js/someFunctions.js]
//#p include [js/abc_\[def\].js]
//#p include [js/empty.js]

$(function()
{
    $('#jsc').html('<div>index.js</div>');
    $('#jsc').append('<div>' + func1() + '</div>');
    $('#jsc').append('<div>5 + 2 = ' + addTwo(5) + '</div>');
    $('#jsc').append('<div>' + funcInSpecialFile() + '</div>');
});
