var responseData = [];
var transferFunction = null;
var _running = false;
var _elapsed = 0.;
var _timerPeriod = 200;
var _controlTime = 0.;
var _wasDown = false;
var _init = false;
var _params = [];

function onStartup()
{
   var myVar = setInterval(function () {onMainTimer()}, _timerPeriod);
   setupCoalaTime();
}

function setupCoalaTime()
{
  var now = new Date();
  var date = now.toLocaleString('en');
  date = date.replace( /,/g, "" );
  var theUrl = "?date=" + date;
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.open( "GET", theUrl, true );
  xmlHttp.send();
  return xmlHttp.responseText;
}

function onMainTimer()
{
  getCoalaStatus();
  
  var timeslider = document.getElementById("timeslider");
  var timeval = document.getElementById("timeval");
  
  if (timeslider.value >= 60.5 )
  {
      timeval.innerHTML = '∞';
  }
  
  if ( _running && _elapsed >= 0 )
  {
    if ( _controlTime == 0. )
    {
      _controlTime = _params["time_limit"];
      _elapsed = _controlTime;
    }
    var hr = Math.floor(_elapsed/3600);
    var min = Math.floor((_elapsed-hr*3600)/60);
    var sec = Math.round(_elapsed-hr*3600-min*60);
    var disp = hr + ":" + ("00" + min).slice (-2) + ":" + ("00" + sec).slice (-2);
    document.getElementById("elapsed").innerHTML = disp;
    _elapsed -= _timerPeriod*1.0e-3;
  }
  else
  {
    _controlTime = 0;
    document.getElementById("elapsed").innerHTML = "0:00:00";
  }
  
  
  var bypasscheckbox = document.getElementById("bypasscheckbox");
  var mainwrapper = document.getElementById("controlwrapper");
  if ( bypasscheckbox.checked )
    mainwrapper.classList.add("disabledcontent");
  else
    mainwrapper.classList.remove("disabledcontent");
    
  timeslider.disabled = _running;
  timeslider.style.opacity = _running?0.4:1.0;
  
  var fadeslider = document.getElementById("fadeslider");
  fadeslider.disabled = _running;
  fadeslider.style.opacity = _running?0.4:1.0;
    
  var mainwrapper = document.getElementById("mainwrapper");
  if ( _wasDown )
    mainwrapper.classList.add("disabledcontent");
  else
    mainwrapper.classList.remove("disabledcontent");
    
  var modgainslider = document.getElementById("modulatedgainfreqslider");
  var modgainchck = document.getElementById("modulatedgaincheckbox");
  modgainslider.disabled = !modgainchck.checked;
  modgainslider.style.opacity = modgainchck.checked?1.0:0.4;
  
  var chirpchck = document.getElementById("chirpcheckbox");
  var recordchck = document.getElementById("recorddatacheckbox");
  var chirpbegin = document.getElementById("chirpbeginslider");
  var chirpend = document.getElementById("chirpendslider");
  recordchck.disabled = chirpchck.checked;
  chirpbegin.disabled = !chirpchck.checked;
  chirpend.disabled = !chirpchck.checked;
  chirpbegin.style.opacity = chirpchck.checked?1.0:0.4;
  chirpend.style.opacity = chirpchck.checked?1.0:0.4;

  var rtchck = document.getElementById("realtimecheckbox");
  var sharechck = document.getElementById("sharetimecheckbox");
  sharechck.disabled = !rtchck.checked;

  var autoperiodchck = document.getElementById("autoperiodcheckbox");
  var periodslider = document.getElementById("periodslider");
  periodslider.disabled = autoperiodchck.checked || _running;
  periodslider.style.opacity = (autoperiodchck.checked || _running)?0.4:1.0;
  
  var fileSelect = document.getElementById('file-select');
  var uploadButton = document.getElementById('upload-button');
  uploadButton.disabled = (fileSelect.files.length === 0);
  
  var uploadButton = document.getElementById('start');
  uploadButton.disabled = _running;
  var uploadButton = document.getElementById('stop');
  uploadButton.disabled = !_running;
  var info = document.getElementById('info-area');
  if ( _running )
    info.style.backgroundColor = "rgba(0,220,0,0.05)";
  else
    info.style.backgroundColor = "rgba(0,0,220,0.1)";
}

