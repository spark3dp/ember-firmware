<?php

    $timeLeft = "NA";
    $handle = @fopen("/tmp/StatusToNetPipe", "r");
    if ($handle) {
        $timeLeft = fgets($handle, 4096);
    }
    fclose($handle);

    echo $timeLeft;
?>

