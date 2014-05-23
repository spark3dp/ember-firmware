<html>
<head>


<?php
	if (isset($_GET['cmd']))
	{
		$cmd = $_GET['cmd'];
		
        file_put_contents('/tmp/CommandPipe', "$cmd\n", FILE_APPEND | LOCK_EX);
	}
?>


<title>Smith Command Input</title>
<style type="text/css">
	p { display: table-cell; }
	button { font-size: 80px; width: 800px; margin: 10px; padding: 20px 20px;}
</style>

</head>
<!-- <body  onload="GetStatus()"> -->
<p><font size="20">Printer state or time remaining: <span id="timeLeft"></span></p>
<div style="width: 800px; margin: 0px auto;">
    <p><font size="20">Commands:</font></p>
    <button type="button" onclick="location.href='UI.php?cmd=Cancel'">CANCEL</button>
    <button type="button" onclick="location.href='UI.php?cmd=Reset'">RESET</button>
    <button type="button" onclick="location.href='UI.php?cmd=Pause'">PAUSE</button>
    <button type="button" onclick="location.href='UI.php?cmd=Resume'">RESUME</button>
</div>
</html>