function getCoalaStatus()
{
    var theUrl = "?coalastatusrequest";
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open( "GET", theUrl, true );
    xmlHttp.onerror = function(e)
    {
      document.getElementById('message').innerHTML = "<span style=\"color:red;\">Connection error!.. :-(</span>";
      _wasDown = true;
    };
    xmlHttp.onreadystatechange = function()
    {
      if ( xmlHttp.status == 200 && xmlHttp.readyState == 4 )
      {
        var content = xmlHttp.responseText;
        parseStatus( content );
        updateControls();
        _running = _params["running"]=="true";
        document.getElementById("bypasscheckbox").checked = _params["bypass"]=="true";
        
        var value = _params["modal_control"]=="true";
        document.getElementById("modalswitchimg").src = value?"images/switchon.png":"images/switchoff.png";
        document.getElementById("activecontrolcheckbox").checked = value;
        
        value = _params["modulated_gain"]=="true";
        document.getElementById("modulatedgainswitchimg").src = value?"images/switchon.png":"images/switchoff.png";
        document.getElementById("modulatedgaincheckbox").checked = value;
        
        value = _params["biquad"]=="true";
        document.getElementById("biquadswitchimg").src = value?"images/switchon.png":"images/switchoff.png";
        document.getElementById("biquadcheckbox").checked = value;

        value = _params["gen"]=="true";
        document.getElementById("genswitchimg").src = value?"images/switchon.png":"images/switchoff.png";
        document.getElementById("gencheckbox").checked = value;
        
        value = _params["chirp"]=="true";
        document.getElementById("chirpswitchimg").src = value?"images/switchon.png":"images/switchoff.png";
        document.getElementById("chirpcheckbox").checked = value;
        
        value = _params["realtime"]=="true";
        document.getElementById("realtimecheckbox").checked = value;
        value = _params["share_time"]=="true";
        document.getElementById("sharetimecheckbox").checked = value;
        value = _params["record"]=="true";
        document.getElementById("recorddatacheckbox").checked = value;
        value = _params["auto_sample_period"]=="true";
        document.getElementById("autoperiodcheckbox").checked = value;
        
        if ( _params["auto_sample_period"] == "true" )
        {
          var sp = _params["sample_period"];
          value = Math.round(sp*1.0e7)/10;
          document.getElementById("periodslider").value = value;
          document.getElementById("periodval").innerHTML = value;
        }
        document.getElementById("version-field").innerHTML = _params["version"];
        
        content = content.replace( /\+/g, "<br/>" );
        content = content.replace( /\_/g, " " );
        content = content.replace( /\=/g, ": " );
        content = content.replace( /true/g, "<span style=\"color:green;font-weight:bold;\">TRUE</span>" );
        content = content.replace( /false/g, "<span style=\"color:hotpink;\">FALSE</span>" );
        document.getElementById("coalastatus").innerHTML = content;
        if ( _wasDown )
        {
          location.reload();
          _wasDown = false;
        }
      }
    };
    xmlHttp.send();
    return xmlHttp.responseText;
}

function parseStatus( status )
{
  var l = status.length;
  var r = 0;
  _params = [];
  while ( r < l )
  {
      var pos = status.substr( r ).search("=");
      if ( pos == -1 )
        break;
      var key = status.substr( r, pos );
      var pos2 = status.substr( r+pos+1 ).search("\\+");
      if ( pos2 == -1 )
        break;
      var val = status.substr( r+pos+1, pos2 );
      _params[key] = val;
      r += pos+pos2+2;
  }
}

function updateControls()
{
  if ( !_init )
  {
    _init = true;
    //document.getElementById("chirpcheckbox").checked = ( status.search("chirp=true") != -1 );
    //document.getElementById("biquadcheckbox").checked = ( status.search("biquad=true") != -1 );
    //document.getElementById("modulatedgaincheckbox").checked = ( status.search("modulated_gain=true") != -1 );

    var value = _params["time_limit"];
    document.getElementById("timeslider").value = value;
    document.getElementById("timeval").innerHTML = value;
    
    value = _params["smoothing_time"];
    document.getElementById("fadeslider").value = value;
    document.getElementById("fadeval").innerHTML = value;
    
    value = _params["start_freq"];
    if ( value !== undefined )
    {
      document.getElementById("chirpbeginslider").value = value;
      document.getElementById("chirpbeginval").innerHTML = value;
    }
    
    value = _params["end_freq"];
    if ( value !== undefined )
    {
      document.getElementById("chirpendslider").value = value;
      document.getElementById("chirpendval").innerHTML = value;
    }

    var truc = _params["oscstatus"];
    if ( _params["oscstatus"]=="1" )
      document.getElementById("toggleosc").value = "stop OSC server";
    else
      document.getElementById("toggleosc").value = "start OSC server";
  }
}

function handleInput(newValue, type)
{
    var display = newValue;
    if ( type == 'timeval' && newValue >= 60.5 )
    {
        display = '∞';
        newValue = 10800;
    }
    document.getElementById(type).innerHTML = display;
    var theUrl = "?" + type + "=" + newValue;
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open( "GET", theUrl, true );
    xmlHttp.send();
    return xmlHttp.responseText;
}

