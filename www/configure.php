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
              <li class="active"><a href="/about.php"><?php _("About")?></a></li>
              <li><a href="/documentation.php"><?php _("Documentation") ?></a></li>
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
  		<li class="active"><?php _("Configure")?> {{printerName}}</span></li>  
		</ul>  
	</div>  
</div>  

<div class="row">
  <!-- <span class="span2"><?php _("Edit script ")?></span> -->
  <div class="span3 btn-group">
    <a class="btn dropdown-toggle" data-toggle="dropdown" href="#">
      <?php _("Select script to edit") ?>
      <span class="caret"></span>
    </a>
    <ul class="dropdown-menu" id="scriptlist">
    </ul>
  </div>
  <span class="span7" style="line-height:30px"><?php _("Current script:")?> <span id="cname"></span></span>
<button class="btn btn-success span2" onclick="savescript()"><i class="icon-save"></i> <?php _("Save") ?></button>
</div>

<div class="row"><span class="span12">
<textarea id="editor" class="span12" style="height:200px"></textarea></span>
</div>
</div> <!-- /container -->

<!-- Placed at the end of the document so the pages load faster -->
<script src="/jquery/jquery.min.js"></script>
<script src="/jquery/jquery.form.js"></script>
<script src="/bt/bootstrap.min.js"></script>
<script type="text/javascript">
function htmlescape(t) {
	return t.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}
function refreshScripts() {
	$.getJSON('/printer/script/{{slug}}?a=list', function(data) {
		updateScripts(data);
	});
}
function updateScripts(data) {
	  if(!data.data)  return;
		s = '';
  	$.each(data.data, function(key,val) {
  		s+='<li><a href="javascript:loadscript(\''+val.name+'\')">'+htmlescape(val.name)+'</a></li>';
  	});
  	$('#scriptlist').html(s);
}
var lastscript="";
function loadscript(name) {
	$.post("/printer/script/{{slug}}",{"a":"load","f":name},function(text) {
	  lastscript=name;
	  $('#cname').html(name);
	  $('#editor').val(text);
	});
}
function savescript() {
  if(lastscript=="") return;
	$.post("/printer/script/{{slug}}",{"a":"save","f":lastscript,"text":$('#editor').val()});
}
$(document).ready(function() {
  refreshScripts();
  loadscript("Start");
});
</script>
</body></html>
