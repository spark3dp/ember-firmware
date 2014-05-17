<html>
<head>


<script>
 var req_status = null;
 var curr_id = 0;

 function GetStatus() {
   // make url unique so IE doesn't return cached copy
   url = "status.php?disp_id=" + curr_id + "&ms=" + new Date().getTime();

   var xmlhttp=new XMLHttpRequest();
   xmlhttp.onreadystatechange=function() {
     if (xmlhttp.readyState==4 && xmlhttp.status==200) {
       document.getElementById("timeLeft").innerHTML=xmlhttp.responseText;
     }  
     else {
       document.getElementById("timeLeft").innerHTML="unknown";
     }

   }
   xmlhttp.open("GET", url, true);
   xmlhttp.send();
   curr_id += 1;
 }
 </script>


<title>Smith Command Input</title>
<style type="text/css">
	p { display: table-cell; }
	button { font-size: 80px; width: 800px; margin: 10px; padding: 20px 20px;}
</style>
<?php
	if (isset($_GET['cmd']))
	{
		$cmd = $_GET['cmd'];
		
        file_put_contents('/tmp/UICommandPipe', $cmd, FILE_APPEND | LOCK_EX);
	}
?>
</head>
<body  onmouseover="GetStatus()">
<div style="width: 800px; margin: 0px auto;">
    <p><font size="20">Printer state or seconds remaining: <span id="timeLeft"></span></p>
    <p><font size="20">Commands:</font></p>
    <button type="button" onclick="location.href='UI.php?cmd=1'">START / CANCEL</button>
    <button type="button" onclick="location.href='UI.php?cmd=2'">RESET</button>
    <button type="button" onclick="location.href='UI.php?cmd=3'">PAUSE / RESUME</button>
    <button type="button" onclick="location.href='UI.php?cmd=4'">SLEEP / WAKE</button>
</div>
</html>