function handleCheckbox( cb, type )
{
    if ( type == "chirpon" )
    {
      var recordchck = document.getElementById("recorddatacheckbox");
      recordchck.checked = cb.checked;
      handleCheckbox( recordchck, "recorddataon" );
    }
    var theUrl = "?" + type + "=" + ( cb.checked === true?1:0 );
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open( "GET", theUrl, true );
    xmlHttp.send();
    return xmlHttp.responseText;
}

function handleButton( type )
{
    var message = '/coala/';
    if ( type == "toggleosc" )
    {
      message += 'osc/enable/';
      if ( _params["oscstatus"]=="1" )
        message += '0';
      else
        message += '1';
    }
    else
    {
      message = "?" + type + "=1";
    }
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open( "GET", message, true );
    xmlHttp.send();
    return xmlHttp.responseText;
}

function handleOutDataRequest()
{
    document.getElementById('message').innerHTML = "Downloading output data...";
    $.fileDownload('?datarequest=outdata', {
       successCallback:  function (url) {
          document.getElementById('message').innerHTML = "Output data downloaded successfully :-)";
       },
       failCallback:     function (html, url) {
          document.getElementById('message').innerHTML = "Output data download failed :-(";
       }
    });
    return;
}

function handleMatlabFilesUpload(item)
{
  var form = document.getElementById('upload-form');
  var fileSelect = document.getElementById('file-select');
  var uploadButton = document.getElementById('upload-button');
  uploadButton.innerHTML = 'Uploading...';
  var files = fileSelect.files;
  var content = "";
  var count = 0;
  for (var i = 0; i < files.length; i++)
  {   
    (function(file) {
        var name = file.name;
        var reader = new FileReader();  
        reader.onload = function(e) {
            content += "startoffile=\""+name+"\"\n";
            content += e.target.result+"\nendoffile\n";
            ++count;
            if ( count == files.length )
            {
                uploadButton.innerHTML = 'Uploading...';
                document.getElementById('message').innerHTML = 'Uploading ' + files.length + ' files...';
                var xhr = new XMLHttpRequest();
                xhr.open('POST', '?dataupload=matlab', true );
                var bytes = new Uint8Array( content.length );
                for ( var k = 0; k < content.length; ++k )
                  bytes[k] = content.charCodeAt(k);
                //var blob = new Blob( bytes, {type: 'application/octet-binary'} ); //text/plain
                xhr.onload = function ()
                {
                  if (xhr.status === 200 || xhr.status === 204 )
                  {
                    form.reset();
                    uploadButton.innerHTML = 'Upload Matlab files';
                    document.getElementById('message').innerHTML = "Matlab files uploaded! :-)";
                  }
                  else
                  {
                    alert('An error occurred! :-(');
                  }
                };
                xhr.setRequestHeader("Content-type", "application/octet-stream");
                xhr.setRequestHeader("Content-Transfer-Encoding", "binary");
                xhr.send(bytes);
            }
        };
        reader.readAsText(file, 'CP1251'); //UTF-8
    })(files[i]);
  }
}

function isNumeric(n)
{
  return !isNaN(parseFloat(n)) && isFinite(n);
}

function nearestPow2( aSize )
{
  return Math.pow( 2, Math.floor( Math.log( aSize ) / Math.log( 2 ) ) );
}

function nextPow2( aSize )
{
  return 1 + Math.floor( Math.log( aSize ) / Math.log( 2 ) );
}

function handleTransferFunctionRequest()
{
    var url = "?datarequest=frequencyresponse"; //"../data/out/adc.txt";
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open( "GET", url, true );
    xmlHttp.onreadystatechange = function()
    {
      if ( xmlHttp.status == 200 )
      {
        if ( xmlHttp.readyState == 4 )
        {
          var raw = xmlHttp.responseText;
          var console = raw;
          var rawData = raw.split("\n");
          var l = rawData.length;
          var fftl = Math.pow(2, nextPow2( l ));
          var response = new complex_array.ComplexArray( fftl );
          var k  = 0;
          var count1 = 0;
          for ( ; k < l; k++ )
          {
            if ( rawData[k].search("excitation") != -1  || count1 > fftl )
            {
              break;
            }
            if ( isNumeric( rawData[k] ) )
            {
              ++count1;
              response.real[k] = rawData[k];
            }
          }
          var excitation = new complex_array.ComplexArray( fftl );
          var count2 = 0;
          ++k;
          for ( ; k < l; k++ )
          {
            if ( count2 > fftl )
              break;
            if ( isNumeric( rawData[k] ) )
            {
              ++count2;
              excitation.real[k] = rawData[k];
            }
          }
          response.FFT();
          excitation.FFT();
          var a, b, a1, b1, Ω;
          for ( var i = 0; i < response.length ; ++i )
          {
            a = response.real[i]/count1;
            b = response.imag[i]/count1;
            a1 = excitation.real[i];
            b1 = excitation.imag[i];
            Ω = (a1*a1+b1*b1);
            if ( Ω !== 0 )
            {
              response.real[i] = (a*a1+b*b1)/Ω;
              response.imag[i] = (b*a1-a*b1)/Ω;
            }
          }
          drawFftToCanvas('fft', response );
        }
      }
    };
    xmlHttp.send();
}

