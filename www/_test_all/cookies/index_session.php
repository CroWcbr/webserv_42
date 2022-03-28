<head>
  <title>Test Session</title>
  <style>
  		body { width: 100em; margin: 0 auto;
  			background-image: url('image/fon2.jpg');
  			font-family: Tahoma, Verdana, Arial, sans-serif; }
  	</style>
</head>

<?php
	session_start();
	$num = (isset($_SESSION["num"])) ? $_SESSION["num"] : 0;
	$num++;
	$_SESSION["num"] = $num;
	if ($num >= 10)
	{
		echo "Num > 10, begin from zero again";
		$_SESSION["num"] = 0;
	}
	else
		echo "Page update $num times";
?>

<br><br>
<th><a href="/" style="font-family:Monotype Corsiva;color:black;font-size:25px">Return home</a><br/></th>

