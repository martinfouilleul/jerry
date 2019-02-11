inlets = 1;
outlets = 1;


if (jsarguments.length>1)
	path = jsarguments[1];

function bang(){}

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

function cleanup( s )
{
	return s;
 	while ( true )
	{
		var i = s.search("<<<");
		if ( i == -1 )
			break;
		outlet(1,i);
		break;
		var k = s.search(">>>");
		if ( k>i)
		{
			s = s.substr(0,i) + s.substr(k+2);
			//outlet(1,k);
		}
		else break;
	}
	return s;
	//s.replaceAll(" ", "@032");
	s = s.split(' ').join("@032");
	s=s.replaceAll("#", "@035");
	s=s.replaceAll("*", "@042");
	s=s.replaceAll(",", "@044");
	s=s.replaceAll("/", "@047");
	s=s.replaceAll("?", "@063");
	s=s.replaceAll("[", "@091");
	s=s.replaceAll("]", "@093");
	s=s.replaceAll("{", "@123");
	s=s.replaceAll("}", "@125");
	return s;
	/*' ' 	space 	32
# 	number sign 	35
* 	asterisk 	42
, 	comma 	44
/ 	forward slash 	47
? 	question mark 	63
[ 	open bracket 	91
] 	close bracket 	93
{ 	open curly brace 	123
} 	close curly brace 	125
*/
}

function anything()
{
	var a = arrayfromargs(messagename, arguments);
	post("received message " + a + arguments + "arg size = " + arguments.length + "\n");
	if ( arguments.length == 1 )
	{
		var content = arguments[0];
		if ( content != "" )
		{
			content = cleanup( content )
			outlet(0,content);
		}
	}
}