function drawFftToCanvas( element_id, fftData )
{
    var element = document.getElementById(element_id);
    var canvas = document.getElementById("fftcanvas");
    if ( canvas === null )
    {
      canvas = document.createElement('canvas');
      canvas.id = "fftcanvas";
    }
    var context = canvas.getContext('2d');
    var n = fftData.length/2;
    
    var chirpbegin = document.getElementById("chirpbeginslider");
    var chirpend = document.getElementById("chirpendslider");
    var samplePeriod = document.getElementById("periodslider");
    var sampleFreq = 1.0e6/samplePeriod.value;
    
    var minFreq = chirpbegin.value;
    var maxFreq = chirpend.value;
  
    var nMin = Math.floor(n*2*minFreq/sampleFreq);
    var nMax = n*2*maxFreq/sampleFreq;
  
    canvas.width = 1000;
    canvas.height = 500;
    var bottomMargin = 20;
    var rightMargin = 30;
    var width = canvas.width-rightMargin;
    var height = canvas.height-bottomMargin;
    var fullHeight = canvas.height;
    var fullWidtt = canvas.width;
    element.appendChild(canvas);
    
    //frame:
    context.beginPath();
    context.strokeStyle = 'green';
    context.moveTo(0,0);
    context.lineTo(width, 0);
    context.lineTo(width, height);
    context.lineTo(0, height);
    context.lineTo(0, 0);
    context.lineTo(width, 0);
    context.stroke();
    
    //frequency axis:
    var freqStep = Math.ceil(maxFreq/1000)*100;
    context.font="10px Verdana";
    context.beginPath();
    context.strokeStyle = 'pink';
    context.setLineDash([5, 5]);
    context.moveTo(nMin*width/nMax, 0);
    context.lineTo(nMin*width/nMax, height);
    context.stroke();
    context.setLineDash([1, 0]);
    if ( minFreq%freqStep !== 0 )
      context.strokeText( minFreq, minFreq*width/maxFreq, fullHeight-8);
    if ( maxFreq%freqStep !== 0 )
      context.strokeText( maxFreq, width, fullHeight-8);
    
    context.strokeStyle = 'grey';
    context.font="8px";
    for ( var i = 0; i <= maxFreq; i += freqStep )
    {
      context.strokeText( i, i*width/maxFreq, fullHeight);
    }

    //fft trace:
    context.beginPath();
    context.strokeStyle = 'blue';
    var factorY = 1.0;
    var factorX = 1;
    var shiftY = 10;
    var max = -999999;
    var min = 999999;
    var meanValue = 0;
    var steps = 32;
    var actualStart = 0;
    for ( var i = nMin+actualStart; i < nMax; i++ )
    {
      var value = Math.sqrt(fftData.real[i]*fftData.real[i] + fftData.imag[i]*fftData.imag[i]);
      value = Math.log10( value );
      if ( i%steps )
          meanValue += value;
      else
      {
        meanValue /= steps;
        if ( max < value )
          max = value;
        if ( min > value )
          min = value;
        meanValue = 0;
      }
    }
    if ( min < -8 )
      min = -8;
    for ( var i = nMin+actualStart; i < nMax; ++i )
    {
      var value = Math.sqrt(fftData.real[i]*fftData.real[i] + fftData.imag[i]*fftData.imag[i]);
      value = Math.log10( value );
      if ( value < -8 )
        value = -8;
      if ( i == nMin+actualStart )
      {
        context.moveTo( i * width / nMax, height*(1-factorY*(value+Math.abs(min))/(max-min))-shiftY);
      }
      else
      {
        if ( i%steps )
          meanValue += value;
        else
        {
          meanValue /= steps;
          context.lineTo( i * width / nMax, height*(1-factorY*(meanValue+Math.abs(min))/(max-min))-shiftY );
          meanValue = 0;
        }
      }
    }
    context.stroke();
 }
