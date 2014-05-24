<html>
<head>

<script type="text/javascript">

function GetStatus()
{
    location.href = "UI.php?cmd=GetStatus";
}

</script>

<title>Smith Command Input</title>
<style type="text/css">
	p { display: grid; }
	button { font-size: 50px; width: 300px; margin: 10px; padding: 20px 20px;}
</style>

</head>
<body onload="setTimeout('GetStatus();', 5000)">

<table border="2">
<tr>
<td rowspan="5" width="800">

<?php
	if (isset($_GET['cmd']))
	{
		$cmd = $_GET['cmd'];
		
        file_put_contents('/tmp/CommandPipe', "$cmd\n", FILE_APPEND | LOCK_EX);

        if($cmd == "GetStatus")
        {
            echo "<p><font size='10'>Printer status:</p>";
            // read from the status pipe & echo it to screen
            $handle = @fopen("/tmp/StatusToWebPipe", "r");
            if ($handle) {
                for($i = 0; $i < 12; $i += 1)
                {
                    echo "<p><font size='10'>";
                    echo fgets($handle, 4096);
                    echo "</p>";
                }
            }
            fclose($handle);
        }
	}
?>

</td>
<td><button type="button" onclick="location.href='UI.php?cmd=Cancel'">Cancel</button></td>
</tr>

<tr>
<td><button type="button" onclick="location.href='UI.php?cmd=Reset'">Reset</button></td>
</tr>

<tr>
<td><button type="button" onclick="location.href='UI.php?cmd=Pause'">Pause</button></td>
</tr>

<tr>
<td><button type="button" onclick="location.href='UI.php?cmd=Resume'">Resume</button></td>
</tr>

<tr>
<td><button type="button" onclick="location.href='UI.php?cmd=GetStatus'">Get Status</button></td>
</tr>





</table>
</html>
