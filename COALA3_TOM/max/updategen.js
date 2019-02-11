inlets = 1;
outlets = 2;

var myval=0;
var freqs = new Array();

if (jsarguments.length>1)
	myval = jsarguments[1];

function bang()
{
	//f = new File("freqs.txt", "read");
	post("coucou bang!\n");
}

function msg_int(v)
{
	post("received int " + v + "\n");
	myval = v;
	bang();
}

function msg_float(v)
{
	post("received float " + v + "\n");
	myval = v;
	bang();
}

function list()
{
	var a = arrayfromargs(arguments);
	post("received list " + a + "\n");
	myval = a;
	bang();
}

function anything()
{
	var a = arrayfromargs(messagename, arguments);
	//post("received message " + a + arguments + "arg size = " + arguments.length + "\n");
	if ( messagename == "updatecode" )
	{
		post("coucou process!\n");
		//var inputfreq = arguments[0];
		//outlet(0, rank);
		//outlet(1, rank);
	    var genh = new File("/usr/local/coala/gen_exported.h", "read");
	    var gencpp = new File("/usr/local/coala/gen_exported.cpp", "read");
		var theUrl = "http://192.168.2.14:10000/?start";
    	var request = new XMLHttpRequest();
		request.open( "POST", theUrl, true );
		
		var text = genh.getAsText("utf-8");
		var bytes = new Uint8Array( text.length );
        for ( var k = 0; k < text.length; ++k )
		{
			
			//var l = f.readline(100);
        	//bytes[k] = text.charCodeAt(k);
		}
		request.send(genh);
	}
}