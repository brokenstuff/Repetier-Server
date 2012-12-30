<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link href="/bt/css/bootstrap.css" rel="stylesheet">
    <link rel="stylesheet" href="/bt/css/font-awesome.css">
    <link rel="stylesheet" href="/bt/css/bootstrap-arrows.css">
    <style type="text/css">
      body {
        padding-top: 60px;
        padding-bottom: 40px;
      }
      .sidebar-nav {
        padding: 9px 0;
      }
    </style>
    <link href="/bt/css/bootstrap-responsive.css" rel="stylesheet">

    <!-- HTML5 shim, for IE6-8 support of HTML5 elements -->
    <!--[if lt IE 9]>
      <script src="http://html5shim.googlecode.com/svn/trunk/html5.js"></script>
    <![endif]-->
<title>Repetier-Server</title>
</head>
<body>
	
    <div class="navbar navbar-inverse navbar-fixed-top">
      <div class="navbar-inner">
        <div class="container">
          <a class="btn btn-navbar" data-toggle="collapse" data-target=".nav-collapse">
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
          </a>
          <a class="brand" href="http://www.repetier.com/server">Repetier-Server {{version}}</a>
          <div class="nav-collapse collapse">
            <ul class="nav">
              <li><a href="/index.php"><?php _("Home") ?></a></li>
              <li><a href="/about.php"><?php _("About")?></a></li>
              <li class="active"><a href="/documentation.php"><?php _("Documentation") ?></a></li>
            </ul>
          </div><!--/.nav-collapse -->
        </div>
      </div>
    </div>
    <div class="container">
    <div class="row">  
	<div class="span12">  
		<ul class="breadcrumb">  
  		<li>  
    		<a href="index.php"><i class="icon-home"></i> <?php _("Printer overview") ?></a> <span class="divider"> &gt; </span>  
  		</li>  
  		<li class="active"><?php _("Documentation")?></span></li>  
		</ul>  
	</div>  
</div>  
<div class="row"><span class="span12">
<h1>Introduction</h1>
<p>The server follows a different concept than normal host software does. While a normal
host manages one printer as long as the host is running and only that host can use the
printer, the server is made to run all the time and handles multiple connections to any
number of printer connected. For the server it is completely ok to have 2 or three persons
adding new jobs to it in parallel. Off course you make some damage moving the head from
different places, but you always see what the other user does. And the server queues
jobs, so the jobs are handled one after the other.</p>
<h1>Connecting printer</h1>
<p>If you go to the start page http://<i>YourHostname</i>:8080 you see the list off
configured printer and their state. Each printer can have three states:</p>
<ul>
<li>Disconnected: You have disconnected the printer from the server. You need this to
upload new firmware or for a direct connection with other software needing that serial port.</li>
<li>Offline: The server watches the port, but no printer is connected.</li>
<li>Online: The server is connected with the printer.</li>
</ul>
<p>There is no need to go online/offline by yourself. The server checks the ports for
availability and automatically connects the printer if you connect it.</p>
<h1>Running your printer</h1>
<p>The server has a simple web API to allow different software to access the printer in parallel.
The most simple way is to use this web interface or a host supporting the API. The server
manages four jobs: Printing models, store models in a database, manual control of the
printer and printer configuration.</p>
<h2>Printing models</h2>
<p>To print a model you have to create the g-code containing all necessary commands
including homing and temperature handling. You send this file to the server where it gets
queued up. You can watch the queue and start any job that is completely uploaded. After
the job is started many buttons in the manual control will get disabled for safety reasons.
Once the job is finished you get a message and can start the next job.
</p>
<h2>Model database</h2>
<p>If you have to print some models more then once, it is tedious to upload them again and
again. For that reason the server has also a per printer model database. Models
uploaded there can be moved to the job queue with one click.</p>
<h2>Manual control</h2>
<p>You can also send command to the printer manually. The web interface has a separate
form for this with the most common commands as button. It also shows important parameter
like position and temperatures. These get updated every second using a poll to the server.
That way multiple clients always see the same temperatures and positions.
</p>
<h2>Printer configuration</h2>
<p>Currently you have to change the config files and restart the server. A web form
to change the parameter will come ...</p>

</p>
</span></div>
    </div> <!-- /container -->

<!-- Placed at the end of the document so the pages load faster -->
<script src="/jquery/jquery.min.js"></script>
<script src="/jquery/jquery.form.js"></script>
<script src="/bt/bootstrap.min.js"></script>
</body></html>
