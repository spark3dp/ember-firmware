<html>
<head>
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

        $pctComplete = 0;
        echo '<p><font size="20">';
        echo $pctComplete;
        echo ' % complete';
	}
?>
</head>
<body>
<div style="width: 800px; margin: 0px auto;">
    <p><font size="20">Commands:</font></p>
    <button type="button" onclick="location.href='UI.php?cmd=1'">START / CANCEL</button>
    <button type="button" onclick="location.href='UI.php?cmd=2'">RESET</button>
    <button type="button" onclick="location.href='UI.php?cmd=3'">PAUSE / RESUME</button>
    <button type="button" onclick="location.href='UI.php?cmd=4'">SLEEP / WAKE</button>
</div>