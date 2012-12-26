<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link href="/bt/css/bootstrap.css" rel="stylesheet">
    <link rel="stylesheet" href="/bt/css/font-awesome.css">
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
</div>
    <div class="navbar navbar-inverse navbar-fixed-top">
      <div class="navbar-inner">
        <div class="container">
          <a class="btn btn-navbar" data-toggle="collapse" data-target=".nav-collapse">
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
          </a>
          <a class="brand" href="#">Repetier-Server</a>
          <div class="nav-collapse collapse">
            <ul class="nav">
              <li class="active"><a href="#">Home</a></li>
              <li><a href="#about">About</a></li>
              <li><a href="#contact">Contact</a></li>
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
    <i class="icon-home"></i> <?php _("Printer overview") ?></a>  
  </li>  
</ul>  
</div>  
</div>  

<div class="row"><div class="span12">
      <table class="table table-striped table-hover table-bordered">  
        <thead>  
          <tr>  
            <th><i class="icon-print"></i> <?php _("Printer")?></th>  
            <th><i class="icon-off"></i> <?php _("Online")?></th>  
            <th><i class="icon-tasks"></i> <?php _("Job")?></th>  
            <th><i class="icon-cog"></i> <?php _("Action")?></th>
          </tr>  
        </thead>  
        <tbody id="printerlist">  
        </tbody>  
      </table> 
</div></div>
    </div> <!-- /container -->

    <!-- Le javascript
    ================================================== -->
    <!-- Placed at the end of the document so the pages load faster -->
    <script src="/jquery/jquery.min.js"></script>
    <script src="/bt/bootstrap.min.js"></script>

<script type="text/javascript">

function updatePrinterList() {
$.getJSON('/printer/list', function(data) {
  s = "";
  $.each(data.data, function(key,val) {
    s+="<tr><td>"+val.name+"</td><td>";
    if(val.online) s+= '<span class="label label-success">Online</span>';
    else s+='<span class="label label-important">Offline</span>';
    s+="</td><td>"+val.job+"</td><td>";
    s+='<a href="printer.php?pn='+val.slug+'" class="btn"><i class="icon-signin"></i> <?php _("Manage") ?></a>';
    s+="</td></tr>";
  });
  $('#printerlist').html(s);  
   setTimeout('updatePrinterList()', 10000);
});
}

$(document).ready(function() {
   updatePrinterList();
 });
</script>
</body>
</html>
