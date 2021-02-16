/*

author         Oliver Blaser
date           15.02.2021
copyright      GNU GPLv3 - Copyright (c) 2021 Oliver Blaser

*/

function multiLineInsert(data)
{
    let html = '<div class="tbi">';
    html += data;
    html += '</div>';

    $('#jsc').append(html);

    $('#jsc').prepend('<div class="tbi"><div style="border: 1px solid black; border-radius: 3px; display: inline-block; padding: 0px 5px;">inserted before</div></div>');
}

// HTTP GET work around
function getWA(str, cb) { cb(str); }

$(function()
{
    let html = '<div>';

//#p rm

    html += '<div class="tbr">';

    //#p rmn 1
    html += '<div>rmn</div>';
    html += 'bli bla blop';

    //#p ins:alert('should never occur');

    html += '</div>';
    
//#p endrm

    html += '<div>this text stays</div>';
    html += '</div>';

    $('#jsc').html(html);

    getWA('just some text\nand another line', function(res)
    //$.get('data.txt', function(res) // well, GET requests are a HTTP thing so they dont work for filesystem access.
    {
        console.log('GET response:');
        console.log(res);

        res = res.replace('\n', '<br/>');

        //#p rmn 1
        $('#jsc').append('<div class="tbr">gets also removed<p>' + res + '</p></div>');
        $('#jsc').append('<p>the requested data will be inserted here:</p>');
        //#p ins:multiLineInsert(res);
    });
});
