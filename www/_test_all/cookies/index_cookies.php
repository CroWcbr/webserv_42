<head>
  <title>Test Cookies</title>
  <style>
  		body { width: 100em; margin: 0 auto;
  			background-image: url('image/fon2.jpg');
  			font-family: Tahoma, Verdana, Arial, sans-serif; }
  </style>
</head>

<?php
	$num = (isset($_COOKIE["num"])) ? $_COOKIE["num"] : 0;
	$num++;
	setcookie ("num", $num, time() + 30);
	echo "Page update $num times";
?>

<br><br>
<th><a href="/" style="font-family:Monotype Corsiva;color:black;font-size:25px">Return home</a><br/></th>

