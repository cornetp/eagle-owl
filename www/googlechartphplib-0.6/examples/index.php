<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"> 
<html xmlns="http://www.w3.org/1999/xhtml" dir="ltr" lang="fr-FR"> 
 
<head profile="http://gmpg.org/xfn/11"> 
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" /> 
	<title>GoogleChart PHP Library - Examples</title> 
</head>

<body>

<?php

	$files = glob('*');
	$this_file = basename(__FILE__);
	foreach ( $files as $f ) {
		if ( $f === $this_file || ! is_file($f) )
			continue;

		printf(
			'<h2>%s</h2><p class="center"><img src="%1$s" alt="%1$s" /></p>',
			$f
		);
	}
?>

</body>
</html>
